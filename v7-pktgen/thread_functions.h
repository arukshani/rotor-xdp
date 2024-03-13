static void print_pkt_info(uint8_t *pkt, uint32_t len)
{
	struct ethhdr *eth = (struct ethhdr *) pkt;
	__u16 proto = ntohs(eth->h_proto);

	char *fmt = "DEBUG-pkt len=%04d Eth-proto:0x%X %s "
		"src:%s -> dst:%s\n";
	char src_str[128] = { 0 };
	char dst_str[128] = { 0 };

    // printf(fmt, len, proto, "Unknown", "", "");

	if (proto == ETH_P_IP) {
		struct iphdr *ipv4 = (struct iphdr *) (eth + 1);
		inet_ntop(AF_INET, &ipv4->saddr, src_str, sizeof(src_str));
		inet_ntop(AF_INET, &ipv4->daddr, dst_str, sizeof(dst_str));
		printf(fmt, len, proto, "IPv4", src_str, dst_str);

		if (ipv4->protocol == IPPROTO_UDP) 
		{
			struct udphdr *udp_hdr;
			udp_hdr = (struct udphdr *)(ipv4 + 1);
			printf("UDP %d, %d \n", ntohs(udp_hdr->source), ntohs(udp_hdr->dest));
		}
		
	} else {
		printf(fmt, len, proto, "Unknown", "", "");
	}
    
    
    // else if (proto == ETH_P_ARP) {
	// 	printf(fmt, len, proto, "ARP", "", "");
	// } else if (proto == ETH_P_IPV6) {
	// 	struct ipv6hdr *ipv6 = (struct ipv6hdr *) (eth + 1);
	// 	inet_ntop(AF_INET6, &ipv6->saddr, src_str, sizeof(src_str));
	// 	inet_ntop(AF_INET6, &ipv6->daddr, dst_str, sizeof(dst_str));
	// 	printf(fmt, len, proto, "IPv6", src_str, dst_str);
	// } else {
	// 	printf(fmt, len, proto, "Unknown", "", "");
	// }
}

static inline void ether_addr_copy_assignment(u8 *dst, const u8 *src)
{
	u16 *a = (u16 *)dst;
	const u16 *b = (const u16 *)src;

	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
}

bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}


// Header structure of GRE tap packet:
//  Ethernet type of GRE encapsulated packet is ETH_P_TEB (gretap)
//  outer eth
//  outer ip
//  outer udp (new for corundum to help with RSS)
//  gre
//  inner eth
//  inner ip
//  payload
static void process_rx_packet(void *data, struct port_params *params, uint32_t len, u64 addr, struct return_process_rx *return_val)
{
	
	int is_nic = strcmp(params->iface, nic_iface);

    if (prefix(OUTER_VETH_PREFIX, params->iface))
	{
		// printf("From VETH \n");
		struct iphdr *outer_iphdr;
		struct ethhdr *outer_eth_hdr;

		struct iphdr *inner_ip_hdr_tmp = (struct iphdr *)(data +
														  sizeof(struct ethhdr));

		compute_ip_checksum(inner_ip_hdr_tmp);

		// unsigned short *ipPayload = (data + inner_ip_hdr_tmp->ihl * 4);

		//new
		struct udphdr *inner_udp_hdr;
		struct tcphdr *inner_tcp_hdr;
		uint16_t udp_source;

		if (inner_ip_hdr_tmp->protocol == IPPROTO_UDP) 
		{
			unsigned short *ipPayload = (data +
					    sizeof(struct ethhdr) +
					    sizeof(struct iphdr));
			compute_udp_checksum(inner_ip_hdr_tmp, ipPayload);
			inner_udp_hdr = (struct udphdr *)(data +
					    sizeof(struct ethhdr) +
					    sizeof(struct iphdr));
			udp_source = ((inner_udp_hdr->source ^ inner_udp_hdr->dest) | 0xc000);

		} else if (inner_ip_hdr_tmp->protocol == IPPROTO_TCP) 
		{
			unsigned short *ipPayload = (data +
					    sizeof(struct ethhdr) +
					    sizeof(struct iphdr));
			compute_tcp_checksum(inner_ip_hdr_tmp, ipPayload);
			inner_tcp_hdr = (struct tcphdr *)(data +
					    sizeof(struct ethhdr) +
					    sizeof(struct iphdr));
			udp_source = ((inner_tcp_hdr->source ^ inner_tcp_hdr->dest) | 0xc000);
			
		} else
		{
			udp_source = htons(0xC008);
		}

		int olen = 0;
		olen += ETH_HLEN;
		olen += sizeof(struct udphdr); //new
		olen += sizeof(struct gre_hdr);

		int encap_size = 0; // outer_eth + outer_ip + outer_udp + gre
		int encap_outer_eth_len = ETH_HLEN;
		int encap_outer_ip_len = sizeof(struct iphdr);
		int encap_outer_udp_len = sizeof(struct udphdr); //new
		int encap_gre_len = sizeof(struct gre_hdr);

		encap_size += encap_outer_eth_len;
		encap_size += encap_outer_ip_len;
		encap_size += encap_outer_udp_len; //new
		encap_size += encap_gre_len;

		int offset = 0 + encap_size;
		u64 new_addr = addr + offset;
		int new_len = len + encap_size;
        // printf("len: %d \n", len);
        // printf("new_len: %d \n", new_len);

		u64 new_new_addr = xsk_umem__add_offset_to_addr(new_addr);
		u8 *new_data = xsk_umem__get_data(params->bp->addr, new_new_addr);
		memcpy(new_data, data, len);

		struct ethhdr *eth = (struct ethhdr *)new_data;
		struct iphdr *inner_ip_hdr = (struct iphdr *)(new_data +
													  sizeof(struct ethhdr));

		if (ntohs(eth->h_proto) != ETH_P_IP ||
			len < (sizeof(*eth) + sizeof(*inner_ip_hdr)))
		{
			printf("not ETH_P_IP or size is not within the len \n");
			return false;
		}

		outer_eth_hdr = (struct ethhdr *)data;
		ether_addr_copy_assignment(outer_eth_hdr->h_source, &out_eth_src);

		char dest_char[16];
		// snprintf(dest_char, 16, "%pI4", inner_ip_hdr_tmp->daddr); 
		// short dest_ipAddress[4];
		// extractIpAddress(dest_char, &dest_ipAddress[0]);
		unsigned char bytes[4];
		bytes[0] = inner_ip_hdr_tmp->daddr & 0xFF;
		bytes[1] = (inner_ip_hdr_tmp->daddr >> 8) & 0xFF;
		bytes[2] = (inner_ip_hdr_tmp->daddr >> 16) & 0xFF;
		bytes[3] = (inner_ip_hdr_tmp->daddr >> 24) & 0xFF;   
		// printf("%d.%d.%d.%d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
		snprintf(dest_char, 16, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], 1); 
		struct sockaddr_in construct_dest_ip;
		inet_aton(dest_char, &construct_dest_ip.sin_addr);

		// printf("construct_dest_ip.sin_addr.s_addr: %d \n", construct_dest_ip.sin_addr.s_addr);
		struct ip_set *dest_ip_index = mg_map_get(&ip_table, construct_dest_ip.sin_addr.s_addr);
		// printf("dest_ip_index->index: %d \n", dest_ip_index->index);
		// struct ip_set *dest_ip_index = mg_map_get(&ip_table, inner_ip_hdr_tmp->daddr);
		struct mac_addr *dest_mac_val = mg_map_get(&mac_table, dest_ip_index->index);
		struct ip_set *ns_ip_index = mg_map_get(&ns_ip_table, inner_ip_hdr_tmp->daddr);
		// return_val->ring_buf_index = dest_ip_index->index - 1;
		return_val->ring_buf_index = ns_ip_index->index - 1;
		ether_addr_copy_assignment(outer_eth_hdr->h_dest, dest_mac_val->bytes);

		outer_eth_hdr->h_proto = htons(ETH_P_IP);

		outer_iphdr = (struct iphdr *)(data +
									   sizeof(struct ethhdr));
		__builtin_memcpy(outer_iphdr, inner_ip_hdr_tmp, sizeof(*outer_iphdr));
		// outer_iphdr->protocol = IPPROTO_GRE; //new
		outer_iphdr->protocol = IPPROTO_UDP; //new
		outer_iphdr->tot_len = bpf_htons(olen + bpf_ntohs(inner_ip_hdr_tmp->tot_len));
		outer_iphdr->daddr = construct_dest_ip.sin_addr.s_addr;

		//new
		struct udphdr *udp_hdr;
		udp_hdr = (struct udphdr *)(data +
									 sizeof(struct ethhdr) + sizeof(struct iphdr));
		udp_hdr->source = udp_source;
		udp_hdr->dest = htons(0x1292); //4754 (https://datatracker.ietf.org/doc/html/rfc8086#page-11)
		udp_hdr->len = bpf_htons( sizeof(struct udphdr) + sizeof(struct gre_hdr) + ETH_HLEN +
							bpf_ntohs(inner_ip_hdr_tmp->tot_len));

		struct gre_hdr *gre_hdr;
		gre_hdr = (struct gre_hdr *)(data +
									 sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));

		gre_hdr->proto = bpf_htons(ETH_P_TEB);
		if (strcmp(params->iface, "vethout2") == 0) {
			gre_hdr->flags = 0;
		} else if (strcmp(params->iface, "vethout3") == 0) {
			gre_hdr->flags = 1;
		} else if (strcmp(params->iface, "vethout4") == 0) {
			gre_hdr->flags = 2;
		} else if (strcmp(params->iface, "vethout5") == 0) {
			gre_hdr->flags = 3;
		} else if (strcmp(params->iface, "vethout6") == 0) {
			gre_hdr->flags = 4;
		} else if (strcmp(params->iface, "vethout7") == 0) {
			gre_hdr->flags = 5;
		} else if (strcmp(params->iface, "vethout8") == 0) {
			gre_hdr->flags = 6;
		} else if (strcmp(params->iface, "vethout9") == 0) {
			gre_hdr->flags = 7;
		}
		
		return_val->new_len = new_len;

		// printf("From VETH packet received\n");
	}
	else if (is_nic == 0)
	{
// #if DEBUG_PAUSE_Q == 1
// 		timestamp_arr[time_index] = now;
// 		time_index++;
// #endif
		// printf("From NIC \n");
		// printf("src_ip: %d \n", src_ip);
		

		struct ethhdr *eth = (struct ethhdr *)data;
		struct iphdr *outer_ip_hdr = (struct iphdr *)(data +
													  sizeof(struct ethhdr));

		struct udphdr *outer_udp_hdr = (struct udphdr *)(outer_ip_hdr + 1);
		
		struct gre_hdr *greh = (struct gre_hdr *)(outer_udp_hdr + 1);

		// if (ntohs(eth->h_proto) != ETH_P_IP || outer_ip_hdr->protocol != IPPROTO_GRE ||
		// 			ntohs(greh->proto) != ETH_P_TEB)
		// {
		// 	printf("not a GRE packet \n");
		// 	return false;
		// }
		struct ethhdr *inner_eth = (struct ethhdr *)(greh + 1);
		// if (ntohs(inner_eth->h_proto) != ETH_P_IP) {
		// 	printf("inner eth proto is not ETH_P_IP %x \n", inner_eth->h_proto);
		//     return false;
		// }
		

		struct iphdr *inner_ip_hdr = (struct iphdr *)(inner_eth + 1);

		// printf("outer_ip_hdr->daddr: %d \n", outer_ip_hdr->daddr);
		// printf("inner_ip_hdr->daddr: %d \n", inner_ip_hdr->daddr);

		// if (src_ip != (inner_ip_hdr->daddr))
		if (src_ip != (outer_ip_hdr->daddr))
		{
			// printf("Not destined for local node \n");
			// send it back out NIC
			struct ip_set *next_dest_ip_index = mg_map_get(&ip_table, inner_ip_hdr->daddr);
			// int next_mac_index;
			// getRouteElement(route_table, next_dest_ip_index->index, topo, &next_mac_index);
			struct mac_addr *next_dest_mac_val = mg_map_get(&mac_table, next_dest_ip_index->index);
			// __builtin_memcpy(eth->h_dest, next_dest_mac_val->bytes, sizeof(eth->h_dest));
			ether_addr_copy_assignment(eth->h_dest, next_dest_mac_val->bytes);
			// __builtin_memcpy(eth->h_source, out_eth_src, sizeof(eth->h_source));
			// memcpy(eth->h_source, out_eth_src, sizeof(eth->h_source));
			ether_addr_copy_assignment(eth->h_source, &out_eth_src);
			return_val->ring_buf_index = next_dest_ip_index->index - 1;

			// Telemetry
			//  #if DEBUG == 1
			//  	timestamp_arr[time_index] = now;
			//  	node_ip[time_index] = src_ip;
			//  	slot[time_index]=1;
			//  	topo_arr[time_index] = topo;
			//  	next_node[time_index] = next_mac_index;
			//  	time_index++;
			//  #endif

			// Debug
			//  printf("next_mac_index = %d\n", next_mac_index);
			//  int i;
			//  for (i = 0; i < 6; ++i)
			//  	printf(" %02x", (unsigned char) next_dest_mac_val->bytes[i]);
			//  puts("\n");

			return_val->new_len = 1; // indicates that packet should go back out through NIC
		}
		else
		{

			return_val->which_veth = greh->flags;

			// send it to local veth
			void *cutoff_pos = greh + 1;
			int cutoff_len = (int)(cutoff_pos - data);
            // printf("cutoff_len: %d \n", cutoff_len);
			int new_len = len - cutoff_len;

			int offset = 0 + cutoff_len;
			u64 inner_eth_start_addr = addr + offset;

			u8 *new_data = xsk_umem__get_data(params->bp->addr, inner_eth_start_addr);
			memcpy(xsk_umem__get_data(params->bp->addr, addr), new_data, new_len);

			// Telemetry
			//  #if DEBUG == 1
			//  	timestamp_arr[time_index] = now;
			//  	node_ip[time_index] = src_ip;
			//  	slot[time_index]=2;
			//  	topo_arr[time_index] = topo;
			//  	next_node[time_index] = 0;
			//  	time_index++;
			//  #endif

			return_val->new_len = new_len;
			
		}
	}
}

// static inline u32
// port_rx_burst(struct port *p, struct mpmc_queue *local_dest_queue[NUM_OF_PER_DEST_QUEUES])
// {
//     u32 n_pkts, pos, i;

//     n_pkts = bcache_cons_check(p->bc, MAX_BURST_RX);
    

// 	if (!n_pkts) {
// 		printf("There are no consumer slabs....\n");
// 		return 0;
// 	}

//     /* RXQ. */
//     struct bpool *bp = p->bc->bp;
// 	n_pkts = xsk_ring_cons__peek(&p->rxq, n_pkts, &pos);
// 	if (!n_pkts)
// 	{
// 		if (xsk_ring_prod__needs_wakeup(&bp->umem_fq))
// 		{
// 			struct pollfd pollfd = {
// 				.fd = xsk_socket__fd(p->xsk),
// 				.events = POLLIN,
// 			};

// 			poll(&pollfd, 1, 0);
// 		}
// 		return 0;
// 	}
    

// 	struct return_process_rx *ret_val = calloc(1, sizeof(struct return_process_rx));
//     for (i = 0; i < n_pkts; i++)
// 	{
// 		u64 addr_rx = xsk_ring_cons__rx_desc(&p->rxq, pos + i)->addr;
// 		u32 len = xsk_ring_cons__rx_desc(&p->rxq, pos + i)->len;
//         //process each packet and copy to per dest queue
//         u64 addr = xsk_umem__add_offset_to_addr(addr_rx);
// 		u8 *pkt = xsk_umem__get_data(p->params.bp->addr, addr);
//         process_rx_packet(pkt, &p->params,len, addr_rx, ret_val);

//         if (ret_val->new_len != 0) 
//         {
//             if (local_dest_queue[ret_val->ring_buf_index] != NULL)
//             {	
// 				if (transit_local_dest_queue[ret_val->ring_buf_index] != NULL)
//             	{
// 					void *obj;
// 					if (mpmc_queue_pull(transit_local_dest_queue[ret_val->ring_buf_index], &obj) != NULL) {
// 						u8 *updated_pkt = xsk_umem__get_data(p->params.bp->addr, addr);
// 						struct burst_tx *btx = calloc(1, sizeof(struct burst_tx));
// 						u64 transit_addr = (u64 *)obj;
// 						transit_addr = updated_pkt;
// 						// memcpy(transit_addr, updated_pkt, ret_val->new_len);
// 						btx->addr = transit_addr;
// 						btx->len = ret_val->new_len;
						
// 						int ret = mpmc_queue_push(local_dest_queue[ret_val->ring_buf_index], (void *) btx);
// 						if (!ret) 
// 						{
// 							printf("local_dest_queue is full \n");
// 						}
// 					} else {
// 						printf("transit_local_dest_queue returns NULL obj \n");
// 					}
// 				} else {
// 					printf("transit_local_dest_queue IS NULL \n");
// 				}
            
//                 bcache_prod(p->bc, addr_rx);
//             }
//             else
//             {
//                 printf("There is no queue to push the packet(queue_indexx): %d \n", ret_val->ring_buf_index);
//             }
//         }else
//         {
//             printf("packet length is 0 \n");
//             bcache_prod(p->bc, addr_rx);
//         }
// 		// bcache_prod(p->bc, addr_rx);
// 	}
// 	free(ret_val);

//     xsk_ring_cons__release(&p->rxq, n_pkts);
// 	p->n_pkts_rx += n_pkts;

// 	/* UMEM FQ. */
// 	for (;;)
// 	{
// 		int status;

// 		status = xsk_ring_prod__reserve(&bp->umem_fq, n_pkts, &pos);
// 		if (status == n_pkts)
// 			break;

// 		if (xsk_ring_prod__needs_wakeup(&bp->umem_fq))
// 		{
// 			struct pollfd pollfd = {
// 				.fd = xsk_socket__fd(p->xsk),
// 				.events = POLLIN,
// 			};

// 			poll(&pollfd, 1, 0);
// 		}
// 	}

// 	for (i = 0; i < n_pkts; i++)
// 		*xsk_ring_prod__fill_addr(&bp->umem_fq, pos + i) =
// 			bcache_cons(p->bc);

// 	xsk_ring_prod__submit(&bp->umem_fq, n_pkts);

// 	return n_pkts;
// }


static void *
thread_func_veth_rx(void *arg)
{
    struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	// u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	struct mpmc_queue *local_dest_queue;
	// struct mpmc_queue *transit_local_dest_queue[NUM_OF_PER_DEST_QUEUES];
	local_dest_queue = t->local_dest_queue_array[0];

	int x = 0;
	// for (x = 0; x < NUM_OF_PER_DEST_QUEUES; x++)
	// {
	// 	local_dest_queue[x] = t->local_dest_queue_array[x];
	// 	// transit_local_dest_queue[x] = t->transit_local_dest_queue_array[x];
	// }
	printf("src_port_pkt_gen:%d, dst_port_pkt_gen:%d , pkt_index:%d, %d\n", t->src_port_pkt_gen, t->dst_port_pkt_gen, t->pkt_index, t->pkt_index_2);
	// static u8 pkt_data[XSK_UMEM__DEFAULT_FRAME_SIZE];
	gen_eth_hdr_data(t->src_port_pkt_gen, t->dst_port_pkt_gen, t->pkt_index);
	// gen_eth_hdr_data(t->src_port_pkt_gen_2, t->dst_port_pkt_gen, t->pkt_index_2);
	// printf("HELLO after gen_eth_hdr_data++++++++++++++++++\n");

	int pktindex[2] = {t->pkt_index, t->pkt_index_2};

	// for (x = 0; x < 1; x++)
	int p_index=0;
    while (!t->quit)
	{
		p_index = p_index ^= 1;
        struct port *fake_port_rx = t->ports_rx[0]; //fake port just to get slabs
		if (fake_port_rx != NULL) {
			u32 n_pkts = bcache_cons_check(fake_port_rx->bc, MAX_BURST_RX);
			
			// n_pkts = port_rx_burst(fake_port_rx, local_dest_queue);
				
			if (!n_pkts) 
			{
				continue;
			}

			fake_port_rx->n_pkts_rx += n_pkts;

			int j;
			for (j = 0; j < n_pkts; j++)
			{
				u64 pkt_addr = bcache_cons(fake_port_rx->bc);
				if (fake_port_rx->bc->bp != NULL)
				{
					// memcpy(xsk_umem__get_data(fake_port_rx->bc->bp->addr, pkt_addr), pkt_data[pktindex[p_index]], PKT_SIZE);
					memcpy(xsk_umem__get_data(fake_port_rx->bc->bp->addr, pkt_addr), pkt_data[t->pkt_index], PKT_SIZE);
					// print_pkt_info(xsk_umem__get_data(fake_port_rx->bc->bp->addr, pkt_addr), PKT_SIZE);

					struct burst_tx *btx = calloc(1, sizeof(struct burst_tx));
					if (btx != NULL)
					{
						btx->addr = pkt_addr;
						btx->len = PKT_SIZE;
						btx->n_pkts++;

						// printf("veth rx: addr: %d, len: %d \n", pkt_addr, PKT_SIZE);
						

						if (local_dest_queue != NULL)
						{
							int ret = mpmc_queue_push(local_dest_queue, (void *) btx);
							if (!ret) 
							{
								// printf("local_dest_queue is full \n");
								//Release buffers to pool
								bcache_prod(fake_port_rx->bc, pkt_addr);
							}
						}
						else
						{
							printf("TODO: There is no queue to push the packet(ret_val->ring_buf_index): %d \n", 0);
						}
					}
				}
			}
		} else {
			printf("FAKE PORT IS NULL \n");
		}

    }
    printf("return from thread_func_veth_rx \n");
	return NULL;
}

// static inline u32
// port_nic_rx_burst(struct port *p, struct mpmc_queue *non_local_dest_queue[NUM_OF_PER_DEST_QUEUES])
// {
//     u32 n_pkts, pos, i;

//     n_pkts = bcache_cons_check(p->bc, MAX_BURST_RX);

// 	if (!n_pkts) {
// 		printf("There are no consumer slabs....\n");
// 		return 0;
// 	}

//     /* RXQ. */
//     struct bpool *bp = p->bc->bp;
// 	n_pkts = xsk_ring_cons__peek(&p->rxq, n_pkts, &pos);
// 	if (!n_pkts)
// 	{
// 		if (xsk_ring_prod__needs_wakeup(&bp->umem_fq))
// 		{
// 			struct pollfd pollfd = {
// 				.fd = xsk_socket__fd(p->xsk),
// 				.events = POLLIN,
// 			};

// 			poll(&pollfd, 1, 0);
// 		}
// 		return 0;
// 	}

// 	struct return_process_rx *ret_val = calloc(1, sizeof(struct return_process_rx));
//     for (i = 0; i < n_pkts; i++)
// 	{
// 		u64 addr_rx = xsk_ring_cons__rx_desc(&p->rxq, pos + i)->addr;
// 		u32 len = xsk_ring_cons__rx_desc(&p->rxq, pos + i)->len;
//         //process each packet and copy to per dest queue
//         u64 addr = xsk_umem__add_offset_to_addr(addr_rx);
// 		u8 *pkt = xsk_umem__get_data(p->params.bp->addr, addr);
//         process_rx_packet(pkt, &p->params,len, addr_rx, ret_val);

//         if (ret_val->new_len != 0) 
//         {
//             if (ret_val->new_len == 1)
//             {
//                 printf("TODO: NON Local queues implement later \n");
//             }
//             else 
//             {
// 				bcache_prod(p->bc, addr_rx);

//                 // if (veth_side_queue[ret_val->which_veth] != NULL)
// 				// {
// 				// 	if (transit_veth_side_queue[ret_val->which_veth] != NULL)
//             	// 	{
// 				// 		void *obj;
// 				// 		if (mpmc_queue_pull(transit_veth_side_queue[ret_val->which_veth], &obj) != NULL) {
// 				// 			struct burst_tx *btx = calloc(1, sizeof(struct burst_tx));
// 				// 			u8 *updated_pkt = xsk_umem__get_data(p->params.bp->addr, addr);
// 				// 			u64 transit_addr = (u64 *)obj;
// 				// 			transit_addr = updated_pkt;
// 				// 			// memcpy(transit_addr, updated_pkt, ret_val->new_len);
// 				// 			btx->addr = transit_addr;
// 				// 			btx->len = ret_val->new_len;

// 				// 			int ret = mpmc_queue_push(veth_side_queue[ret_val->which_veth], (void *) btx);
// 				// 			if (!ret) 
// 				// 			{
// 				// 				printf("veth_side_queue is full \n");
// 				// 			}
// 				// 		}
// 				// 		else {
// 				// 			printf("transit_veth_side_queue returns NULL obj \n");
// 				// 		}
// 				// 	}
// 				// 	else
// 				// 	{
// 				// 		printf("transit_veth_side_queue is NULL \n");
// 				// 	}
//                 //     bcache_prod(p->bc, addr_rx);
//                 // }
//                 // else
//                 // {
//                 //     printf("TODO: There is no veth_side_queue %d to push the packet \n", 0);
//                 // }
//             }
//         }
//         else
//         {
//             bcache_prod(p->bc, addr_rx);
//         }
// 	}
// 	free(ret_val);

//     xsk_ring_cons__release(&p->rxq, n_pkts);
// 	p->n_pkts_rx += n_pkts;

// 	/* UMEM FQ. */
// 	for (;;)
// 	{
// 		int status;

// 		status = xsk_ring_prod__reserve(&bp->umem_fq, n_pkts, &pos);
// 		if (status == n_pkts)
// 			break;

// 		if (xsk_ring_prod__needs_wakeup(&bp->umem_fq))
// 		{
// 			struct pollfd pollfd = {
// 				.fd = xsk_socket__fd(p->xsk),
// 				.events = POLLIN,
// 			};

// 			poll(&pollfd, 1, 0);
// 		}
// 	}

// 	for (i = 0; i < n_pkts; i++)
// 		*xsk_ring_prod__fill_addr(&bp->umem_fq, pos + i) =
// 			bcache_cons(p->bc);

// 	xsk_ring_prod__submit(&bp->umem_fq, n_pkts);

// 	// printf("nic rx done \n");

// 	return n_pkts;
// }

static inline u32
port_nic_rx_burst(struct port *p, struct burst_rx *b)
{
	u32 n_pkts, pos, i;
	struct bpool *bp = p->bc->bp;

	/* Free buffers for FQ replenish. */
	n_pkts = ARRAY_SIZE(b->addr);

	n_pkts = bcache_cons_check(p->bc, n_pkts);

	if (!n_pkts) {
		printf("There are no consumer slabs....\n");
		return 0;
	}
		

	// printf("bp->n_slabs_available %ld \n", p->bc->bp->n_slabs_available);

	/* RXQ. */
	n_pkts = xsk_ring_cons__peek(&p->rxq, n_pkts, &pos);
	if (!n_pkts)
	{
		if (xsk_ring_prod__needs_wakeup(&bp->umem_fq))
		{
			struct pollfd pollfd = {
				.fd = xsk_socket__fd(p->xsk),
				.events = POLLIN,
			};

			poll(&pollfd, 1, 0);
		}
		// printf("There are no packets in rx ring....\n");
		// recvfrom(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT, NULL, NULL);
		return 0;
	}

	for (i = 0; i < n_pkts; i++)
	{
		b->addr[i] = xsk_ring_cons__rx_desc(&p->rxq, pos + i)->addr;
		b->len[i] = xsk_ring_cons__rx_desc(&p->rxq, pos + i)->len;
	}

	xsk_ring_cons__release(&p->rxq, n_pkts);
	p->n_pkts_rx += n_pkts;

	/* UMEM FQ. */
	for (;;)
	{
		int status;

		status = xsk_ring_prod__reserve(&bp->umem_fq, n_pkts, &pos);
		if (status == n_pkts)
			break;

		if (xsk_ring_prod__needs_wakeup(&bp->umem_fq))
		{
			struct pollfd pollfd = {
				.fd = xsk_socket__fd(p->xsk),
				.events = POLLIN,
			};

			poll(&pollfd, 1, 0);
			// recvfrom(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT, NULL, NULL);
		}
	}

	for (i = 0; i < n_pkts; i++)
		*xsk_ring_prod__fill_addr(&bp->umem_fq, pos + i) =
			bcache_cons(p->bc);

	xsk_ring_prod__submit(&bp->umem_fq, n_pkts);

	return n_pkts;
}

static void *
thread_func_nic_rx(void *arg)
{
    struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	// u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	struct mpmc_queue *non_local_dest_queue[NUM_OF_PER_DEST_QUEUES];

	int assigned_queue_count = t->assigned_queue_count;

	int w;
	for (w = 0; w < assigned_queue_count; w++)
	{
			non_local_dest_queue[w] = t->non_local_dest_queue_array[w];
	}

    while (!t->quit)
	{
        int k = 0;
		for (k = 0; k < assigned_queue_count; k++)
		{
            struct port *port_rx = t->ports_rx[k];
            struct burst_rx *brx = &t->burst_rx;

			u32 n_pkts, j;

			/* RX. */
			n_pkts = port_nic_rx_burst(port_rx, brx);
			

			if (!n_pkts) 
			{
					// nic_rx_no_packet_counter++;
					continue;
			}

			for (j = 0; j < n_pkts; j++)
			{
				u64 addr = xsk_umem__add_offset_to_addr(brx->addr[j]);
				u8 *pkt = xsk_umem__get_data(port_rx->params.bp->addr,
											addr);
											
				// print_pkt_info(pkt, brx->len[j]);
				bcache_prod(port_rx->bc, brx->addr[j]);
			}
        }
    }
    printf("return from thread_func_nic_rx \n");
	return NULL;
}

static inline void
port_tx_burst_collector_nic(struct port *p, struct burst_tx_collector *b, int free_btx, int wait_all)
{
	struct bpool *bp = p->bc->bp;
	u32 n_pkts, pos, i;
	int status;

	/* UMEM CQ. */
	n_pkts = p->params.bp->umem_cfg.comp_size;

	n_pkts = xsk_ring_cons__peek(&bp->umem_cq, n_pkts, &pos);

    // printf("cq has packets: %d \n", n_pkts);


	for (i = 0; i < n_pkts; i++)
	{
		u64 addr = *xsk_ring_cons__comp_addr(&bp->umem_cq, pos + i);

		// bcache_prod_tx(p->bc, addr);
		bcache_prod(p->bc, addr);
	}

	xsk_ring_cons__release(&bp->umem_cq, n_pkts);

	/* TXQ. */
	n_pkts = b->n_pkts;

	for (;;)
	{
		status = xsk_ring_prod__reserve(&p->txq, n_pkts, &pos);
		if (status == n_pkts)
		{
            // printf("status == n_pkts \n");
			break;
		}

		if (xsk_ring_prod__needs_wakeup(&p->txq))
			sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT,
			       NULL, 0);	
	}
	

	for (i = 0; i < n_pkts; i++)
	{
        // u64 pkt_addr =  bcache_cons_tx(p->bc);
		// u8 *pkt = xsk_umem__get_data(p->params.bp->addr, pkt_addr);
		// memcpy(pkt, b->addr[i], b->len[i]);
		// xsk_ring_prod__tx_desc(&p->txq, pos + i)->addr = pkt_addr;
		// xsk_ring_prod__tx_desc(&p->txq, pos + i)->len = b->len[i];

		xsk_ring_prod__tx_desc(&p->txq, pos + i)->addr = b->addr[i];
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->len = b->len[i];
		
	}

	xsk_ring_prod__submit(&p->txq, n_pkts);
	if (xsk_ring_prod__needs_wakeup(&p->txq))
			sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT,
			       NULL, 0);

	p->n_pkts_tx += n_pkts;
}

static void *
thread_func_nic_tx(void *arg)
{
    struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	// u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	struct mpmc_queue *local_dest_queue[NUM_OF_PER_DEST_QUEUES];
	// struct mpmc_queue *transit_local_dest_queue[NUM_OF_PER_DEST_QUEUES];

	int assigned_queue_count = t->assigned_queue_count;

	int w;
	for (w = 0; w < assigned_queue_count; w++)
	{
			local_dest_queue[w] = t->local_dest_queue_array[w];
			// transit_local_dest_queue[w] = t->transit_local_dest_queue_array[w];
	}

	//TODO: Fix this so that each nic port has its own burst_tx_collector
	// struct burst_tx_collector *btx_collector = &t->burst_tx_collector[0];

    while (!t->quit)
	{
        int k = 0;
		for (k = 0; k < assigned_queue_count; k++)
		{
            struct port *port_tx = t->ports_tx[k];

			// u32 n_pkts_in_cache = bcache_cons_tx_check(port_tx->bc, MAX_BURST_TX);
			// if (!n_pkts_in_cache) 
            // {
			// 	printf("There are no consumer tx slabs....\n");
            //     continue;
            // }

            struct burst_tx_collector *btx_collector = &t->burst_tx_collector[0];
            int btx_index = 0;
            if (local_dest_queue[k] != NULL)
            {
                while ((mpmc_queue_available(local_dest_queue[k])) && (btx_index < MAX_BURST_TX))
                {
                    void *obj;
                    if (mpmc_queue_pull(local_dest_queue[k], &obj) != NULL) {
                        struct burst_tx *btx = (struct burst_tx *)obj;
						// printf("addr: %d, len: %d \n", btx->addr, btx->len);
                        btx_collector->len[btx_index] = btx->len;
						btx_collector->addr[btx_index] = btx->addr;
                        free(obj);

						// printf("addr: %d, len: %d \n", btx_collector->addr[btx_index], btx_collector->len[btx_index]);
						// u8 *pkt = xsk_umem__get_data(port_tx->params.bp->addr, btx_collector->addr[btx_index]);
						// print_pkt_info(pkt, btx_collector->len[btx_index]);
                        btx_index++;
                        btx_collector->n_pkts = btx_index;
						
                    }
                }
            } else {
                printf("local_dest_queue is NULL \n");
            }
            if (btx_index)
            {
				// if(btx_index < 20) 
				// {
				// 	printf("btx_index: %d \n", btx_index);
				// }
                // printf("there are packets to NIC tx \n");
                port_tx_burst_collector_nic(port_tx, btx_collector, 0, 0);
            } 
        
            btx_collector->n_pkts = 0;
        }
    }
    printf("return from thread_func_nic_tx \n");
	return NULL;
}

static inline void
port_tx_burst_collector_veth(struct port *p, struct burst_tx_collector *b, int free_btx, int wait_all, 
				struct mpmc_queue *transit_veth_side_queue)
{
	struct bpool *bp = p->bc->bp;
	u32 n_pkts, pos, i;
	int status;

	/* UMEM CQ. */
	n_pkts = p->params.bp->umem_cfg.comp_size;

	n_pkts = xsk_ring_cons__peek(&bp->umem_cq, n_pkts, &pos);

    // printf("cq has packets: %d \n", n_pkts);


	for (i = 0; i < n_pkts; i++)
	{
		u64 addr = *xsk_ring_cons__comp_addr(&bp->umem_cq, pos + i);

		bcache_prod_tx(p->bc, addr);
	}

	xsk_ring_cons__release(&bp->umem_cq, n_pkts);

	/* TXQ. */
	n_pkts = b->n_pkts;

	for (;;)
	{
		status = xsk_ring_prod__reserve(&p->txq, n_pkts, &pos);
		if (status == n_pkts)
		{
            // printf("status == n_pkts \n");
			break;
		}

		if (xsk_ring_prod__needs_wakeup(&p->txq))
			sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT,
			       NULL, 0);

		
	}

	for (i = 0; i < n_pkts; i++)
	{
        u64 pkt_addr =  bcache_cons_tx(p->bc);
		u8 *pkt = xsk_umem__get_data(p->params.bp->addr, pkt_addr);
		memcpy(pkt, b->addr[i], b->len[i]);
		// print_pkt_info(pkt, b->len[i]);
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->addr = pkt_addr;
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->len = b->len[i];

		if (transit_veth_side_queue != NULL)
		{
			int ret = mpmc_queue_push(transit_veth_side_queue, (void *) b->addr[i]);
			if (!ret) 
			{
				printf("transit_veth_side_queue is full \n");
			}
		}
		else{
			printf("transit_veth_side_queue is NULL \n");
		}
	}

	// printf("veth tx done \n");

	xsk_ring_prod__submit(&p->txq, n_pkts);
	if (xsk_ring_prod__needs_wakeup(&p->txq))
			sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT,
			       NULL, 0);

	p->n_pkts_tx += n_pkts;
}

// static void *
// thread_func_veth_tx(void *arg)
// {
//     struct thread_data *t = arg;
// 	cpu_set_t cpu_cores;
// 	// u32 i;

// 	CPU_ZERO(&cpu_cores);
// 	CPU_SET(t->cpu_core_id, &cpu_cores);
// 	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

//     struct mpmc_queue *veth_side_queue_single = t->veth_side_queue_array[0];
// 	struct mpmc_queue *transit_veth_side_queue_single = t->transit_veth_side_queue_array[0];

//     while (!t->quit) {
// 		struct port *port_tx = t->ports_tx[0];

// 		u32 n_pkts_in_cache = bcache_cons_tx_check(port_tx->bc, MAX_BURST_TX);
// 		if (!n_pkts_in_cache) 
// 		{
// 			printf("There are no consumer tx slabs....\n");
// 			continue;
// 		}

//         struct burst_tx_collector *btx_collector = &t->burst_tx_collector[0];
//         int btx_index = 0;

//         if (veth_side_queue_single != NULL)
// 		{
//             while ((mpmc_queue_available(veth_side_queue_single)) && (btx_index < n_pkts_in_cache))
// 			{
//                 void *obj;
//                 if (mpmc_queue_pull(veth_side_queue_single, &obj) != NULL) {
//                     struct burst_tx *btx = (struct burst_tx *)obj;
//                     // btx_collector->addr[btx_index] = bcache_cons_tx(port_tx->bc);
//                     btx_collector->len[btx_index] = btx->len;
// 					btx_collector->addr[btx_index] = btx->addr;
//                     // memcpy(btx_collector->pkt[btx_index], btx->pkt, btx->len);
//                     free(obj);
// 					// printf("veth tx \n");
//                     btx_index++;
//                     btx_collector->n_pkts = btx_index;
//                 }
//             }
//         }
//         else 
//         {
// 			printf("veth side queue is null \n");
// 		}
//         if (btx_index)
//         {
//             // printf("There are packets to goto veth tx \n");
//             port_tx_burst_collector_veth(port_tx, btx_collector, 0, 0, transit_veth_side_queue_single);
//         } 
    
//         btx_collector->n_pkts = 0;
//     }
// }