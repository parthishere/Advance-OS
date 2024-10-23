#include <linux/kernel.h>
#include <asm/unistd.h> // contains syscall numbers
#include <linux/kprobes.h>
#include <asm/pgtable_types.h>
#include <linux/kallsyms.h>
#include <linux/fs.h>
#include <linux/file.h>


typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
kallsyms_lookup_name_t kallsyms_lookup_name_my;
unsigned int (*core_kernel_text_my)(unsigned long addr);

void **sys_call_table_addr;


// Kprobe structure for finding kallsyms_lookup_name
static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name"
};


asmlinkage long sys_parth(void){
    printk("HEY! sys_parth called !\n");
    /*call the original sys_exit*/

    int ret = 0;
    printk(KERN_INFO "Init\n");

    
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

    core_kernel_text_my = kallsyms_lookup_name_my("core_kernel_text");
    int syscall_number;
    for (syscall_number=0; syscall_number< 172; syscall_number++){
        int malyu = core_kernel_text_my((unsigned long)sys_call_table_addr[syscall_number]);
        if(!malyu){
            printk("syscall was intercepted .... %d\n", syscall_number);
            return 1;
        }
    }
    printk("syscall was not intercepted \n");
    return (-1);
}