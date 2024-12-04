// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "vmlinux.h"
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

# define CLONE_VM      0x00000100 /* Set if VM shared between processes.  */
# define CLONE_FS      0x00000200 /* Set if fs info shared between processes.  */
# define CLONE_FILES   0x00000400 /* Set if open files shared between processes.  */
# define CLONE_SIGHAND 0x00000800 /* Set if signal handlers shared.  */
# define CLONE_PIDFD   0x00001000 /* Set if a pidfd should be placed
				     in parent.  */
# define CLONE_PTRACE  0x00002000 /* Set if tracing continues on the child.  */
# define CLONE_VFORK   0x00004000 /* Set if the parent wants the child to
				     wake it up on mm_release.  */
# define CLONE_PARENT  0x00008000 /* Set if we want to have the same
				     parent as the cloner.  */
# define CLONE_THREAD  0x00010000 /* Set to add to same thread group.  */
# define CLONE_NEWNS   0x00020000 /* Set to create new namespace.  */
# define CLONE_SYSVSEM 0x00040000 /* Set to shared SVID SEM_UNDO semantics.  */
# define CLONE_SETTLS  0x00080000 /* Set TLS info.  */
# define CLONE_PARENT_SETTID 0x00100000 /* Store TID in userlevel buffer
					   before MM copy.  */
# define CLONE_CHILD_CLEARTID 0x00200000 /* Register exit futex and memory
					    location to clear.  */
# define CLONE_DETACHED 0x00400000 /* Create clone detached.  */
# define CLONE_UNTRACED 0x00800000 /* Set if the tracing process can't
				      force CLONE_PTRACE on this clone.  */
# define CLONE_CHILD_SETTID 0x01000000 /* Store TID in userlevel buffer in
					  the child.  */
# define CLONE_NEWCGROUP    0x02000000	/* New cgroup namespace.  */
# define CLONE_NEWUTS	0x04000000	/* New utsname group.  */
# define CLONE_NEWIPC	0x08000000	/* New ipcs.  */
# define CLONE_NEWUSER	0x10000000	/* New user namespace.  */
# define CLONE_NEWPID	0x20000000	/* New pid namespace.  */
# define CLONE_NEWNET	0x40000000	/* New network namespace.  */
# define CLONE_IO	0x80000000	/* Clone I/O context.  */


struct event_proc
{
    char allowed;
    unsigned int uid;
    unsigned int pid;
    unsigned int ppid;
    char path[64];
};

struct task_data
{
	pid_t pid;
	pid_t tgid;
	u32 ns;
	u32 flags;
	char comm[TASK_COMM_LEN];
};

struct
{
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__uint(max_entries, 1024);
	__type(key, u32);
	__type(value, struct task_data);
} data_map SEC(".maps");



int target_pid = 0;
unsigned long long dev;
unsigned long long ino;

SEC("ksyscall/clone")
int BPF_KSYSCALL(probe_clone, unsigned long flags, unsigned long stack, int *parent_tid, int *child_tid, unsigned long tls)
{
	struct task_data data;
	struct pid_namespace *pid_ns;

    // Get command name
	char comm[TASK_COMM_LEN];
	bpf_get_current_comm(&comm, sizeof(comm));


	u64 pid_tgid = bpf_get_current_pid_tgid();
	u32 pid = pid_tgid >> 32;
	u32 tgid = (u32)(pid_tgid & 0xFFFF);

	
    
    struct task_struct *task = (struct task_struct *)bpf_get_current_task();
    bpf_probe_read(&pid_ns, sizeof(pid_ns), &nsproxy->pid_ns_for_children);


	struct bpf_pidns_info ns = {};
	if (bpf_get_ns_current_pid_tgid(pid_ns->ns.dev, pid_ns->ns.inum, &ns, sizeof(ns)))
		return 0;

	
	

	// struct task_struct *t = (struct task_struct *)bpf_get_current_task();
	// u32 upid = t->nsproxy->pid_ns_for_children->last_pid;
	// bpf_printk("pid=%d; upid=%d!\\n", pid, upid);

	data.pid = ns.pid;
	data.tgid = ns.tgid;
	data.flags = flags;
	memcpy(data.comm, comm, sizeof(data.comm));

	bpf_printk("************* Clone Called ***************\n");
	bpf_printk("PID.ns: %d, TGID.ns: %d | PID: %d, TGID: %d | Command name: %s\n", data.pid, data.tgid, pid, tgid, comm);
	bpf_printk("Flags: %lu Parent TID: %d, Child TID: %d Stack: %lu TLS %lu \n", flags, *parent_tid, *child_tid, stack, tls);

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

	bpf_map_update_elem(&data_map, &pid, &data, BPF_ANY);

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
	u64 pid_tgid = bpf_get_current_pid_tgid();
    u32 pid = pid_tgid >> 32;

	struct task_data *data  = (struct task_data *)bpf_map_lookup_elem((void *)&data_map, &pid);

	if(pid != data->pid){
		return 0;
	}

	unsigned long syscall_id = ctx->args[1]; // ID

	struct pt_regs *regs;
	regs = (struct pt_regs *)ctx->args[0];

    
    bpf_printk("Catched function call; PID = : %d.\n", pid);
    bpf_printk("  id: %u\n", syscall_id);

    uint64_t arg3 = 0;
    bpf_probe_read(&arg3, sizeof(uint64_t), &PT_REGS_PARM3(regs));
    bpf_printk("  Arg3: %u \n", arg3);

	return 0;
}



struct net_dev_queue_params{
	unsigned short command_type;
	unsigned char command_flags;
	unsigned char common_preempt_count;
	int command_pid;

	void * skbaddr;
	unsigned int len;
	char name[4];
};

SEC("tracepoint/net/net_dev_queue")
int monitor_ingress(struct net_dev_queue_params *ctx)
{

	u64 pid_tgid = bpf_get_current_pid_tgid();
    u32 pid = pid_tgid >> 32;

	if(pid != target_pid){
		return 0;
	}
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

	u64 pid_tgid = bpf_get_current_pid_tgid();
    u32 pid = pid_tgid >> 32;

	if(pid != target_pid){
		return 0;
	}
	// The program should associate network packets with containers using cgroups or namespaces and expose bandwidth metrics to user space.
	/*
	name: netif_receive_skb
	ID: 1622
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

	return 0;
}
