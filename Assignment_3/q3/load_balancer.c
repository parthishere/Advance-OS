// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/resource.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>

#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include "load_balancer.skel.h"


#define BRIDGE "10.0.0.1"
#define BACKEND_ONE "10.0.0.2"
#define BACKEND_TWO "10.0.0.3"
#define LOAD_BALANCER "10.0.0.10"


#define BRIDGE_NAME "br0"
#define PART_MAC "DE:AD:AA:AA:00:"

struct backend_info {
    __u32 ip;
    unsigned char mac[6];
};

static int parse_mac(const char *str, unsigned char *mac) {
    if (sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
        fprintf(stderr, "Invalid MAC address format\n");
        return -1;
    }
    return 0;
}


static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}


// Callback function to handle events from the ring buffer
static int handle_event(void *ctx, void *data, size_t data_sz){
	if (data_sz < 20) {  // Minimum TCP header size
        fprintf(stderr, "Received incomplete TCP header %ld\n", data_sz);
        return 0;
    }


	// Parse the raw TCP header bytes
    struct tcphdr {
        uint16_t source;
        uint16_t dest;
        uint32_t seq;
        uint32_t ack_seq;
        uint16_t res1:4,
                 doff:4,
                 fin:1,
                 syn:1,
                 rst:1,
                 psh:1,
                 ack:1,
                 urg:1,
                 ece:1,
                 cwr:1;
        uint16_t window;
        uint16_t check;
        uint16_t urg_ptr;
        // Options and padding may follow
    } __attribute__((packed));

	if (data_sz < sizeof(struct tcphdr)) {
        fprintf(stderr, "Data size (%zu) less than TCP header size\n", data_sz);
        return 0;
    }

	struct tcphdr *tcp = (struct tcphdr *)data;
	
	// Convert fields from network byte order to host byte order
    uint16_t source_port = ntohs(tcp->source);
    uint16_t dest_port = ntohs(tcp->dest);
    uint32_t seq = ntohl(tcp->seq);
    uint32_t ack_seq = ntohl(tcp->ack_seq);
    uint16_t window = ntohs(tcp->window);

	// Extract flags
    uint8_t flags = 0;
    flags |= (tcp->fin) ? 0x01 : 0x00;
    flags |= (tcp->syn) ? 0x02 : 0x00;
    flags |= (tcp->rst) ? 0x04 : 0x00;
    flags |= (tcp->psh) ? 0x08 : 0x00;
    flags |= (tcp->ack) ? 0x10 : 0x00;
    flags |= (tcp->urg) ? 0x20 : 0x00;
    flags |= (tcp->ece) ? 0x40 : 0x00;
    flags |= (tcp->cwr) ? 0x80 : 0x00;

	printf("Captured TCP Header:\n");
    printf("  Source Port: %u\n", source_port);
    printf("  Destination Port: %u\n", dest_port);
    printf("  Sequence Number: %u\n", seq);
    printf("  Acknowledgment Number: %u\n", ack_seq);
    printf("  Data Offset: %u\n", tcp->doff);
    printf("  Flags: 0x%02x\n", flags);
    printf("  Window Size: %u\n", window);
    printf("\n");

	return 0;
}

int main(int argc, char **argv)
{
	struct load_balancer_bpf *skel;
	struct ring_buffer *rb = NULL;
	int ifindex;
    int err;

    if (argc != 6) {
        fprintf(stderr, "Usage: %s <ifname> <backend1_ip> <backend1_mac> <backend2_ip> <backend2_mac>\n", argv[0]);
        return 1;
    }

    const char *ifname = argv[1];
    struct backend_info backend[2];


    // Parse backend 1
    if (inet_pton(AF_INET, argv[2], &backend[0].ip) != 1) {
        fprintf(stderr, "Invalid backend 1 IP address\n");
        return 1;
    }
    if (parse_mac(argv[3], backend[0].mac) < 0) {
        return 1;
    }


    // Parse backend 2
    if (inet_pton(AF_INET, argv[4], &backend[1].ip) != 1) {
        fprintf(stderr, "Invalid backend 2 IP address\n");
        return 1;
    }
    if (parse_mac(argv[5], backend[1].mac) < 0) {
        return 1;
    }



    ifindex = if_nametoindex(ifname);
    if (ifindex == 0)
    {
        fprintf(stderr, "Invalid interface name %s\n", ifname);
        return 1;
    }

	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);

	/* Open BPF application */
	skel = load_balancer_bpf__open();
	if (!skel)
	{
		fprintf(stderr, "Failed to open BPF skeleton\n");
		return 1;
	}

    
    for (int i=0; i<2; i++){
        if(bpf_map_update_elem(bpf_map__fd(skel->maps.backends), &i, &backend[i], 0) < 0)
            goto cleanup;
    }

    printf("XDP load balancer configured with backends:\n");
    printf("Backend 1 - IP: %s, MAC: %s\n", argv[2], argv[3]);
    printf("Backend 2 - IP: %s, MAC: %s\n", argv[4], argv[5]);

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "sudo bash ./setup.sh %s %s %s %s", BRIDGE, BACKEND_ONE, BACKEND_TWO, LOAD_BALANCER);

    system(buffer);

	/* ensure BPF program only handles write() syscalls from our process */
	// skel->bss->my_pid = getpid();

	/* Load & verify BPF programs */
	err = load_balancer_bpf__load(skel);
	if (err)
	{
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}

	/* Attach tracepoint handler */
	err = load_balancer_bpf__attach(skel);
	if (err)
	{
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		goto cleanup;
	}


	skel->links.handle_xdp = bpf_program__attach_xdp(skel->progs.handle_xdp, ifindex);
	if (!skel->links.handle_xdp)
    {
        err = -errno;
        fprintf(stderr, "Failed to attach XDP program: %s\n", strerror(errno));
        goto cleanup;
    }

	rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), handle_event, NULL, NULL);
	if (!rb)
	{
		fprintf(stderr, "Failed to create ring buffer\n");
		err = -1;
		goto cleanup;
	}

	printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
		   "to see output of the BPF programs.\n");

	for (;;)
	{
		err = ring_buffer__poll(rb, -1);
		if (err == -EINTR)
			continue;
		if (err < 0)
		{
			fprintf(stderr, "Error polling ring buffer: %d\n", err);
			break;
		}
		/* trigger our BPF program */
		// fprintf(stderr, ".");
		// sleep(1);
	}

cleanup:
	ring_buffer__free(rb);
    load_balancer_bpf__detach(skel);
	load_balancer_bpf__destroy(skel);
    system("sudo bash ./teardown.sh");
	return -err;
}
