/*********************************************************************
 * Advanced OS Assignment 2
 * File: userspace.c
 *
 * Purpose:
 *     This userspace program demonstrates the interaction with a custom
 *     system call (syscall 333) in Linux kernel. It serves as a test
 *     program to verify the implementation and functionality of the
 *     custom system call.
 *
 * Author: Parth Thakkar
 * Date: 8/11/24
 *
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *********************************************************************/

/* Required header files for system functionality */
#include <stdio.h>          /* Standard I/O operations */
#include <linux/kernel.h>   /* Kernel specific definitions */
#include <sys/syscall.h>    /* System call definitions and interfaces */
#include <unistd.h>         /* POSIX operating system API */

/**
 * @function: main
 *
 * @purpose: Entry point of the program that invokes a custom system call
 *          (syscall number 333) and displays its return value.
 *
 * @parameters: None
 *
 * @returns:
 *     - 0: Successful execution
 *     - Non-zero: Error occurred during execution
 *
 * @description: This function demonstrates the usage of syscall() to
 *              invoke a custom system call with number 333. It prints
 *              the return value from the system call to verify its
 *              successful execution.
 */
int main()
{
    /* Invoke system call 333 and store its return value */
    long int amma = syscall(333);

    /* Print the return value from the system call */
    printf("System call sys_hello returned %ld\n", amma);

    /* Return success status to the operating system */
    return 0;
}
