

static void signal_handler(int sig)
{
	quit = 1;
}

static void *
thread_func_veth_rx(void *arg)
{
    struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	// u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

	struct mpmc_queue *local_dest_queue[NUM_OF_PER_DEST_QUEUES];

	int x = 0;
	for (x = 0; x < NUM_OF_PER_DEST_QUEUES; x++)
	{
		local_dest_queue[x] = t->local_dest_queue_array[x];
	}

    while (!t->quit)
	{
        struct port *port_rx = t->ports_rx[0];
		u32 n_pkts;
		n_pkts = port_rx_burst(port_rx, local_dest_queue);
			
		if (!n_pkts) 
		{
			continue;
		}

    }
    printf("return from thread_func_veth_rx \n");
	return NULL;
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

	struct mpmc_queue *local_dest_queue;
	local_dest_queue = t->local_dest_queue_array[0];

	//TODO: Fix this so that each nic port has its own burst_tx_collector

    while (!t->quit)
	{
        struct port *port_tx = t->ports_tx[0];
		struct burst_tx_collector *btx_collector = &t->burst_tx_collector[0];
		int btx_index = 0;
		if (local_dest_queue != NULL)
		{
			while ((mpmc_queue_available(local_dest_queue)) && (btx_index < TX_BURST_COUNT))
			{
				void *obj;
				if (mpmc_queue_pull(local_dest_queue, &obj) != NULL) {
					struct burst_tx *btx = (struct burst_tx *)obj;
					btx_collector->addr[btx_index] = bcache_cons_tx(port_tx->bc);
					btx_collector->len[btx_index] = btx->len;
					memcpy(btx_collector->pkt[btx_index], btx->pkt, btx->len);
					free(obj);

					btx_index++;
					btx_collector->n_pkts = btx_index;
				}
			}
		} else {
			printf("local_dest_queue is NULL \n");
		}
		if (btx_index)
		{
			port_tx_burst_collector(port_tx, btx_collector, 0, 0);
		} 
	
		btx_collector->n_pkts = 0;
    }
    printf("return from thread_func_nic_tx \n");
	return NULL;
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

    while (!t->quit)
	{
        int x = 0;
    }
    printf("return from thread_func_nic_rx \n");
	return NULL;
}

static void *
thread_func_veth_tx(void *arg)
{
    struct thread_data *t = arg;
	cpu_set_t cpu_cores;
	// u32 i;

	CPU_ZERO(&cpu_cores);
	CPU_SET(t->cpu_core_id, &cpu_cores);
	pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_cores);

    while (!t->quit)
	{
        int x = 0;
    }
    printf("return from thread_func_veth_tx \n");
	return NULL;
}