

typedef __u64 u64;
typedef __u32 u32;
typedef __u16 u16;
typedef __u8  u8;

#define DEBUG 1

// #define DEBUG_PAUSE_Q 0

#ifndef CLOCK_INVALID
#define CLOCK_INVALID -1
#endif

#define CLOCKFD 3
#define FD_TO_CLOCKID(fd)	((clockid_t) ((((unsigned int) ~fd) << 3) | CLOCKFD))
#define CLOCKID_TO_FD(clk)	((unsigned int) ~((clk) >> 3))

#define STRERR_BUFSIZE          1024
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#ifndef MAX_BURST_RX
#define MAX_BURST_RX 20
#endif

#ifndef MAX_BURST_TX
#define MAX_BURST_TX 20
#endif

#ifndef MAX_BURST_TX_OBJS
#define MAX_BURST_TX_OBJS 4096
// #define MAX_BURST_TX_OBJS 2048
#endif

#ifndef NUM_OF_PER_DEST_QUEUES
#define NUM_OF_PER_DEST_QUEUES 32
#endif

#ifndef TOTAL_NIC_THREADS
#define TOTAL_NIC_THREADS 8
#endif

#define SCHED_PRI__DEFAULT	0

#ifndef WORKER_INFO_CSV
#define WORKER_INFO_CSV "/tmp/all_worker_info.csv"
#endif

#ifndef OUTER_VETH_PREFIX
#define OUTER_VETH_PREFIX "vethout"
#endif

#ifndef START_THREAD_CORE_ID
#define START_THREAD_CORE_ID 6
#endif

#ifndef MAX_PORTS
#define MAX_PORTS 12
#endif

#ifndef MAX_THREADS
#define MAX_THREADS 64
#endif

#ifndef MAX_PORTS_PER_THREAD
#define MAX_PORTS_PER_THREAD 16
#endif

//=======================Plumbing related===========================
struct config
{
	int ifindex;
	char *ifname;
	bool reuse_maps;
	char pin_dir[512];
	char filename[512];
	char progsec[32];
	char src_mac[18];
	char dest_mac[18];
	int xsk_if_queue;
	bool xsk_poll_mode;
};

static struct xdp_program *xdp_prog[26];

/*
 * Port
 *
 * Each of the forwarding ports sits on top of an AF_XDP socket. In order for
 * packet forwarding to happen with no packet buffer copy, all the sockets need
 * to share the same UMEM area, which is used as the buffer pool memory.
 */
//=======================Mempool related===========================
struct bpool_params {
	u32 n_buffers;
	u32 buffer_size;
	int mmap_flags;

	u32 n_users_max;
	u32 n_buffers_per_slab;
};

struct port_params {
	struct xsk_socket_config xsk_cfg;
	struct bpool *bp;
	const char *iface;
	u32 iface_queue;
	u32 ns_index;
};

struct port {
	struct port_params params;

	struct bcache *bc;

	struct xsk_ring_cons rxq;
	struct xsk_ring_prod txq;
	struct xsk_ring_prod umem_fq;
	struct xsk_ring_cons umem_cq;
	struct xsk_socket *xsk;
	int umem_fq_initialized;

	u64 n_pkts_rx;
	u64 n_pkts_tx;
};

struct bpool {
	struct bpool_params params;
	pthread_mutex_t lock;
	void *addr;

	u64 **slabs;
	// u64 **slabs_reserved;
	u64 *buffers;
	// u64 *buffers_reserved;

	u64 n_slabs;
	// u64 n_slabs_reserved;
	u64 n_buffers;

	u64 n_slabs_available;
	// u64 n_slabs_reserved_available;

	struct xsk_umem_config umem_cfg;
	struct xsk_ring_prod umem_fq;
	struct xsk_ring_cons umem_cq;
	struct xsk_umem *umem;
};

struct bcache {
	struct bpool *bp;

	u64 *slab_cons;
	u64 *slab_prod;

	u64 n_buffers_cons;
	u64 n_buffers_prod;
};

/*
 * Process
 */
static const struct bpool_params bpool_params_default = {
	// .n_buffers = 64 * 1024 * 8,
	.n_buffers = 128 * 1024,
	.buffer_size = XSK_UMEM__DEFAULT_FRAME_SIZE,
	.mmap_flags = 0,

	.n_users_max = 16,
	.n_buffers_per_slab = XSK_RING_PROD__DEFAULT_NUM_DESCS * 2 
};

static const struct xsk_umem_config umem_cfg_default = {
	.fill_size = XSK_RING_PROD__DEFAULT_NUM_DESCS * 2,
	// .fill_size = XSK_RING_PROD__DEFAULT_NUM_DESCS,
	.comp_size = XSK_RING_CONS__DEFAULT_NUM_DESCS,
	.frame_size = XSK_UMEM__DEFAULT_FRAME_SIZE,
	.frame_headroom = XSK_UMEM__DEFAULT_FRAME_HEADROOM,
	.flags = 0,
};

static const struct port_params port_params_default = {
	.xsk_cfg = {
		.rx_size = XSK_RING_CONS__DEFAULT_NUM_DESCS,
		.tx_size = XSK_RING_PROD__DEFAULT_NUM_DESCS,
		.libbpf_flags = XSK_LIBBPF_FLAGS__INHIBIT_PROG_LOAD,
		.xdp_flags = XDP_FLAGS_DRV_MODE,
		// .xdp_flags = XDP_FLAGS_SKB_MODE,
		.bind_flags = XDP_USE_NEED_WAKEUP,
	},

	.bp = NULL,
	.iface = NULL,
	.iface_queue = 0,
	// .ns_index = 0,
};

static struct bpool_params bpool_params;
static struct xsk_umem_config umem_cfg;
static struct bpool *bp;

static struct port_params port_params[MAX_PORTS];
static struct port *ports[MAX_PORTS];


struct ifaddrs *ifaddr, *ifa;
char *nic_iface;
struct HashNode** ip_set;
mg_Map mac_table; //mac table
mg_Map ip_table; //ip table
mg_Map ns_ip_table; //ns ip table
mg_Map dest_queue_table; //destination queue table
mg_Map ns_table;
struct ip_set {
	int index;
};
// struct ns_set {
// 	int index;
// };
char *ptp_clock_name;

struct burst_rx {
	u64 addr[MAX_BURST_RX];
	u32 len[MAX_BURST_RX];
};

struct burst_tx {
	u64 addr[1];
	u32 len[1];
	// u32 ns_index;
	u32 n_pkts;
};

struct burst_tx_collector {
	u64 addr[MAX_BURST_TX];
	u32 len[MAX_BURST_TX];
	u32 n_pkts;
};

static int n_ports;
static int n_nic_ports;
// static int n_veth_ports;
static int veth_port_count;
static int veth_count_per_nic_q;

/*
 * Thread
 *
 * Packet forwarding threads.
 */

struct thread_data {
	struct port *ports_rx[MAX_PORTS_PER_THREAD];
	struct port *ports_tx[MAX_PORTS_PER_THREAD];
	u32 n_ports_rx;
	struct burst_rx burst_rx;
	struct burst_tx_collector burst_tx_collector[MAX_PORTS_PER_THREAD];
	u32 cpu_core_id;
	int quit;
	struct mpmc_queue *local_dest_queue_array[NUM_OF_PER_DEST_QUEUES];
	struct mpmc_queue *non_local_dest_queue_array[NUM_OF_PER_DEST_QUEUES];
	struct mpmc_queue *veth_side_queue_array[13];
	int assigned_queue_count; // this is actually assigned nic port count
	int assigned_perdest_count; // how to map per-dest queues and non-loca dest to NIC queues 
};

struct tread_topo_data {
	u32 cpu_core_id;
	int quit;
};

static pthread_t thread_track_topo_change;
static pthread_t threads[MAX_THREADS];
static struct thread_data thread_data[MAX_THREADS];
static int n_threads;
static struct tread_topo_data tread_topo_data;



struct mpmc_queue *veth_side_queue[13];
struct mpmc_queue *local_per_dest_queue[NUM_OF_PER_DEST_QUEUES];
struct mpmc_queue *non_local_per_dest_queue[NUM_OF_PER_DEST_QUEUES];

__u32 t1ms;

struct return_process_rx { 
	int new_len;
	int ring_buf_index;
	int which_veth;
};

unsigned long total_veth_rx = 0;
unsigned long total_veth_tx = 0;
unsigned long total_nic_rx = 0;
unsigned long total_nic_tx = 0;

unsigned long veth_rx_no_packet_counter = 0;
unsigned long veth_rx_has_packet_counter = 0;
u64 nic_rx_no_packet_counter = 0;
u64 nic_rx_has_packet_counter = 0;

unsigned long veth_tx_no_packet_counter = 0;
unsigned long veth_tx_has_packet_counter = 0;
unsigned long nic_tx_no_packet_counter = 0;
unsigned long nic_tx_has_packet_counter = 0;

static u64 n_pkts_rx[MAX_PORTS];
static u64 n_pkts_tx[MAX_PORTS];
static u64 n_cleanup_tx[MAX_PORTS];

// char out_veth_arr[8][10] = {"crout12", "crout13", "crout14", "crout15", "crout16", "crout17", "crout18", "crout19"};
char out_veth_arr[8][10] = {"vethout2", "vethout3", "vethout4", "vethout5", "vethout6", "vethout7", "vethout8", "vethout9"};

static const struct sched_map {
	const char *name;
	int policy;
} schmap[] = {
	{ "OTHER", SCHED_OTHER },
	{ "FIFO", SCHED_FIFO },
	{ NULL }
};

// struct mac_addr {
//    unsigned char bytes[ETH_ALEN+1];
// };

struct gre_hdr
{
	__be16 flags;
	__be16 proto;
	__be16 hopcount;
} __attribute__((packed));

// Telemetry
//+++++++++++++SEQ++++++++++++++++++++++++
// struct timespec timestamp_arr[30000000];
// uint8_t topo_arr[30000000];
// long time_index = 0;

__u32 seq[30000000];
u16 src_port[30000000];

// uint32_t node_ip[90000000];
// int slot[20000000]; // 0-from_veth, 1-intermediate_node, 2-to_veth
// u16 dst_port[20000000];
// __u32 ack_seq[20000000];
// int is_syn[20000000];
// int is_ack[20000000];
// int is_fin[20000000];
// __u32 ns_packet_len[20000000];
// __u32 tcp_rcv_wnd[20000000];
// int next_node[30000000];
// int hop_count[30000000];
//+++++++++++++END SEQ++++++++++++++++++++++++

// uint8_t topo_prev[20000000];

uint8_t topo_curr[20000000];
struct timespec topo_change_time[20000000];
long topo_track_index = 0;

int local_buff[30000000];
int local_q_num[30000000];
struct timespec buff_time[30000000];
long local_buff_track = 0;

int veth_buff[30000000];
struct timespec veth_buff_time[30000000];
long veth_buff_track = 0;

__u32 t1ms;
struct timespec now;
uint64_t time_into_cycle_ns;
uint8_t topo = 0;

// uint64_t slot_time_ns = 100000;  // 100 us
// uint64_t cycle_time_ns = 3200000; // 3200 us
uint64_t slot_time_ns = 200000;  // 200 us
uint64_t cycle_time_ns = 6400000; // 6400 us
// uint64_t slot_time_ns = 1000000;  // 1 ms
// uint64_t cycle_time_ns = 32000000; // 32 ms

// uint64_t slot_time_ns = 130000;  // 130 us
// uint64_t cycle_time_ns = 4160000; // 4160 us
clockid_t clkid;

uint64_t local_dest_queue_overflow_count;
uint64_t non_local_dest_queue_overflow_count;
uint64_t veth_queue_overflow_count;