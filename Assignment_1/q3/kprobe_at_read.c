#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>


// Jprobes is now a deprecated feature. People who are depending on it should
// migrate to other tracing features or use older kernels. Please consider to
// migrate your tool to one of the following options:

static int __kprobes pre_handler(struct kprobe *p, struct pt_regs *regs){

    printk(KERN_INFO "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printk(KERN_INFO "Debug Info LKM: Entry Callback\n");

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
    for (int i = 0; i < 16; i++) {
        printk(KERN_INFO "[%d]: 0x%lx\n", i, stack[i]);
    }
    printk(KERN_INFO "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

    /* A dump_stack() here will give a stack backtrace */
	dump_stack();
    return 0;

}

static void __kprobes post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags){

	printk(KERN_INFO "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printk(KERN_INFO "Debug Info LKM: Return Callback\n");
 

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
    for (int i = 0; i < 16; i++) {
        printk(KERN_INFO "[%d]: 0x%lx\n", i, stack[i]);
    }
    printk(KERN_INFO "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

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

static struct kprobe my_kprobe = {
	
	// .symbol_name	= "__x64_sys_write", // for write
	.symbol_name	= "kernel_clone", // for fork
	.pre_handler     = pre_handler,
	.post_handler    = post_handler,
	
};

static int __init kprobe_init(void)
{
    // unsigned long * ksys_write = (unsigned long *)kallsyms_lookup_name("ksys_write");
    int ret;

	ret = register_kprobe(&my_kprobe);
	if (ret < 0) {
		printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);
		return -1;
	}
	printk(KERN_INFO "Planted kprobe at %p, handler addr %p and %p\n",
	       my_kprobe.addr, my_kprobe.pre_handler, my_kprobe.pre_handler);

    return 0;
}

static void __exit kprobe_exit(void){
    unregister_kprobe(&my_kprobe);
    printk(KERN_INFO "Planted kprobe at %p, handler addr %p and %p\n",
	       my_kprobe.addr, my_kprobe.pre_handler, my_kprobe.pre_handler);
}

module_init(kprobe_init)
module_exit(kprobe_exit)


MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("Rootkit Kprobe");
MODULE_LICENSE("GPL");

//ffffffff8e8e5400 T ksys_write
// kallsyms_lookup_name(const char *name) Just 






