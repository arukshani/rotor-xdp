// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2022 Intel Corporation. */

#define _GNU_SOURCE
#include <stdbool.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <linux/err.h>
#include <linux/if_link.h>
#include <linux/if_xdp.h>
#include <xdp/libxdp.h>
#include <xdp/xsk.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <bpf/bpf_endian.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/if_link.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/ptp_clock.h>
#include <linux/udp.h>
#include <linux/tcp.h>

#include "common_funcs.h"
#include "map.h"
#include "structures.h"
#include "mpmc_queue.h"
#include "memory.h"
#include "plumbing.h"
#include "mempool.h"
#include "calculate_checksum.h"
#include "pkt_gen.h"
#include "thread_functions.h"

static int quit;

static void signal_handler(int sig)
{
	// printf("signal_handler");
	quit = 1;
}


// Telemetry
uint32_t node_ip[20000];
// char description[20000][100]; //strcpy(description[0], aString);
int slot[20000]; // 0-from_veth, 1-intermediate_node, 2-to_veth
struct timespec timestamp_arr[20000000];
uint8_t topo_arr[20000];
int next_node[20000];
long time_index = 0;

__u32 t1ms;
struct timespec now;
uint64_t time_into_cycle_ns;
// uint8_t topo;
uint64_t slot_time_ns = 1000000;  // 1 ms
uint64_t cycle_time_ns = 2000000; // 2 ms

static void
print_port(u32 port_id)
{
	struct port *port = ports[port_id];

	printf("Port %u: interface = %s, queue = %u\n",
		   port_id, port->params.iface, port->params.iface_queue);
}

static void
print_thread(u32 thread_id)
{
	struct thread_data *t = &thread_data[thread_id];
	u32 i;

	printf("Thread %u (CPU core %u): ",
		   thread_id, t->cpu_core_id);

	for (i = 0; i < t->n_ports_rx; i++)
	{
		struct port *port_rx = t->ports_rx[i];
		struct port *port_tx = t->ports_tx[i];

		printf("(%s, %u) -> (%s, %u), ",
			   port_rx->params.iface,
			   port_rx->params.iface_queue,
			   port_tx->params.iface,
			   port_tx->params.iface_queue);
	}

	printf("\n");
}

static void
print_port_stats_separator(void)
{
	printf("+-%4s-+-%12s-+-%13s-+-%12s-+-%13s-+\n",
	       "----",
	       "------------",
	       "-------------",
	       "------------",
	       "-------------");
}

static void
print_port_stats_header(void)
{
	print_port_stats_separator();
	printf("| %4s | %12s | %13s | %12s | %13s |\n",
	       "Port",
	       "RX packets",
	       "RX rate (pps)",
	       "TX packets",
	       "TX_rate (pps)");
	print_port_stats_separator();
}

static void
print_port_stats_trailer(void)
{
	print_port_stats_separator();
	printf("\n");
}

static void
print_port_stats(int port_id, u64 ns_diff)
{
	struct port *p = ports[port_id];
	double rx_pps, tx_pps;

	rx_pps = (p->n_pkts_rx - n_pkts_rx[port_id]) * 1000000000. / ns_diff;
	tx_pps = (p->n_pkts_tx - n_pkts_tx[port_id]) * 1000000000. / ns_diff;

	printf("| %4d | %12llu | %13.0f | %12llu | %13.0f |\n",
	       port_id,
	       p->n_pkts_rx,
	       rx_pps,
	       p->n_pkts_tx,
	       tx_pps);

	n_pkts_rx[port_id] = p->n_pkts_rx;
	n_pkts_tx[port_id] = p->n_pkts_tx;
}

static void
print_port_stats_all(u64 ns_diff)
{
	int i;

	print_port_stats_header();
	for (i = 0; i < n_nic_ports; i++)
		print_port_stats(i, ns_diff);

	// for (i = n_nic_ports; i < n_ports; i++) //fake port stats
	// 	print_port_stats(i, ns_diff);

	print_port_stats_trailer();
}

static void
print_port_throughput(u64 ns_diff)
{
	int i;
	double rx_bps=0, tx_bps=0, rx_pps=0, tx_pps=0;
	print_port_stats_header();
	for (i = 0; i < n_nic_ports; i++)
	{
		
		double local_rx_pps, local_tx_pps;
		struct port *p = ports[i];
		local_rx_pps = ((p->n_pkts_rx - n_pkts_rx[i]) * 1000000000. / ns_diff);
		local_tx_pps = ((p->n_pkts_tx - n_pkts_tx[i]) * 1000000000. / ns_diff);

		rx_pps += local_rx_pps;
		tx_pps += local_tx_pps;
		// rx_bps = (rx_pps * 8 * 3496) / 1000000000;
		// tx_bps = (tx_pps * 8 * 3496) / 1000000000;
		// printf("port:%d, rx_bps: %13.0fGbps, tx_bps: %13.0fGbps \n", i, rx_pps, tx_pps);

		
		printf("| %4d | %12llu | %13.0f | %12llu | %13.0f |\n",
	       i,
	       p->n_pkts_rx,
	       local_rx_pps,
	       p->n_pkts_tx,
	       local_tx_pps);
		

		n_pkts_rx[i] = p->n_pkts_rx;
		n_pkts_tx[i] = p->n_pkts_tx;

	}
	print_port_stats_trailer();

	printf("All ports:%d, rx_pps: %13.0f, tx_pps: %13.0f \n", i, rx_pps, tx_pps);

	rx_bps = (rx_pps * 8 * 3496) / 1000000000;
	tx_bps = (tx_pps * 8 * 3496) / 1000000000;

	printf("All ports:%d, rx_Gbps: %13.0f, tx_Gbps: %13.0f \n", i, rx_bps, tx_bps);
	rx_pps=0;
	tx_pps=0;
	rx_bps = 0;
	tx_bps = 0;
	
}

// int random(int min, int max){
//    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
// }

int main(int argc, char **argv)
{
	struct in_addr *ifa_inaddr;
	struct in_addr addr;
	int s, n;

	if (argc != 6)
	{
		fprintf(stderr, "Usage: getifaddr <IP>\n");
		return EXIT_FAILURE;
	}

	if (inet_aton(argv[1], &addr) == 0)
	{
		perror("inet_aton");
		return EXIT_FAILURE;
	}
	if (getifaddrs(&ifaddr) == -1)
	{
		perror("getifaddrs");
		return EXIT_FAILURE;
	}

	char *run_time = argv[2];
	int running_time = atoi(run_time);
	printf("Running time : %d \n", running_time);

    char *num_nses = argv[3];
	int num_of_nses = atoi(num_nses);
	printf("Number of Namespaces : %d \n", num_of_nses);

	char *num_nic_qs = argv[4];
	n_nic_ports = atoi(num_nic_qs);
	printf("Number of NIC Queues : %d \n", n_nic_ports);

	char *ns_ip_file = argv[5];
	printf("%s\n", ns_ip_file);

	/* Walk through linked list, maintaining head pointer so we
		can free list later */
	for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
	{
		if (ifa->ifa_addr == NULL)
			continue;

		/* We seek only for IPv4 addresses */
		if (ifa->ifa_addr->sa_family != AF_INET)
			continue;

		ifa_inaddr = &(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr);
		if (memcmp(ifa_inaddr, &addr, sizeof(struct in_addr)) == 0)
		{
			printf("Interface: %s\n", ifa->ifa_name);
			nic_iface = ifa->ifa_name;
		}
	}

	//ns_ip_table
	mg_map_init(&ns_ip_table, sizeof(int), 32);
	FILE *file_ns_ip = fopen(ns_ip_file, "r");
	if (file_ns_ip)
	{
		char buffer[1024], *ptr;
		int dest_index = 1;
		while (fgets(buffer, 1024, file_ns_ip))
		{
			// printf("~~~~~~NODE~~~~~~~~~\n");
			ptr = strtok(buffer, ",");
			int col_index = 1;
			while (ptr != NULL)
			{
				// printf("'%s'\n", ptr);
				if (col_index == 1)
				{
					uint32_t dest = inet_addr(ptr);
					struct ip_set local_ip_index = {.index = dest_index};
					mg_map_add(&ns_ip_table, dest, &local_ip_index);
					struct ip_set *dest_ip_index = mg_map_get(&ip_table, dest);
					// printf("dest_ip_index after %d \n", dest_ip_index->index);
				}
				ptr = strtok(NULL, ",");
				col_index++;
			}
			dest_index++;
		}
		fclose(file_ns_ip);
	}

	// printf(" hello %ld", sizeof(u64 *));
	int i,y,x,z,w;

	n_ports = num_of_nses + n_nic_ports; // + for NIC queues

	/* Parse args. */
	for (i = 0; i < n_nic_ports; i++)
	{
		memcpy(&bpool_params[i], &bpool_params_default,
		   sizeof(struct bpool_params));
        memcpy(&umem_cfg[i], &umem_cfg_default,
		   sizeof(struct xsk_umem_config));
		memcpy(&port_params[i], &port_params_default,
			   sizeof(struct port_params));
	}
		
	load_xdp_program();

    for (y = 0; y < n_nic_ports; y++)
	{
        port_params[y].iface = nic_iface; //"enp65s0f0np0", "ens4" etc...
	    port_params[y].iface_queue = y;
		// port_params[y].ns_index = 0;
    }

	z = 0;
    veth_port_count = num_of_nses;
	int start_index_for_veth_ports = n_nic_ports;
    // for (x = start_index_for_veth_ports; x < n_ports; x++)
	// {
	// 	port_params[x].iface = out_veth_arr[z]; 
	//     port_params[x].iface_queue = 0;
	// 	z = z + 1;
	// }
	
	n_threads = (n_nic_ports * 2) + (veth_port_count * 1);
	// n_threads = TOTAL_NIC_THREADS + (veth_port_count * 2); //8 threads for nic rx and tx (for all 16 nic queues)
	printf("Total number of rx and tx threads : %d \n", n_threads);

	int thread_core_id = START_THREAD_CORE_ID;
	for (x = 0; x < n_threads; x++)
	{
		thread_data[x].cpu_core_id = thread_core_id; 
		thread_core_id = thread_core_id + 1;
		if (thread_core_id == 36)
		{
			thread_core_id = 54;
		}
	}
	

	/* Buffer pool initialization. */
	// bp = bpool_init(&bpool_params, &umem_cfg);
	// if (!bp)
	// {
	// 	printf("Buffer pool initialization failed.\n");
	// 	return -1;
	// }
	// printf("Buffer pool created successfully.\n");
	// printf("================================\n");

	/* Ports initialization. */
	for (i = 0; i < n_nic_ports; i++)
	{
		printf("==============Initialize buffer pool for port: %d ================= \n", i);
        bp[i] = bpool_init(&bpool_params[i], &umem_cfg[i]);
        port_params[i].bp = bp[i];
		// port_params[i].bp = bp;

		ports[i] = port_init(&port_params[i]);
		if (!ports[i])
		{
			printf("Port %d initialization failed.\n", i);
			return -1;
		}
		print_port(i);
	}
	

	enter_xsks_into_map_for_nic();
	i = 0;
	int pkt_fake_ports_end = n_nic_ports + veth_port_count;
	for (x = n_nic_ports; x < pkt_fake_ports_end; x++)
	{
		ports[x] = fake_port_init(bp[i]);
		i++;
	}

	//Enter xsk into map for veth
	// for (x = n_nic_ports; x < n_ports; x++)
	// {
	// 	enter_xsks_into_map(x);
	// }

	printf("All ports created successfully.\n");
	// clkid = get_nic_clock_id();

	//+++++++Source MAC and IP++++++++++++++
	getMACAddress(nic_iface, out_eth_src);
	src_ip = getIpAddress(nic_iface);

	//+++++++++++++++++++++IP and MAC set++++++++++++++++++++++
	mg_map_init(&mac_table, sizeof(struct mac_addr), 32);
	mg_map_init(&ip_table, sizeof(int), 32);
	// FILE *file = fopen("/tmp/all_worker_info.csv", "r");
	FILE *file = fopen(WORKER_INFO_CSV, "r");
	if (file)
	{
		char buffer[1024], *ptr;
		int dest_index = 1;
		while (fgets(buffer, 1024, file))
		{
			// printf("~~~~~~NODE~~~~~~~~~\n");
			ptr = strtok(buffer, ",");
			int col_index = 1;
			while (ptr != NULL)
			{
				// printf("'%s'\n", ptr);
				if (col_index == 6)
				{
					uint32_t dest = inet_addr(ptr);
					struct ip_set local_ip_index = {.index = dest_index};
					mg_map_add(&ip_table, dest, &local_ip_index);
					struct ip_set *dest_ip_index = mg_map_get(&ip_table, dest);
					// printf("dest_ip_index after %d \n", dest_ip_index->index);
				}
				if (col_index == 3)
				{
					// printf("mac addr = %s\n", ptr);
					uint8_t mac_addr[6];
					sscanf(ptr, "%x:%x:%x:%x:%x:%x",
						   &mac_addr[0],
						   &mac_addr[1],
						   &mac_addr[2],
						   &mac_addr[3],
						   &mac_addr[4],
						   &mac_addr[5]) < 6;
					struct mac_addr *dest_mac = calloc(1, sizeof(struct mac_addr));
					__builtin_memcpy(dest_mac->bytes, mac_addr, sizeof(mac_addr));
					mg_map_add(&mac_table, dest_index, dest_mac);
				}
				ptr = strtok(NULL, ",");
				col_index++;
			}
			dest_index++;
		}
		fclose(file);
	}

	// struct mpmc_queue return_path_veth_queue[13];
	struct mpmc_queue local_dest_queue[NUM_OF_PER_DEST_QUEUES];
	struct mpmc_queue non_local_dest_queue[NUM_OF_PER_DEST_QUEUES];

	// struct mpmc_queue trnasit_veth_queue[13];
	// struct mpmc_queue transit_dest_queue[NUM_OF_PER_DEST_QUEUES];
	
	// local queues
	for (i = 0; i < NUM_OF_PER_DEST_QUEUES; i++)
	{
		mpmc_queue_init(&local_dest_queue[i], MAX_BURST_TX_OBJS, &memtype_heap);
		local_per_dest_queue[i] = &local_dest_queue[i];

		// mpmc_queue_init(&transit_dest_queue[i], MAX_BURST_TX_OBJS, &memtype_heap);
		// transit_local_per_dest_queue[i] = &transit_dest_queue[i];

		// transit_bp_local_dest[i] = transit_bpool_init();

		// int j;
		// for (j = 0; j < MAX_BURST_TX_OBJS; j++)
		// {
		// 	u64 transit_addr = get_transit_buffer_addr(transit_bp_local_dest[i]);
			
		// 	int ret = mpmc_queue_push(transit_local_per_dest_queue[i], (void *)transit_addr);
		// 	if (!ret) 
		// 	{
		// 		printf("veth_side_queue is full \n");
		// 	}
		// }
	}
	

	// non-local queues
	for (i = 0; i < NUM_OF_PER_DEST_QUEUES; i++)
	{
		mpmc_queue_init(&non_local_dest_queue[i], MAX_BURST_TX_OBJS, &memtype_heap);
		non_local_per_dest_queue[i] = &non_local_dest_queue[i];
	}

    
    // veth_port_count = n_ports - 1;
	// int w;
    // for (w = 0; w < veth_port_count; w++)
	// {
	// 	mpmc_queue_init(&return_path_veth_queue[w], MAX_BURST_TX_OBJS, &memtype_heap);
	// 	veth_side_queue[w] = &return_path_veth_queue[w];

	// 	mpmc_queue_init(&trnasit_veth_queue[w], MAX_BURST_TX_OBJS, &memtype_heap);
	// 	transit_veth_side_queue[w] = &trnasit_veth_queue[w];

	// 	transit_bp_veth[w] = transit_bpool_init();

	// 	int j;
	// 	for (j = 0; j < MAX_BURST_TX_OBJS; j++)
	// 	{
	// 		int ret = mpmc_queue_push(transit_veth_side_queue[w], (void *)get_transit_buffer_addr(transit_bp_veth[w]));
	// 		if (!ret) 
	// 		{
	// 			printf("veth_side_queue is full \n");
	// 		}
	// 	}
	// }

	/* NIC TX Threads. */
    // int nic_tx_thread_count = TOTAL_NIC_THREADS/2;
	// int total_nic_threads = TOTAL_NIC_THREADS;
    int nic_tx_thread_count = n_nic_ports;
	int total_nic_threads = n_nic_ports * 2;

	int queue_index = 0;
	int assigned_queue_count = n_nic_ports/nic_tx_thread_count;
	for (i = 0; i < nic_tx_thread_count; i++)
	{
		struct thread_data *t = &thread_data[i];
		
        // veth->nic (nic tx: pull from local and non-local)
        int k;
		for (k = 0; k < assigned_queue_count; k++)
		{
			t->ports_tx[k] = ports[queue_index];
			t->local_dest_queue_array[k] = local_per_dest_queue[queue_index];
			// t->transit_local_dest_queue_array[k] = transit_local_per_dest_queue[queue_index];
			queue_index++;
		}
		t->assigned_queue_count = assigned_queue_count;
    
		t->n_ports_rx = 1;
	}

	
    /* NIC RX Threads. */
	// int nic_rx_thread_count = TOTAL_NIC_THREADS/2;
    int nic_rx_thread_count = n_nic_ports;

    int j = 0;
	queue_index = 0;
	assigned_queue_count = n_nic_ports/nic_rx_thread_count;
	for (i = nic_tx_thread_count; i < total_nic_threads; i++)
	{
		struct thread_data *t = &thread_data[i];

		int k = 0;
		for (k = 0; k < assigned_queue_count; k++)
		{
			t->ports_rx[k] = ports[queue_index]; //NIC
			t->non_local_dest_queue_array[k] = non_local_per_dest_queue[queue_index];
			queue_index++;
		}
		
		t->assigned_queue_count = assigned_queue_count;
        
		//This is because any nic port can receive packets. Only requirement is that packets from same 
		// namespaces shouldn't be handled by more than one thread. Since we only run one application
		//inside a namespace this should be ok.
		// int g;
        // for (g = 0; g < veth_port_count; g++)
        // {
        //     t->veth_side_queue_array[g] = veth_side_queue[g];
		// 	t->transit_veth_side_queue_array[g] = transit_veth_side_queue[g];
        // }

        t->n_ports_rx = 1;
    }
	
	//VETH RX THREADS
	int veth_rx_threads_start_index = total_nic_threads;
	int veth_rx_threads_end_point = total_nic_threads + veth_port_count;
	int v = 0, m=0;
	start_index_for_veth_ports  = n_nic_ports;
	// u16 src_port = 4000;
	// u16 dst_port = 5000;
	int lower[11] = {1025, 7000, 14000, 21000, 28000, 35000, 42000, 49000, 50000, 10000, 3333};
	int higher[11] = {7000, 14000, 21000, 30000, 35000, 42000, 49000, 65534, 65534, 20000, 4444};
	for (i = veth_rx_threads_start_index; i < veth_rx_threads_end_point; i++)
	{
		struct thread_data *t = &thread_data[i];
		// veth->nic (veth rx: push to local queues)
		t->ports_rx[0] = ports[start_index_for_veth_ports]; //veth
		start_index_for_veth_ports = start_index_for_veth_ports + 1;

		t->local_dest_queue_array[0] = local_per_dest_queue[v];
		

		// for (v=0; v < NUM_OF_PER_DEST_QUEUES; v++) 
		// {
		// 	t->local_dest_queue_array[v] = local_per_dest_queue[v];
		// 	// t->transit_local_dest_queue_array[v] = transit_local_per_dest_queue[v];
		// }
		t->src_port_pkt_gen = (rand() % (2000 - 1025 + 1)) + 1025;
		t->dst_port_pkt_gen = (rand() % (65534 - 2000 + 1)) + 2000;
		// t->src_port_pkt_gen = (rand() % (higher[v] - lower[v] + 1)) + lower[v];
		// t->src_port_pkt_gen_2 = (rand() % (higher[v] - lower[v] + 1)) + lower[v];
		// t->dst_port_pkt_gen = (rand() % (higher[v] - lower[v] + 1)) + lower[v];
		// t->src_port_pkt_gen = 2333;
		// t->dst_port_pkt_gen = 8888;
		t->pkt_index=m;
		m++;
		// t->pkt_index_2=m;
		// m++;
		
		// src_port = src_port + 10;
		// dst_port = dst_port + 20;
		t->n_ports_rx = 1;
		v++;
	}

	//VETH TX THREADS
	// int veth_tx_threads_start_index = veth_rx_threads_end_point;
	// start_index_for_veth_ports  = n_nic_ports;
	// int m = 0;
	// //veth tx threads
	// for (i = veth_tx_threads_start_index; i < n_threads; i++)
	// {
	// 	struct thread_data *t = &thread_data[i];

	// 	t->ports_tx[0] = ports[start_index_for_veth_ports]; //veth
	// 	start_index_for_veth_ports = start_index_for_veth_ports + 1;
	// 	t->veth_side_queue_array[0] = veth_side_queue[m];
	// 	t->transit_veth_side_queue_array[0] = transit_veth_side_queue[m];
	// 	m = m + 1;
	// 	t->n_ports_rx = 1;
	// }

	struct sched_param schparam;
	static int opt_schprio = SCHED_PRI__DEFAULT;

	memset(&schparam, 0, sizeof(schparam));
	schparam.sched_priority = opt_schprio;
	static int opt_schpolicy = SCHED_OTHER;
	
	int ret = sched_setscheduler(0, opt_schpolicy, &schparam);
	if (ret) {
		fprintf(stderr, "Error(%d) in setting priority(%d): %s\n",
			errno, opt_schprio, strerror(errno));
		// goto out;
	}

	//START NIC TX THREADS
	for (i = 0; i < nic_tx_thread_count; i++)
	{
		int status;
		status = pthread_create(&threads[i],
									NULL,
									thread_func_nic_tx,
									&thread_data[i]);
		printf("Create NIC TX thread %d: %d \n", i, thread_data[i].cpu_core_id);
	}

	int nix_rx_end = n_nic_ports * 2;

	//START NIC RX THREADS
	for (i = nic_tx_thread_count; i < total_nic_threads; i++)
	{
		int status;
		status = pthread_create(&threads[i],
									NULL,
									thread_func_nic_rx,
									&thread_data[i]);
		printf("Create NIC RX thread %d: %d \n", i, thread_data[i].cpu_core_id);
	}

	if (running_time == 2) //run the packet gen
	{
		//START VETH RX THREADS
		for (i = veth_rx_threads_start_index; i < veth_rx_threads_end_point; i++)
		{
			int status;
			status = pthread_create(&threads[i],
									NULL,
									thread_func_veth_rx,
									&thread_data[i]);
			printf("Create VETH RX thread %d: %d \n", i, thread_data[i].cpu_core_id);

		}
	}

	//START VETH TX THREADS
	// for (i = veth_tx_threads_start_index; i < n_threads; i++)
	// {
	// 	int status;
	// 	status = pthread_create(&threads[i],
	// 							NULL,
	// 							thread_func_veth_tx,
	// 							&thread_data[i]);
	// 	printf("Create VETH TX thread %d: %d \n", i, thread_data[i].cpu_core_id);
	// }

	printf("All threads created successfully.\n");

	/* Print statistics. */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);

	// read_time();

	// time_t secs = (time_t)running_time; // 10 minutes (can be retrieved from user's input)
	time_t startTime = time(NULL);

	struct timespec time_pps;
	u64 ns0;
	clock_gettime(CLOCK_MONOTONIC, &time_pps);
	ns0 = time_pps.tv_sec * 1000000000UL + time_pps.tv_nsec;
	// while (time(NULL) - startTime < secs)
	for ( ; !quit; ) 
	{
		// read_time();
		u64 ns1, ns_diff;
		sleep(1);
		clock_gettime(CLOCK_MONOTONIC, &time_pps);
		ns1 = time_pps.tv_sec * 1000000000UL + time_pps.tv_nsec;
		ns_diff = ns1 - ns0;
		ns0 = ns1;

		// print_port_stats_all(ns_diff);
		print_port_throughput(ns_diff);
	}

	/* Threads completion. */
	printf("Quit.\n");
	// printf("=========TIMING======================= \n");
	// printf("total_veth_rx %ld s \n", (total_veth_rx / 1000000000));
	// printf("total_nic_tx %ld s \n", (total_nic_tx / 1000000000));
	// printf("total_nic_rx %ld s \n", (total_nic_rx / 1000000000));
	// printf("total_veth_tx %ld s \n", (total_veth_tx / 1000000000));
	// printf("=========END_OF_TIMING================ \n");

	// printf("veth_rx_no_packet_counter %ld \n", veth_rx_no_packet_counter);
	// printf("veth_rx_has_packet_counter %ld \n", veth_rx_has_packet_counter);
	// printf("nic_tx_no_packet_counter %ld \n", nic_tx_no_packet_counter);
	// printf("nic_tx_has_packet_counter %ld \n", nic_tx_has_packet_counter);
	// printf("nic_rx_no_packet_counter %ld \n", nic_rx_no_packet_counter);
	// printf("nic_rx_has_packet_counter %ld \n", nic_rx_has_packet_counter);

	/* output each array element's value */

	// printf("time_index: %ld \n", time_index);

	// #if DEBUG == 1
	// 	printf("debug");
	// 	int z;
	// 	FILE *fpt;
	// 	fpt = fopen("/tmp/opera_data.csv", "w+");
	// 	fprintf(fpt,"node_ip,slot,topo_arr,next_node,time_ns,time_part_sec,time_part_nsec\n");
	// 	for (z = 0; z < time_index; z++ ) {
	// 		unsigned long now_ns = get_nsec(&timestamp_arr[z]);
	// 		fprintf(fpt,"%d,%d,%d,%d,%ld,%ld,%ld\n",node_ip[z],slot[z],topo_arr[z],next_node[z],now_ns,timestamp_arr[z].tv_sec,timestamp_arr[z].tv_nsec);
	// 	}
	// 	fclose(fpt);
	// #endif

#if DEBUG_PAUSE_Q == 1
	int z;
	FILE *fpt;
	fpt = fopen("/tmp/opera_data.csv", "w+");
	fprintf(fpt, "time_ns,time_part_sec,time_part_nsec\n");
	for (z = 0; z < time_index; z++)
	{
		unsigned long now_ns = get_nsec(&timestamp_arr[z]);
		fprintf(fpt, "%ld,%ld,%ld\n", now_ns, timestamp_arr[z].tv_sec, timestamp_arr[z].tv_nsec);
	}
	fclose(fpt);
#endif

	for (i = 0; i < n_threads; i++)
	{
		thread_data[i].quit = 1;
		printf("Quit thread %d \n", i);
	}

	for (i = 0; i < n_threads; i++)
		pthread_join(threads[i], NULL);

	printf("After thread join \n");

	for (i = 0; i < n_nic_ports; i++)
	{
		port_free(ports[i]);
		// bpool_free(bp[i]);
	}

	int veth_port_end = n_nic_ports + veth_port_count;
	for (i = n_nic_ports; i < veth_port_end; i++)
	{
		fake_port_free(ports[i]);
	}

	for (i = 0; i < n_nic_ports; i++)
	{
		bpool_free(bp[i]);
	}
		

	// bpool_free(bp);

	remove_xdp_program_nic();
	// remove_xdp_program_veth();

	// deleteRouteMatrix(route_table);
	freeifaddrs(ifaddr);
	mg_map_cleanup(&ip_table);
	mg_map_cleanup(&mac_table);
	
	
    // for (w = 0; w < veth_port_count; w++)
	// {
	// 	int ret = mpmc_queue_destroy(veth_side_queue[w]);
	// 	if (ret)
	// 		printf("Failed to destroy queue: %d", ret);

	// 	ret = mpmc_queue_destroy(transit_veth_side_queue[w]);
	// 	if (ret)
	// 		printf("Failed to destroy queue: %d", ret);

	// 	transit_bpool_free(transit_bp_veth[w]);
	// }

	for (w = 0; w < NUM_OF_PER_DEST_QUEUES; w++)
	{
		int ret = mpmc_queue_destroy(local_per_dest_queue[w]);
		if (ret)
			printf("Failed to destroy queue: %d", ret);

		// ret = mpmc_queue_destroy(transit_local_per_dest_queue[w]);
		// if (ret)
		// 	printf("Failed to destroy queue: %d", ret);

		ret = mpmc_queue_destroy(non_local_per_dest_queue[w]);
		if (ret)
			printf("Failed to destroy queue: %d", ret);

		// transit_bpool_free(transit_bp_local_dest[w]);
	}

	return 0;
}