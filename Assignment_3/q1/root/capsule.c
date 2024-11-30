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

// sync primitive
// int checkpoint[2];

// //wrapper for pivot root syscall
// int pivot_root(char *a,char *b)
// {
// 	if (mount(a,a,"bind",MS_BIND | MS_REC,"")<0){
// 		printf("error mount: %s\n",strerror(errno));
// 	}
// 	if (mkdir(b,0755) <0){
// 		printf("error mkdir %s\n",strerror(errno));
// 	}
// 	printf("pivot setup ok\n");

// 	return syscall(SYS_pivot_root,a,b);
// }

int child_function(void *arg)
{

    // init sync primitive
//   close(checkpoint[1]);

    printf("New `net` Namespace:\n");
    system("ip link");

    pid_t pid = getpid();

    struct child_config *config = arg;

    INFO_PRINT("Child process initialized");
    DEBUG_PRINT("Configuration:");
    DEBUG_PRINT("  Hostname: %s", config->hostname);
    DEBUG_PRINT("  Mount directory: %s", config->mount_dir);

    DEBUG_PRINT("Setting hostname to: %s", config->hostname);
    if (sethostname(config->hostname, strlen(config->hostname)) == -1)
    {
        perror("sethostname");
        exit(EXIT_FAILURE);
    }
    //set new system info
	setdomainname(config->hostname, strlen(config->hostname));	


    // PID Namespace alone:
    // - Has new PIDs
    // - BUT can't see them properly
    // - Because /proc shows host view

    // Mount Namespace alone:
    // - Can mount new /proc
    // - BUT shows wrong PIDs
    // - Because using host PID view

    // Together:
    // - New PIDs (PID NS)
    // - Correct /proc view (Mount NS)
    // - Everything works properly!
    // Setup mounts

    // Change root
    DEBUG_PRINT("Changing root to: %s", config->mount_dir);
    // if (chroot(config->mount_dir) == -1)
    // {
    //     perror("chroot");
    //     exit(EXIT_FAILURE);
    // }

    // if (chdir("/") == -1)
    // {
    //     perror("chdir");
    //     exit(EXIT_FAILURE);
    // }

    DEBUG_PRINT("Configuring mount namespace");
    if (setup_mounts() == -1)
    {
        perror("setup mounts");
        exit(EXIT_FAILURE);
    }

    // char c;
    // // wait for network setup in parent
    // read(checkpoint[0], &c, 1);

    // setup network
    system("ip link set lo up");
    system("ip link set veth1 up");
    system("ip addr add 169.254.1.2/30 dev veth1");

    // Execute shell
    INFO_PRINT("Launching shell");
    char *args[] = {"/bin/bash", NULL};
    execv("/bin/bash", args);
    perror("execv");
    return EXIT_FAILURE;
}


int main(int argc, char *argv[])
{
   
    printf("Original `net` Namespace:\n");
    system("ip link");

    INFO_PRINT("Capsule initialization started");
    DEBUG_PRINT("Process ID: %d", getpid());

    if (argc != 3)
    {
        printf("Usage: ./capsule <zipfile> <CONTAINER_NO:1/2>\n");
        return 1;
    }
    DEBUG_PRINT("Input zip file: %s", argv[1]);


    network_config_t conf = {0}; 
    strncpy(conf.container_ip, ((strcmp(argv[2],"1") == 0) ? CONTAINER_ONE: CONTAINER_TWO), sizeof(conf.container_ip));
    strncpy(conf.container_mac, ((strcmp(argv[2],"1") == 0) ? CONTAINER_ONE_MAC: CONTAINER_TWO_MAC), sizeof(conf.container_mac));

    strncpy(conf.bridge_ip, BRIDGE_IP, sizeof(conf.bridge_ip));
    strncpy(conf.bridge_mac, BRIDGE_MAC, sizeof(conf.bridge_mac));
    strncpy(conf.bridge_name, BRIDGE_NAME, sizeof(conf.bridge_name));

    strncpy(conf.veth_bridge_pb_end, VETH_BRIDGE_PB_NAME, sizeof(conf.veth_bridge_pb_end));
    strncpy(conf.veth_pc_pb_end, VETH_PC_PB_NAME, sizeof(conf.veth_pc_pb_end));

    strncpy(conf.veth_container_cb_end, ((strcmp(argv[2],"1") == 0) ? VETH_CONTAINER_CB1_NAME : VETH_CONTAINER_CB2_NAME), sizeof(conf.veth_container_cb_end));
    strncpy(conf.veth_bridge_cb_end, ((strcmp(argv[2],"1") == 0) ? VETH_BRIDGE_CB1_NAME : VETH_BRIDGE_CB2_NAME), sizeof(conf.veth_bridge_cb_end));

    conf.pid = 0;

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

    // set resource limits 
    set_resource_limits();

    system("sudo mount --make-rprivate /");
    // Create child process with new namespaces
    int flags = CLONE_NEWNS | CLONE_NEWIPC | CLONE_NEWUTS | CLONE_NEWNET | CLONE_NEWPID | SIGCHLD ;

    DEBUG_PRINT("Namespace flags configured: 0x%x", flags);
    INFO_PRINT("Creating new namespaces");

	printf("starting...\n");
    pid_t child_pid = clone(child_function, stack + STACK_SIZE, flags, &ch_config);
    if (child_pid == -1)
    {
        perror("clone");
        exit(EXIT_FAILURE);
    }
    conf.pid = child_pid;
    INFO_PRINT("Child pid %d\n", child_pid);
    // conf.pid = 1;
    // initialize_networking(&conf);

    // init sync primitive
    // pipe(checkpoint);

    char* cmd;
    asprintf(&cmd, "ip link set veth1 netns %d", child_pid);
    system("ip link add veth0 type veth peer name veth1");
    system(cmd);
    system("ip link set veth0 up");
    system("ip link list");
    system("ip addr add 169.254.1.1/30 dev veth0");
    system("ip link list");


    // asprintf(&cmd, "ip netns exec %d ip link set lo up", child_pid);
    // system(cmd);

    // asprintf(&cmd, "ip netns exec %d ip link set veth1 up", child_pid);
    // system(cmd);

    // asprintf(&cmd, "ip netns exec %d ip addr add 169.254.1.2/30 dev veth1", child_pid);
    // system(cmd);
    free(cmd);

    // signal "done"
    // close(checkpoint[1]);


    // char path[1024];
    // char pid_str[32];
    // int fd;

    // snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/cgroup.procs", "mygroup");
    // snprintf(pid_str, sizeof(pid_str), "%d", child_pid);

    // fd = open(path, O_WRONLY);
    // if (fd == -1)
    // {
    //     perror("Failed to open tasks file");
    //     return -1;
    // }

    // if (write(fd, pid_str, strlen(pid_str)) == -1)
    // {
    //     perror("Failed to add process to cgroup");
    //     close(fd);
    //     return -1;
    // }

    // close(fd);

    INFO_PRINT("Child process created successfully (PID: %d)", child_pid);

    // Here, <pid> should be replaced by the process ID of the process in the child namespace as observed by the parent
   


    // Wait for child
    if (waitpid(child_pid, NULL, 0) == -1)
    {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    // Cleanup
    free(stack);
    INFO_PRINT("Capsule execution completed successfully");

    // delete folderMOUNT_DIR
    cleanup_resources(MOUNT_DIR, "mygroup");
    cleanup_networking(&conf);
    // system("rm -rf /test_progs")

    return EXIT_SUCCESS;
}


// #define _GNU_SOURCE
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/mount.h>
// #include <stdio.h>
// #include <sched.h>
// #include <signal.h>
// #include <unistd.h>
// #include <stdlib.h>

// #define STACK_SIZE (1024 * 1024)

// // sync primitive
// int checkpoint[2];

// static char child_stack[STACK_SIZE];
// char* const child_args[] = {
//   "/bin/bash",
//   NULL
// };

// int child_main(void* arg)
// {
//   char c;

//   // init sync primitive
//   close(checkpoint[1]);

//   // setup hostname
//   printf(" - [%5d] World !\n", getpid());
//   sethostname("In Namespace", 12);

//   // remount "/proc" to get accurate "top" && "ps" output
//   mount("proc", "/proc", "proc", 0, NULL);

//   // wait for network setup in parent
//   read(checkpoint[0], &c, 1);

//   // setup network
//   system("ip link set lo up");
//   system("ip link set veth1 up");
//   system("ip addr add 169.254.1.2/30 dev veth1");

//   execv(child_args[0], child_args);
//   printf("Ooops\n");
//   return 1;
// }

// int main()
// {
//   // init sync primitive
//   pipe(checkpoint);

//   printf(" - [%5d] Hello ?\n", getpid());

//   int child_pid = clone(child_main, child_stack+STACK_SIZE,
//       CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | SIGCHLD, NULL);

//   // further init: create a veth pair
//   char* cmd;
//   asprintf(&cmd, "ip link set veth1 netns %d", child_pid);
//   system("ip link add veth0 type veth peer name veth1");
//   system(cmd);
//   system("ip link set veth0 up");
//   system("ip addr add 169.254.1.1/30 dev veth0");
//   free(cmd);

//   // signal "done"
//   close(checkpoint[1]);

//   waitpid(child_pid, NULL, 0);
//   return 0;
// }