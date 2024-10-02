#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/errno.h>

#define DEVICE_NAME "chardev"
#define BUFFER_SIZE 100


MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("chardev driver");
MODULE_LICENSE("GPL");

static int Major;
int data[BUFFER_SIZE];

/* Methods */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file){
    printk("OPEN \n");
    return 0;
}



/* Called when a process, which already opened the dev file, attempts to
   read from it.
*/
static ssize_t device_read(struct file *filp,
   char *buffer,    /* The buffer to fill with data */
   size_t length,   /* The length of the buffer     */
   loff_t *offset)  /* Our offset in the file       */
{   
    printk("READ length %ld offset: %lld \n", length, *offset);
    return 0;
}

/*  Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t device_write(struct file *filp,
const char *buffer,
   size_t length,
   loff_t *offset)
{
    printk("WRITE length %ld offset: %lld \n", length, *offset);
    return 0;
}


static loff_t device_llseek(struct file *file, loff_t offset, int value){
    printk("LLSEEK length %lld offset: %d \n", offset, value);
    return 0;
}


/* Called when a process closes the device file */
static int device_release(struct inode *inode, struct file *file)
{
    printk("RELEASE  \n");

   return 0;
}

/*
0	KERN_EMERG	An emergency condition; the system is probably dead
1	KERN_ALERT	A problem that requires immediate attention
2	KERN_CRIT	A critical condition
3	KERN_ERR	An error
4	KERN_WARNING	A warning
5	KERN_NOTICE	A normal, but perhaps noteworthy, condition
6	KERN_INFO	An informational message
7	KERN_DEBUG	A debug message, typically superfluous
*/


/*
struct file_operations {
    struct module *owner;
    loff_t (*llseek) (struct file *, loff_t, int);
    ssize_t (*read) (struct file *, char *, size_t, loff_t *);
    ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
    int (*readdir) (struct file *, void *, filldir_t);
    unsigned int (*poll) (struct file *, struct poll_table_struct *);
    int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
    int (*mmap) (struct file *, struct vm_area_struct *);
    int (*open) (struct inode *, struct file *);
    int (*flush) (struct file *);
    int (*release) (struct inode *, struct file *);
    int (*fsync) (struct file *, struct dentry *, int datasync);
    int (*fasync) (int, struct file *, int);
    int (*lock) (struct file *, int, struct file_lock *);
        ssize_t (*readv) (struct file *, const struct iovec *, unsigned long,
        loff_t *);
        ssize_t (*writev) (struct file *, const struct iovec *, unsigned long,
        loff_t *);
};
*/
static struct file_operations fops = {
    llseek: device_llseek,
    read: device_read,
    write: device_write,
    open: device_open,
    release: device_release,
};


static int __init custom_init(void){
    printk(KERN_INFO "Hello world !\n");
    int return_val = register_chrdev(0, DEVICE_NAME, &fops);
    if(return_val < 0){
        printk(KERN_ERR "Registering device failed : %d \n", return_val);
        return return_val;
    }
    Major = return_val;

    return 0;
}

static void __exit custom_exit(void){
    unregister_chrdev(Major, DEVICE_NAME);
    printk(KERN_INFO "Exit\n");
}



module_init(custom_init);
module_exit(custom_exit);