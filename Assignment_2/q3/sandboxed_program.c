/*********************************************************************
 * Advanced OS Assignment 2
 * File: seccomp_sandbox.c
 * 
 * Purpose: 
 *     Implements a secure computing (seccomp) sandbox using three 
 *     different approaches: prctl, libseccomp, and eBPF filters.
 *     Demonstrates system call filtering and process isolation.
 * 
 * Features:
 *     - Multiple seccomp implementation methods
 *     - System call filtering
 *     - File operation restrictions
 *     - Process isolation
 * 
 * Author: Parth Thakkar
 * Date: 8/11/24
 * 
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *********************************************************************/

/* Required header files */
#include <stdio.h>           /* Standard I/O operations */
#include <linux/seccomp.h>   /* Seccomp definitions */
#include <seccomp.h>         /* Libseccomp API */
#include <linux/prctl.h>     /* Process control operations */
#include <sys/prctl.h>       /* prctl system call */
#include <unistd.h>          /* UNIX standard functions */
#include <fcntl.h>           /* File control operations */
#include <stdlib.h>          /* Standard library */
#include <linux/filter.h>    /* BPF filter definitions */
#include <linux/audit.h>     /* Audit architecture definitions */
#include <stddef.h>          /* Standard definitions */

// // Template for creating security rules:
// #define MY_SECURITY_CHECK(what_to_check)
//     LOAD_VALUE(what_to_check),
//     CHECK_CONDITION(what_to_check),
//     ACTION_IF_FAILS

// BPF_LD+BPF_W+BPF_ABS  // Load a 32-bit word (W) from absolute offset (ABS)
// arch_nr                // Offset in seccomp_data where architecture is stored
// BPF_JMP+BPF_JEQ+BPF_K // Jump if equal (JEQ) to constant (K)
// ARCH_NR               // The architecture we want (e.g., AUDIT_ARCH_X86_64)
// 1, 0                  // If equal skip 1 instruction, if not skip 0
// SECCOMP_RET_KILL      // Return value that kills the process

// /**
//  * struct seccomp_data - the format the BPF program executes over.
//  * @nr: the system call number
//  * @arch: indicates system call convention as an AUDIT_ARCH_* value
//  *        as defined in <linux/audit.h>.
//  * @instruction_pointer: at the time of the system call.
//  * @args: up to 6 system call arguments always stored as 64-bit values
//  *        regardless of the architecture.
//  */
// struct seccomp_data {
// 	int nr;
// 	__u32 arch;
// 	__u64 instruction_pointer;
// 	__u64 args[6];
// };


int part_to_run, type_to_run;
scmp_filter_ctx ctx;

/**
 * @function: graceful_exit
 * 
 * @purpose: Performs cleanup and exits the program
 * 
 * @param rc: Exit status code
 * 
 * @note: Releases seccomp context before exit
 */
void graceful_exit(int rc)
{
    seccomp_release(ctx);
    exit(rc);
}

/**
 * @function: normal_seccomp
 * 
 * @purpose: Implements strict seccomp mode using prctl
 *          Only allows read, write, and exit system calls
 * 
 * @note: Uses SECCOMP_MODE_STRICT for basic filtering
 */
void normal_seccomp()
{
    printf("Strict mode enabled - only read/write/exit allowed\n");

    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_STRICT) != NULL)
    {
        perror("prctl(SECCOMP_MODE_STRICT)");
        exit(EXIT_FAILURE);
    }
}

/**
 * @function: libseccomp_setup
 * 
 * @purpose: Implements seccomp filtering using libseccomp
 * 
 * @param fd: File descriptor to allow operations on
 * 
 * @note: Sets up fine-grained syscall filtering rules
 */
void libseccomp_setup(int fd)
{
    int rc;
    if ((ctx = seccomp_init(SCMP_ACT_KILL)) == NULL)
    {
        graceful_exit(1);
    }
    printf("Strict mode enabled - only read/write/exit allowed\n");

    /* Add allowed system calls to the BPF program */
    // int seccomp_rule_add(scmp_filter_ctx ctx, uint32_t action, int syscall, unsigned int arg_cnt, ...);
    // seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(open), 1, SCMP_A2(SCMP_CMP_EQ, O_CREAT)); // kill if we create a file

    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1, SCMP_A0(SCMP_CMP_EQ, fd)); // SCMP_A0(enum scmp_compare op, ...);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 1, SCMP_A0(SCMP_CMP_EQ, 1));

    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);

    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 1, SCMP_A0(SCMP_CMP_EQ, fd));

    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 1, SCMP_A0(SCMP_CMP_EQ, fd));

    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);

    /* Load the BPF program for the current context into the kernel */
    if ((rc = seccomp_load(ctx)) != 0)
    {
        graceful_exit(1);
    }
}


/**
 * @function: ebpf_seccomp
 * 
 * @purpose: Implements seccomp filtering using eBPF
 * 
 * @note: Uses BPF instructions to create custom filter
 */
void ebpf_seccomp()
{
    // What each part means:
    // BPF_LD+BPF_W+BPF_ABS  // Load a 32-bit word (W) from absolute offset (ABS)
    // arch_nr                // Offset in seccomp_data where architecture is stored
    // BPF_JMP+BPF_JEQ+BPF_K // Jump if equal (JEQ) to constant (K)
    // ARCH_NR               // The architecture we want (e.g., AUDIT_ARCH_X86_64)
    // 1, 0                  // If equal skip 1 instruction, if not skip 0
    // SECCOMP_RET_KILL      // Return value that kills the process

    //     #define		BPF_ADD		0x00
    // #define		BPF_SUB		0x10
    // #define		BPF_MUL		0x20
    // #define		BPF_DIV		0x30
    // #define		BPF_OR		0x40
    // #define		BPF_AND		0x50
    // #define		BPF_LSH		0x60
    // #define		BPF_RSH		0x70
    // #define		BPF_NEG		0x80
    // #define		BPF_MOD		0x90
    // #define		BPF_XOR		0xa0

    // #define		BPF_JA		0x00
    // #define		BPF_JEQ		0x10
    // #define		BPF_JGT		0x20
    // #define		BPF_JGE		0x30
    // #define		BPF_JSET        0x40
    struct sock_filter filter[] = {
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, arch))),

        // Check architecture
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, nr))),

        // Allow specific syscalls
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_write, 0, 3),
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof(struct seccomp_data, args[2]))), // LOADING SIZE
        BPF_JUMP(BPF_JMP | BPF_JGE | BPF_K, 100, 0, 1),                               // IF TRUE SKIP ZERO ELSE SKIP ONE
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_read, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_lseek, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_close, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit_group, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),

        // Kill process if syscall not allowed
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL)

    };

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter)/sizeof(filter[0])),
        .filter = filter,
    };

    // execve(_) will not grant any new privileges
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl(PR_SET_NO_NEW_PRIVS)");
        exit(EXIT_FAILURE);
    }
    
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog)) {
        perror("prctl(PR_SET_SECCOMP)");
        exit(EXIT_FAILURE);
    }
}

/**
 * @function: main
 * 
 * @purpose: Program entry point, handles command line arguments
 *          and sets up appropriate seccomp filtering
 * 
 * @param argc: Argument count
 * @param argv: Argument vector
 * 
 * @returns: 0 on success, -1 on error
 */

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Usage : ./sandboxed_program <part-of-program-to-run 1,2,3> <type of filter> %d\n", argc);
        printf(" 1:  unauthorize syscall, 2 create a new file, 3 open file not included in your jailed environmnet \n");
        printf(" 1: normal prctl, 2: lib-seccomp, 3: bpf-seccomp\n");
        return -1;
    }
    else
    {
        part_to_run = atoi(argv[1]);
        type_to_run = atoi(argv[2]);
    }
    int test_file_fd = open("test.txt", 0666);

    switch(type_to_run){
        case 1:
        printf("normal prctl\n");
            normal_seccomp();
            break;
        case 2:
        printf("libseccomp\n");
            libseccomp_setup(test_file_fd);
            break;
        case 3:
            printf("EBPF \n");
            ebpf_seccomp();
            break;
        default:
            perror("wrong choice");
            exit(EXIT_FAILURE);
    }


    char ch;
    while (read(test_file_fd, &ch, 1) != 0)
    {
        printf("%c", ch);
    }

    // fclose(test_file);
    lseek(test_file_fd, 0, SEEK_END);
    const char *log_entry = "simple append to a file \n";
    char buffer_to_write[103];
    int how_many_bytes_written = snprintf(buffer_to_write, "%s", log_entry);
    printf("%s -- %d \n", buffer_to_write, how_many_bytes_written);
    write(test_file_fd, buffer_to_write, 99); // << ==  CHANGE THIS VALUE TO > 100 

    // unauthorize syscall
    if (argv[3] == "1")
    {
        fork();
    }

    // creat a new file
    if (argv[3] == "2")
    {
        open("random.txt", O_CREAT | O_EXCL);
    }

    // open file not included in your jailed environmnet
    if (argv[3] == "2")
    {
        open("/etc/passwd", O_RDONLY);
    }
    
    close(test_file_fd);
    return 0;
}