#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>

// NOTE: Jprobes is now a deprecated feature. Users should migrate to other tracing features or use older kernels.

/**
 * pre_handler - Kprobe pre-handler function
 * @p: Pointer to the kprobe
 * @regs: Pointer to the processor registers at the time of the probe hit
 *
 * This function is called before the probed instruction is executed.
 * It prints debug information including register contents and stack trace.
 *
 * Return: Always returns 0 to continue normal execution
 */
static int __kprobes pre_handler(struct kprobe *p, struct pt_regs *regs){
    printk(KERN_INFO "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printk(KERN_INFO "Debug Info LKM: Entry Callback\n");

    if (regs) {
        // Print the contents of all general-purpose registers
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

    // Print the top 16 entries of the stack
    unsigned long * stack = (unsigned long *)regs->sp;
    printk(KERN_INFO "Stack:\n");
    for (int i = 0; i < 16; i++) {
        printk(KERN_INFO "[%d]: 0x%lx\n", i, stack[i]);
    }
    printk(KERN_INFO "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");

    // Print a stack backtrace
    dump_stack();
    return 0;
}

/**
 * post_handler - Kprobe post-handler function
 * @p: Pointer to the kprobe
 * @regs: Pointer to the processor registers after the probed instruction is executed
 * @flags: Flags indicating the context of the handler
 *
 * This function is called after the probed instruction is executed.
 * It prints debug information similar to the pre-handler.
 */
static void __kprobes post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags){
    printk(KERN_INFO "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    printk(KERN_INFO "Debug Info LKM: Return Callback\n");
 
    if (regs) {
        // Print the contents of all general-purpose registers
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

    // Print the top 16 entries of the stack
    unsigned long * stack = (unsigned long *)regs->sp;
    printk(KERN_INFO "Stack:\n");
    for (int i = 0; i < 16; i++) {
        printk(KERN_INFO "[%d]: 0x%lx\n", i, stack[i]);
    }
    printk(KERN_INFO "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
}

// Structure of kprobe (commented out for reference)
// struct kprobe {
// 	struct hlist_node hlist;
// 	struct list_head list;
// 	unsigned long nmissed;
// 	kprobe_opcode_t *addr;
// 	const char *symbol_name;
// 	unsigned int offset;
// 	kprobe_pre_handler_t pre_handler;
// 	kprobe_post_handler_t post_handler;
// 	kprobe_opcode_t opcode;
// 	struct arch_specific_insn ainsn;
// 	u32 flags;
// };

// Define the kprobe structure
static struct kprobe my_kprobe = {
    // .symbol_name	= "__x64_sys_write", // for write system call
    .symbol_name	= "kernel_clone", // for fork system call
    .pre_handler    = pre_handler,
    .post_handler   = post_handler,
};

/**
 * kprobe_init - Module initialization function
 *
 * This function is called when the module is loaded. It registers the kprobe.
 *
 * Return: 0 on success, negative error code on failure
 */
static int __init kprobe_init(void)
{
    int ret;

    // Register the kprobe
    ret = register_kprobe(&my_kprobe);
    if (ret < 0) {
        printk(KERN_INFO "register_kprobe failed, returned %d\n", ret);
        return -1;
    }
    printk(KERN_INFO "Planted kprobe at %p, handler addr %p and %p\n",
           my_kprobe.addr, my_kprobe.pre_handler, my_kprobe.post_handler);

    return 0;
}

/**
 * kprobe_exit - Module cleanup function
 *
 * This function is called when the module is unloaded. It unregisters the kprobe.
 */
static void __exit kprobe_exit(void){
    unregister_kprobe(&my_kprobe);
    printk(KERN_INFO "Kprobe unregistered\n");
}

// Specify the init and exit functions
module_init(kprobe_init)
module_exit(kprobe_exit)

// Module metadata
MODULE_AUTHOR("Parth Thakkar");
MODULE_DESCRIPTION("Rootkit Kprobe");
MODULE_LICENSE("GPL");

// Note: ffffffff8e8e5400 T ksys_write
// kallsyms_lookup_name(const char *name) can be used to find symbol addresses