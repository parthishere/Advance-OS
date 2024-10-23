#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>

struct proc_dir_entry *Our_Proc_File;
#define PROC_ENTRY_FILENAME "sched"
#define MY_WORK_QUEUE_NAME "WQsched.c"

static int TimerIntrpt = 0;
static void intrpt_routine(void *);
static int die = 0; /* set this to 1 for shutdown */
static struct workqueue_struct *my_workqueue;
static struct work_struct Task;
static DECLARE_WORK(Task, intrpt_routine, NULL);

static void intrpt_routine(void *irrelevant)
{
    TimerIntrpt++;
    if (die == 0)
        queue_delayed_work(my_workqueue, &Task, 100);
}

int __init init_module()
{
    Our_Proc_File = create_proc_entry(PROC_ENTRY_FILENAME, 0644, NULL);
    if (Our_Proc_File == NULL)
    {
        remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
        printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
               PROC_ENTRY_FILENAME);
        return -ENOMEM;
    }
    Our_Proc_File->read_proc = procfile_read;
    Our_Proc_File->owner = THIS_MODULE;
    Our_Proc_File->mode = S_IFREG | S_IRUGO;
    Our_Proc_File->uid = 0;
    Our_Proc_File->gid = 0;
    Our_Proc_File->size = 80;
    my_workqueue = create_workqueue(MY_WORK_QUEUE_NAME);
    queue_delayed_work(my_workqueue, &Task, 100);
    printk(KERN_INFO "/proc/%s created\n", PROC_ENTRY_FILENAME);
    return 0;
}

void __exit cleanup_module()
{
    remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
    printk(KERN_INFO "/proc/%s removed\n", PROC_ENTRY_FILENAME);
    die = 1;
    /* keep intro_routine from queueing itself */
    cancel_delayed_work(&Task); /* no "new ones" */
    flush_workqueue(my_workqueue);
    /* wait till all "old ones" finished */
    destroy_workqueue(my_workqueue);
}

module_init(init_module);
module_exit(cleanup_module);