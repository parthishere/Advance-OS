#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

static asmlinkage long (*original_sys_read)(unsigned int fd, const char __user *buf, size_t count);

static asmlinkage long jopen_callback(unsigned int fd, const char __user *buf, size_t count){

    printk(KERN_INFO "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printk(KERN_INFO "Debug Info LKM: Callback\n");
    // Print clone flags
    printk(KERN_INFO "Clone flags: 0x%lx\n", clone_flags);

    if (regs) {
        printk(KERN_INFO "Registers:\n");
        printk(KERN_INFO "RAX: 0x%lx, RBX: 0x%lx, RCX: 0x%lx, RDX: 0x%lx\n",
               regs->ax, regs->bx, regs->cx, regs->dx);
        printk(KERN_INFO "RSI: 0x%lx, RDI: 0x%lx, RBP: 0x%lx, RSP: 0x%lx\n",
               regs->si, regs->di, regs->bp, regs->sp);
        printk(KERN_INFO "R8:  0x%lx, R9:  0x%lx, R10: 0x%lx, R11: 0x%lx\n",
               regs->r8, regs->r9, regs->r10, regs->r11);
        printk(KERN_INFO "R12: 0x%lx, R13: 0x%lx, R14: 0x%lx, R15: 0x%lx\n",
               regs->r12, regs->r13, regs->r14, regs->r15);
        printk(KERN_INFO "RIP: 0x%lx, EFLAGS: 0x%lx\n",
               regs->ip, regs->flags);
    }

    // Print stack (top 16 entries)
    unsigned long * stack = (unsigned long *)regs->sp;
    printk(KERN_INFO "Stack:\n");
    for (i = 0; i < 16 && !probe_kernel_address(&stack[i], stack[i]); i++) {
        printk(KERN_INFO "[%d]: 0x%lx\n", i, stack[i]);
    }
    printk(KERN_INFO "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

    jprobe_return();
    return 0;

}

	// struct hlist_node hlist; /* Link list to go through probes */
	// struct list_head list; /* list of kprobes for multi-handler support */
	// unsigned long nmissed; /*count the number of times this probe was temporarily disarmed */
	// kprobe_opcode_t *addr; /* location of the probe point */
	// const char *symbol_name; /* Allow user to indicate symbol name of the probe point */
	// unsigned int offset; /* Offset into the symbol */
	// kprobe_pre_handler_t pre_handler; /* Called before addr is executed. */
	// kprobe_post_handler_t post_handler; /* Called after addr is executed, unless... */
	// kprobe_opcode_t opcode; /* Saved opcode (which has been replaced with breakpoint) */
	// struct arch_specific_insn ainsn; /* copy of the original instruction */
	// u32 flags; /* Indicates various status flags. Protected by kprobe_mutex after this kprobe is registered. */

    // struct kprobe 	kp
    // void * 	entry

static struct jprobe my_jprobe = {
	.entry			= (kprobe_opcode_t *)jopen_callback,
	.kp = {
		.symbol_name	= "sys_write",
	},
};

static int __init jprobe_init(void)
{
    // unsigned long * ksys_write = (unsigned long *)kallsyms_lookup_name("ksys_write");
    int ret;

	ret = register_jprobe(&my_jprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	       my_jprobe.kp.addr, my_jprobe.entry);

    return 0;
}

static void __exit jprobe_exit(void){
    unregister_jprobe(&my_jprobe);
    printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)


MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("Rootkit Jprobe");
MODULE_LICENSE("GPL");

//ffffffff8e8e5400 T ksys_write
// kallsyms_lookup_name(const char *name) Just 