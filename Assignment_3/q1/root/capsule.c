/*********************************************************************
 * Advanced OS Assignment 2
 * File: container_implementation.c
 *
 * Purpose:
 *     Implements a container system using Linux namespaces and cgroups
 *     for process isolation and resource control. Features filesystem
 *     isolation, process namespace separation, and resource limiting.
 *
 * Author: Parth Thakkar
 * Date: 8/11/24
 *
 * Copyright (c) 2024 Parth Thakkar
 * All rights reserved.
 *********************************************************************/

#include "src/debug_print.h"
#include "src/zip_utility.h"
#include "src/cleanup.h"
#include "src/child_function.h"
#include "src/networks.h"
#include "src/cgroups.h"
#include "src/mounts.h"

/* Constants and Macros */
/* Define stack size for clone operation (1MB) */
#define STACK_SIZE (1024 * 1024)

/* Default mount directory */
#define MOUNT_DIR "."





int main(int argc, char *argv[])
{

    INFO_PRINT("Capsule initialization started");
    DEBUG_PRINT("Process ID: %d", getpid());

    if (argc != 2)
    {
        printf("Usage: ./capsule <zipfile>\n");
        return 1;
    }
    DEBUG_PRINT("Input zip file: %s", argv[1]);

    struct child_config ch_config = {
        .zip_path = argv[1],
        .hostname = "capsule",
        .mount_dir = MOUNT_DIR};

    // chdir(MOUNT_DIR); // change dir
    // chroot(MOUNT_DIR); // change root

    int err = 0;

    if (extract_zip(argv[1], MOUNT_DIR) == -1)
    {
        fprintf(stderr, "Failed to extract zip file\n");
        exit(EXIT_FAILURE);
    }

    // Set resource limits
    // Allocate stack for child
    DEBUG_PRINT("Allocating stack (size: %d bytes)", STACK_SIZE);
    char *stack = malloc(STACK_SIZE);
    if (!stack)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    set_resource_limits();

    // Create child process with new namespaces
    int flags = CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC |
                CLONE_NEWPID | CLONE_NEWNET;
    DEBUG_PRINT("Namespace flags configured: 0x%x", flags);
    INFO_PRINT("Creating new namespaces");
    DEBUG_PRINT("  Mount namespace (CLONE_NEWNS)");
    DEBUG_PRINT("  UTS namespace (CLONE_NEWUTS)");
    DEBUG_PRINT("  IPC namespace (CLONE_NEWIPC)");
    DEBUG_PRINT("  PID namespace (CLONE_NEWPID)");
    DEBUG_PRINT("  Network namespace (CLONE_NEWNET)");
    DEBUG_PRINT("  User namespace (CLONE_NEWUSER)");

    pid_t child_pid = clone(child_function, stack + STACK_SIZE, flags | SIGCHLD, &ch_config);
    if (child_pid == -1)
    {
        perror("clone");
        exit(EXIT_FAILURE);
    }

    char path[1024];
    char pid_str[32];
    int fd;

    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/cgroup.procs", "mygroup");
    snprintf(pid_str, sizeof(pid_str), "%d", child_pid);

    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        perror("Failed to open tasks file");
        return -1;
    }

    if (write(fd, pid_str, strlen(pid_str)) == -1)
    {
        perror("Failed to add process to cgroup");
        close(fd);
        return -1;
    }

    close(fd);

    INFO_PRINT("Child process created successfully (PID: %d)", child_pid);

    // Wait for child
    if (waitpid(child_pid, NULL, 0) == -1)
    {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    // Cleanup
    free(stack);
    INFO_PRINT("Capsule execution completed successfully");

    // delete folder
    cleanup_resources(MOUNT_DIR, "mygroup");
    // system("rm -rf /test_progs")

    return EXIT_SUCCESS;
}