#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PARTH THAKKAR");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("A SIMPLE ANTI ROOTKIT");

void **sys_call_table_addr;
unsigned int (*core_kernel_text_my)(unsigned long addr);

static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};

static int __init custom_init(void){
    printk(KERN_INFO "init\n");

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

    core_kernel_text_my = kallsyms_lookup_name_my("core_kernel_text");
    for (int syscall_number=0; syscall_number< __NR_syscalls; syscall_number++){
        int malyu = core_kernel_text_my((unsigned long)sys_call_table_addr[syscall_number]);
        if(!malyu){
            printk("syscall was intercepted .... %d\n", syscall_number);
            break;
        }
    }
    printk("syscall was not intercepted \n");
    return 0;
}

static void __exit custom_exit(void){
    printk(KERN_INFO "exiting ...\n");
}

module_init(custom_init);
module_exit(custom_exit);