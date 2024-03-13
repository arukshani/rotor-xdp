static void load_xdp_program(void)
{
	int nic_ifindex = if_nametoindex(nic_iface);
    struct config cfgs[XDP_PROGRAM_COUNT];
    
	int y;
    for (y = 0; y < n_nic_ports; y++)
	{
        // Physical NIC
        struct config nic_cfg = {
            .ifindex = nic_ifindex,
            .ifname = nic_iface,
            .xsk_if_queue = y,
            .xsk_poll_mode = true,
            .filename = "nic_kern.o",
            .progsec = "xdp_sock_1"};

        cfgs[y] = nic_cfg;
    }

    int x, z;
    z = 0;
    int start_index_for_veth_ports = n_nic_ports;
    for (x = start_index_for_veth_ports; x < n_ports; x++)
	{
        int veth_ifindex = if_nametoindex(out_veth_arr[z]);
        struct config veth_cfg = {
		.ifindex = veth_ifindex,
		.ifname = out_veth_arr[z],
		.xsk_if_queue = 0,
		.xsk_poll_mode = true,
		.filename = "veth_kern.o",
		.progsec = "xdp_sock_0"};

        z = z + 1;
        cfgs[x] = veth_cfg;
    }

	//===========================Following is just for NIC[One NIC; Multiple QUEUES]========================
    //bcoz we only need one program to load on NIC even though it has multiple queues
	char errmsg[STRERR_BUFSIZE];
	int err;

	printf("xdp_prog[%d] is %s \n", 0, cfgs[0].filename);

	xdp_prog[0] = xdp_program__open_file(cfgs[0].filename, cfgs[0].progsec, NULL);
	err = libxdp_get_error(xdp_prog[0]);
	if (err)
	{
		libxdp_strerror(err, errmsg, sizeof(errmsg));
		fprintf(stderr, "ERROR: program loading failed for NIC: %s\n", errmsg);
		exit(EXIT_FAILURE);
	}

	err = xdp_program__attach(xdp_prog[0], cfgs[0].ifindex, XDP_FLAGS_DRV_MODE, 0);
	if (err)
	{
		libxdp_strerror(err, errmsg, sizeof(errmsg));
		fprintf(stderr, "ERROR: attaching NIC program failed: %s\n", errmsg);
		exit(EXIT_FAILURE);
	}
	//=====================================================================================================

	int start_index_for_veth_cfgs = n_nic_ports; 
    int i;
	for (i = start_index_for_veth_cfgs; i < n_ports; i++)
	{

		char errmsg[STRERR_BUFSIZE];
		int err;

		printf("xdp_prog[%d] is %s \n", i, cfgs[i].filename);

		xdp_prog[i] = xdp_program__open_file(cfgs[i].filename, cfgs[i].progsec, NULL);
		err = libxdp_get_error(xdp_prog[i]);
		if (err)
		{
			libxdp_strerror(err, errmsg, sizeof(errmsg));
			fprintf(stderr, "ERROR: program loading failed: %s\n", errmsg);
			exit(EXIT_FAILURE);
		}

		err = xdp_program__attach(xdp_prog[i], cfgs[i].ifindex, XDP_FLAGS_DRV_MODE, 0);
		if (err)
		{
			libxdp_strerror(err, errmsg, sizeof(errmsg));
			fprintf(stderr, "ERROR: attaching program failed: %s\n", errmsg);
			exit(EXIT_FAILURE);
		}
	}
}

static void remove_xdp_program_nic(void)
{
	struct xdp_multiprog *mp;
	int err;

	mp = xdp_multiprog__get_from_ifindex(if_nametoindex(nic_iface));
	printf("remove_xdp_program_nic: %s \n", nic_iface);
	if (IS_ERR_OR_NULL(mp))
	{
		printf("No XDP program loaded on %s\n", nic_iface);
	}

	err = xdp_multiprog__detach(mp);
	if (err)
		printf("Unable to detach XDP program for NIC : %s\n", strerror(-err));
}

static void remove_xdp_program_veth(void)
{
	struct xdp_multiprog *mp;
	int i, err;

	for (i = n_nic_ports; i < n_ports; i++)
	{
		mp = xdp_multiprog__get_from_ifindex(if_nametoindex(port_params[i].iface));
		printf("remove_xdp_program_veth: %s \n", port_params[i].iface);
		if (IS_ERR_OR_NULL(mp))
		{
			printf("No XDP program loaded on %s\n", port_params[i].iface);
			continue;
		}

		err = xdp_multiprog__detach(mp);
		if (err)
			printf("Unable to detach XDP program: %s\n", strerror(-err));
	}
}

static int lookup_bpf_map(int prog_fd)
{
	__u32 i, *map_ids, num_maps, prog_len = sizeof(struct bpf_prog_info);
	__u32 map_len = sizeof(struct bpf_map_info);
	struct bpf_prog_info prog_info = {};
	int fd, err, xsks_map_fd = -ENOENT;
	struct bpf_map_info map_info;

	err = bpf_obj_get_info_by_fd(prog_fd, &prog_info, &prog_len);
	if (err)
		return err;

	num_maps = prog_info.nr_map_ids;

	map_ids = calloc(prog_info.nr_map_ids, sizeof(*map_ids));
	if (!map_ids)
		return -ENOMEM;

	memset(&prog_info, 0, prog_len);
	prog_info.nr_map_ids = num_maps;
	prog_info.map_ids = (__u64)(unsigned long)map_ids;

	err = bpf_obj_get_info_by_fd(prog_fd, &prog_info, &prog_len);
	if (err)
	{
		free(map_ids);
		return err;
	}

	for (i = 0; i < prog_info.nr_map_ids; i++)
	{
		fd = bpf_map_get_fd_by_id(map_ids[i]);
		if (fd < 0)
			continue;

		memset(&map_info, 0, map_len);
		err = bpf_obj_get_info_by_fd(fd, &map_info, &map_len);
		if (err)
		{
			close(fd);
			continue;
		}

		if (!strncmp(map_info.name, "xsks_map", sizeof(map_info.name)) &&
			map_info.key_size == 4 && map_info.value_size == 4)
		{
			xsks_map_fd = fd;
			break;
		}

		close(fd);
	}

	free(map_ids);
	return xsks_map_fd;
}

static void enter_xsks_into_map_for_nic()
{
	int xsks_map;

	int first_nic_index = 0;

	xsks_map = lookup_bpf_map(xdp_program__fd(xdp_prog[first_nic_index]));
	if (xsks_map < 0)
	{
		fprintf(stderr, "ERROR: no xsks map found: %s\n",
				strerror(xsks_map));
		exit(EXIT_FAILURE);
	}


	printf("Update bpf map for xdp_prog[%d] %s, \n", first_nic_index, port_params[first_nic_index].iface);

	int y;
    for (y = 0; y < n_nic_ports; y++)
	{
		int fd = xsk_socket__fd(ports[y]->xsk);
		int key, ret;
		key = y;
		ret = bpf_map_update_elem(xsks_map, &key, &fd, 0);
		if (ret)
		{
			fprintf(stderr, "ERROR: bpf_map_update_elem %d %d\n", y, ret);
			exit(EXIT_FAILURE);
		}
	}
}

static void enter_xsks_into_map(u32 index)
{
	int i, xsks_map;

	xsks_map = lookup_bpf_map(xdp_program__fd(xdp_prog[index]));
	if (xsks_map < 0)
	{
		fprintf(stderr, "ERROR: no xsks map found: %s\n",
				strerror(xsks_map));
		exit(EXIT_FAILURE);
	}

	printf("Update bpf map for xdp_prog[%d] %s, \n", index, port_params[index].iface);

	int fd = xsk_socket__fd(ports[index]->xsk);
	int key, ret;
	i = 0;
	key = i;
	ret = bpf_map_update_elem(xsks_map, &key, &fd, 0);
	if (ret)
	{
		fprintf(stderr, "ERROR: bpf_map_update_elem %d %d\n", i, ret);
		exit(EXIT_FAILURE);
	}
}

//==============================================MEMORY POOL================================================

static struct bpool *
bpool_init(struct bpool_params *params,
		   struct xsk_umem_config *umem_cfg)
{
    struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
	u64 n_slabs, n_buffers;
	u64 slabs_size;
	u64 buffers_size;
	u64 total_size, i;
	struct bpool *bp;
	u8 *p;
	int status;

	/* mmap prep. */
	if (setrlimit(RLIMIT_MEMLOCK, &r))
		return NULL;

	/* bpool internals dimensioning. */
	n_slabs = (params->n_buffers + params->n_buffers_per_slab - 1) /
			  params->n_buffers_per_slab;
	n_buffers = n_slabs * params->n_buffers_per_slab;

	slabs_size = n_slabs * sizeof(u64 *);
	buffers_size = n_buffers * sizeof(u64);

	total_size = sizeof(struct bpool) +
				 slabs_size +
				 buffers_size;

	/* bpool memory allocation. */
	p = calloc(total_size, sizeof(u8)); // store the address of the bool memory block
	if (!p)
		return NULL;

	/* bpool memory initialization. */
	bp = (struct bpool *)p; // address of bpool
	memcpy(&bp->params, params, sizeof(*params));
	bp->params.n_buffers = n_buffers;

	bp->slabs = (u64 **)&p[sizeof(struct bpool)];
	bp->buffers = (u64 *)&p[sizeof(struct bpool) +
							slabs_size];
	bp->n_slabs = n_slabs;
	bp->n_buffers = n_buffers;

	for (i = 0; i < n_slabs; i++)
		bp->slabs[i] = &bp->buffers[i * params->n_buffers_per_slab];
	bp->n_slabs_available = n_slabs;

	for (i = 0; i < n_buffers; i++)
		bp->buffers[i] = i * params->buffer_size;

	/* lock. */
	status = pthread_mutex_init(&bp->lock, NULL);
	if (status)
	{
		free(p);
		return NULL;
	}

	/* mmap. */
	bp->addr = mmap(NULL,
					n_buffers * params->buffer_size,
					PROT_READ | PROT_WRITE,
					MAP_PRIVATE | MAP_ANONYMOUS | params->mmap_flags,
					-1,
					0);
	if (bp->addr == MAP_FAILED)
	{
		pthread_mutex_destroy(&bp->lock);
		free(p);
		printf("mem pool mmap failed \n");
		return NULL;
	}

	/* umem. */
	status = xsk_umem__create(&bp->umem,
							  bp->addr,
							  bp->params.n_buffers * bp->params.buffer_size,
							  &bp->umem_fq,
							  &bp->umem_cq,
							  umem_cfg);
	if (status)
	{
		munmap(bp->addr, bp->params.n_buffers * bp->params.buffer_size);
		pthread_mutex_destroy(&bp->lock);
		free(p);
		printf("umem creation failed \n");
		return NULL;
	}
	memcpy(&bp->umem_cfg, umem_cfg, sizeof(*umem_cfg));

	printf("bp->params.n_buffers %d \n", bp->params.n_buffers);
	printf("bp->params.buffer_size %d \n", bp->params.buffer_size);
	printf("total_size umem %d \n", bp->params.n_buffers * bp->params.buffer_size);
	printf("bp->n_slabs %lld \n", bp->n_slabs);
	printf("params->n_buffers_per_slab %d \n", params->n_buffers_per_slab);

	return bp;
}

static void
bpool_free(struct bpool *bp)
{
	if (!bp)
		return;

	xsk_umem__delete(bp->umem);
	munmap(bp->addr, bp->params.n_buffers * bp->params.buffer_size);
	pthread_mutex_destroy(&bp->lock);
	free(bp);
}

static void
bcache_free(struct bcache *bc)
{
	struct bpool *bp;

	if (!bc)
		return;

	/* In order to keep this example simple, the case of freeing any
	 * existing buffers from the cache back to the pool is ignored.
	 */

	bp = bc->bp;
	pthread_mutex_lock(&bp->lock);
	// bp->slabs_reserved[bp->n_slabs_reserved_available] = bc->slab_prod;
	// bp->slabs_reserved[bp->n_slabs_reserved_available + 1] = bc->slab_cons;
	// bp->n_slabs_reserved_available += 2;
    bp->n_slabs_available += 4;
	pthread_mutex_unlock(&bp->lock);

	free(bc);
}

static void
port_free(struct port *p)
{
	if (!p)
		return;

	/* To keep this example simple, the code to free the buffers from the
	 * socket's receive and transmit queues, as well as from the UMEM fill
	 * and completion queues, is not included.
	 */

	if (p->xsk)
	{
		xsk_socket__delete(p->xsk);
	}

	bcache_free(p->bc);

	free(p);
}

/* To work correctly, the implementation requires that the *n_buffers* input
 * argument is never greater than the buffer pool's *n_buffers_per_slab*. This
 * is typically the case, with one exception taking place when large number of
 * buffers are allocated at init time (e.g. for the UMEM fill queue setup).
 */
static inline u32
bcache_cons_check(struct bcache *bc, u32 n_buffers)
{
	struct bpool *bp = bc->bp;
	u64 n_buffers_per_slab = bp->params.n_buffers_per_slab;
	u64 n_buffers_cons = bc->n_buffers_cons;
	u64 n_slabs_available;
	u64 *slab_full;

	/*
	 * Consumer slab is not empty: Use what's available locally. Do not
	 * look for more buffers from the pool when the ask can only be
	 * partially satisfied.
	 */

	if (n_buffers_cons)
	{
		// printf("bc->n_buffers_cons %lld not empty \n", n_buffers_cons);
		return (n_buffers_cons < n_buffers) ? n_buffers_cons : n_buffers;
	}

	/*
	 * Consumer slab is empty: look to trade the current consumer slab
	 * (full) for a full slab from the pool, if any is available.
	 */
	pthread_mutex_lock(&bp->lock);
	n_slabs_available = bp->n_slabs_available;
	// printf("n_buffers_cons %lld \n", n_buffers_cons);
	if (!n_slabs_available)
	{
		pthread_mutex_unlock(&bp->lock);
		printf("there are no consumer slabs \n");
		return 0;
	}

	n_slabs_available--;
	slab_full = bp->slabs[n_slabs_available];
	bp->slabs[n_slabs_available] = bc->slab_cons;
	bp->n_slabs_available = n_slabs_available;
	pthread_mutex_unlock(&bp->lock);

	// printf("after n_slabs_available--  %lld \n", bp->n_slabs_available);

	bc->slab_cons = slab_full;
	bc->n_buffers_cons = n_buffers_per_slab;
	return n_buffers;
}

static inline u64
bcache_cons(struct bcache *bc)
{
	u64 n_buffers_cons = bc->n_buffers_cons - 1;
	u64 buffer;

	buffer = bc->slab_cons[n_buffers_cons];
	bc->n_buffers_cons = n_buffers_cons;
	// printf("bc->n_buffers_cons %lld \n", n_buffers_cons);
	return buffer;
}

static inline u64
bcache_cons_tx(struct bcache *bc)
{
	u64 n_buffers_cons_tx = bc->n_buffers_cons_tx - 1;
	u64 buffer;

	buffer = bc->slab_cons_tx[n_buffers_cons_tx];
	bc->n_buffers_cons_tx = n_buffers_cons_tx;
	// printf("bc->n_buffers_cons %lld \n", n_buffers_cons);
	return buffer;
}

static inline void
bcache_prod(struct bcache *bc, u64 buffer)
{
	struct bpool *bp = bc->bp;
	u64 n_buffers_per_slab = bp->params.n_buffers_per_slab;
	u64 n_buffers_prod = bc->n_buffers_prod;
	u64 n_slabs_available;
	u64 *slab_empty;

	/*
	 * Producer slab is not yet full: store the current buffer to it.
	 */
	if (n_buffers_prod < n_buffers_per_slab)
	{
		// printf("prod slab is NOT full; %ld n_slabs_available %d n_buffers_prod \n", bp->n_slabs_available, n_buffers_prod);
		bc->slab_prod[n_buffers_prod] = buffer;
		bc->n_buffers_prod = n_buffers_prod + 1;
		return;
	}

	// printf("prod slab FULL bp->n_slabs_available %lld \n", bp->n_slabs_available);

	/*
	 * Producer slab is full: trade the cache's current producer slab
	 * (full) for an empty slab from the pool, then store the current
	 * buffer to the new producer slab. As one full slab exists in the
	 * cache, it is guaranteed that there is at least one empty slab
	 * available in the pool.
	 */
	pthread_mutex_lock(&bp->lock);
	n_slabs_available = bp->n_slabs_available;
	slab_empty = bp->slabs[n_slabs_available];
	bp->slabs[n_slabs_available] = bc->slab_prod;
	bp->n_slabs_available = n_slabs_available + 1;
	pthread_mutex_unlock(&bp->lock);

	slab_empty[0] = buffer;
	bc->slab_prod = slab_empty;
	bc->n_buffers_prod = 1;

	// printf("AFTER prod slab FULL bp->n_slabs_available %lld \n", bp->n_slabs_available);
}

static inline void
bcache_prod_tx(struct bcache *bc, u64 buffer)
{
	struct bpool *bp = bc->bp;
	u64 n_buffers_per_slab = bp->params.n_buffers_per_slab;
	u64 n_buffers_prod_tx = bc->n_buffers_prod_tx;
	u64 n_slabs_available;
	u64 *slab_empty;

	/*
	 * Producer slab is not yet full: store the current buffer to it.
	 */
	if (n_buffers_prod_tx < n_buffers_per_slab)
	{
		// printf("prod slab is NOT full; %ld n_slabs_available %d n_buffers_prod \n", bp->n_slabs_available, n_buffers_prod);
		bc->slab_prod_tx[n_buffers_prod_tx] = buffer;
		bc->n_buffers_prod_tx = n_buffers_prod_tx + 1;
		return;
	}

	// printf("prod slab FULL bp->n_slabs_available %lld \n", bp->n_slabs_available);

	/*
	 * Producer slab is full: trade the cache's current producer slab
	 * (full) for an empty slab from the pool, then store the current
	 * buffer to the new producer slab. As one full slab exists in the
	 * cache, it is guaranteed that there is at least one empty slab
	 * available in the pool.
	 */
	pthread_mutex_lock(&bp->lock);
	n_slabs_available = bp->n_slabs_available;
	slab_empty = bp->slabs[n_slabs_available];
	bp->slabs[n_slabs_available] = bc->slab_prod_tx;
	bp->n_slabs_available = n_slabs_available + 1;
	pthread_mutex_unlock(&bp->lock);

	slab_empty[0] = buffer;
	bc->slab_prod_tx = slab_empty;
	bc->n_buffers_prod_tx = 1;

	// printf("AFTER prod slab FULL bp->n_slabs_available %lld \n", bp->n_slabs_available);
}

static u32
bcache_slab_size(struct bcache *bc)
{
	struct bpool *bp = bc->bp;

	return bp->params.n_buffers_per_slab;
}

static struct bcache *
bcache_init(struct bpool *bp)
{
	struct bcache *bc;

	bc = calloc(1, sizeof(struct bcache));
	if (!bc)
		return NULL;

	bc->bp = bp;
	bc->n_buffers_cons = 0;
	bc->n_buffers_prod = 0;
	bc->n_buffers_cons_tx = 0;
	bc->n_buffers_prod_tx = 0;

	pthread_mutex_lock(&bp->lock);
	if (bp->n_slabs_available == 0)
	{
		pthread_mutex_unlock(&bp->lock);
		free(bc);
		return NULL;
	}

	bc->slab_cons = bp->slabs[bp->n_slabs_available - 1];
	bc->slab_prod = bp->slabs[bp->n_slabs_available - 2];
	bc->slab_cons_tx = bp->slabs[bp->n_slabs_available - 3];
	bc->slab_prod_tx = bp->slabs[bp->n_slabs_available - 4];
	bp->n_slabs_available -= 4;
    u64 n_buffers_per_slab = bp->params.n_buffers_per_slab;
    bc->n_buffers_cons = n_buffers_per_slab;
	bc->n_buffers_cons_tx = n_buffers_per_slab;
	pthread_mutex_unlock(&bp->lock);

	return bc;
}

static void apply_setsockopt(struct xsk_socket *xsk)
{
	int sock_opt;
	return;

	// if (!opt_busy_poll)
	// 	return;

	// sock_opt = 1;
	// if (setsockopt(xsk_socket__fd(xsk), SOL_SOCKET, SO_PREFER_BUSY_POLL,
	// 			   (void *)&sock_opt, sizeof(sock_opt)) < 0)
	// 	printf("Error!!!");

	// sock_opt = 20;
	sock_opt = 1;
	if (setsockopt(xsk_socket__fd(xsk), SOL_SOCKET, SO_BUSY_POLL,
				   (void *)&sock_opt, sizeof(sock_opt)) < 0)
		printf("Error!!!");

	// sock_opt = 1;
	// if (setsockopt(xsk_socket__fd(xsk), SOL_SOCKET, SO_BUSY_POLL_BUDGET,
	// 			   (void *)&sock_opt, sizeof(sock_opt)) < 0)
	// 	printf("Error!!!");
}

static struct port *
port_init(struct port_params *params)
{
	struct port *p;
	u32 umem_fq_size, pos = 0;
	int status, i;

	/* Memory allocation and initialization. */
	p = calloc(sizeof(struct port), 1);
	if (!p) {
		printf("Memory allocation failed for the port \n");
		return NULL;
	}
		
	memcpy(&p->params, params, sizeof(p->params));
	umem_fq_size = params->bp->umem_cfg.fill_size;

	/* bcache. */
	p->bc = bcache_init(params->bp);
	// if (!p->bc ||
	// 	(bcache_slab_size(p->bc) < umem_fq_size) ||
	// 	(bcache_cons_check(p->bc, umem_fq_size) < umem_fq_size))
	// {
	// 	port_free(p);
	// 	printf("bcache_init failed for the port \n");
	// 	return NULL;
	// }
    if (!p->bc || (bcache_slab_size(p->bc) < umem_fq_size))
	{
		port_free(p);
		printf("bcache_init failed for the port \n");
		return NULL;
	}

	/* xsk socket. */
	// status = xsk_socket__create_shared(&p->xsk,
	// 								   params->iface,
	// 								   params->iface_queue,
	// 								   params->bp->umem,
	// 								   &p->rxq,
	// 								   &p->txq,
	// 								   &p->umem_fq,
	// 								   &p->umem_cq,
	// 								   &params->xsk_cfg);

	status = xsk_socket__create(&p->xsk,
					   params->iface,
					   params->iface_queue,
					   params->bp->umem,
					   &p->rxq,
					   &p->txq,
					//    &p->umem_fq,
					//    &p->umem_cq,
					   &params->xsk_cfg);

    apply_setsockopt(p->xsk);


	if (status)
	{
		printf("ERROR in xsk_socket__create_shared \n");
		port_free(p);
		return NULL;
	}

    struct bpool *bp = p->bc->bp;

	/* umem fq. */
	xsk_ring_prod__reserve(&bp->umem_fq, umem_fq_size, &pos);

	for (i = 0; i < umem_fq_size; i++)
		*xsk_ring_prod__fill_addr(&bp->umem_fq, pos + i) =
			bcache_cons(p->bc);

	
	xsk_ring_prod__submit(&bp->umem_fq, umem_fq_size);
	
	p->umem_fq_initialized = 1;

	return p;
}