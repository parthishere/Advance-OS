// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>


#include "load_balancer_helper.h"

/*
Packet header definitions and byte order
Since an XDP program only receives a pointer to a raw data buffer, it will need to do its own parsing of packet headers. To aid in this, the kernel headers define structs that contain the packet header fields. Parsing packets generally involves a lot of casting of data buffers to the right struct types, as we will see in the assignments below. The header definitions we will be using in this lesson are the following:

Struct	Header file
struct ethhdr	<linux/if_ether.h>
struct ipv6hdr	<linux/ipv6.h>
struct iphdr	<linux/ip.h>
struct icmp6hdr	<linux/icmpv6.h>
struct icmphdr	<linux/icmp.h>
Since the packet data comes straight off the wire, the data fields will be in network byte order. Use the bpf_ntohs() and bpf_htons() functions to convert to and from host byte order, respectively. See the comment at the top of bpf_endian.h for why the bpf_-prefixed versions are needed.

*/

// packet contains ethernet_header + ip header + icmpheader

/*
PT
I am not sure how we docode this depending on packet type, that is defined in ether net header depending on which type is it, it will have diffrent structures give me tree view kind depending on which type, what structure we get in the data


I'll create a tree view of packet types and their corresponding structures based on Ethernet header types. Each level shows what structures you might encounter:


Copy
Ethernet Header (ethhdr)
├── IPv4 (ETH_P_IP, 0x0800)
│   ├── TCP (IPPROTO_TCP, 6)
│   │   ├── struct ethhdr
│   │   ├── struct iphdr
│   │   └── struct tcphdr
│   │
│   ├── UDP (IPPROTO_UDP, 17)
│   │   ├── struct ethhdr
│   │   ├── struct iphdr
│   │   └── struct udphdr
│   │
│   └── ICMP (IPPROTO_ICMP, 1)
│       ├── struct ethhdr
│       ├── struct iphdr
│       └── struct icmphdr
│
├── IPv6 (ETH_P_IPV6, 0x86DD)
│   ├── TCP
│   │   ├── struct ethhdr
│   │   ├── struct ipv6hdr
│   │   └── struct tcphdr
│   │
│   ├── UDP
│   │   ├── struct ethhdr
│   │   ├── struct ipv6hdr
│   │   └── struct udphdr
│   │
│   └── ICMPv6
│       ├── struct ethhdr
│       ├── struct ipv6hdr
│       └── struct icmp6hdr
│
├── ARP (ETH_P_ARP, 0x0806)
│   ├── struct ethhdr
│   └── struct arphdr

*/

#define ETH_P_IP 0x0800 // defined in if_ether.h file

#define ETH_ALEN	6

char LICENSE[] SEC("license") = "Dual BSD/GPL";

// int my_pid = 0;

struct
{
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 1 << 24); // 16MB
} rb SEC(".maps");


struct backend_info {
	__u32 ip;
	unsigned char mac[ETH_ALEN];
};

// Backend IP and MAC address map
struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__uint(max_entries, 2); // two backends
	__type(key, __u32);
	__type(value, struct backend_info);
} backends SEC(".maps");

int client_ip = bpf_htonl(0xa000001);  // 10.0.0.1
int load_balancer_ip = bpf_htonl(0xa00000a); // 10.0.0.10

unsigned char client_mac[ETH_ALEN] = {0xDE, 0xAD, 0xAA, 0xAA, 0x0, 0x1};
unsigned char load_balancer_mac[ETH_ALEN] = {0xDE, 0xAD, 0xAA, 0xAA, 0x0, 0x10};



static __always_inline __u16 iph_csum(struct iphdr *iph)
{

	bpf_printk("before_checksum %02X, iphl %d" ,iph->check, iph->ihl);
	iph->check = 0;


	__u32 count = 5 << 2;
	__u16 * addr = (__u16 *)iph;
	unsigned long sum = 0;
	while (count > 1) {
		sum += *addr;
		addr += 1;
		count -= 2;
	}
	
	//Fold sum to 16 bits: add carrier to result
	if (sum>>16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}
	//one's complement
	sum = ~sum % 0xFFFF;
	


	unsigned long long csum = bpf_csum_diff(0, 0, (unsigned int *)iph, sizeof(struct iphdr), 0);
	for (int i = 0; i < 4; i++)
    {
        if (csum >> 16)
            csum = (csum & 0xffff) + (csum >> 16);
    }
	csum = ~csum & 0xFFFF;
	bpf_printk("after checksum %02X %02X\n\r" ,csum, sum);





    // iph->check = 0;

	// uint16_t sum = ((iph->ihl << 8) | iph->version) + ((iph->tos << 8) | iph->tot_len) + 
	// ((iph->id << 8) + iph->frag_off) + 
	// ((iph->ttl << 8) + iph->protocol) + 
	// ((iph->addrs.saddr << 8) + iph->addrs.daddr);

	// sum = (sum & 0xFFFF) + (sum >> 4);


	return iph->check;
    // unsigned long long csum = bpf_csum_diff(0, 0, (unsigned int *)iph, sizeof(struct iphdr), 0);
    // return csum_fold_helper(csum);
}



static bool is_tcp(struct ethhdr *eth, void *data_end)
{
	if ((void *)(eth + 1) > data_end)
	{
		return false; // we dont have data after ethernet header, we should have iphdr struct
	}

	if (bpf_ntohs(eth->h_proto) != ETH_P_IP)
	{
		return false; // we just need IPV4 Addresses
	}

	/*
	struct iphdr {
	__u8 ihl: 4;
	__u8 version: 4;
	__u8 tos;
	__be16 tot_len;
	__be16 id;
	__be16 frag_off;
	__u8 ttl;
	__u8 protocol;
	__sum16 check;
		union {
			struct {
				__be32 c;
				__be32 daddr;
			};
			struct {
				__be32 saddr;
				__be32 daddr;
			} addrs;
		};
	};
	*/
	struct iphdr *ip = (struct iphdr *)(eth + 1); // next element after eth; basically skipping eth

	if ((void *)(ip + 1) > data_end)
	{
		return false; // there should be tcp header after this // we want to decode TCP see above tree to see different procolcol structure depeneding on what protocol we are decoding
	}

	// netinet/in.h.html
	if (ip->protocol != IPPROTO_TCP)
	{
		return false;
	}

	return true;
}



/*
struct xdp_md {
	__u32 data;
	__u32 data_end;
	__u32 data_meta;
	// Below access go through struct xdp_rxq_info
	__u32 ingress_ifindex;  //rxq->dev->ifindex
	__u32 rx_queue_index;   //rxq->queue_index
};
*/

SEC("xdp")
int handle_xdp(struct xdp_md *ctx)
{
	void *data = (void *)(long)ctx->data;
	void *data_end = (void *)(long)ctx->data_end;
	int pkt_sz = data_end - data;

	bpf_printk("packet size is %d", pkt_sz);

	/*
	// This is an Ethernet frame header.
	struct ethhdr {
			unsigned char        h_dest[ETH_ALEN];        // destination eth addr
			unsigned char        h_source[ETH_ALEN];        // source ether addr
			__be16                h_proto;                // packet type ID field
	} __attribute__((packed));
	*/
	struct ethhdr *eth = data;

	if (!is_tcp(eth, data_end))
	{
		bpf_printk("[ERROR] not TCP");
		return XDP_PASS;
	}
	

	// getting header
	struct iphdr *ip = (struct iphdr *)(eth +1);
	// ip->ihl: Internet Header Length is the length of the internet header in 32 bit words.
	int ip_hdr_len = ip->ihl * 4; // we need to multiply as it is 32bit word, so total size would be (32words) * 4

	if(ip_hdr_len < sizeof(struct iphdr)){
		bpf_printk("[ERROR] ip header len is less than structure");
		return XDP_PASS; // size should be same
	}

	if((void *)ip + ip_hdr_len > data_end){ // comparing the address=
		bpf_printk("[ERROR] ip + header len is greater than total data len");
		return XDP_PASS	; // ip_address (~0x8000) + ip_hdr_len (~16) = 0x8016 should be less than data end (~0x8040)
	}


	//  get the ip
	bpf_printk("Received Source IP: 0x%x", bpf_ntohl(ip->saddr));
    bpf_printk("Received Destination IP: 0x%x", bpf_ntohl(ip->daddr));
    bpf_printk("Received Source MAC: %x:%x:%x:%x:%x:%x", eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    bpf_printk("Received Destination MAC: %x:%x:%x:%x:%x:%x", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);

	
	if (ip->saddr == client_ip)
    {
        bpf_printk("Packet from client");


	}
	else 
	{
		bpf_printk("Packet from backend");
	}

	iph_csum(ip);
	// ip->addrs.daddr; // destination address
	// ip->addrs.saddr; // source address


	/*
	
	struct tcphdr {
		__be16 source;
		__be16 dest;
		__be32 seq;
		__be32 ack_seq;
		__u16 res1: 4;
		__u16 doff: 4;
		__u16 fin: 1;
		__u16 syn: 1;
		__u16 rst: 1;
		__u16 psh: 1;
		__u16 ack: 1;
		__u16 urg: 1;
		__u16 ece: 1;
		__u16 cwr: 1;
		__be16 window;
		__sum16 check;
		__be16 urg_ptr;
	};
	
	*/
	// now we need to parse tcp header
	// skip ip header
	// struct tcphdr * tcp = (struct tcphdr *)(void *)(ip + 1); // one way
	struct tcphdr * tcp = (struct tcphdr *)((void *)ip + ip_hdr_len); // another way

	if ((void *) (tcp + 1) > data_end){
		bpf_printk("[ERROR] There is no packet after TCP");
		return XDP_PASS;
	}

	// Define the number of bytes you want to capture from the TCP header
    // Typically, the TCP header is 20 bytes, but with options, it can be longer
    // Here, we'll capture the first 32 bytes to include possible options
	const int tcp_header_bytes = 32;

	if ((void *)tcp + tcp_header_bytes > data_end){
		bpf_printk("[ERROR] tcp + tcp_header_bytes len is greater than total data len");
		return XDP_PASS;
	}



	void *ringbuf_space = bpf_ringbuf_reserve(&rb, tcp_header_bytes, 0); // reserving 32 byte
	if (!ringbuf_space)
	{
		bpf_printk("[ERROR] Buffer reservation failed");
		return XDP_PASS; // If reservation fails, skip processing
	}


	// Copy the TCP header bytes into the ring buffer
    // Using a loop to ensure compliance with eBPF verifier
	for (int i = 0; i < tcp_header_bytes; i++){
		unsigned char byte = *( ((unsigned char *)tcp) + i );
		((unsigned char *)ringbuf_space)[i] = byte;
	}
	

	bpf_ringbuf_submit(ringbuf_space, 0);

	// Print a debug message (will appear in kernel logs)
    bpf_printk("Captured TCP header (%d bytes)", tcp_header_bytes);

	return XDP_PASS;
}

