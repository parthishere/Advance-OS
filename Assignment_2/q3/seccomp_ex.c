/*********************************************************************
* Advanced OS Assignment 2
* File: seccomp_examples.c
* 
* Purpose:
*     Demonstrates various seccomp (secure computing mode) implementations
*     including basic prctl(), libseccomp, and raw BPF filters. Shows
*     different methods of system call filtering and sandboxing.
*
* Features:
*     - Basic seccomp strict mode
*     - Libseccomp API usage
*     - Raw BPF filter implementation 
*     - Advanced rules with argument filtering
*     - Seccomp with logging
*
* Author: Parth Thakkar
* Date: 8/11/24
*
* Copyright (c) 2024 Parth Thakkar
* All rights reserved.
*********************************************************************/


/* Required header files */
#include <stdio.h>           /* Standard I/O functions */
#include <stdlib.h>          /* Standard library functions */
#include <unistd.h>         /* UNIX standard functions */ 
#include <seccomp.h>         /* Seccomp filtering API */
#include <linux/seccomp.h>   /* Seccomp definitions */
#include <linux/filter.h>    /* BPF filter definitions */
#include <linux/audit.h>     /* Audit architecture definitions */
#include <sys/prctl.h>       /* Process control functions */
#include <sys/syscall.h>     /* System call numbers */
#include <errno.h>           /* Error codes */
#include <stddef.h>          /* Standard type definitions */

/**
* @function: basic_seccomp
* 
* @purpose: Demonstrates basic seccomp implementation using prctl()
*          in strict mode which only allows read/write/exit syscalls
* 
* @note: This is the simplest form of seccomp filtering
*       Once enabled, only basic syscalls are allowed
*/
// Basic Seccomp Example
void basic_seccomp(void) {
    // Enable strict seccomp mode
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT) == -1) {
        perror("prctl(SECCOMP_MODE_STRICT)");
        exit(EXIT_FAILURE);
    }
    
    printf("Strict mode enabled - only read/write/exit allowed\n");
    
    // These will work
    write(STDOUT_FILENO, "Hello\n", 6);
    exit(EXIT_SUCCESS);
    
    // These would fail
    // fork();
    // execve("/bin/sh", NULL, NULL);
}

/**
* @function: libseccomp_example
* 
* @purpose: Shows seccomp implementation using libseccomp API
*          Provides more flexible filtering than basic mode
* 
* @note: Uses higher-level API for filter creation
*/
// Seccomp with libseccomp
void libseccomp_example(void) {
    scmp_filter_ctx ctx;
    
    // Initialize the filter context
    ctx = seccomp_init(SCMP_ACT_KILL); // Default action: kill
    if (ctx == NULL) {
        perror("seccomp_init");
        exit(EXIT_FAILURE);
    }
    
    // Allow specific syscalls
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    
    // Load the filter
    if (seccomp_load(ctx) < 0) {
        perror("seccomp_load");
        exit(EXIT_FAILURE);
    }
    
    // Free the context
    seccomp_release(ctx);
}

// Advanced Seccomp with Complex Rules
void advanced_seccomp(void) {
    scmp_filter_ctx ctx;
    
    ctx = seccomp_init(SCMP_ACT_KILL);
    if (ctx == NULL) {
        perror("seccomp_init");
        exit(EXIT_FAILURE);
    }
    
    // Allow open only for specific files

    
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1,
                     SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)"/allowed/path"));
    
    // Allow write only to stdout/stderr
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
                     SCMP_A0(SCMP_CMP_EQ, STDOUT_FILENO));
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1,
                     SCMP_A0(SCMP_CMP_EQ, STDERR_FILENO));
    
    // Load and release
    if (seccomp_load(ctx) < 0) {
        perror("seccomp_load");
        exit(EXIT_FAILURE);
    }
    seccomp_release(ctx);
}

/**
* @function: bpf_seccomp
* 
* @purpose: Implements seccomp filtering using raw BPF instructions
*          Provides lowest-level control over filtering
* 
* @note: Requires understanding of BPF instruction set
*/
// Seccomp with BPF filters directly
void bpf_seccomp(void) {

    
    struct sock_filter filter[] = {
        // Load architecture
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, 
                (offsetof(struct seccomp_data, arch))),
        
        // Check architecture
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
        
        // Load syscall number
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS,
                (offsetof(struct seccomp_data, nr))),
        
        // Allow specific syscalls
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_write, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
        
        // Kill process if syscall not allowed
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL)
    };
    
    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
        .filter = filter,
    };
    
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl(PR_SET_NO_NEW_PRIVS)");
        exit(EXIT_FAILURE);
    }
    
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
        perror("prctl(PR_SET_SECCOMP)");
        exit(EXIT_FAILURE);
    }
}

// Complete Example with Logging
void seccomp_with_logging(void) {
    scmp_filter_ctx ctx;
    
    // Initialize with logging action
    ctx = seccomp_init(SCMP_ACT_LOG);
    if (ctx == NULL) {
        perror("seccomp_init");
        exit(EXIT_FAILURE);
    }
    
    // Set up rules
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    
    // Add rule with argument filtering
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 1,
                     SCMP_A2(SCMP_CMP_MASKED_EQ, O_WRONLY, 0));
    
    // Export BPF program (optional, for debugging)
    seccomp_export_bpf(ctx, STDOUT_FILENO);
    
    // Load the filter
    if (seccomp_load(ctx) < 0) {
        perror("seccomp_load");
        exit(EXIT_FAILURE);
    }
    
    seccomp_release(ctx);
}

/**
* @function: main
* 
* @purpose: Program entry point demonstrating various seccomp 
*          implementations
* 
* @returns: 0 on success, non-zero on failure
*/
int main(void) {
    printf("Starting seccomp examples...\n");
    
    // Choose one of the examples:
    // basic_seccomp();
    // libseccomp_example();
    advanced_seccomp();
    // bpf_seccomp();
    // seccomp_with_logging();
    
    printf("Seccomp rules applied. Testing allowed operations...\n");
    
    // Test allowed operations
    write(STDOUT_FILENO, "This should work\n", 16);
    
    // Test blocked operations
    printf("Trying to execute blocked syscall...\n");
    execve("/bin/sh", NULL, NULL); // This should fail
    
    return 0;
}