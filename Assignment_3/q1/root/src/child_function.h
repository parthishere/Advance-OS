#ifndef __CHILD_FUNCTION_H__
#define __CHILD_FUNCTION_H__

#include "debug_print.h"
#include "mounts.h"
#include "networks.h"




//wrapper for pivot root syscall
int pivot_root(char *a,char *b)
{
	if (mount(a,a,"bind",MS_BIND | MS_REC,"")<0){
		printf("error mount: %s\n",strerror(errno));
	}
	if (mkdir(b,0755) <0){
		printf("error mkdir %s\n",strerror(errno));
	}
	printf("pivot setup ok\n");

	return syscall(SYS_pivot_root,a,b);
}

int child_function(void *arg)
{

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

    setup_container_network(&config->network_config);

    if (pivot_root(config->mount_dir,".old")<0){
		printf("error pivot: %s\n",strerror(errno));
	}

    // Change root
    DEBUG_PRINT("Changing root to: %s", config->mount_dir);
    if (chroot(config->mount_dir) == -1)
    {
        perror("chroot");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") == -1)
    {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    DEBUG_PRINT("Configuring mount namespace");
    if (setup_mounts() == -1)
    {
        perror("setup mounts");
        exit(EXIT_FAILURE);
    }

    // Execute shell
    INFO_PRINT("Launching shell");
    char *args[] = {"/bin/bash", NULL};
    execv("/bin/bash", args);
    perror("execv");
    return EXIT_FAILURE;
}

#endif