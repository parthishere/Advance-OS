// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "vmlinux.h"
#include <bpf/bpf_core_read.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

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
	pid_t parent_pid;
	pid_t cloned_child_pid;
	pid_t parent_tgid;
	pid_t cloned_child_tgid;
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

static int parent_pid = 0;
static int target_pid = 0;
unsigned long long dev;
unsigned long long ino;

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

SEC("ksyscall/clone")
int BPF_KSYSCALL(probe_clone, unsigned long flags, unsigned long stack, int *parent_tid, int *child_tid, unsigned long tls)
{

	struct task_data child_data;
	struct pid_namespace *pid_ns;

	u64 pid_tgid = bpf_get_current_pid_tgid();
	u32 pid = pid_tgid >> 32;
	u32 tgid = (u32)(pid_tgid & 0xFFFF);

	// Get command name
	char comm[TASK_COMM_LEN];
	bpf_get_current_comm(&comm, sizeof(comm));
	if (strcmp(comm, "capsule") != 0)
		return 0;

	parent_pid = pid;
	// child_data.tgid = tgid;
	// child_data.flags = flags;

	bpf_printk("************* Clone Called ***************\n");
	bpf_printk("PID: %d, TGID: %d parent_pid %d | Command name: %s\n", pid, tgid, parent_pid, comm);
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
	pid_t pid;

	pid = bpf_get_current_pid_tgid() >> 32;

	if (parent_pid == pid)
	{
		bpf_printk("KPROBE EXIT: pid = %d, ret = %ld\n", pid, ret);
		target_pid = ret;
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
	struct task_data child_data;
	u64 pid_tgid = bpf_get_current_pid_tgid();
	u32 pid = pid_tgid >> 32;

	// struct task_data *data  = (struct task_data *)bpf_map_lookup_elem((void *)&data_map, &pid);
	struct task_struct *task = (struct task_struct *)bpf_get_current_task();

	// Variables to store task info
	u32 parent_pid = 0;
	u32 current_pid = 0;
	char comm[TASK_COMM_LEN] = {};

	// Read parent PID safely using BPF helper
	// struct task_struct *parent;
	// bpf_probe_read(&parent, sizeof(parent), &task->parent);
	// if (parent)
	// {
	// 	bpf_probe_read(&parent_pid, sizeof(parent_pid), &parent->pid);
	// }

	// Read current task's PID
	bpf_probe_read(&current_pid, sizeof(current_pid), &task->pid);

	// Read task name (comm)
	bpf_probe_read(&comm, sizeof(comm), task->comm);


	parent_pid = BPF_CORE_READ(task, real_parent, tgid);
	if (target_pid != pid && parent_pid != target_pid)
		return 0;

	unsigned long syscall_id = ctx->args[1]; // ID

	struct pt_regs *regs;
	regs = (struct pt_regs *)ctx->args[0];

	uint64_t arg3 = 0;
	bpf_probe_read(&arg3, sizeof(uint64_t), &PT_REGS_PARM3(regs));
	// bpf_printk("Catched function call: PID = : %d | SYSCALL_NR: %u | ARG3: %u \n", pid, syscall_id, arg3);
	/*
	struct task_struct {
	// Thread information if configured in task
	struct thread_info thread_info;

	// Current state of the task
	unsigned int __state;

	// Saved state for spinlock sleepers
	unsigned int saved_state;

	// Pointer to task's kernel stack
	void *stack;

	// Reference count for task usage
	refcount_t usage;

	// Per task flags (PF_*)
	unsigned int flags;

	// Ptrace flags
	unsigned int ptrace;

	// Task's priority
	int prio;

	// Static priority of task
	int static_prio;

	// Normal priority
	int normal_prio;

	// RT priority
	unsigned int rt_priority;

	// Current process ID
	pid_t pid;

	// Thread group ID (process ID)
	pid_t tgid;

	// Real parent process pointer
	struct task_struct __rcu *real_parent;

	// Parent process pointer (recipient of SIGCHLD)
	struct task_struct __rcu *parent;

	// List of children processes
	struct list_head children;

	// List of siblings
	struct list_head sibling;

	// Leader of process group
	struct task_struct *group_leader;

	// Exit code for the task
	int exit_code;

	// Signal to be sent on exit
	int exit_signal;

	// Name of the task (executable name)
	char comm[TASK_COMM_LEN];

	// File system info
	struct fs_struct *fs;

	// Open file information
	struct files_struct *files;

	// Namespace info
	struct nsproxy *nsproxy;

	// Signal handlers
	struct signal_struct *signal;

	// Shared signal handlers
	struct sighand_struct __rcu *sighand;

	// Blocked signals
	sigset_t blocked;

	// Process credentials
	const struct cred __rcu *cred;
	// Memory management structure
	struct mm_struct *mm;
	// Active memory management structure
	struct mm_struct *active_mm;
	// VM state
	struct reclaim_state *reclaim_state;
	// CPU-specific state of task
	struct thread_struct thread;
};
	*/

	// Get current task
	
	
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
int monitor_ingress(struct net_dev_queue_params *ctx)
{

	u64 pid_tgid = bpf_get_current_pid_tgid();
	u32 pid = pid_tgid >> 32;

	if (pid != target_pid)
	{
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

	if (pid != target_pid)
	{
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
