/*
 * Fold a partial checksum
 * This function code has been taken from
 * Linux kernel include/asm-generic/checksum.h
 */
static inline __sum16 csum_fold(__wsum csum)
{
	u32 sum = (u32)csum;

	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	return (__sum16)~sum;
}

/*
 * This function code has been taken from
 * Linux kernel lib/checksum.c
 */
static inline u32 from64to32(u64 x)
{
	/* add up 32-bit and 32-bit for 32+c bit */
	x = (x & 0xffffffff) + (x >> 32);
	/* add up carry.. */
	x = (x & 0xffffffff) + (x >> 32);
	return (u32)x;
}

__wsum csum_tcpudp_nofold(__be32 saddr, __be32 daddr,
			  __u32 len, __u8 proto, __wsum sum);

/*
 * This function code has been taken from
 * Linux kernel lib/checksum.c
 */
__wsum csum_tcpudp_nofold(__be32 saddr, __be32 daddr,
			  __u32 len, __u8 proto, __wsum sum)
{
	unsigned long long s = (u32)sum;

	s += (u32)saddr;
	s += (u32)daddr;
#ifdef __BIG_ENDIAN__
	s += proto + len;
#else
	s += (proto + len) << 8;
#endif
	return (__wsum)from64to32(s);
}

/*
 * This function has been taken from
 * Linux kernel include/asm-generic/checksum.h
 */
static inline __sum16
csum_tcpudp_magic(__be32 saddr, __be32 daddr, __u32 len,
		  __u8 proto, __wsum sum)
{
	return csum_fold(csum_tcpudp_nofold(saddr, daddr, len, proto, sum));
}

static inline u16 udp_csum(u32 saddr, u32 daddr, u32 len,
			   u8 proto, u16 *udp_pkt)
{
	u32 csum = 0;
	u32 cnt = 0;

	/* udp hdr and data */
	for (; cnt < len; cnt += 2)
		csum += udp_pkt[cnt >> 1];

	return csum_tcpudp_magic(saddr, daddr, len, proto, csum);
}

static void *memset32_htonl(void *dest, u32 val, u32 size)
{
	u32 *ptr = (u32 *)dest;
	int i;

	val = htonl(val);

	for (i = 0; i < (size & (~0x3)); i += 4)
		ptr[i >> 2] = val;

	for (; i < size; i++)
		((char *)dest)[i] = ((char *)&val)[i & 3];

	return dest;
}

static void gen_eth_hdr_data(u16 src_port, u16 dst_port, int pkt_index)
{
    struct pktgen_hdr *pktgen_hdr;
	struct udphdr *udp_hdr;
	struct iphdr *ip_hdr;

    struct ethhdr *eth_hdr = (struct ethhdr *)pkt_data[pkt_index];

    udp_hdr = (struct udphdr *)(pkt_data[pkt_index] +
					    sizeof(struct ethhdr) +
					    sizeof(struct iphdr));
    ip_hdr = (struct iphdr *)(pkt_data[pkt_index] +
                    sizeof(struct ethhdr));
    pktgen_hdr = (struct pktgen_hdr *)(pkt_data[pkt_index] +
                        sizeof(struct ethhdr) +
                        sizeof(struct iphdr) +
                        sizeof(struct udphdr));

	// printf("HELLO  1++++++++++++++++++\n");
    
    /* ethernet header */
    memcpy(eth_hdr->h_dest, &opt_txdmac, ETH_ALEN);
    memcpy(eth_hdr->h_source, &opt_txsmac, ETH_ALEN);
    eth_hdr->h_proto = htons(ETH_P_IP);

	// printf("HELLO  2++++++++++++++++++\n");

    /* IP header */
	ip_hdr->version = IPVERSION;
	ip_hdr->ihl = 0x5; /* 20 byte header */
	ip_hdr->tos = 0x0;
	ip_hdr->tot_len = htons(IP_PKT_SIZE);
	ip_hdr->id = 0;
	ip_hdr->frag_off = 0;
	ip_hdr->ttl = IPDEFTTL;
	ip_hdr->protocol = IPPROTO_UDP;
	ip_hdr->saddr = htonl(0x0a140101); //192.168.1.1
	ip_hdr->daddr = htonl(0x0a140201); //192.168.2.1

    ip_hdr->check = 0;
	// ip_hdr->check = ip_fast_csum((const void *)ip_hdr, ip_hdr->ihl);
    compute_ip_checksum(ip_hdr);

	// printf("HELLO  3++++++++++++++++++\n");

    /* UDP header */
	// udp_hdr->source = htons(0x1000);
	// udp_hdr->dest = htons(0x1000);
	udp_hdr->source = htons(src_port);
	udp_hdr->dest = htons(dst_port);
	udp_hdr->len = htons(UDP_PKT_SIZE);

    /* UDP data */
	memset32_htonl(pkt_data[pkt_index] + PKT_HDR_SIZE, opt_pkt_fill_pattern,
		       UDP_PKT_DATA_SIZE);

	/* UDP header checksum */
	udp_hdr->check = 0;
	udp_hdr->check = udp_csum(ip_hdr->saddr, ip_hdr->daddr, UDP_PKT_SIZE,
				  IPPROTO_UDP, (u16 *)udp_hdr);
	// printf("HELLO  4++++++++++++++++++\n");
	// udp_hdr->check = udp_csum(ip_hdr->saddr, ip_hdr->daddr, UDP_PKT_SIZE,
	// 			  IPPROTO_UDP, (u16 *)udp_hdr);
    // unsigned short *ipPayload = (pkt_data +
	// 				    sizeof(struct ethhdr) +
	// 				    sizeof(struct iphdr));
	// compute_udp_checksum(ip_hdr, ipPayload);
}