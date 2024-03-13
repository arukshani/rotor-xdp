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
	bp->n_slabs_available += 2;
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
		xsk_socket__delete(p->xsk);

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

	pthread_mutex_lock(&bp->lock);
	// if (bp->n_slabs_reserved_available == 0)
	if (bp->n_slabs_available == 0)
	{
		pthread_mutex_unlock(&bp->lock);
		free(bc);
		return NULL;
	}

	// bc->slab_cons = bp->slabs_reserved[bp->n_slabs_reserved_available - 1];
	// bc->slab_prod = bp->slabs_reserved[bp->n_slabs_reserved_available - 2];
	bc->slab_cons = bp->slabs[bp->n_slabs_available - 1];
	bc->slab_prod = bp->slabs[bp->n_slabs_available - 2];
	// bp->n_slabs_reserved_available -= 2;
	bp->n_slabs_available -= 2;
	u64 n_buffers_per_slab = bp->params.n_buffers_per_slab;
    bc->n_buffers_cons = n_buffers_per_slab;
	pthread_mutex_unlock(&bp->lock);

	return bc;
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

static struct bpool *
bpool_init(struct bpool_params *params,
		   struct xsk_umem_config *umem_cfg)
{
	struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
	// u64 n_slabs, n_slabs_reserved, n_buffers, n_buffers_reserved;
	// u64 slabs_size, slabs_reserved_size;
	// u64 buffers_size, buffers_reserved_size;
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
	// n_slabs_reserved = params->n_users_max * 2 * 2;
	n_buffers = n_slabs * params->n_buffers_per_slab;
	// n_buffers_reserved = n_slabs_reserved * params->n_buffers_per_slab;

	slabs_size = n_slabs * sizeof(u64 *);
	// slabs_reserved_size = n_slabs_reserved * sizeof(u64 *);
	buffers_size = n_buffers * sizeof(u64);
	// buffers_reserved_size = n_buffers_reserved * sizeof(u64);

	// total_size = sizeof(struct bpool) +
	// 			 slabs_size + slabs_reserved_size +
	// 			 buffers_size + buffers_reserved_size;
	total_size = sizeof(struct bpool) +
				 slabs_size +
				 buffers_size;

	// printf("n_slabs %lld \n", n_slabs);
	// printf("n_slabs_reserved %lld \n", n_slabs_reserved);
	// printf("n_buffers %lld \n", n_buffers);
	// printf("n_buffers_reserved %lld \n", n_buffers_reserved);
	// printf("slabs_size %lld \n", slabs_size);
	// printf("slabs_reserved_size %lld \n", slabs_reserved_size);
	// printf("buffers_size %lld \n", buffers_size);
	// printf("buffers_reserved_size %lld \n", buffers_reserved_size);
	// printf("total_size %lld \n", total_size);

	/* bpool memory allocation. */
	p = calloc(total_size, sizeof(u8)); // store the address of the bool memory block
	if (!p)
		return NULL;

	// printf("============= \n");
	// printf("bp->slabs p[] %ld \n", sizeof(struct bpool));
	// printf("bp->slabs_reserved p[] %lld \n", sizeof(struct bpool) + slabs_size);
	// printf("bp->buffers  p[] %lld \n", sizeof(struct bpool) + slabs_size + slabs_reserved_size);
	// printf("bp->buffers_reserved  p[] %lld \n", sizeof(struct bpool) + slabs_size + slabs_reserved_size + buffers_size);
	// printf("============= \n");

	/* bpool memory initialization. */
	bp = (struct bpool *)p; // address of bpool
	memcpy(&bp->params, params, sizeof(*params));
	bp->params.n_buffers = n_buffers;

	bp->slabs = (u64 **)&p[sizeof(struct bpool)];
	// bp->slabs_reserved = (u64 **)&p[sizeof(struct bpool) +
	// 								slabs_size];
	// bp->buffers = (u64 *)&p[sizeof(struct bpool) +
	// 						slabs_size + slabs_reserved_size];
	bp->buffers = (u64 *)&p[sizeof(struct bpool) +
							slabs_size];
	// bp->buffers_reserved = (u64 *)&p[sizeof(struct bpool) +
	// 								 slabs_size + slabs_reserved_size + buffers_size];

	bp->n_slabs = n_slabs;
	// bp->n_slabs_reserved = n_slabs_reserved;
	bp->n_buffers = n_buffers;

	for (i = 0; i < n_slabs; i++)
		bp->slabs[i] = &bp->buffers[i * params->n_buffers_per_slab];
	bp->n_slabs_available = n_slabs;

	// for (i = 0; i < n_slabs_reserved; i++)
	// 	bp->slabs_reserved[i] = &bp->buffers_reserved[i *
	// 												  params->n_buffers_per_slab];
	// bp->n_slabs_reserved_available = n_slabs_reserved;

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
	printf("bp->n_buffers %lld \n", bp->n_buffers);
	printf("params->n_buffers_per_slab %d \n", params->n_buffers_per_slab);
	// printf("bp->n_slabs_reserved %lld \n", bp->n_slabs_reserved);

	return bp;
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
	if (!p->bc ||
		(bcache_slab_size(p->bc) < umem_fq_size) ||
		(bcache_cons_check(p->bc, umem_fq_size) < umem_fq_size))
	{
		port_free(p);
		printf("bcache_init failed for the port \n");
		return NULL;
	}

	// printf("hello++++++++ %d \n", params->iface_queue);
	// printf("hello++++++++ %s \n", params->iface);

	// if (IS_ERR_OR_NULL(&p->rxq)) {
	// 	printf("NULL \n");
	// }

	/* xsk socket. */
	status = xsk_socket__create_shared(&p->xsk,
									   params->iface,
									   params->iface_queue,
									   params->bp->umem,
									   &p->rxq,
									   &p->txq,
									   &p->umem_fq,
									   &p->umem_cq,
									   &params->xsk_cfg);

	apply_setsockopt(p->xsk);
	// printf("xsk_socket__create_shared returns %d\n", status) ;

	// status = xsk_socket__create(&p->xsk,
	// 				   params->iface,
	// 				   params->iface_queue,
	// 				   params->bp->umem,
	// 				   &p->rxq,
	// 				   &p->txq,
	// 				   &p->umem_fq,
	// 				   &p->umem_cq,
	// 				   &params->xsk_cfg);
	if (status)
	{
		printf("ERROR in xsk_socket__create_shared \n");
		port_free(p);
		return NULL;
	}

	/* umem fq. */
	xsk_ring_prod__reserve(&p->umem_fq, umem_fq_size, &pos);

	for (i = 0; i < umem_fq_size; i++)
		*xsk_ring_prod__fill_addr(&p->umem_fq, pos + i) =
			bcache_cons(p->bc);

	// printf("b4 %d, \n", p->bc->n_buffers_cons);
	xsk_ring_prod__submit(&p->umem_fq, umem_fq_size);
	// printf("af %d, \n", p->bc->n_buffers_cons);
	p->umem_fq_initialized = 1;

	return p;
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

