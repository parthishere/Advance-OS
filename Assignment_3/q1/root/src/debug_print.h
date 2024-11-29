#ifndef __DEBUG_PRINT_H__
#define __DEBUG_PRINT_H__


/* Define _GNU_SOURCE to enable Linux-specific features */
#define _GNU_SOURCE

/* System Headers */
#include <stdio.h>         /* Standard I/O operations */
#include <linux/seccomp.h> /* Secure computing mode */
#include <unistd.h>        /* UNIX standard functions */
#include <stdlib.h>        /* Standard library definitions */
#include <fcntl.h>         /* File control options */
#include <zip.h>           /* ZIP archive handling */
#include <sys/wait.h>      /* Process wait functions */
#include <sys/mount.h>     /* Mount operations */
#include <sys/stat.h>      /* File status and permissions */
#include <sys/types.h>     /* System data type definitions */
#include <linux/sched.h>   /* Scheduling parameters and clone flags */
#include <sched.h>         /* Process scheduling operations */
#include <sys/syscall.h>   /* System call numbers */
#include <unistd.h>        /* UNIX standard function definitions */
#include <signal.h>        /* Signal handling */
#include <string.h>        /* String operations */
#include <errno.h>         /* System error numbers */

#include <linux/capability.h>
#include <sys/types.h>

#define BRIDGE_NAME "br0"
#define BRIDGE_IP "10.0.0.1/24"
#define BRIDGE_MAC "AA:AA:AA:AA:00:01"

#define VETH_BRIDGE_PB_NAME "veth0"
#define VETH_PC_PB_NAME "veth-br-0"

#define VETH_CONTAINER_CB1_NAME "veth1"
#define VETH_BRIDGE_CB1_NAME "veth-br-1"

#define VETH_CONTAINER_CB2_NAME "veth2"
#define VETH_BRIDGE_CB2_NAME "veth-br-2"

#define CONTAINER_ONE "10.0.0.2/24"
#define CONTAINER_ONE_MAC "AA:AA:AA:AA:00:10"

#define CONTAINER_TWO "10.0.0.3/24"
#define CONTAINER_TWO_MAC "AA:AA:AA:AA:00:11"

/* Structure Definitions */


// Network configuration structure
typedef struct 
{
    char container_ip[100];
    char container_mac[18];
    char bridge_ip[16]; // fixed 
    char bridge_mac[18];
    char bridge_name[100]; // bridge name (br0)

    char veth_bridge_pb_end[100]; // bridge pc end (in bridge)
    char veth_pc_pb_end[100]; // bridge_pc_end (in pc)

    char veth_bridge_cb_end[100]; // interface bridge (veth-br0-1 pair)
    char veth_container_cb_end[100]; // other pair veth0
    int pid;
}network_config_t;


/**
 * @struct child_config
 * @brief Configuration structure for child process initialization
 *
 * Contains necessary parameters for setting up a containerized environment
 */
struct child_config
{
    char *zip_path;  /* Path to the container root filesystem ZIP */
    char *hostname;  /* Hostname for the container */
    char *mount_dir; /* Mount point for container filesystem */
    network_config_t network_config;
};


/* ANSI Color Codes for console output formatting */
#define RED "\x1b[31m"    /* Error messages */
#define GREEN "\x1b[32m"  /* Success/Info messages */
#define YELLOW "\x1b[33m" /* Warning messages */
#define BLUE "\x1b[34m"   /* Debug messages */
#define RESET "\x1b[0m"   /* Reset color formatting */

/* Debug Print Macros
 * These macros provide consistent formatting for different types of messages
 * Each includes process ID for better debugging in multi-process scenarios
 */
#define DEBUG_PRINT(fmt, ...) \
    printf(BLUE "[DEBUG][%d] " fmt RESET "\n", getpid(), ##__VA_ARGS__)

#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, RED "[ERROR][%d] " fmt RESET "\n", getpid(), ##__VA_ARGS__)

#define INFO_PRINT(fmt, ...) \
    printf(GREEN "[INFO][%d] " fmt RESET "\n", getpid(), ##__VA_ARGS__)

#define WARN_PRINT(fmt, ...) \
    printf(YELLOW "[WARN][%d] " fmt RESET "\n", getpid(), ##__VA_ARGS__)

static int checkreturn(int res, const char *name, int line)
{
    if (res >= 0)
        return res;
    fprintf(stderr, "continer.c:%d: error: %s() failed: r=%d errno=%d (%s)\n",
            line, name, res, errno, strerror(errno));
    exit(-1);
}

#define ok(fname, arg...) checkreturn(fname(arg), #fname, __LINE__)


#endif