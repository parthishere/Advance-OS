#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Parth Thakkar>");
MODULE_DESCRIPTION("Device Driver");
MODULE_VERSION("1.0");

/*************** TASKLET  ******************/
#define GPIO_INTERRUPT 0
#if (GPIO_INTERRUPT == 1)
#define GPIO_INT_PIN 17
#else
// #define FIRST_EXTERNAL_VECTOR		0x20
//  #define ISA_IRQ_VECTOR(irq)		(((FIRST_EXTERNAL_VECTOR + 16) & ~15) + irq)
#define IRQ_NO 11
#endif
// struct tasklet_struct
// {
// 	struct tasklet_struct *next;
// 	unsigned long state;
// 	atomic_t count;
// 	bool use_callback;
// 	union {
// 		void (*func)(unsigned long data);
// 		void (*callback)(struct tasklet_struct *t);
// 	};
// 	unsigned long data;
// };

// #define DECLARE_TASKLET(name, _callback)
// struct tasklet_struct name = {
// 	.count = ATOMIC_INIT(0),
// 	.callback = _callback,
// 	.use_callback = true,
// }
int irq_number;

void tasklet_bottom_half_callback(unsigned long);

/* Init the Tasklet by Static Method */
DECLARE_TASKLET(my_tasklet_struct, (void *)tasklet_bottom_half_callback);

/*Tasklet Function*/
void tasklet_bottom_half_callback(unsigned long arg)
{
    printk(KERN_INFO "Executing bottom half of interrupts with : arg = %ld\n", arg);
}

// prototype in interrupts.h
// typedef irqreturn_t (*irq_handler_t)(int, void *);
static irqreturn_t irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "Shared IRQ: Interrupt Occurred");

    /*Scheduling Task to Tasklet*/
    tasklet_schedule(&my_tasklet_struct);
    return IRQ_HANDLED;
}

/**************** CHAR DEVICE ********************/
#define CHAR_DRIVER_NAME "temperature"
#define CHAR_DRIVER_CLASS "healthmonitor_class"
#define CLASS_DEVICE_NAME "temperature_device"
#define DEVICE_SYSFS_NAME "temperature_sysfs"
static int temperature_value = 0;

// dev_t is a 32-bit quantity with 12 bits set aside for the major number and 20 for the minor number.
dev_t my_char_device_id;

// struct cdev {
// 	struct kobject kobj;
// 	struct module *owner;
// 	const struct file_operations *ops;
// 	struct list_head list;
// 	dev_t dev;
// 	unsigned int count;
// };
static struct cdev temperature_cdev;

/**
 * struct class - device classes
 * @name:	Name of the class.
 * @class_groups: Default attributes of this class.
 * @dev_groups:	Default attributes of the devices that belong to the class.
 * @dev_uevent:	Called when a device is added, removed from this class, or a
 *		few other things that generate uevents to add the environment
 *		variables.
 * @devnode:	Callback to provide the devtmpfs.
 * @class_release: Called to release this class.
 * @dev_release: Called to release the device.
 * @shutdown_pre: Called at shut-down time before driver shutdown.
 * @ns_type:	Callbacks so sysfs can detemine namespaces.
 * @namespace:	Namespace of the device belongs to this class.
 * @get_ownership: Allows class to specify uid/gid of the sysfs directories
 *		for the devices belonging to the class. Usually tied to
 *		device's namespace.
 * @pm:		The default device power management operations of this class.
 *
 * A class is a higher-level view of a device that abstracts out low-level
 * implementation details. Drivers may see a SCSI disk or an ATA disk, but,
 * at the class level, they are all simply disks. Classes allow user space
 * to work with devices based on what they do, rather than how they are
 * connected or how they work.
 */
struct class *healthmonitoring_class;

/*************** Driver Functions **********************/
ssize_t temperature_dev_read(struct file *filp, char __user *buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "Read function\n");
#if (GPIO_INTERRUPT == 0)
    asm("int $0x3B"); // Corresponding to irq 11
#endif
    return 0;
}

ssize_t temperature_dev_write(struct file *flip, const char __user *buffer, size_t len, loff_t *offset)
{
    printk(KERN_INFO "Write Function\n");
    return len;
}

int temperature_dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}

int temperature_dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device File Closed...!!!\n");
    return 0;
}

static struct file_operations fops =
    {
        .owner = THIS_MODULE,               // extern struct module __this_module; #define THIS_MODULE (&__this_module)
        .read = temperature_dev_read,       // ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
        .write = temperature_dev_write,     // ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
        .open = temperature_dev_open,       // int (*open) (struct inode *, struct file *);
        .release = temperature_dev_release, // int (*release) (struct inode *, struct file *);
};

/*************** Sysfs Functions **********************/

struct kobject *temperature_kobj_ref;

static ssize_t sysfs_show(struct kobject *kobj,
                          struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj,
                           struct kobj_attribute *attr, const char *buf, size_t count);

// struct kobj_attribute {
// 	struct attribute {
// 	const char		*name;
// 	umode_t			mode;
// #ifdef CONFIG_DEBUG_LOCK_ALLOC
// 	bool			ignore_lockdep:1;
// 	struct lock_class_key	*key;
// 	struct lock_class_key	skey;
// #endif
// };
// 	ssize_t (*show)(struct kobject *kobj, struct kobj_attribute *attr,
// 			char *buf);
// 	ssize_t (*store)(struct kobject *kobj, struct kobj_attribute *attr,
// 			 const char *buf, size_t count);
// };

// #define __ATTR(_name, _mode, _show, _store) {
// 	.attr = {.name = __stringify(_name),
// 		 .mode = VERIFY_OCTAL_PERMISSIONS(_mode) },
// 	.show	= _show,
// 	.store	= _store,
// }
struct kobj_attribute temperature_sysfs_attr = __ATTR(temperature_value, 0660, sysfs_show, sysfs_store);

/*
** This function will be called when we read the sysfs file
*/
static ssize_t sysfs_show(struct kobject *kobj,
                          struct kobj_attribute *attr, char *buf)
{
    printk(KERN_INFO "Sysfs - Read!!!\n");
    return sprintf(buf, "%d", temperature_value);
}
/*
** This function will be called when we write the sysfsfs file
*/
static ssize_t sysfs_store(struct kobject *kobj,
                           struct kobj_attribute *attr, const char *buf, size_t count)
{
    printk(KERN_INFO "Sysfs - Write!!!\n");
    sscanf(buf, "%d", &temperature_value);
    return count;
}

/******** Initialization routines ************ */
static int __init temperature_driver_init(void)
{
    // NAME
    // alloc_chrdev_region - register a range of char device numbers
    // SYNOPSIS
    // int alloc_chrdev_region(dev_t * dev, unsigned baseminor, unsigned count, const char * name);
    // ARGUMENTS
    // dev
    // output parameter for first assigned number
    // baseminor
    // first of the requested range of minor numbers
    // count
    // the number of minor numbers required
    // name
    // the name of the associated device or driver
    // DESCRIPTION
    // Allocates a range of char device numbers. The major number will be chosen dynamically, and returned (along with the first minor number) in dev. Returns zero or a negative error code.
    if (alloc_chrdev_region(&my_char_device_id, 0, 1, CHAR_DRIVER_NAME) < 0)
    { // we are not assigning any major number to it !
        printk(KERN_INFO "Cannot allocate major number\n");
        return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n", MAJOR(my_char_device_id), MINOR(my_char_device_id));

    /*Creating cdev structure*/
    // NAME
    // cdev_init - initialize a cdev structure
    // SYNOPSIS
    // void cdev_init(struct cdev * cdev, const struct file_operations * fops);
    // ARGUMENTS
    // cdev
    // the structure to initialize
    // fops
    // the file_operations for this device
    // DESCRIPTION
    // Initializes cdev, remembering fops, making it ready to add to the system with cdev_add.
    cdev_init(&temperature_cdev, &fops);

    /*Adding character device to the system*/
    // NAME
    // cdev_add - add a char device to the system
    // SYNOPSIS
    // int cdev_add(struct cdev * p, dev_t dev, unsigned count);
    // ARGUMENTS
    // p
    // the cdev structure for the device
    // dev
    // the first device number for which this device is responsible
    // count
    // the number of consecutive minor numbers corresponding to this device
    // DESCRIPTION
    // cdev_add adds the device represented by p to the system, making it live immediately. A negative error code is returned on failure.
    if (cdev_add(&temperature_cdev, my_char_device_id, 1) < 0)
    {
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto failed_cdev_add;
    }

    /*Creating struct class*/
    //  struct class * __must_check class_create(const char *name);
    // NAME
    // __class_create - create a struct class structure
    // SYNOPSIS
    // struct class * __class_create(struct module * owner, const char * name, struct lock_class_key * key);
    // ARGUMENTS
    // owner
    // pointer to the module that is to “own” this struct class
    // name
    // pointer to a string for the name of this class.
    // key
    // the lock_class_key for this class; used by mutex lock debugging
    // DESCRIPTION
    // This is used to create a struct class pointer that can then be used in calls to device_create.
    // Returns struct class pointer on success, or ERR_PTR on error.
    // Note, the pointer creatIt helps organize similar devices together
    // It's created in /sys/class/in sys/
    if (IS_ERR(healthmonitoring_class = class_create(CHAR_DRIVER_CLASS)))
    {
        printk(KERN_INFO "Cannot create the struct class\n");
        goto failed_class_create;
    }

    /*Creating device*/
    //  NAME
    // device_create - creates a device and registers it with sysfs
    // SYNOPSIS
    // struct device * device_create(struct class * class, struct device * parent, dev_t devt, void * drvdata, const char * fmt, ...);
    // ARGUMENTS
    // class
    // pointer to the struct class that this device should be registered to
    // parent
    // pointer to the parent struct device of this new device, if any
    // devt
    // the dev_t for the char device to be added
    // drvdata
    // the data to be added to the device for callbacks
    // fmt
    // string for the device's name
    // ...
    // variable arguments
    // DESCRIPTION
    // This function can be used by char device classes. A struct device will be created in sysfs, registered to the specified class.
    // A “dev” file will be created, showing the dev_t for the device, if the dev_t is not 0,0. If a pointer to a parent struct device is passed in, the newly created struct device will be a child of that device in sysfs. The pointer to the struct device will be returned from the call. Any further sysfs files that might be required can be created using this pointer.
    // Returns struct device pointer on success, or ERR_PTR on error.
    if (IS_ERR(device_create(healthmonitoring_class, NULL, my_char_device_id, NULL, CLASS_DEVICE_NAME)))
    {
        printk(KERN_INFO "Cannot create the Device 1\n");
        goto falied_device_create;
    }

    /*Creating a directory in /sys/kernel/ */
    // NAME
    // kobject_create_and_add - create a struct kobject dynamically and register it with sysfs
    // SYNOPSIS
    // struct kobject * kobject_create_and_add(const char * name, struct kobject * parent);
    // ARGUMENTS
    // name
    // the name for the kobject
    // parent
    // the parent kobject of this kobject, if any.
    // DESCRIPTION
    // This function creates a kobject structure dynamically and registers it with sysfs. When you are finished with this structure, call kobject_put and the structure will be dynamically freed when it is no longer being used.
    // If the kobject was not able to be created, NULL will be returned.
    temperature_kobj_ref = kobject_create_and_add(DEVICE_SYSFS_NAME, kernel_kobj); // kernel_kobj default parent !

    /*Creating sysfs file for etx_value*/
    //      NAME
    // sysfs_create_file - create an attribute file for an object.
    // SYNOPSIS
    // int sysfs_create_file(struct kobject * kobj, const struct attribute * attr);
    // ARGUMENTS
    // kobj
    // object we're creating for.
    // attr
    // attribute descriptor.
    if (sysfs_create_file(temperature_kobj_ref, &temperature_sysfs_attr.attr))
    {
        printk(KERN_INFO "Cannot create sysfs file......\n");
        goto failed_sysfs_create_file;
    }

#if (GPIO_INTERRUPT == 1)
    // Request GPIO
    if (gpio_request(GPIO_INT_PIN, "temp_interrupt"))
    {
        printk(KERN_ERR "Failed to request GPIO %d\n", GPIO_INT_PIN);
        goto gpio_fail;
    }

    // Configure GPIO as input

    if (gpio_direction_input(GPIO_INT_PIN))
    {
        printk(KERN_ERR "Failed to set GPIO direction\n");
        goto gpio_direction_fail;
    }

    // Get IRQ number from GPIO
    irq_number = gpio_to_irq(GPIO_INT_PIN);
    if (irq_number < 0)
    {
        printk(KERN_ERR "Failed to get IRQ number for GPIO\n");
        goto irq_fail;
    }

    // Request IRQ with proper flags
    int ret = request_irq(irq_number,
                          irq_handler,
                          IRQF_TRIGGER_RISING | IRQF_SHARED,
                          "temperature_interrupt",
                          &temperature_cdev); // Use device structure as dev_id
    if (ret)
    {
        printk(KERN_ERR "Failed to request IRQ\n");
        goto irq_request_fail;
    }

    printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;

irq_request_fail:
    gpio_free(GPIO_INT_PIN);
irq_fail:
gpio_direction_fail:
    gpio_free(GPIO_INT_PIN);
gpio_fail:
#else

    // int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev)¶
    // Parameters
    // unsigned int irq
    // The interrupt line to allocate
    // irq_handler_t handler
    // Function to be called when the IRQ occurs. Primary handler for threaded interrupts If NULL, the default primary handler is installed
    // unsigned long flags
    /*
     * These flags used only by the kernel as part of the
     * irq handling routines.
     *
     * IRQF_SHARED - allow sharing the irq among several devices
     * IRQF_PROBE_SHARED - set by callers when they expect sharing mismatches to occur
     * IRQF_TIMER - Flag to mark this interrupt as timer interrupt
     * IRQF_PERCPU - Interrupt is per cpu
     * IRQF_NOBALANCING - Flag to exclude this interrupt from irq balancing
     * IRQF_IRQPOLL - Interrupt is used for polling (only the interrupt that is
     *                registered first in a shared interrupt is considered for
     *                performance reasons)
     * IRQF_ONESHOT - Interrupt is not reenabled after the hardirq handler finished.
     *                Used by threaded interrupts which need to keep the
     *                irq line disabled until the threaded handler has been run.
     * IRQF_NO_SUSPEND - Do not disable this IRQ during suspend.  Does not guarantee
     *                   that this interrupt will wake the system from a suspended
     *                   state.  See Documentation/power/suspend-and-interrupts.rst
     * IRQF_FORCE_RESUME - Force enable it on resume even if IRQF_NO_SUSPEND is set
     * IRQF_NO_THREAD - Interrupt cannot be threaded
     * IRQF_EARLY_RESUME - Resume IRQ early during syscore instead of at device
     *                resume time.
     * IRQF_COND_SUSPEND - If the IRQ is shared with a NO_SUSPEND user, execute this
     *                interrupt handler after suspending interrupts. For system
     *                wakeup devices users need to implement wakeup detection in
     *                their interrupt handlers.
     * IRQF_NO_AUTOEN - Don't enable IRQ or NMI automatically when users request it.
     *                Users will enable it explicitly by enable_irq() or enable_nmi()
     *                later.
     * IRQF_NO_DEBUG - Exclude from runnaway detection for IPI and similar handlers,
     *		   depends on IRQF_PERCPU.
     * IRQF_COND_ONESHOT - Agree to do IRQF_ONESHOT if already set for a shared
     *                 interrupt.
     */
    // const char *name
    // Name of the device generating this interrupt // in our case it should be same as device which we created in sysfs
    // void *dev
    // A cookie passed to the handler function
    // Description
    // This call allocates an interrupt and establishes a handler; see the documentation for request_threaded_irq() for details.
    if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, CLASS_DEVICE_NAME, (void *)irq_handler))
    {
        printk(KERN_INFO "my_device: cannot register IRQ ");
        goto failed_request_irq;
    }

    printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;

failed_request_irq:
    free_irq(IRQ_NO, (void *)(irq_handler));
#endif

failed_sysfs_create_file:
    kobject_put(temperature_kobj_ref);
    sysfs_remove_file(kernel_kobj, &temperature_sysfs_attr.attr);
falied_device_create:
    device_destroy(healthmonitoring_class, my_char_device_id);
    class_destroy(healthmonitoring_class);
failed_class_create:
    cdev_del(&temperature_cdev);
failed_cdev_add:
    unregister_chrdev_region(my_char_device_id, 1);

    return -1;
}

static void __exit ioexpander_driver_exit(void)
{

#if (GPIO_INTERRUPT == 1)
    free_irq(irq_number, &temperature_cdev);
    gpio_free(GPIO_INT_PIN);
    tasklet_kill(&my_tasklet_struct);
#else
    free_irq(IRQ_NO, (void *)(irq_handler));
#endif

    kobject_put(temperature_kobj_ref);
    sysfs_remove_file(kernel_kobj, &temperature_sysfs_attr.attr);
    device_destroy(healthmonitoring_class, my_char_device_id);
    class_destroy(healthmonitoring_class);
    cdev_del(&temperature_cdev);
    unregister_chrdev_region(my_char_device_id, 1);
    printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}

module_init(temperature_driver_init);
module_exit(ioexpander_driver_exit);