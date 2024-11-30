// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>


char LICENSE[] SEC("license") = "Dual BSD/GPL";

	// int my_pid = 0;

SEC("kprobe/clone")
int monitor_clone(void *ctx)
{
	
}


SEC("tracepoint/syscalls/sys_enter")
int monitor_sys_enter(void *ctx)
{
	
}

SEC("tracepoint/net/net_dev_queue")
int monitor_ingress(void * ctx){

}
SEC("tracepoint/net/netif_receive_skb")
int monitor_egress(void * ctx){

}
