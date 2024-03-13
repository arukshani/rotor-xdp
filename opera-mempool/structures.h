typedef __u64 u64;
typedef __u32 u32;
typedef __u16 u16;
typedef __u8  u8;

struct ifaddrs *ifaddr, *ifa;
char *nic_iface;
static int n_nic_ports;
static int n_veth_ports;
static int n_ports;
static int n_threads;

//===================Plumbing related================

#define XDP_PROGRAM_COUNT 26
#define STRERR_BUFSIZE          1024

char out_veth_arr[13][10] = {"veth1", "veth3", "vethout23", 
"vethout24", "vethout26", "vethout27", "vethout28", "vethout29",
 "vethout30", "vethout31", "vethout32", "vethout33", "vethout34"};

static struct xdp_program *xdp_prog[XDP_PROGRAM_COUNT];

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


 //================PORT RELATED===================

#ifndef MAX_PORTS
#define MAX_PORTS 24
#endif

struct bpool_params {
	u32 n_buffers;
	u32 buffer_size;
	int mmap_flags;
	u32 n_buffers_per_slab;
};

static const struct bpool_params bpool_params_default = {
	.n_buffers = 8 * 4096,
	.buffer_size = XSK_UMEM__DEFAULT_FRAME_SIZE,
	.mmap_flags = 0,
	.n_buffers_per_slab = XSK_RING_PROD__DEFAULT_NUM_DESCS * 2 
};

static const struct xsk_umem_config umem_cfg_default = {
	.fill_size = XSK_RING_PROD__DEFAULT_NUM_DESCS * 2,
	.comp_size = XSK_RING_CONS__DEFAULT_NUM_DESCS,
	.frame_size = XSK_UMEM__DEFAULT_FRAME_SIZE,
	.frame_headroom = XSK_UMEM__DEFAULT_FRAME_HEADROOM,
	.flags = 0,
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

struct port_params {
	struct xsk_socket_config xsk_cfg;
	const char *iface;
	u32 iface_queue;
	struct bpool *bp;
};

static const struct port_params port_params_default = {
	.xsk_cfg = {
		.rx_size = XSK_RING_CONS__DEFAULT_NUM_DESCS,
		.tx_size = XSK_RING_PROD__DEFAULT_NUM_DESCS,
		.libbpf_flags = XSK_LIBBPF_FLAGS__INHIBIT_PROG_LOAD,
		.xdp_flags = XDP_FLAGS_DRV_MODE,
		.bind_flags = XDP_USE_NEED_WAKEUP,
	},
	.iface = NULL,
	.iface_queue = 0,
	.bp = NULL,
};

struct bcache {
	struct bpool *bp;

	u64 *slab_cons;
	u64 *slab_prod;

	u64 *slab_cons_tx;
	u64 *slab_prod_tx;

	u64 n_buffers_cons;
	u64 n_buffers_prod;

	u64 n_buffers_cons_tx;
	u64 n_buffers_prod_tx;
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

static struct port_params port_params[MAX_PORTS];
static struct bpool_params bpool_params[MAX_PORTS];
static struct xsk_umem_config umem_cfg[MAX_PORTS];
static struct bpool *bp[MAX_PORTS];
static struct port *ports[MAX_PORTS];
static u64 n_pkts_rx[MAX_PORTS];
static u64 n_pkts_tx[MAX_PORTS];

//============================QUEUE RELATED==========================

#ifndef NUM_OF_PER_DEST_QUEUES
#define NUM_OF_PER_DEST_QUEUES 16
#endif

#ifndef MAX_BURST_OBJS
#define MAX_BURST_OBJS 4096
#endif

struct mpmc_queue *veth_side_queue[13];
struct mpmc_queue *local_per_dest_queue[NUM_OF_PER_DEST_QUEUES];
struct mpmc_queue *non_local_per_dest_queue[NUM_OF_PER_DEST_QUEUES];


//============================PACKET RELATED==========================

#ifndef RX_BURST_COUNT
#define RX_BURST_COUNT 20
#endif

#ifndef TX_BURST_COUNT
#define TX_BURST_COUNT 20
#endif

#ifndef WORKER_INFO_CSV
#define WORKER_INFO_CSV "/home/dathapathu/emulator/github_code/all_worker_info.csv"
#endif

struct gre_hdr
{
	__be16 flags;
	__be16 proto;
} __attribute__((packed));

struct ip_set {
	int index;
};

struct mac_addr {
   unsigned char bytes[ETH_ALEN+1];
};

struct burst_tx_collector {
	u64 addr[TX_BURST_COUNT];
	u32 len[TX_BURST_COUNT];
	uint8_t *pkt[TX_BURST_COUNT];
	u32 n_pkts;
};

struct burst_tx {
	u64 addr;
	u32 len;
	uint8_t *pkt;
	u32 n_pkts;
};

struct return_process_rx { 
	int new_len;
	int ring_buf_index;
	int which_veth;
};

mg_Map mac_table; //mac table
mg_Map ip_table; //ip table
unsigned char out_eth_src[ETH_ALEN+1];
uint32_t src_ip;

//============================THREAD RELATED==========================
#ifndef START_THREAD_CORE_ID
#define START_THREAD_CORE_ID 33
#endif

#ifndef MAX_PORTS_PER_THREAD
#define MAX_PORTS_PER_THREAD 1
#endif

#ifndef MAX_THREADS
#define MAX_THREADS 64
#endif

struct thread_data {
	struct port *ports_rx[MAX_PORTS_PER_THREAD];
	struct port *ports_tx[MAX_PORTS_PER_THREAD];
	// u32 n_ports_rx;
	// struct burst_rx burst_rx;
	struct burst_tx_collector burst_tx_collector[MAX_PORTS_PER_THREAD];
	u32 cpu_core_id;
	int quit;
	struct mpmc_queue *local_dest_queue_array[NUM_OF_PER_DEST_QUEUES];
	// struct mpmc_queue *non_local_dest_queue_array[NUM_OF_PER_DEST_QUEUES];
	struct mpmc_queue *veth_side_queue_array[13];
	// int assigned_queue_count;
};

static pthread_t threads[MAX_THREADS];
static struct thread_data thread_data[MAX_THREADS];
static int quit;