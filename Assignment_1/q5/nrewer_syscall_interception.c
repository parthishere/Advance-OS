#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <asm/unistd.h> // contains syscall numbers
#include <linux/kprobes.h>

// #define KPROBE_LOOKUP 1
// #define START_MEM 0xffffffff81000000
// #define END_MEM   0xffffffffa2000000


MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("chardev driver");
MODULE_LICENSE("GPL");

// static struct kprobe my_kp = {
//     .symbol_name = "kallsyms_lookup_name"
// };

// static unsigned long * __sys_call_table = NULL;

// static unsigned long kallsyms_lookup_name_func(const char *name)
// {
//     unsigned long addr = 0;
// #ifdef KPROBE_LOOKUP
//     typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
//     kallsyms_lookup_name_t kallsyms_lookup_name;
//     register_kprobe(&my_kp);
//     kallsyms_lookup_name = (kallsyms_lookup_name_t)my_kp.addr;
//     unregister_kprobe(&my_kp);
//     addr = kallsyms_lookup_name(name);
// #endif
    
//     return addr;
// }

// static unsigned long *find_sys_call_table(void)
// {
//     unsigned long *syscall_table;
//     unsigned long int i;

//     for (i = START_MEM; i < END_MEM; i += sizeof(void *)) {
//         syscall_table = (unsigned long *)i;

//         if (syscall_table[__NR_close] == kallsyms_lookup_name_func("__x64_sys_close")) {
//             return syscall_table;
//         }
//     }
//     return NULL;
// }

// static int __init custom_init(void){

//     printk(KERN_INFO "HERE WE GO FOR INTERCEPTION !!! \n"); 
//     __sys_call_table = find_sys_call_table();
//     printk(KERN_INFO "syscall table address : %p\n", __sys_call_table);
//     // __sys_call_table[]

//     return 0;
// }



static unsigned long sys_call_table_addr = 0;

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    sys_call_table_addr = (unsigned long)regs->di;
    return 0;
}

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

static int __init custom_init(void)
{
    int ret;

    kp.pre_handler = handler_pre;
    ret = register_kprobe(&kp);
    if (ret < 0) {
        printk(KERN_ERR "register_kprobe failed, returned %d\n", ret);
        return ret;
    }

    typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
    kallsyms_lookup_name_t kallsyms_lookup_name;
    kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
    sys_call_table_addr = kallsyms_lookup_name("sys_call_table");

    unregister_kprobe(&kp);

    if (sys_call_table_addr) {
        printk(KERN_INFO "sys_call_table address: 0x%lx\n", sys_call_table_addr);
    } else {
        printk(KERN_ERR "Failed to find sys_call_table\n");
        return -1;
    }

    return 0;
}


static void __exit custom_exit(void){

    printk(KERN_INFO "Noooooo..... my interception ,,,,,  exiting....\n");
}


module_init(custom_init);
module_exit(custom_exit);