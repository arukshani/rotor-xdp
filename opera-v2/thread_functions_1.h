static inline void ether_addr_copy_assignment(u8 *dst, const u8 *src)
{
	u16 *a = (u16 *)dst;
	const u16 *b = (const u16 *)src;

	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
}

static inline u32
port_rx_burst(struct port *p, struct burst_rx *b, int index)
{
	u32 n_pkts, pos, i;

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
		if (xsk_ring_prod__needs_wakeup(&p->umem_fq))
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

		status = xsk_ring_prod__reserve(&p->umem_fq, n_pkts, &pos);
		if (status == n_pkts)
			break;

		if (xsk_ring_prod__needs_wakeup(&p->umem_fq))
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
		*xsk_ring_prod__fill_addr(&p->umem_fq, pos + i) =
			bcache_cons(p->bc);

	xsk_ring_prod__submit(&p->umem_fq, n_pkts);

	return n_pkts;
}

static inline void
port_tx_burst(struct port *p, struct burst_tx *b, int free_btx, int wait_all)
{
	u32 n_pkts, pos, i;
	int status;

	/* UMEM CQ. */
	n_pkts = p->params.bp->umem_cfg.comp_size;

	n_pkts = xsk_ring_cons__peek(&p->umem_cq, n_pkts, &pos);

	// printf("n_pkts in port_tx_burst %ld \n", n_pkts);
	// printf("bp->n_slabs_available %ld \n", p->bc->bp->n_slabs_available);

	for (i = 0; i < n_pkts; i++)
	{
		u64 addr = *xsk_ring_cons__comp_addr(&p->umem_cq, pos + i);

		bcache_prod(p->bc, addr);
	}

	xsk_ring_cons__release(&p->umem_cq, n_pkts);

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
		{
			if (wait_all)
			{
				sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_WAITALL,
					   NULL, 0);
			}
			else
			{
				sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT,
					   NULL, 0);
			}
		}
	}

	// printf("Fill tx desc for n_pkts %ld \n", n_pkts);
	// printf("Port tx burst \n");

	for (i = 0; i < n_pkts; i++)
	{
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->addr = b->addr[i];
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->len = b->len[i];
	}

	if (free_btx)
	{
		free(b);
	}

	xsk_ring_prod__submit(&p->txq, n_pkts);
	if (xsk_ring_prod__needs_wakeup(&p->txq))
	{
		if (wait_all)
		{
			sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_WAITALL, NULL, 0);
		}
		else
		{
			sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT, NULL, 0);
		}
	}

	p->n_pkts_tx += n_pkts;
}

static inline void
port_tx_burst_collector(struct port *p, struct burst_tx_collector *b, int free_btx, int wait_all)
{
	u32 n_pkts, pos, i;
	int status;

	/* UMEM CQ. */
	n_pkts = p->params.bp->umem_cfg.comp_size;

	n_pkts = xsk_ring_cons__peek(&p->umem_cq, n_pkts, &pos);


	for (i = 0; i < n_pkts; i++)
	{
		u64 addr = *xsk_ring_cons__comp_addr(&p->umem_cq, pos + i);

		bcache_prod(p->bc, addr);
	}

	xsk_ring_cons__release(&p->umem_cq, n_pkts);

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
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->addr = b->addr[i];
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->len = b->len[i];
	}

	xsk_ring_prod__submit(&p->txq, n_pkts);
	if (xsk_ring_prod__needs_wakeup(&p->txq))
			sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT,
			       NULL, 0);

	p->n_pkts_tx += n_pkts;
}

static inline void
flush_tx(struct port *p)
{
	if (xsk_ring_prod__needs_wakeup(&p->txq))
	{
		sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT, NULL, 0);
	}
}

bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

//from veth to NIC
static int get_destination_queue_index(void *data, struct port_params *params)
{
	int is_nic = strcmp(params->iface, nic_iface);
	// printf("params->iface %s \n", params->iface);

	// if (is_veth == 0 || is_veth3 == 0)
    if (prefix(OUTER_VETH_PREFIX, params->iface))
	{
		struct iphdr *inner_ip_hdr_tmp = (struct iphdr *)(data +
														  sizeof(struct ethhdr));

			// #if DEBUG == 1
			// 	if (inner_ip_hdr_tmp->protocol == IPPROTO_TCP) 
			// 	{
			// 		struct tcphdr *inner_tcp_hdr = (struct tcphdr *)(data +
			// 					sizeof(struct ethhdr) +
			// 					sizeof(struct iphdr));

			// 		seq[time_index] = ntohl(inner_tcp_hdr->seq);
			// 		ack_seq[time_index] = ntohl(inner_tcp_hdr->ack_seq);
			// 		src_port[time_index] = ntohs(inner_tcp_hdr->source);
			// 		dst_port[time_index] = ntohs(inner_tcp_hdr->dest);
					
			// 		if (ntohl(inner_tcp_hdr->syn)) {
			// 			is_syn[time_index] = 1;
			// 		} 
					
			// 		if (ntohl(inner_tcp_hdr->ack)){
			// 			is_ack[time_index] = 1;
			// 		} 
					
			// 		if (ntohl(inner_tcp_hdr->fin)){
			// 			is_fin[time_index] = 1;
			// 		} 

			// 		// tcp_rcv_wnd[time_index] = (ntohl(inner_tcp_hdr->window) * 14); //multiply by scale factor 

			// 		timestamp_arr[time_index] = now;
			// 		slot[time_index]=0;
			// 		topo_arr[time_index] = topo;
			// 		hop_count[time_index] = 0;
			// 		ns_packet_len[time_index] = 0; 
			// 		time_index++;
			// 	}

			// 	// if (inner_ip_hdr_tmp->protocol == IPPROTO_UDP) 
			// 	// if (inner_ip_hdr_tmp->protocol == IPPROTO_ICMP) 
			// 	// {
			// 	// 	struct icmphdr *inner_icmp_hdr = (struct icmphdr *)(inner_ip_hdr_tmp + 1);
			// 	// 	seq[time_index] = ntohs(inner_icmp_hdr->un.echo.sequence);
			// 	// 	timestamp_arr[time_index] = now;
			// 	// 	slot[time_index]=0;
			// 	// 	topo_arr[time_index] = topo;
			// 	// 	hop_count[time_index] = 0;
			// 	// 	time_index++;
			// 	// }

			// #endif

		
		char dest_char[16];
		unsigned char bytes[4];
		bytes[0] = inner_ip_hdr_tmp->daddr & 0xFF;
		bytes[1] = (inner_ip_hdr_tmp->daddr >> 8) & 0xFF;
		bytes[2] = (inner_ip_hdr_tmp->daddr >> 16) & 0xFF;
		bytes[3] = (inner_ip_hdr_tmp->daddr >> 24) & 0xFF;  
		snprintf(dest_char, 16, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], 1); 
		struct sockaddr_in construct_dest_ip;
		inet_aton(dest_char, &construct_dest_ip.sin_addr);

		struct ip_set *dest_ip_index = mg_map_get(&ip_table, construct_dest_ip.sin_addr.s_addr);
		// printf("dest_ip_index = %d\n", dest_ip_index->index);
		return (dest_ip_index->index - 1);
	} 
}

// from nic-to-nic or nic-to-veth
static void get_queue_index_for_nic_rx(void *data, struct port_params *params, uint32_t len, u64 addr, struct return_process_rx *return_val)
{
	int is_nic = strcmp(params->iface, nic_iface);
	if (is_nic == 0)
	{
		// printf("from NIC 1 \n");
		struct ethhdr *eth = (struct ethhdr *)data;
		struct iphdr *outer_ip_hdr = (struct iphdr *)(data +
													  sizeof(struct ethhdr));
		struct gre_hdr *greh = (struct gre_hdr *)(outer_ip_hdr + 1);

		if (ntohs(eth->h_proto) != ETH_P_IP || outer_ip_hdr->protocol != IPPROTO_GRE ||
					ntohs(greh->proto) != ETH_P_TEB)
		{
			printf("not a GRE packet \n");
			return false;
		}
		struct ethhdr *inner_eth = (struct ethhdr *)(greh + 1);
		if (ntohs(inner_eth->h_proto) != ETH_P_IP) {
			printf("inner eth proto is not ETH_P_IP %x \n", inner_eth->h_proto);
		    return false;
		}

		struct iphdr *inner_ip_hdr = (struct iphdr *)(inner_eth + 1);
		if (src_ip != (outer_ip_hdr->daddr))
		{
			// printf("Not destined for local node \n");

			char dest_char[16];
			unsigned char bytes[4];
			bytes[0] = inner_ip_hdr->daddr & 0xFF;
			bytes[1] = (inner_ip_hdr->daddr >> 8) & 0xFF;
			bytes[2] = (inner_ip_hdr->daddr >> 16) & 0xFF;
			bytes[3] = (inner_ip_hdr->daddr >> 24) & 0xFF;  
			snprintf(dest_char, 16, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], 1); 
			struct sockaddr_in construct_dest_ip;
			inet_aton(dest_char, &construct_dest_ip.sin_addr);
			
			// send it back out NIC
			struct ip_set *dest_ip_index = mg_map_get(&ip_table, construct_dest_ip.sin_addr.s_addr);
			return_val->ring_buf_index = dest_ip_index->index - 1;
			return_val->new_len = 1; 
		} else
		{
			// printf("Destined for local node \n");
			int hops = greh->hopcount + 1;

			char fourthoctec = (inner_ip_hdr->daddr >> 24) & 0xFF;
			int octec_int = (int)(fourthoctec);

			return_val->which_veth =  octec_int - 2;//greh->flags
			if (return_val->which_veth != 0)
			{
				printf("octec_int: %d ; return_val->which_veth %d \n", octec_int, return_val->which_veth);
			}

			// send it to local veth
			void *cutoff_pos = greh + 1;
			int cutoff_len = (int)(cutoff_pos - data);
			int new_len = len - cutoff_len;

			// #if DEBUG == 1
			// 	if (inner_ip_hdr->protocol == IPPROTO_TCP) 
			// 	{
			// 		struct tcphdr *inner_tcp_hdr = (struct tcphdr *)(inner_ip_hdr + 1);

			// 		seq[time_index] = ntohl(inner_tcp_hdr->seq);
			// 		ack_seq[time_index] = ntohl(inner_tcp_hdr->ack_seq);
			// 		src_port[time_index] = ntohs(inner_tcp_hdr->source);
			// 		dst_port[time_index] = ntohs(inner_tcp_hdr->dest);
			// 		if (ntohl(inner_tcp_hdr->syn)) {
			// 			is_syn[time_index] = 1;
			// 		} 
					
			// 		if (ntohl(inner_tcp_hdr->ack)){
			// 			is_ack[time_index] = 1;
			// 		} 
					
			// 		if (ntohl(inner_tcp_hdr->fin)){
			// 			is_fin[time_index] = 1;
			// 		} 

			// 		// tcp_rcv_wnd[time_index] = (ntohl(inner_tcp_hdr->window) * 14); //multiply by scale factor 

			// 		timestamp_arr[time_index] = now;
			// 		slot[time_index]=2;
			// 		topo_arr[time_index] = topo;
			// 		hop_count[time_index] = hops;
			// 		ns_packet_len[time_index] = new_len; 
			// 		time_index++;
			// 	}
			// 	// if (inner_ip_hdr->protocol == IPPROTO_UDP) 
			// 	// if (inner_ip_hdr->protocol == IPPROTO_ICMP) 
			// 	// {
			// 	// 	struct icmphdr *inner_icmp_hdr = (struct icmphdr *)(inner_ip_hdr + 1);
			// 	// 	seq[time_index] = ntohs(inner_icmp_hdr->un.echo.sequence);
			// 	// 	timestamp_arr[time_index] = now;
			// 	// 	slot[time_index]=2;
			// 	// 	topo_arr[time_index] = topo;
			// 	// 	hop_count[time_index] = hops;
			// 	// 	time_index++;
			// 	// }
			// #endif

			int offset = 0 + cutoff_len;
			u64 inner_eth_start_addr = addr + offset;

			u8 *new_data = xsk_umem__get_data(params->bp->addr, inner_eth_start_addr);
			memcpy(xsk_umem__get_data(params->bp->addr, addr), new_data, new_len);

			return_val->new_len = new_len;
		}
	}
}

static int encap_veth(int dest_index, void *data, struct port_params *params, uint32_t len, u64 addr)
{
	struct iphdr *outer_iphdr;
	struct ethhdr *outer_eth_hdr;
	struct iphdr *inner_ip_hdr_tmp = (struct iphdr *)(data +
														  sizeof(struct ethhdr));

	// #if DEBUG == 1
		// if (inner_ip_hdr_tmp->protocol == IPPROTO_TCP) 
		// {
		// 	struct tcphdr *inner_tcp_hdr = (struct tcphdr *)(data +
		// 			    sizeof(struct ethhdr) +
		// 			    sizeof(struct iphdr));

		// 	seq[time_index] = ntohl(inner_tcp_hdr->seq);
		// 	// ack_seq[time_index] = ntohl(inner_tcp_hdr->ack_seq);
		// 	src_port[time_index] = ntohs(inner_tcp_hdr->source);
		// 	// dst_port[time_index] = ntohs(inner_tcp_hdr->dest);
			
		// 	// if (ntohl(inner_tcp_hdr->syn)) {
		// 	// 	is_syn[time_index] = 1;
		// 	// } 
			
		// 	// if (ntohl(inner_tcp_hdr->ack)){
		// 	// 	is_ack[time_index] = 1;
		// 	// } 
			
		// 	// if (ntohl(inner_tcp_hdr->fin)){
		// 	// 	is_fin[time_index] = 1;
		// 	// } 

		// 	// tcp_rcv_wnd[time_index] = (ntohl(inner_tcp_hdr->window) * 14); //multiply by scale factor 

		// 	timestamp_arr[time_index] = now;
		// 	// slot[time_index]=0;
		// 	topo_arr[time_index] = topo;
		// 	// hop_count[time_index] = 0;
		// 	// ns_packet_len[time_index] = len; 
		// 	time_index++;
		// }

		// if (inner_ip_hdr_tmp->protocol == IPPROTO_UDP) 
		// if (inner_ip_hdr_tmp->protocol == IPPROTO_ICMP) 
		// {
		// 	struct icmphdr *inner_icmp_hdr = (struct icmphdr *)(inner_ip_hdr_tmp + 1);
		// 	seq[time_index] = ntohs(inner_icmp_hdr->un.echo.sequence);
		// 	timestamp_arr[time_index] = now;
		// 	slot[time_index]=0;
		// 	topo_arr[time_index] = topo;
		// 	hop_count[time_index] = 0;
		// 	time_index++;
		// }

	// #endif

	int olen = 0;
	olen += ETH_HLEN;
	olen += sizeof(struct gre_hdr);

	int encap_size = 0; // outer_eth + outer_ip + gre
	int encap_outer_eth_len = ETH_HLEN;
	int encap_outer_ip_len = sizeof(struct iphdr);
	int encap_gre_len = sizeof(struct gre_hdr);

	encap_size += encap_outer_eth_len;
	encap_size += encap_outer_ip_len;
	encap_size += encap_gre_len;

	int offset = 0 + encap_size;
	u64 new_addr = addr + offset;
	int new_len = len + encap_size;

	u64 new_new_addr = xsk_umem__add_offset_to_addr(new_addr);
	u8 *new_data = xsk_umem__get_data(params->bp->addr, new_new_addr);

	memcpy(new_data, data, len);
	
	int mac_index;
	int destindex = dest_index + 1;
	getRouteElement(route_table, destindex, topo, &mac_index);
	struct mac_addr *dest_mac_val = mg_map_get(&mac_table, mac_index);
	outer_eth_hdr = (struct ethhdr *)data;
	ether_addr_copy_assignment(outer_eth_hdr->h_dest, dest_mac_val->bytes);

	outer_eth_hdr->h_proto = htons(ETH_P_IP);

	outer_iphdr = (struct iphdr *)(data +
									sizeof(struct ethhdr));
	
	__builtin_memcpy(outer_iphdr, inner_ip_hdr_tmp, sizeof(*outer_iphdr));

	char dest_char[16];
	unsigned char bytes[4];
	bytes[0] = inner_ip_hdr_tmp->daddr & 0xFF;
	bytes[1] = (inner_ip_hdr_tmp->daddr >> 8) & 0xFF;
	bytes[2] = (inner_ip_hdr_tmp->daddr >> 16) & 0xFF;
	bytes[3] = (inner_ip_hdr_tmp->daddr >> 24) & 0xFF;  
	snprintf(dest_char, 16, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], 1); 
	struct sockaddr_in construct_dest_ip;
	inet_aton(dest_char, &construct_dest_ip.sin_addr);
	
	outer_iphdr->protocol = IPPROTO_GRE;
	outer_iphdr->tot_len = bpf_htons(olen + bpf_ntohs(inner_ip_hdr_tmp->tot_len));
	outer_iphdr->daddr = construct_dest_ip.sin_addr.s_addr;

	

	struct gre_hdr *gre_hdr;
	gre_hdr = (struct gre_hdr *)(data +
									sizeof(struct ethhdr) + sizeof(struct iphdr));

	gre_hdr->proto = bpf_htons(ETH_P_TEB);
	// if (strcmp(params->iface, "vethout2") == 0) {
	// 	gre_hdr->flags = 0;
	// } else if (strcmp(params->iface, "vethout3") == 0) {
	// 	gre_hdr->flags = 1;
	// } 
	gre_hdr->hopcount = 0;

	return new_len;

}

static void encap_indirection(int dest_index, void *data, struct port_params *params, uint32_t len, u64 addr)
{
	int next_mac_index;
	int destindex = dest_index + 1;
	getRouteElement(route_table, destindex, topo, &next_mac_index);
	// printf("next_dest_ip_index = %d, next_mac_index=%d \n", next_dest_ip_index->index, next_mac_index);
	
	struct ethhdr *eth = (struct ethhdr *)data;
	struct iphdr *outer_ip_hdr = (struct iphdr *)(data +
													sizeof(struct ethhdr));
	struct gre_hdr *greh = (struct gre_hdr *)(outer_ip_hdr + 1);
	greh->hopcount = greh->hopcount + 1;

	struct ethhdr *inner_eth = (struct ethhdr *)(greh + 1);
	struct iphdr *inner_ip_hdr = (struct iphdr *)(inner_eth + 1);
	char dest_char[16];
	unsigned char bytes[4];
	bytes[0] = inner_ip_hdr->daddr & 0xFF;
	bytes[1] = (inner_ip_hdr->daddr >> 8) & 0xFF;
	bytes[2] = (inner_ip_hdr->daddr >> 16) & 0xFF;
	bytes[3] = (inner_ip_hdr->daddr >> 24) & 0xFF;  
	snprintf(dest_char, 16, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], 1); 
	struct sockaddr_in construct_dest_ip;
	inet_aton(dest_char, &construct_dest_ip.sin_addr);


	struct mac_addr *next_dest_mac_val = mg_map_get(&mac_table, next_mac_index);
	ether_addr_copy_assignment(eth->h_dest, next_dest_mac_val->bytes);
	ether_addr_copy_assignment(eth->h_source, &out_eth_src);
	outer_ip_hdr->daddr = construct_dest_ip.sin_addr.s_addr;
}


// from_VETH -> to_NIC
static void *
thread_func_veth(void *arg)
{
	struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	struct mpmc_queue *local_dest_queue[NUM_OF_PER_DEST_QUEUES];

	int x = 0;
	for (x = 0; x < NUM_OF_PER_DEST_QUEUES; x++)
	{
		local_dest_queue[x] = t->local_dest_queue_array[x];
	}

	// struct return_process_rx *ret_val = calloc(1, sizeof(struct return_process_rx));

	while (!t->quit)
	{
		// ret_val->new_len = 0;
		// ret_val->ring_buf_index = 0;

		struct port *port_rx = t->ports_rx[0];
		struct burst_rx *brx = &t->burst_rx;

		u32 n_pkts, j;

		/* RX. */
		n_pkts = port_rx_burst(port_rx, brx, i);

		if (!n_pkts) 
		{
			// veth_rx_no_packet_counter++;
			continue;
		}

		// printf("veth rx n_pkts: %d \n", n_pkts);

		/* Process & TX. */
		for (j = 0; j < n_pkts; j++)
		{

			// printf("bp->n_slabs_available %ld \n", port_rx->bc->bp->n_slabs_available);

			u64 addr = xsk_umem__add_offset_to_addr(brx->addr[j]);
			u8 *pkt = xsk_umem__get_data(port_rx->params.bp->addr,
										addr);

			// process_rx_packet_old(pkt, &port_rx->params, brx->len[j], brx->addr[j], ret_val);
			int dest_index = get_destination_queue_index(pkt, &port_rx->params);

			struct burst_tx *btx = calloc(1, sizeof(struct burst_tx));
			if (btx != NULL)
			{
				btx->addr[0] = brx->addr[j];
				// btx->len[0] = ret_val->new_len;
				btx->len[0] = brx->len[j];

				// printf("packet len %d \n", ret_val->new_len);
				
				// if (ret_val->new_len !=0) 
				if (brx->addr[j] != 0)
				{
					btx->n_pkts++;
					// struct mpmc_queue *dest_queue = local_dest_queue[ret_val->ring_buf_index];
					// printf("ret_val->ring_buf_index: %d \n", ret_val->ring_buf_index);
					// if (local_dest_queue[ret_val->ring_buf_index] != NULL)
					if (local_dest_queue[dest_index] != NULL)
					{
						// printf("push pakcet %d to local dest queue: %d \n", btx->addr[0], dest_index);
						// mpmc_queue_push(dest_queue, (void *) btx);
						// int ret = mpmc_queue_push(local_dest_queue[ret_val->ring_buf_index], (void *) btx);
						int ret = mpmc_queue_push(local_dest_queue[dest_index], (void *) btx);
						if (!ret) 
						{
							local_dest_queue_overflow_count++;
							// printf("local_dest_queue is full \n");
							//Release buffers to pool
							bcache_prod(port_rx->bc, brx->addr[j]);
							free(btx);
						}
					}
					else
					{
						printf("TODO: There is no queue to push the packet(ret_val->ring_buf_index): %d \n", dest_index);
					}
				}
				else
				{
					printf("brx->addr[j] is zero \n");
					bcache_prod(port_rx->bc, brx->addr[j]);
				}
			} else {
				printf("brx is null. cannot allocate memory \n");
			}
		}
	}
	// free(ret_val);
	printf("return from thread_func_veth \n");
	return NULL;
}

// from_VETH -> to_NIC (TX)
static void *
thread_func_veth_to_nic_tx(void *arg)
{
	struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	struct mpmc_queue *local_dest_queue[NUM_OF_PER_DEST_QUEUES];
	struct mpmc_queue *non_local_dest_queue[NUM_OF_PER_DEST_QUEUES];

	int assigned_queue_count = t->assigned_queue_count;
	int assigned_perdest_count = t->assigned_perdest_count;

	int w;
	for (w = 0; w < assigned_perdest_count; w++)
	{
			local_dest_queue[w] = t->local_dest_queue_array[w];
	}

	for (w = 0; w < assigned_perdest_count; w++)
	{
			non_local_dest_queue[w] = t->non_local_dest_queue_array[w];
	}

	//TODO: Fix this so that each nic port has its own burst_tx_collector
	struct burst_tx_collector *btx_collector = &t->burst_tx_collector[0];

	while (!t->quit)
	{
		int k = 0;
		for (k = 0; k < assigned_queue_count; k++)
		{
			struct port *port_tx = t->ports_tx[k];
		
			int w;
			for (w = 0; w < assigned_perdest_count; w++)
			{
				int btx_index = 0;
				if (non_local_dest_queue[w] != NULL)
				{
					while ((mpmc_queue_available(non_local_dest_queue[w])) && (btx_index < MAX_BURST_TX))
					{
						void *obj2;
						if (mpmc_queue_pull(non_local_dest_queue[w], &obj2) != NULL) {
							// hasPackets = true;
							struct burst_tx *btx2 = (struct burst_tx *)obj2;
							// if (btx2 != NULL)
							// {
								u64 addr = xsk_umem__add_offset_to_addr(btx2->addr[0]);
								u8 *pkt = xsk_umem__get_data(port_tx->params.bp->addr,
											addr);
								if (pkt != NULL)
								{
									encap_indirection(w, pkt, &port_tx->params, btx2->len[0], btx2->addr[0]);
									btx_collector->addr[btx_index] = btx2->addr[0];
									btx_collector->len[btx_index] = btx2->len[0];
									// printf("Pull packet %d from local queue %d to nic tx \n", btx2->addr[0], w);

									free(btx2);

									btx_index++;
									btx_collector->n_pkts = btx_index;
								} else {
									printf("packet is NULL for indirection \n");
								}
							// } else {
							// 	free(btx2);
							// 	printf("BTX is NULL for indirection \n");
							// }
						}
					}
				} else {
					printf("non_local_dest_queue is NULL \n");
				}
				if (btx_index)
				{
					// printf("There are packets from queue %d to nic tx \n", k);
					port_tx_burst_collector(port_tx, btx_collector, 0, 0);
				} 
				btx_collector->n_pkts = 0;
			}
			
			for (w = 0; w < assigned_perdest_count; w++)
			{
				int btx_index = 0;
				if (local_dest_queue[w] != NULL)
				{
					int buffer_occupancy = 0;
					while ((buffer_occupancy = mpmc_queue_available(local_dest_queue[w])) && (btx_index < MAX_BURST_TX))
					{
						#if DEBUG == 1
							local_buff[local_buff_track] = buffer_occupancy;
							local_q_num[local_buff_track] = w;
							buff_time[local_buff_track] = now;
							local_buff_track++;
						#endif

						void *obj2;
						if (mpmc_queue_pull(local_dest_queue[w], &obj2) != NULL) {
							struct burst_tx *btx2 = (struct burst_tx *)obj2;
							// if (btx2 != NULL)
							// {
								u64 addr = xsk_umem__add_offset_to_addr(btx2->addr[0]);
								u8 *pkt = xsk_umem__get_data(port_tx->params.bp->addr,
											addr);
								if (pkt != NULL)
								{
									// printf("Pull packet %d from local queue %d to nic tx \n", btx2->addr[0], w);
									int new_len = encap_veth(w, pkt, &port_tx->params, btx2->len[0], btx2->addr[0]);
									btx_collector->addr[btx_index] = btx2->addr[0];
									btx_collector->len[btx_index] = new_len;

									free(btx2);

									btx_index++;
									btx_collector->n_pkts = btx_index;
								} else {
									printf("packet is NULL \n");
								}
							// } else {
							// 	free(btx2);
							// 	printf("BTX is NULL \n");
							// }
						}
					}
				} else {
					printf("local_dest_queue is NULL \n");
				}
				if (btx_index)
				{
					// printf("There are packets from queue %d to nic tx \n", k);
					port_tx_burst_collector(port_tx, btx_collector, 0, 0);
				} 
			
				btx_collector->n_pkts = 0;
			}
		}
	}
	printf("return from thread_func_veth_to_nic_tx \n");
	return NULL;
}

// from_NIC -> to_VETH or from_NIC -> to_NIC
static void *
thread_func_nic(void *arg)
{
	struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	struct mpmc_queue *non_local_dest_queue[NUM_OF_PER_DEST_QUEUES];

	int assigned_queue_count = t->assigned_queue_count;

	int w;
	for (w = 0; w < NUM_OF_PER_DEST_QUEUES; w++)
	{
			non_local_dest_queue[w] = t->non_local_dest_queue_array[w];
	}

	struct mpmc_queue *veth_side_queue[13]; 
    
	for (w = 0; w < veth_port_count; w++)
	{
		veth_side_queue[w] = t->veth_side_queue_array[w];
	}

	struct return_process_rx *ret_val = calloc(1, sizeof(struct return_process_rx));

	while (!t->quit)
	{
		int k = 0;
		for (k = 0; k < assigned_queue_count; k++)
		{
			ret_val->new_len = 0;
			ret_val->ring_buf_index = 0;
			struct port *port_rx = t->ports_rx[k];
			struct burst_rx *brx = &t->burst_rx;

			u32 n_pkts, j;

			/* RX. */
			n_pkts = port_rx_burst(port_rx, brx, i);
			

			if (!n_pkts) 
			{
					// nic_rx_no_packet_counter++;
					continue;
			}

			/* Process & TX. */
			for (j = 0; j < n_pkts; j++)
			{
				u64 addr = xsk_umem__add_offset_to_addr(brx->addr[j]);
				u8 *pkt = xsk_umem__get_data(port_rx->params.bp->addr,
											addr);

				//Free packets without processing (for debuggin) 
				// bcache_prod(port_rx->bc, brx->addr[j]);

				// process_rx_packet_old(pkt, &port_rx->params, brx->len[j], brx->addr[j], ret_val);
				get_queue_index_for_nic_rx(pkt, &port_rx->params, brx->len[j], brx->addr[j], ret_val);
				// process_rx_packet_with_filter(pkt, &port_rx->params, brx->len[j], brx->addr[j], ret_val);
				// Needs to send packet back out NIC
		
				struct burst_tx *btx = calloc(1, sizeof(struct burst_tx));
				if (ret_val->new_len == 1)
				{
					btx->addr[0] = brx->addr[j];
					btx->len[0] = brx->len[j];
					btx->n_pkts++;
					
					if (non_local_dest_queue[ret_val->ring_buf_index] != NULL)
					{
						int ret = mpmc_queue_push(non_local_dest_queue[ret_val->ring_buf_index], (void *) btx);
						if (!ret) 
						{
							non_local_dest_queue_overflow_count++;
							// printf("non local_dest_queue is full \n");
							//Release buffers to pool
							bcache_prod(port_rx->bc, brx->addr[j]);
							free(btx);
						}
					}
				} else
				{
					
					btx->addr[0] = brx->addr[j];
					btx->len[0] = ret_val->new_len;
					btx->n_pkts++;

					if (veth_side_queue[ret_val->which_veth] != NULL)
					{
						int ret = mpmc_queue_push(veth_side_queue[ret_val->which_veth], (void *) btx);
						if (!ret) 
						{
							veth_queue_overflow_count++;
							// printf("veth_side_queue is full \n");
							//Release buffers to pool
							bcache_prod(port_rx->bc, brx->addr[j]);
							free(btx);
						}
					}
					else
					{
						printf("TODO: There is no veth_side_queue %d to push the packet \n", ret_val->which_veth);
					}
				}
			}	
		}
	}
	
	free(ret_val);
	printf("return from thread_func_nic \n");
	return NULL;
}

// from_NIC -> to_VETH tx
static void *
thread_func_nic_to_veth_tx(void *arg)
{
	struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	struct burst_tx_collector *btx_collector = &t->burst_tx_collector[0];
	struct mpmc_queue *veth_side_queue = t->veth_side_queue_array[0];


	while (!t->quit) {
		struct port *port_tx = t->ports_tx[0];
		int btx_index = 0;
		btx_collector->n_pkts = 0;

		//++++++++++++++++++++++DRAIN VETH1 SIDE QUEUE++++++++++++++++++++++++
		if (veth_side_queue != NULL)
		{
            // printf("drain veth 0 tx\n");
			// while ((!ringbuf_is_empty(veth_side_queue)) && (btx_index < MAX_BURST_TX))
			int buffer_occupancy = 0;
			while ((buffer_occupancy = mpmc_queue_available(veth_side_queue)) && (btx_index < MAX_BURST_TX))
			{
				#if DEBUG == 1
					veth_buff[veth_buff_track] = buffer_occupancy;
					veth_buff_time[veth_buff_track] = now;
					veth_buff_track++;
				#endif
				// printf("DRAIN VETH 0 SIDE QUEUE \n");
				void *obj;
				if (mpmc_queue_pull(veth_side_queue, &obj) != NULL) {
					struct burst_tx *btx = (struct burst_tx *)obj;

					// if (btx != NULL)
					// {
						btx_collector->addr[btx_index] = btx->addr[0];
						btx_collector->len[btx_index] = btx->len[0];
						free(btx);

						btx_index++;
						btx_collector->n_pkts = btx_index;
					// } else {
					// 	free(btx);
					// }
				}
			}
		} 
		else {
			printf("veth side queue is null \n");
		}
		if (btx_index)
		{
			port_tx_burst_collector(port_tx, btx_collector, 0, 0);
		}
		
	}
	printf("return from thread_func_nic_to_veth_tx \n");
	return NULL;
}


static void *
track_topo_change(void *arg)
{
	struct tread_topo_data *t = arg;
	cpu_set_t cpu_cores;
	u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	int prev_topo = 0;

	while (!t->quit) {
		if (prev_topo != topo)
		{
			printf("Topo change from %d to %d at %ld\n", prev_topo, topo, now);
			prev_topo = topo;
		}
	}

}

