#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kprobes.h>


MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("Rootkit");
MODULE_LICENSE("GPL");

/*
As create_proc_entry function is deprecated, The newer functions are named proc_*. You can see their declarations in include/linux/proc_fs.h.

In particular, proc_create creates a proc entry. You can check out the implementation of the other (quite useful) functions in the source file at fs/proc/generic.c. You may be particularly interested in proc_mkdir and proc_create_data.
*/



static int die = 0;
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




static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

asmlinkage long (*my_sys_parth)(void); 

uint8_t was_writable = 0;
void **sys_call_table_addr = (void *)0xffffffff84a002e0;


static void
mykmod_work_handler(struct work_struct *w)
{   
        int onesec = msecs_to_jiffies(1000);
        pr_info("mykmod work %u jiffies\n", (unsigned)onesec);
        queue_delayed_work(my_workqueue, &delayed_work_struct, onesec);
}


static int __init custom_init_module(void)
{

    int ret = 0;
    printk(KERN_INFO "Init\n");

    typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
    kallsyms_lookup_name_t kallsyms_lookup_name_my;
    register_kprobe(&kp);
    kallsyms_lookup_name_my = (kallsyms_lookup_name_t) kp.addr;
    unregister_kprobe(&kp);
    sys_call_table_addr = (void *)kallsyms_lookup_name_my("sys_call_table");
    if (sys_call_table_addr == NULL)
    {
        pr_info("sys_call_table not found using kprobe my kallsyms\n");
        return -1;
    }
    printk(KERN_INFO "sys_call_table pointer from kprobe is %p\n", sys_call_table_addr);
    my_sys_parth = kallsyms_lookup_name_my("sys_parth");

    int onesec = msecs_to_jiffies(1000);
    pr_info("lkm loaded %u jiffies\n", (unsigned)onesec);
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

static void __exit custom_cleanup_module(void)
{

     if (my_workqueue)
	{        
		flush_workqueue(my_workqueue);
		destroy_workqueue(my_workqueue);
	} 
     pr_info("mykmod exit\n");
}

module_init(custom_init_module);
module_exit(custom_cleanup_module);