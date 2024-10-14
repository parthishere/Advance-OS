#include <linux/init.h>      // Needed for module_init and module_exit macros
#include <linux/module.h>    // Needed by all modules
#include <linux/fs.h>        // Needed for file operations
#include <linux/errno.h>     // Needed for error codes
#include <linux/spinlock.h>  // Needed for spinlocks
#include <linux/uaccess.h>   // Needed for copy_to_user and copy_from_user

#define DEVICE_NAME "chardev"  // Name of the device
#define BUFFER_SIZE 1024       // Size of the buffer (1KB as specified)

// Define a spinlock for synchronization
static DEFINE_SPINLOCK(your_lock);

// Module metadata
MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("chardev driver");
MODULE_LICENSE("GPL");

// Global variables
static int Major;                                  // Major number assigned to our device driver
static uint8_t custom_chardrv_data[BUFFER_SIZE];   // 1KB buffer to simulate the device
static uint8_t device_open_count;                  // Number of times the device has been opened

/**
 * device_open - Called when a process tries to open the device file
 * @inode: Pointer to inode object (contains information about the file to open)
 * @file: Pointer to file object (represents the open file)
 *
 * This function is called whenever a process attempts to open the device file.
 * It increments the open counter and prints a debug message.
 *
 * Return: 0 on success
 */
static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "chardev: Device opened\n");  // Log the open operation
    device_open_count++;                           // Increment the open counter
    return 0;                                      // Return success
}

/**
 * device_read - Called when a process tries to read from the device file
 * @filp: Pointer to the file object
 * @buffer: The buffer to fill with data
 * @length: The length of the buffer
 * @offset: Our offset in the file
 *
 * This function is called when a process attempts to read from the device.
 * It copies data from the device's buffer to the user's buffer.
 *
 * Return: Number of bytes read, or negative error code on failure
 */
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset) {   
    int bytes_to_read = 0, bytes_not_copied = 0; 
    unsigned long flags;  // For saving interrupt state

    // Check if the user's buffer is valid
    if(buffer == NULL) {
        return -EINVAL;  // Invalid argument
    }

    // Acquire the spinlock and disable interrupts
    spin_lock_irqsave(&your_lock, flags);
    
    // Calculate how many bytes to read
    bytes_to_read = ((*offset + length) < BUFFER_SIZE) ? length : (BUFFER_SIZE - *offset);
    
    // Copy data from kernel space to user space
    bytes_not_copied = copy_to_user(buffer, &custom_chardrv_data[*offset], bytes_to_read);
    if(bytes_not_copied != 0) {
        spin_unlock_irqrestore(&your_lock, flags);  // Release the lock before returning
        return -EFAULT;  // Bad address
    }

    *offset += bytes_to_read;  // Update the file position
    
    // Release the spinlock and restore interrupts
    spin_unlock_irqrestore(&your_lock, flags);

    printk(KERN_INFO "chardev: Read %d bytes, new offset: %lld\n", bytes_to_read, *offset);
    return bytes_to_read;  // Return the number of bytes read
}

/**
 * device_write - Called when a process tries to write to the device file
 * @filp: Pointer to the file object
 * @buffer: The buffer containing the data to write
 * @length: The length of the data to write
 * @offset: Our offset in the file
 *
 * This function is called when a process writes to the dev file.
 * It copies data from the user's buffer to the device's buffer.
 *
 * Return: Number of bytes written, or negative error code on failure
 */
static ssize_t device_write(struct file *filp, const char *buffer, size_t length, loff_t *offset) {
    int bytes_to_write, bytes_not_copied; 
    unsigned long flags;  // For saving interrupt state

    // Validate input parameters
    if(buffer == NULL || length <= 0) {
        printk(KERN_ERR "chardev: Invalid write parameters\n");
        return -EINVAL;  // Invalid argument
    } 

    // Acquire the spinlock and disable interrupts
    spin_lock_irqsave(&your_lock, flags);
    
    // Calculate how many bytes to write
    bytes_to_write = (length > (BUFFER_SIZE - *offset - 1)) ? (BUFFER_SIZE - *offset - 1) : length;

    // Copy data from user space to kernel space
    bytes_not_copied = copy_from_user(&custom_chardrv_data[*offset], buffer, bytes_to_write);
    if(bytes_not_copied != 0) {
        spin_unlock_irqrestore(&your_lock, flags);  // Release the lock before returning
        return -EFAULT;  // Bad address
    }

    *offset += bytes_to_write;  // Update the file position
    
    // Release the spinlock and restore interrupts
    spin_unlock_irqrestore(&your_lock, flags);

    printk(KERN_INFO "chardev: Wrote %d bytes, new offset: %lld\n", bytes_to_write, *offset);
    return bytes_to_write;  // Return the number of bytes written
}

/**
 * device_llseek - Called when a process tries to change the file position
 * @flip: Pointer to the file object
 * @offset: The offset to set
 * @whence: The reference point for the offset
 *
 * This function changes the file position based on the offset and whence values.
 *
 * Return: The new position in the file, or negative error code on failure
 */
static loff_t device_llseek(struct file *flip, loff_t offset, int whence) {
    loff_t newpos = 0;

    switch(whence) {
        case SEEK_SET:  // The offset is set to offset bytes
            newpos = offset;
            break;
        case SEEK_CUR:  // The offset is set to its current location plus offset bytes
            newpos = flip->f_pos + offset;
            break;
        case SEEK_END:  // The offset is set to the size of the file plus offset bytes
            newpos = BUFFER_SIZE + offset;
            break;
        default:  // Invalid whence
            return -EINVAL;
    }

    // Ensure the new position is within bounds
    if (newpos < 0 || newpos > BUFFER_SIZE) {
        return -EINVAL;
    }

    flip->f_pos = newpos;  // Set the new file position
    printk(KERN_INFO "chardev: Seek to offset: %lld\n", newpos);
    return newpos;  // Return the new position
}

/**
 * device_release - Called when a process closes the device file
 * @inode: Pointer to inode object
 * @file: Pointer to file object
 *
 * This function is called when a process closes the device file.
 * It decrements the open counter and prints a debug message.
 *
 * Return: 0 on success
 */
static int device_release(struct inode *inode, struct file *file) {
    device_open_count--;  // Decrement the open counter
    printk(KERN_INFO "chardev: Device released\n");  // Log the release operation
    return 0;  // Return success
}

// Structure that declares the usual file access functions
static struct file_operations fops = {
    .llseek = device_llseek,   // lseek operation
    .read = device_read,       // read operation
    .write = device_write,     // write operation
    .open = device_open,       // open operation
    .release = device_release, // release operation
};

/**
 * custom_init - Initialization function
 *
 * This function is called when the module is loaded. It initializes the device.
 *
 * Return: 0 on success, negative error code on failure
 */
static int __init custom_init(void) {
    // Initialize the buffer with '0' characters
    memset(custom_chardrv_data, '0', sizeof(custom_chardrv_data));
    printk(KERN_INFO "chardev: Initializing the chardev LKM\n");

    // Register the character device
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if (Major < 0) {
        printk(KERN_ALERT "chardev: Registering char device failed with %d\n", Major);
        return Major;  // Return error code
    }

    printk(KERN_INFO "chardev: registered correctly with major number %d\n", Major);
    return 0;  // Return success
}

/**
 * custom_exit - Cleanup function
 *
 * This function is called when the module is unloaded.
 * It unregisters the device and performs cleanup.
 */
static void __exit custom_exit(void) {
    unregister_chrdev(Major, DEVICE_NAME);  // Unregister the device
    printk(KERN_INFO "chardev: Goodbye from the LKM!\n");
}

// Specify the initialization and exit functions
module_init(custom_init);
module_exit(custom_exit);