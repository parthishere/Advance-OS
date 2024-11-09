/*********************************************************************
 * Advanced OS Assignment 2
 * File: user_task_module.c
 * 
 * Purpose: 
 *     Implements a kernel module that manages user-space tasks using
 *     work queues and provides multiple methods for kernel-user 
 *     communication: signals, procfs, and character devices.
 * 
 * Features:
 *     - Work queue based task scheduling
 *     - Multiple user notification methods
 *     - Synchronization using completions
 *     - Resource cleanup management
 * 
 * Author: Parth Thakkar
 * Date: 8/11/24
 * 
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *********************************************************************/

/* Required header files */
#include <linux/module.h>    /* Core module functionality */
#include <linux/workqueue.h> /* Work queue API */
#include <linux/slab.h>      /* Kernel memory allocation */
#include <linux/delay.h>     /* Delay functions */
#include <linux/sched.h>     /* Process scheduling */
#include <linux/uaccess.h>   /* User space access */
#include <linux/fs.h>        /* File operations */
#include <linux/string.h>    /* String operations */
#include <linux/signal.h>    /* Signal handling */


/**
 * @struct: user_task
 * @brief: Structure to hold task-specific data and synchronization primitives
 * 
 * @field work: Work structure for queuing in workqueue
 * @field user_pid: Process ID of the user-space program
 * @field data: Buffer for data exchange with user-space
 * @field done: Completion structure for synchronization
 */
// Structure to hold task data
struct user_task {
    struct work_struct work;
    pid_t user_pid;          // User process PID
    char data[256];          // Data to pass to userspace
    struct completion done;   // Completion for synchronization
};

/* Global workqueue for task management */
static struct workqueue_struct *user_task_wq;

/**
 * @function: send_signal_to_userspace
 * 
 * @purpose: Sends a SIGUSR1 signal to the specified user process
 * 
 * @param task: Pointer to the user_task structure
 * 
 * @note: Uses RCU locking for safe process lookup
 */
// Method 1: Using signals to notify userspace
static void send_signal_to_userspace(struct user_task *task)
{
    struct siginfo info;
    struct task_struct *user_process;

    memset(&info, 0, sizeof(info));
    info.si_signo = SIGUSR1;
    info.si_code = SI_QUEUE;
    
    /* Safely lookup and signal the user process */
    rcu_read_lock();
    user_process = pid_task(find_vpid(task->user_pid), PIDTYPE_PID);
    if (user_process) {
        send_sig_info(SIGUSR1, &info, user_process);
    }
    rcu_read_unlock();
}

// Method 2: Using procfs interface
static struct proc_dir_entry *proc_entry;

// Method 3: Using device file
static struct file_operations user_task_fops;
static int major_number;

// Work handler
static void user_task_handler(struct work_struct *work)
{
    struct user_task *task = container_of(work, struct user_task, work);

    // Execute the task
    printk(KERN_INFO "Executing task for PID %d\n", task->user_pid);

    // Method 1: Signal the userspace process
    send_signal_to_userspace(task);

    // Method 2: Update proc entry
    // (Implementation would go here)

    // Method 3: Make data available via device file
    // (Implementation would go here)

    complete(&task->done);
}

// Function to schedule a userspace task
static int schedule_user_task(pid_t pid, const char *data)
{
    struct user_task *task;

    task = kzalloc(sizeof(*task), GFP_KERNEL);
    if (!task)
        return -ENOMEM;

    // Initialize task data
    INIT_WORK(&task->work, user_task_handler);
    task->user_pid = pid;
    strlcpy(task->data, data, sizeof(task->data));
    init_completion(&task->done);

    // Queue the work
    if (!queue_work(user_task_wq, &task->work)) {
        kfree(task);
        return -EAGAIN;
    }

    return 0;
}

// Proc file operations
static ssize_t proc_read(struct file *file, char __user *ubuf, 
                        size_t count, loff_t *ppos)
{
    // Implementation to read task results
    return 0;
}

static ssize_t proc_write(struct file *file, const char __user *ubuf,
                         size_t count, loff_t *ppos)
{
    char buf[256];
    pid_t pid;
    
    if (count >= sizeof(buf))
        return -EINVAL;

    if (copy_from_user(buf, ubuf, count))
        return -EFAULT;

    buf[count] = '\0';

    // Parse pid from input
    if (sscanf(buf, "%d", &pid) != 1)
        return -EINVAL;

    // Schedule task
    if (schedule_user_task(pid, "Sample task data"))
        return -EAGAIN;

    return count;
}

static const struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

// Device file operations
static ssize_t device_read(struct file *file, char __user *ubuf,
                          size_t count, loff_t *ppos)
{
    // Implementation to read task results
    return 0;
}

static ssize_t device_write(struct file *file, const char __user *ubuf,
                           size_t count, loff_t *ppos)
{
    // Implementation to schedule tasks
    return count;
}

static struct file_operations user_task_fops = {
    .read = device_read,
    .write = device_write,
};

// Module initialization
static int __init user_task_init(void)
{
    int ret;

    // Create workqueue
    user_task_wq = create_singlethread_workqueue("user_task_wq");
    if (!user_task_wq)
        return -ENOMEM;

    // Create proc entry
    proc_entry = proc_create("user_task", 0666, NULL, &proc_fops);
    if (!proc_entry) {
        destroy_workqueue(user_task_wq);
        return -ENOMEM;
    }

    // Register character device
    major_number = register_chrdev(0, "user_task", &user_task_fops);
    if (major_number < 0) {
        remove_proc_entry("user_task", NULL);
        destroy_workqueue(user_task_wq);
        return major_number;
    }

    return 0;
}

// Module cleanup
static void __exit user_task_exit(void)
{
    // Clean up character device
    unregister_chrdev(major_number, "user_task");

    // Remove proc entry
    remove_proc_entry("user_task", NULL);

    // Flush and destroy workqueue
    flush_workqueue(user_task_wq);
    destroy_workqueue(user_task_wq);
}

module_init(user_task_init);
module_exit(user_task_exit);


___________________________________

/*********************************************************************
 * Advanced OS Assignment 2
 * File: user_task_app.c
 * 
 * Purpose: 
 *     User-space application that interacts with the kernel module
 *     through multiple interfaces: signals, procfs, and character 
 *     devices. Demonstrates kernel-user communication methods.
 * 
 * Features:
 *     - Signal handling for kernel notifications
 *     - Proc filesystem interface usage
 *     - Character device communication
 * 
 * Author: Parth Thakkar
 * Date: 8/11/24
 * 
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *********************************************************************/


/* Required header files */
#include <stdio.h>      /* Standard I/O operations */
#include <signal.h>     /* Signal handling */
#include <unistd.h>     /* UNIX standard functions */
#include <fcntl.h>      /* File control operations */
#include <string.h>     /* String operations */
#include <stdlib.h>     /* Standard library functions */


/**
 * @function: signal_handler
 * 
 * @purpose: Handles SIGUSR1 signals from the kernel module and reads
 *          task results from proc or device files
 * 
 * @param signo: Signal number received
 * 
 * @note: Opens and reads from /proc/user_task for task results
 */
// Signal handler for kernel notifications
static void signal_handler(int signo)
{
    if (signo == SIGUSR1) {
        printf("Received notification from kernel\n");
        
        // Read results from proc or device file
        int fd = open("/proc/user_task", O_RDONLY);
        if (fd >= 0) {
            char buf[256];
            ssize_t n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                buf[n] = '\0';
                printf("Task result: %s\n", buf);
            }
            close(fd);
        }
    }
}

int main()
{
    // Register signal handler
    if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
        perror("signal");
        return 1;
    }

    // Method 1: Write PID to proc file
    int fd = open("/proc/user_task", O_WRONLY);
    if (fd >= 0) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d\n", getpid());
        write(fd, buf, strlen(buf));
        close(fd);
    }

    // Method 2: Use device file
    fd = open("/dev/user_task", O_RDWR);
    if (fd >= 0) {
        // Write request
        char request[] = "execute_task";
        write(fd, request, strlen(request));

        // Read result
        char result[256];
        read(fd, result, sizeof(result));
        close(fd);
    }

    // Wait for signals
    while (1) {
        pause();
    }

    return 0;
}