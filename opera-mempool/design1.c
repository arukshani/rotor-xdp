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

#include "mpmc_queue.h"
#include "memory.h"
#include "map.h"
#include "common_funcs.h"
#include "structures.h"
#include "plumbing.h"
#include "packet_process.h"
#include "thread_functions.h"

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
	for (i = 0; i < n_ports; i++)
		print_port_stats(i, ns_diff);
	print_port_stats_trailer();
}

static void
print_port(u32 port_id)
{
	struct port *port = ports[port_id];

	printf("Port %u: interface = %s, queue = %u\n",
		   port_id, port->params.iface, port->params.iface_queue);
}

int main(int argc, char **argv)
{
    struct in_addr *ifa_inaddr;
	struct in_addr addr;
	int n,i,x,y,z,w,v,g;

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

	char *print_statistics = argv[5];
	int print_stats = atoi(print_statistics);
	printf("Print Statistics : %d \n", print_stats);

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

	//=======================PACKET RELATED=======================================
	getMACAddress(nic_iface, out_eth_src);
	mg_map_init(&mac_table, sizeof(struct mac_addr), 32);
	mg_map_init(&ip_table, sizeof(int), 32);
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
				if (col_index == 8)
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

    n_ports = num_of_nses + n_nic_ports;
    n_veth_ports = num_of_nses;

    for (i = 0; i < n_ports; i++)
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
        port_params[y].iface = nic_iface; 
	    port_params[y].iface_queue = y;
    }

	z = 0;
	int start_index_for_veth_ports = n_nic_ports;
    for (x = start_index_for_veth_ports; x < n_ports; x++)
	{
		port_params[x].iface = out_veth_arr[z]; 
	    port_params[x].iface_queue = 0;
		z = z + 1;
	}

    for (i = 0; i < n_ports; i++)
	{
        printf("==============Initialize buffer pool for port: %d ================= \n", i);
        bp[i] = bpool_init(&bpool_params[i], &umem_cfg[i]);
        port_params[i].bp = bp[i];

        ports[i] = port_init(&port_params[i]);
		if (!ports[i])
		{
			printf("Port %d initialization failed.\n", i);
			return -1;
		}
        print_port(i);
    }


    printf("===========Update  bpf maps============================\n");
    enter_xsks_into_map_for_nic();

	//Enter xsk into map for veth
	for (x = n_nic_ports; x < n_ports; x++)
	{
		enter_xsks_into_map(x);
	}

	printf("===========Queue creation============================\n");
	struct mpmc_queue return_path_veth_queue[13];
	struct mpmc_queue local_dest_queue[NUM_OF_PER_DEST_QUEUES];
	struct mpmc_queue non_local_dest_queue[NUM_OF_PER_DEST_QUEUES];

	// local queues
	for (i = 0; i < NUM_OF_PER_DEST_QUEUES; i++)
	{
		mpmc_queue_init(&local_dest_queue[i], MAX_BURST_OBJS, &memtype_heap);
		local_per_dest_queue[i] = &local_dest_queue[i];
	}
	// non-local queues
	for (i = 0; i < NUM_OF_PER_DEST_QUEUES; i++)
	{
		mpmc_queue_init(&non_local_dest_queue[i], MAX_BURST_OBJS, &memtype_heap);
		non_local_per_dest_queue[i] = &non_local_dest_queue[i];
	}
	// veth side queues
    for (w = 0; w < n_veth_ports; w++)
	{
		mpmc_queue_init(&return_path_veth_queue[w], MAX_BURST_OBJS, &memtype_heap);
		veth_side_queue[w] = &return_path_veth_queue[w];
	}

	printf("===========Thread creation============================\n");
	n_threads = 4; //TODO:based on number of ports
	printf("Total number of RX and TX threads : %d \n", n_threads);
	int thread_core_id = START_THREAD_CORE_ID;
	for (x = 0; x < n_threads; x++)
	{
		thread_data[x].cpu_core_id = thread_core_id; 
		thread_core_id = thread_core_id + 2;
	}

	/* VETH RX Queue Assignment. */
	struct thread_data *t_vrx = &thread_data[0];
	t_vrx->ports_rx[0] = ports[1]; //veth port
	for (v=0; v < NUM_OF_PER_DEST_QUEUES; v++) 
	{
		t_vrx->local_dest_queue_array[v] = local_per_dest_queue[v];
	}

	/* VETH RX Threads. */
	int status;
	status = pthread_create(&threads[0],
							NULL,
							thread_func_veth_rx,
							&thread_data[0]);
	printf("Create VETH RX thread %d: %d \n", 0, thread_data[0].cpu_core_id);

	if (status) {
		printf("Thread %d creation failed.\n", i);
		// return -1;
	}

	/* NIC TX Queue Assignment. */
	struct thread_data *t_ntx = &thread_data[1];
	t_ntx->ports_tx[0] = ports[0]; //NIC port
	t_ntx->local_dest_queue_array[0] = local_per_dest_queue[1]; //only 1 queue is assigned

	/* NIC TX Threads. */
	status = pthread_create(&threads[1],
							NULL,
							thread_func_nic_tx,
							&thread_data[1]);
	printf("Create VETH RX thread %d: %d \n", 1, thread_data[1].cpu_core_id);

	if (status) {
		printf("Thread %d creation failed.\n", i);
		// return -1;
	}

	/* NIC RX Queue Assignment. */
	struct thread_data *t_nrx = &thread_data[2];
	t_nrx->ports_rx[0] = ports[0]; //NIC port
	for (g = 0; g < n_veth_ports; g++)
	{
		t_nrx->veth_side_queue_array[g] = veth_side_queue[g];
	}

	/* NIC RX Threads. */
	// status = pthread_create(&threads[2],
	// 						NULL,
	// 						thread_func_nic_rx,
	// 						&thread_data[2]);
	// printf("Create NIC RX thread %d: %d \n", 2, thread_data[2].cpu_core_id);

	// if (status) {
	// 	printf("Thread %d creation failed.\n", i);
	// 	// return -1;
	// }

	/* VETH TX Threads. */
	// status = pthread_create(&threads[3],
	// 						NULL,
	// 						thread_func_veth_tx,
	// 						&thread_data[3]);
	// printf("Create VETH TX thread %d: %d \n", 3, thread_data[3].cpu_core_id);

	// if (status) {
	// 	printf("Thread %d creation failed.\n", i);
	// 	// return -1;
	// }

	printf("All threads created successfully.\n");


	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);

	// time_t secs = (time_t)running_time; 
	// time_t startTime = time(NULL);

	// while (time(NULL) - startTime < secs)
	// {
	// 	//Do nothing
	// }

	struct timespec time_pps;
	clock_gettime(CLOCK_MONOTONIC, &time_pps);
	u64 ns0 = time_pps.tv_sec * 1000000000UL + time_pps.tv_nsec;
	for ( ; !quit; ) {
		u64 ns1, ns_diff;

		sleep(1);
		clock_gettime(CLOCK_MONOTONIC, &time_pps);
		ns1 = time_pps.tv_sec * 1000000000UL + time_pps.tv_nsec;
		ns_diff = ns1 - ns0;
		ns0 = ns1;

		print_port_stats_all(ns_diff);
	}

	/* Threads completion. */
	printf("Quit.\n");

	for (i = 0; i < n_threads; i++)
		thread_data[i].quit = 1;

	pthread_join(threads[0], NULL);
	// for (i = 0; i < n_threads; i++)
	// 	pthread_join(threads[i], NULL);

    //============================================FREE MEMORY============================================
    freeifaddrs(ifaddr);
	mg_map_cleanup(&ip_table);
	mg_map_cleanup(&mac_table);

	for (i = 0; i < n_ports; i++)
    {
		port_free(ports[i]);
		bpool_free(bp[i]);
    }
		
    printf("===========Remove XDP Programs from NIC and VETHS============\n");
    remove_xdp_program_nic();
	remove_xdp_program_veth();

	for (w = 0; w < n_veth_ports; w++)
	{
		int ret = mpmc_queue_destroy(veth_side_queue[w]);
		if (ret)
			printf("Failed to destroy queue: %d", ret);
	}

	for (w = 0; w < NUM_OF_PER_DEST_QUEUES; w++)
	{
		int ret = mpmc_queue_destroy(local_per_dest_queue[w]);
		if (ret)
			printf("Failed to destroy queue: %d", ret);

		ret = mpmc_queue_destroy(non_local_per_dest_queue[w]);
		if (ret)
			printf("Failed to destroy queue: %d", ret);
	}
}