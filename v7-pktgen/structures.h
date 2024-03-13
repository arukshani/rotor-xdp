

typedef __u64 u64;
typedef __u32 u32;
typedef __u16 u16;
typedef __u8  u8;

// #define DEBUG 0

#define DEBUG_PAUSE_Q 0

#ifndef CLOCK_INVALID
#define CLOCK_INVALID -1
#endif

#define CLOCKFD 3
#define FD_TO_CLOCKID(fd)	((clockid_t) ((((unsigned int) ~fd) << 3) | CLOCKFD))
#define CLOCKID_TO_FD(clk)	((unsigned int) ~((clk) >> 3))

#define STRERR_BUFSIZE          1024
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#ifndef MAX_BURST_RX
#define MAX_BURST_RX 256
#endif

#ifndef MAX_BURST_TX
#define MAX_BURST_TX 256
#endif

#ifndef MAX_BURST_TX_OBJS
#define MAX_BURST_TX_OBJS 8192
#endif

#ifndef NUM_OF_PER_DEST_QUEUES
#define NUM_OF_PER_DEST_QUEUES 16
#endif

#ifndef TOTAL_NIC_THREADS
#define TOTAL_NIC_THREADS 8
#endif

#define SCHED_PRI__DEFAULT	0

#ifndef WORKER_INFO_CSV
#define WORKER_INFO_CSV "/opt/opera-xdp/opera-setup-leed/configs/cr_worker_info.csv"
#endif

#ifndef OUTER_VETH_PREFIX
#define OUTER_VETH_PREFIX "vethout"
#endif

#ifndef START_THREAD_CORE_ID
#define START_THREAD_CORE_ID 19
#endif

#ifndef MAX_PORTS
#define MAX_PORTS 24
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
struct transit_bpool {
	void *addr;
	u64 *buffers;
	u64 n_buffers;
};

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
	// struct xsk_ring_prod umem_fq;
	// struct xsk_ring_cons umem_cq;
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
	u64 *buffers;

	u64 n_slabs;
	u64 n_buffers;

	u64 n_slabs_available;

	struct xsk_umem_config umem_cfg;
	struct xsk_ring_prod umem_fq;
	struct xsk_ring_cons umem_cq;
	struct xsk_umem *umem;
};

struct bcache {
	struct bpool *bp;

	u64 *slab_cons; //rx
	u64 *slab_prod; //rx

	u64 *slab_cons_tx;
	u64 *slab_prod_tx;

	u64 n_buffers_cons; //rx
	u64 n_buffers_prod; //rx

	u64 n_buffers_cons_tx;
	u64 n_buffers_prod_tx;
};

/*
 * Process
 */
static const struct bpool_params bpool_params_default = {
	// .n_buffers = 64 * 1024,
	.n_buffers = 4096 * 16,
	.buffer_size = XSK_UMEM__DEFAULT_FRAME_SIZE,
	.mmap_flags = 0,

	.n_users_max = 16,
	.n_buffers_per_slab = XSK_RING_PROD__DEFAULT_NUM_DESCS * 2 
};

static const struct xsk_umem_config umem_cfg_default = {
	.fill_size = XSK_RING_PROD__DEFAULT_NUM_DESCS * 2,
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
		// .xdp_flags = XDP_FLAGS_DRV_MODE,
		.xdp_flags = XDP_FLAGS_SKB_MODE,
		.bind_flags = XDP_USE_NEED_WAKEUP,
	},

	.bp = NULL,
	.iface = NULL,
	.iface_queue = 0,
	// .ns_index = 0,
};

// static struct bpool_params bpool_params;
// static struct xsk_umem_config umem_cfg;
// static struct bpool *bp;
static struct bpool *bp[MAX_PORTS];
static struct xsk_umem_config umem_cfg[MAX_PORTS];
static struct bpool_params bpool_params[MAX_PORTS];

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

// struct burst_tx {
// 	u64 addr[1];
// 	u32 len[1];
// 	// u32 ns_index;
// 	u32 n_pkts;
// };

// struct burst_tx_collector {
// 	u64 addr[MAX_BURST_TX];
// 	u32 len[MAX_BURST_TX];
// 	u32 n_pkts;
// };

struct burst_tx_collector {
	u64 addr[256];
	u32 len[256];
	// u8 pkt[MAX_BURST_TX][XSK_UMEM__DEFAULT_FRAME_SIZE];
	u32 n_pkts;
};

struct burst_tx {
	u64 addr;
	u32 len;
	// u8 pkt[XSK_UMEM__DEFAULT_FRAME_SIZE];
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
	int assigned_queue_count;
	struct mpmc_queue *transit_local_dest_queue_array[NUM_OF_PER_DEST_QUEUES];
	struct mpmc_queue *transit_veth_side_queue_array[13];
	u16 src_port_pkt_gen;
	u16 src_port_pkt_gen_2; //two source ports for better rx utilization(rss)
	u16 dst_port_pkt_gen;
	int pkt_index;
	int pkt_index_2;
};

static pthread_t threads[MAX_THREADS];
static struct thread_data thread_data[MAX_THREADS];
static int n_threads;



struct mpmc_queue *veth_side_queue[13];
struct mpmc_queue *local_per_dest_queue[NUM_OF_PER_DEST_QUEUES];
struct mpmc_queue *non_local_per_dest_queue[NUM_OF_PER_DEST_QUEUES];

struct mpmc_queue *transit_veth_side_queue[13];
struct mpmc_queue *transit_local_per_dest_queue[NUM_OF_PER_DEST_QUEUES];
static struct transit_bpool *transit_bp_veth[13];
static struct transit_bpool *transit_bp_local_dest[NUM_OF_PER_DEST_QUEUES];

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

struct mac_addr {
   unsigned char bytes[ETH_ALEN+1];
};

struct gre_hdr
{
	__be16 flags;
	__be16 proto;
} __attribute__((packed));



static u8 pkt_data[MAX_PORTS][XSK_UMEM__DEFAULT_FRAME_SIZE];

#define PKTGEN_MAGIC 0xbe9be955
struct pktgen_hdr {
	__be32 pgh_magic;
	__be32 seq_num;
	__be32 tv_sec;
	__be32 tv_usec;
};

static u32 opt_pkt_fill_pattern = 0x12345678;

#define ETH_FCS_SIZE 4
#define ETH_HDR_SIZE (sizeof(struct ethhdr))
#define PKTGEN_HDR_SIZE 0
#define PKT_HDR_SIZE (ETH_HDR_SIZE + sizeof(struct iphdr) + \
		      sizeof(struct udphdr) + PKTGEN_HDR_SIZE)
#define PKTGEN_HDR_OFFSET (ETH_HDR_SIZE + sizeof(struct iphdr) + \
			   sizeof(struct udphdr))
#define PKTGEN_SIZE_MIN (PKTGEN_HDR_OFFSET + sizeof(struct pktgen_hdr) + \
			 ETH_FCS_SIZE)

#define PKT_SIZE		(3500 - ETH_FCS_SIZE)
#define IP_PKT_SIZE		(PKT_SIZE - ETH_HDR_SIZE)
#define UDP_PKT_SIZE		(IP_PKT_SIZE - sizeof(struct iphdr))
#define UDP_PKT_DATA_SIZE	(UDP_PKT_SIZE - \
				 (sizeof(struct udphdr) + PKTGEN_HDR_SIZE))

static struct ether_addr opt_txdmac = {{ 0x00, 0x21, 0xb2,
					 0x25, 0x1d, 0xf1 }}; //00:21:b2:25:1d:f1
static struct ether_addr opt_txsmac = {{ 0x00, 0x21, 0xb2,
					 0x25, 0x1d, 0xb1 }}; //00:21:b2:25:1d:b1