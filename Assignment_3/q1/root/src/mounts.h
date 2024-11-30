#ifndef __MOUNTS_H__
#define __MOUNTS_H__

#include "debug_print.h"





int setup_mounts()
{

    
    // DEBUG_PRINT("Initializing mount namespace setup");
    // /* mount the sandbox on top of itself in our new namespace */
    // /* it will become our root filesystem */

    // // ok(mount, "tmpfs","/dev","tmpfs", MS_NOSUID | MS_STRICTATIME, NULL);
    
	
    if (mount("proc", "/proc", "proc", 0, NULL) == -1)
    {
        perror("mount proc");
        exit(EXIT_FAILURE);
    }
    INFO_PRINT("Successfully mounted proc filesystem");
    if (mount("tmpfs", "/dev", "tmpfs", 0, NULL) == -1)
    {
        perror("mount tmpfs");
        exit(EXIT_FAILURE);
    }
    INFO_PRINT("Successfully mounted etc filesystem");

    
    // ok(mount, "sysfs", "/sys", "sysfs", 0, NULL);

    // if (mount("sysfs", "/sys", "sysfs", MS_REC|MS_PRIVATE, NULL) == -1) {
    //     ERROR_PRINT("Failed to mount sysfs: %s", strerror(errno));
    //     return -1;
    // }
    // INFO_PRINT("Mounted sysfs");

    // system("mount | grep cgroup");
    // system("cat /proc/self/cgroup");
    // system("ls -l /sys/fs/cgroup/");
    // // // Mount cgroup
    // if (mount("cgroup2", "/sys/fs/cgroup", "cgroup2", 0, NULL) == -1) {

    //     ERROR_PRINT("Failed to mount cgroup: %s", strerror(errno));
    //     return -1;
    // }
    // INFO_PRINT("Mounted cgroup filesystem");

    return 1;
}


#endif