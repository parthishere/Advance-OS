// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "vmlinux.h"
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_endian.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

#define ETH_P_IP 0x0800 // defined in if_ether.h file

#define CLONE_VM 0x00000100				/* Set if VM shared between processes.  */
#define CLONE_FS 0x00000200				/* Set if fs info shared between processes.  */
#define CLONE_FILES 0x00000400			/* Set if open files shared between processes.  */
#define CLONE_SIGHAND 0x00000800		/* Set if signal handlers shared.  */
#define CLONE_PIDFD 0x00001000			/* Set if a pidfd should be placed \
						   in parent.  */
#define CLONE_PTRACE 0x00002000			/* Set if tracing continues on the child.  */
#define CLONE_VFORK 0x00004000			/* Set if the parent wants the child to \
						   wake it up on mm_release.  */
#define CLONE_PARENT 0x00008000			/* Set if we want to have the same \
						   parent as the cloner.  */
#define CLONE_THREAD 0x00010000			/* Set to add to same thread group.  */
#define CLONE_NEWNS 0x00020000			/* Set to create new namespace.  */
#define CLONE_SYSVSEM 0x00040000		/* Set to shared SVID SEM_UNDO semantics.  */
#define CLONE_SETTLS 0x00080000			/* Set TLS info.  */
#define CLONE_PARENT_SETTID 0x00100000	/* Store TID in userlevel buffer \
					   before MM copy.  */
#define CLONE_CHILD_CLEARTID 0x00200000 /* Register exit futex and memory \
					   location to clear.  */
#define CLONE_DETACHED 0x00400000		/* Create clone detached.  */
#define CLONE_UNTRACED 0x00800000		/* Set if the tracing process can't \
						   force CLONE_PTRACE on this clone.  */
#define CLONE_CHILD_SETTID 0x01000000	/* Store TID in userlevel buffer in \
					   the child.  */
#define CLONE_NEWCGROUP 0x02000000		/* New cgroup namespace.  */
#define CLONE_NEWUTS 0x04000000			/* New utsname group.  */
#define CLONE_NEWIPC 0x08000000			/* New ipcs.  */
#define CLONE_NEWUSER 0x10000000		/* New user namespace.  */
#define CLONE_NEWPID 0x20000000			/* New pid namespace.  */
#define CLONE_NEWNET 0x40000000			/* New network namespace.  */
#define CLONE_IO 0x80000000				/* Clone I/O context.  */

struct task_data
{
	pid_t cloned_child_parent_pid;
	pid_t cloned_child_parent_tgid;
	pid_t cloned_child_task_pid;
	pid_t cloned_child_task_tgid;
	u32 syscall_number;
	u32 flags;
	unsigned int ppid;
	char comm[TASK_COMM_LEN];
};

struct
{
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__uint(max_entries, 1024);
	__type(key, u32);
	__type(value, struct task_data);
} data_map SEC(".maps");

struct
{
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 1024);
} rb SEC(".maps");


int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1)
	{
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

static struct task_data child_data = {0};

SEC("ksyscall/clone")
int BPF_KSYSCALL(probe_clone, unsigned long flags, unsigned long stack, int *parent_tid, int *child_tid, unsigned long tls)
{

	// struct pid_namespace *pid_ns;

	u64 pid_tgid = bpf_get_current_pid_tgid();
	u32 pid = pid_tgid >> 32;
	u32 tgid = (u32)(pid_tgid & 0xFFFF);

	// Get command name
	bpf_get_current_comm(child_data.comm, sizeof(child_data.comm));

	if (strcmp(child_data.comm, "capsule") != 0)
		return 0;

	child_data.cloned_child_parent_pid = pid;
	child_data.cloned_child_parent_tgid = tgid;
	child_data.flags = flags;


	bpf_printk("************* Clone Called ***************\n");
	bpf_printk("cloned_child_parent: PID: %d, TGID: %d | Command name: %s\n", child_data.cloned_child_parent_pid, child_data.cloned_child_parent_tgid, child_data.comm);
	bpf_printk("Flags: %lu, Stack: %lu TLS %lu \n", flags, stack, tls);

	// // Print clone flags
	if (flags & CLONE_NEWPID)
		bpf_printk("Creating new PID namespace\n");
	if (flags & CLONE_NEWNET)
		bpf_printk("Creating new network namespace\n");
	if (flags & CLONE_NEWNS)
		bpf_printk("Creating new mount namespace\n");
	if (flags & CLONE_NEWUTS)
		bpf_printk("Creating new UTS namespace\n");
	if (flags & CLONE_NEWIPC)
		bpf_printk("Creating new IPC namespace\n");

	return 0;
}

SEC("kretprobe/__x64_sys_clone")
int BPF_KRETPROBE(clone_exit, long ret)
{
	u64 pid_tgid = bpf_get_current_pid_tgid();
	u32 pid = pid_tgid >> 32;


	if (child_data.cloned_child_parent_pid == pid)
	{
		child_data.cloned_child_task_pid = ret;
		bpf_printk("KPROBE EXIT: pid = %ld, ret = %ld\n", child_data.cloned_child_parent_pid, child_data.cloned_child_task_pid);
	}
	return 0;
}

// sudo cat /sys/kernel/debug/tracing/events/raw_syscalls/sys_enter/format
// name: sys_enter
// ID: 22
// format:
//  field:unsigned short common_type;   offset:0;   size:2; signed:0;
//  field:unsigned char common_flags;   offset:2;   size:1; signed:0;
//  field:unsigned char common_preempt_count;   offset:3;   size:1;signed:0;
//  field:int common_pid;   offset:4;   size:4; signed:1;

//  field:long id;  offset:8;   size:8; signed:1;
//  field:unsigned long args[6];    offset:16;  size:48;    signed:0;

// struct sys_enter_args
// {
//     uint16_t common_type;
//     uint8_t common_flags;
//     uint8_t common_preempt_count;
//     int32_t common_pid;
//     int64_t id;
//     uint64_t args[6];   // Je 4 Bytes
// };

// struct bpf_raw_tracepoint_args {
//     __u64 args[0];
// };

/*

#ifdef CONFIG_HAVE_SYSCALL_TRACEPOINTS

TRACE_EVENT_FN(sys_enter,

	TP_PROTO(struct pt_regs *regs, long id),

	TP_ARGS(regs, id),

	TP_STRUCT__entry(
		__field(	long,		id		)
		__array(	unsigned long,	args,	6	)
	),

	TP_fast_assign(
		__entry->id	= id;
		syscall_get_arguments(current, regs, __entry->args);
	),

	TP_printk("NR %ld (%lx, %lx, %lx, %lx, %lx, %lx)",
		  __entry->id,
		  __entry->args[0], __entry->args[1], __entry->args[2],
		  __entry->args[3], __entry->args[4], __entry->args[5]),

	syscall_regfunc, syscall_unregfunc
);

*/
SEC("raw_tracepoint/sys_enter")
int raw_tracepoint__sys_enter(struct bpf_raw_tracepoint_args *ctx)
{
	// // struct task_data *data  = (struct task_data *)bpf_map_lookup_elem((void *)&data_map, &pid);
	struct task_struct *task = (struct task_struct *)bpf_get_current_task();

	// Variables to store task info
	pid_t parent_pid = 0;
	pid_t current_pid = 0;

	// Read parent PID safely using BPF helper
	struct task_struct *parent;
	bpf_probe_read(&parent, sizeof(parent), &task->parent);
	if (parent)
	{
		bpf_probe_read(&parent_pid, sizeof(parent_pid), &parent->pid);
	}else{
		bpf_printk("parent id not found ! \n\r");
	}

	// Read current task's PID
	bpf_probe_read(&current_pid, sizeof(current_pid), &task->pid);

	// Read task name (comm)
	char comm[TASK_COMM_LEN];
	bpf_probe_read(comm, sizeof(comm), task->comm);

	if (child_data.cloned_child_task_pid != parent_pid && current_pid != child_data.cloned_child_task_pid)
		return 0;

	
	unsigned long syscall_id = ctx->args[1]; // ID

	struct pt_regs *regs;
	regs = (struct pt_regs *)ctx->args[0];

	uint64_t arg3 = 0;
	bpf_probe_read(&arg3, sizeof(uint64_t), &PT_REGS_PARM3(regs));

	bpf_printk("Syscall: PID=%d, PPID=%d, comm=%s, syscall=%u, arg3=%u\n",
			   current_pid, parent_pid, comm, syscall_id, arg3);

   
	return 0;
}

struct net_dev_queue_params
{
	unsigned short command_type;
	unsigned char command_flags;
	unsigned char common_preempt_count;
	int command_pid;

	void *skbaddr;
	unsigned int len;
	char name[4];
};



SEC("tracepoint/net/net_dev_queue")
int monitor_ingress(struct trace_event_raw_net_dev_template *ctx)
{

	// struct task_data *data  = (struct task_data *)bpf_map_lookup_elem((void *)&data_map, &pid);
	struct task_struct *task = (struct task_struct *)bpf_get_current_task();

	// Variables to store task info
	pid_t parent_pid = 0;
	pid_t current_pid = 0;

	// Read parent PID safely using BPF helper
	struct task_struct *parent;
	bpf_probe_read(&parent, sizeof(parent), &task->parent);
	if (parent)
	{
		bpf_probe_read(&parent_pid, sizeof(parent_pid), &parent->pid);
	}else{
		bpf_printk("parent id not found ! \n\r");
	}

	// Read current task's PID
	bpf_probe_read(&current_pid, sizeof(current_pid), &task->pid);

	// Read task name (comm)
	char comm[TASK_COMM_LEN];
	bpf_probe_read(comm, sizeof(comm), task->comm);

	if (child_data.cloned_child_task_pid != parent_pid && current_pid != child_data.cloned_child_task_pid)
		return 0;


	task->nsproxy->net_ns->ns.inum;

	struct sk_buff skb;
    bpf_probe_read(&skb, sizeof(skb), ctx->skbaddr);

	struct iphdr iph;
    bpf_core_read(&iph, sizeof(iph), skb.head + skb.network_header);



    if (skb.protocol != bpf_htons(ETH_P_IP)) {
        // This is not an IP packet
        bpf_printk("Not an IP packet 0x%xn", skb.protocol);
        return -1;
    }

    bpf_printk(" %pI4 -> %pI4 ", iph.saddr, iph.daddr);

	/*
	name: net_dev_queue
	ID: 1623
	format:
		field:unsigned short common_type;	offset:0;	size:2;	signed:0;
		field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
		field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
		field:int common_pid;	offset:4;	size:4;	signed:1;

		field:void * skbaddr;	offset:8;	size:8;	signed:0;
		field:unsigned int len;	offset:16;	size:4;	signed:0;
		field:__data_loc char[] name;	offset:20;	size:4;	signed:0;

	print fmt: "dev=%s skbaddr=%p len=%u", __get_str(name), REC->skbaddr, REC->len

	*/

	// The program should associate network packets with containers using cgroups or namespaces and expose bandwidth metrics to user space.
	return 0;
}

SEC("tracepoint/net/netif_receive_skb")
int monitor_egress(struct net_dev_queue_params *ctx)
{

	// struct task_data *data  = (struct task_data *)bpf_map_lookup_elem((void *)&data_map, &pid);
	struct task_struct *task = (struct task_struct *)bpf_get_current_task();

	// Variables to store task info
	pid_t parent_pid = 0;
	pid_t current_pid = 0;

	// Read parent PID safely using BPF helper
	struct task_struct *parent;
	bpf_probe_read(&parent, sizeof(parent), &task->parent);
	if (parent)
	{
		bpf_probe_read(&parent_pid, sizeof(parent_pid), &parent->pid);
	}else{
		bpf_printk("parent id not found ! \n\r");
	}

	// Read current task's PID
	bpf_probe_read(&current_pid, sizeof(current_pid), &task->pid);

	// Read task name (comm)
	char comm[TASK_COMM_LEN];
	bpf_probe_read(comm, sizeof(comm), task->comm);
	
	if (child_data.cloned_child_task_pid != parent_pid && current_pid != child_data.cloned_child_task_pid)
		return 0;



	task->nsproxy->net_ns->ns.inum;

	struct sk_buff skb;
    bpf_probe_read(&skb, sizeof(skb), ctx->skbaddr);

	struct iphdr iph;
    bpf_core_read(&iph, sizeof(iph), skb.head + skb.network_header);



    if (skb.protocol != bpf_htons(ETH_P_IP)) {
        // This is not an IP packet
        bpf_printk("Not an IP packet 0x%xn", skb.protocol);
        return -1;
    }

    bpf_printk(" %pI4 -> %pI4 ", iph.saddr, iph.daddr);

	/*
	name: net_dev_queue
	ID: 1623
	format:
		field:unsigned short common_type;	offset:0;	size:2;	signed:0;
		field:unsigned char common_flags;	offset:2;	size:1;	signed:0;
		field:unsigned char common_preempt_count;	offset:3;	size:1;	signed:0;
		field:int common_pid;	offset:4;	size:4;	signed:1;

		field:void * skbaddr;	offset:8;	size:8;	signed:0;
		field:unsigned int len;	offset:16;	size:4;	signed:0;
		field:__data_loc char[] name;	offset:20;	size:4;	signed:0;

	print fmt: "dev=%s skbaddr=%p len=%u", __get_str(name), REC->skbaddr, REC->len

	*/

	// The program should associate network packets with containers using cgroups or namespaces and expose bandwidth metrics to user space.
	return 0;
}
