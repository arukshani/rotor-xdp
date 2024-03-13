static inline void ether_addr_copy_assignment(u8 *dst, const u8 *src)
{
	u16 *a = (u16 *)dst;
	const u16 *b = (const u16 *)src;

	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
}

static void process_rx_packet(void *data, struct port_params *params, uint32_t len, u64 addr, struct return_process_rx *return_val)
{
    int is_nic = strcmp(params->iface, nic_iface);
    if (!is_nic)
    {
        //NIC
    } else 
    {
        //VETH
        struct iphdr *outer_iphdr;
        struct ethhdr *outer_eth_hdr;
        struct iphdr *inner_ip_hdr_tmp = (struct iphdr *)(data + sizeof(struct ethhdr));

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
		struct ip_set *dest_ip_index = mg_map_get(&ip_table, inner_ip_hdr_tmp->daddr);
		struct mac_addr *dest_mac_val = mg_map_get(&mac_table, dest_ip_index->index);
		return_val->ring_buf_index = dest_ip_index->index - 1;

        ether_addr_copy_assignment(outer_eth_hdr->h_dest, dest_mac_val->bytes);

		outer_eth_hdr->h_proto = htons(ETH_P_IP);

		outer_iphdr = (struct iphdr *)(data +
									   sizeof(struct ethhdr));
        __builtin_memcpy(outer_iphdr, inner_ip_hdr_tmp, sizeof(*outer_iphdr));
        outer_iphdr->protocol = IPPROTO_GRE;
		outer_iphdr->tot_len = bpf_htons(olen + bpf_ntohs(inner_ip_hdr_tmp->tot_len));

		struct gre_hdr *gre_hdr;
		gre_hdr = (struct gre_hdr *)(data +
									 sizeof(struct ethhdr) + sizeof(struct iphdr));

		gre_hdr->proto = bpf_htons(ETH_P_TEB);
        if (strcmp(params->iface, "veth1") == 0) {
			gre_hdr->flags = 0;
		} else if (strcmp(params->iface, "veth3") == 0) {
			gre_hdr->flags = 1;
		} else if (strcmp(params->iface, "vethout23") == 0) {
			gre_hdr->flags = 2;
		} else if (strcmp(params->iface, "vethout24") == 0) {
			gre_hdr->flags = 3;
		} else if (strcmp(params->iface, "vethout26") == 0) {
			gre_hdr->flags = 4;
		} else if (strcmp(params->iface, "vethout27") == 0) {
			gre_hdr->flags = 5;
		} else if (strcmp(params->iface, "vethout28") == 0) {
			gre_hdr->flags = 6;
		} else if (strcmp(params->iface, "vethout29") == 0) {
			gre_hdr->flags = 7;
		} else if (strcmp(params->iface, "vethout30") == 0) {
			gre_hdr->flags = 8;
		} else if (strcmp(params->iface, "vethout31") == 0) {
			gre_hdr->flags = 9;
		} else if (strcmp(params->iface, "vethout32") == 0) {
			gre_hdr->flags = 10;
		} else if (strcmp(params->iface, "vethout33") == 0) {
			gre_hdr->flags = 11;
		} else if (strcmp(params->iface, "vethout34") == 0) {
			gre_hdr->flags = 12;
		}
        return_val->new_len = new_len;
                            
    }
}

static inline u32
port_rx_burst(struct port *p, struct mpmc_queue *local_dest_queue[NUM_OF_PER_DEST_QUEUES])
{
    u32 n_pkts, pos, i, queue_index;

    n_pkts = bcache_cons_check(p->bc, RX_BURST_COUNT);

	if (!n_pkts) {
		printf("There are no consumer slabs....\n");
		return 0;
	}

    /* RXQ. */
    struct bpool *bp = p->bc->bp;
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
		return 0;
	}

	struct return_process_rx *ret_val = calloc(1, sizeof(struct return_process_rx));
    for (i = 0; i < n_pkts; i++)
	{
		u64 addr_rx = xsk_ring_cons__rx_desc(&p->rxq, pos + i)->addr;
		u32 len = xsk_ring_cons__rx_desc(&p->rxq, pos + i)->len;
        //process each packet and copy to per dest queue
        u64 addr = xsk_umem__add_offset_to_addr(addr_rx);
		u8 *pkt = xsk_umem__get_data(p->params.bp->addr, addr);
        process_rx_packet(pkt, &p->params,len, addr_rx, ret_val);

		if (local_dest_queue[queue_index] != NULL)
		{
			struct burst_tx *btx = calloc(1, sizeof(struct burst_tx));
			// btx->pkt = pkt;
			memcpy(btx->pkt, pkt, ret_val->new_len);
			btx->len = ret_val->new_len;

			int ret = mpmc_queue_push(local_dest_queue[queue_index], (void *) btx);
			if (!ret) 
			{
				printf("local_dest_queue is full \n");
				//Release buffers to pool
				// bcache_prod(p->bc, addr_rx);
			}
			bcache_prod(p->bc, addr_rx);
		}
		else
		{
			printf("There is no queue to push the packet(queue_indexx): %d \n", queue_index);
		}

	}
	free(ret_val);

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
		}
	}

	for (i = 0; i < n_pkts; i++)
		*xsk_ring_prod__fill_addr(&bp->umem_fq, pos + i) =
			bcache_cons(p->bc);

	xsk_ring_prod__submit(&bp->umem_fq, n_pkts);

	return n_pkts;
}

static inline void
port_tx_burst_collector(struct port *p, struct burst_tx_collector *b, int free_btx, int wait_all)
{
	struct bpool *bp = p->bc->bp;
	u32 n_pkts, pos, i;
	int status;

	/* UMEM CQ. */
	n_pkts = p->params.bp->umem_cfg.comp_size;

	n_pkts = xsk_ring_cons__peek(&bp->umem_cq, n_pkts, &pos);


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
		u8 *pkt = xsk_umem__get_data(p->params.bp->addr, b->addr[i]);
		memcpy(pkt, b->pkt[i], b->len[i]);
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->addr = b->addr[i];
		xsk_ring_prod__tx_desc(&p->txq, pos + i)->len = b->len[i];
	}

	xsk_ring_prod__submit(&p->txq, n_pkts);
	if (xsk_ring_prod__needs_wakeup(&p->txq))
			sendto(xsk_socket__fd(p->xsk), NULL, 0, MSG_DONTWAIT,
			       NULL, 0);

	p->n_pkts_tx += n_pkts;
}
