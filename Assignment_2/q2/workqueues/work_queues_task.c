/*********************************************************************
 * Advanced OS Assignment 2
 * File: work_queues_task.c
 * 
 * Purpose: 
 *     Implements a kernel module for periodic system call table monitoring
 *     using work queues. This module schedules periodic checks for 
 *     potential rootkit infections by verifying system call integrity.
 *     It demonstrates kernel work queue usage, kprobes, and system call
 *     table access.
 * 
 * Features:
 *     - Work queue implementation for periodic tasks
 *     - Kprobe usage for symbol resolution
 *     - System call table verification
 *     - Proper cleanup and resource management
 * 
 * Author: Parth Thakkar
 * Date: 8/11/24
 * 
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *********************************************************************/
 
/* Required header files for kernel functionality */
#include <linux/kernel.h>     /* Core kernel functions */
#include <linux/module.h>     /* Module specific functionality */
#include <linux/proc_fs.h>    /* Procfs interface support */
#include <linux/workqueue.h>  /* Work queue implementation */
#include <linux/sched.h>      /* Scheduler functions */
#include <linux/init.h>       /* Module initialization */
#include <linux/interrupt.h>  /* Interrupt handling */
#include <linux/kprobes.h>    /* Kernel probe functionality */


/* Module metadata information */
MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("Rootkit");
MODULE_LICENSE("GPL");

/*
As create_proc_entry function is deprecated, The newer functions are named proc_*. You can see their declarations in include/linux/proc_fs.h.

In particular, proc_create creates a proc entry. You can check out the implementation of the other (quite useful) functions in the source file at fs/proc/generic.c. You may be particularly interested in proc_mkdir and proc_create_data.
*/

/* Global variables and work queue structures */
static int die = 0;  /* Control flag for work queue operation */

/* Function prototypes */
static void mykmod_work_handler(struct work_struct *w);

// struct workqueue_struct {
// 	struct list_head	pwqs;		/* WR: all pwqs of this wq */
// 	struct list_head	list;		/* PR: list of all workqueues */

// 	struct mutex		mutex;		/* protects this wq */
// 	int			work_color;	/* WQ: current work color */
// 	int			flush_color;	/* WQ: current flush color */
// 	atomic_t		nr_pwqs_to_flush; /* flush in progress */
// 	struct wq_flusher	*first_flusher;	/* WQ: first flusher */
// 	struct list_head	flusher_queue;	/* WQ: flush waiters */
// 	struct list_head	flusher_overflow; /* WQ: flush overflow list */

// 	struct list_head	maydays;	/* MD: pwqs requesting rescue */
// 	struct worker		*rescuer;	/* MD: rescue worker */

// 	int			nr_drainers;	/* WQ: drain in progress */

// 	/* See alloc_workqueue() function comment for info on min/max_active */
// 	int			max_active;	/* WO: max active works */
// 	int			min_active;	/* WO: min active works */
// 	int			saved_max_active; /* WQ: saved max_active */
// 	int			saved_min_active; /* WQ: saved min_active */

// 	struct workqueue_attrs	*unbound_attrs;	/* PW: only for unbound wqs */
// 	struct pool_workqueue __rcu *dfl_pwq;   /* PW: only for unbound wqs */

// #ifdef CONFIG_SYSFS
// 	struct wq_device	*wq_dev;	/* I: for sysfs interface */
// #endif
// #ifdef CONFIG_LOCKDEP
// 	char			*lock_name;
// 	struct lock_class_key	key;
// 	struct lockdep_map	lockdep_map;
// #endif
// 	char			name[WQ_NAME_LEN]; /* I: workqueue name */

// 	/*
// 	 * Destruction of workqueue_struct is RCU protected to allow walking
// 	 * the workqueues list without grabbing wq_pool_mutex.
// 	 * This is used to dump all workqueues from sysrq.
// 	 */
// 	struct rcu_head		rcu;

// 	/* hot fields used during command issue, aligned to cacheline */
// 	unsigned int		flags ____cacheline_aligned; /* WQ: WQ_* flags */
// 	struct pool_workqueue __rcu * __percpu *cpu_pwq; /* I: per-cpu pwqs */
// 	struct wq_node_nr_active *node_nr_active[]; /* I: per-node nr_active */
// };
static struct workqueue_struct *my_workqueue;
static DECLARE_DELAYED_WORK(delayed_work_struct, mykmod_work_handler);

// struct work_struct {
// 	atomic_long_t data;
// 	struct list_head entry;
// 	work_func_t func;
// #ifdef CONFIG_LOCKDEP
// 	struct lockdep_map lockdep_map;
// #endif
// };



/* Kprobe structure for symbol resolution */
static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"  /* Symbol to probe for address resolution */
};

/* System call related declarations */
typedef asmlinkage long (*my_sys_parth_t)(void); 
my_sys_parth_t my_sys_parth;


/* System call table access variables */
uint8_t was_writable = 0;
void **sys_call_table_addr = (void *)0xffffffff84a002e0;

/**
 * @function: mykmod_work_handler
 * 
 * @purpose: Work queue handler function that performs periodic system call
 *          verification. Schedules itself to run again after completion.
 * 
 * @param w: Work structure pointer (unused in this implementation)
 * 
 * @returns: None
 * 
 * @note: Reschedules itself every 5 seconds using queue_delayed_work
 */
static void
mykmod_work_handler(struct work_struct *w)
{   
         int fivesec = msecs_to_jiffies(5000);  /* Convert 5 seconds to jiffies */
        pr_info("mykmod work %u jiffies\n", (unsigned)fivesec);
        my_sys_parth();  /* Call our custom system call */
        /* Reschedule the work */
        queue_delayed_work(my_workqueue, &delayed_work_struct, fivesec);
}


/**
 * @function: custom_init_module
 * 
 * @purpose: Module initialization function that sets up work queue,
 *          resolves required symbols, and starts periodic monitoring.
 * 
 * @returns: 
 *     0: Successful initialization
 *    -1: Initialization failed
 * 
 * @note: Uses kprobes to resolve symbol addresses at runtime
 */
static int __init custom_init_module(void)
{
    int ret = 0;
    printk(KERN_INFO "Init\n");

    /* Symbol resolution using kprobes */
    typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
    kallsyms_lookup_name_t kallsyms_lookup_name_my;
    
    /* Register and use kprobe to get symbol address */
    register_kprobe(&kp);
    kallsyms_lookup_name_my = (kallsyms_lookup_name_t) kp.addr;
    unregister_kprobe(&kp);

    /* Resolve system call table address */
    sys_call_table_addr = (void *)kallsyms_lookup_name_my("sys_call_table");
    if (sys_call_table_addr == NULL) {
        pr_info("sys_call_table not found using kprobe my kallsyms\n");
        return -1;
    }

    printk(KERN_INFO "sys_call_table pointer from kprobe is %p\n", sys_call_table_addr);

    /* Resolve custom system call address */
    my_sys_parth = kallsyms_lookup_name_my("sys_parth");
    if(my_sys_parth == NULL){
        printk(KERN_INFO, "Could not find your own syscall");
        return -1;
    }

    /* Initialize and start work queue */
    int onesec = msecs_to_jiffies(1000);
    pr_info("lkm loaded %u jiffies\n", (unsigned)onesec);

    /* Create single-threaded work queue */
    if (!my_workqueue)
        my_workqueue = create_singlethread_workqueue("rtkit_check");
    if (my_workqueue)
        /**
         * queue_delayed_work - queue work on a workqueue after delay
         * @wq: workqueue to use
         * @dwork: delayable work to queue
         * @delay: number of jiffies to wait before queueing
         *
         * Equivalent to queue_delayed_work_on() but tries to use the local CPU.
         */

        // struct delayed_work {
        //     struct work_struct work;
        //     struct timer_list timer;

        //     /* target workqueue and CPU ->timer uses to queue ->work */
        //     struct workqueue_struct *wq;
        //     int cpu;
        // };
        queue_delayed_work(my_workqueue, &delayed_work_struct, onesec);

    


    return 0;
}

/**
 * @function: custom_cleanup_module
 * 
 * @purpose: Module cleanup function that ensures proper resource
 *          deallocation and work queue cleanup.
 * 
 * @returns: None
 * 
 * @note: Flushes and destroys work queue before module unload
 */
static void __exit custom_cleanup_module(void)
{
    /* Clean up work queue if it exists */
    if (my_workqueue) {        
        flush_workqueue(my_workqueue);    /* Wait for pending work to complete */
        destroy_workqueue(my_workqueue);  /* Free work queue resources */
    } 
    pr_info("mykmod exit\n");
}

/* Module entry and exit points */
module_init(custom_init_module);
module_exit(custom_cleanup_module);