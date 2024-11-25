#ifndef __CLEANUP_H__
#define __CLEANUP_H__

#include "debug_print.h"
#include "cgroups.h"

/**
 * @function cleanup_resources
 * @brief Cleans up all resources, unmounts filesystems, and removes cgroups
 *
 * @param mount_dir Directory where filesystems were mounted
 * @param cgroup_name Name of the cgroup to remove
 * @return 0 on success, -1 on failure
 */
int cleanup_resources(const char *mount_dir, const char *cgroup_name)
{
    int status = 0;
    char command[1024];

    INFO_PRINT("Starting cleanup process");

    // 1. First unmount all procfs, sysfs, and other mounts
    DEBUG_PRINT("Unmounting proc filesystem");
    if (umount("/proc") == -1)
    {
        WARN_PRINT("Failed to unmount /proc: %s", strerror(errno));
        status = -1;
    }

    // 2. Remove processes from cgroup
    char path[1024];
    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/cgroup.procs", cgroup_name);

    DEBUG_PRINT("Migrating processes out of cgroup");
    int fd = open("/sys/fs/cgroup/cgroup.procs", O_WRONLY);
    if (fd != -1)
    {
        const char *pid = "0"; // Moving to root cgroup
        if (write(fd, pid, strlen(pid)) == -1)
        {
            WARN_PRINT("Failed to migrate processes from cgroup: %s", strerror(errno));
            status = -1;
        }
        close(fd);
    }

    // 3. Remove cgroup
    DEBUG_PRINT("Removing cgroup: %s", cgroup_name);
    if (remove_cgroup(NULL, cgroup_name) == -1)
    {
        WARN_PRINT("Failed to remove cgroup: %s", strerror(errno));
        status = -1;
    }

    // system("rm -rf test_progs");

    if (status == 0)
    {
        INFO_PRINT("Cleanup completed successfully");
    }
    else
    {
        ERROR_PRINT("Cleanup completed with some errors");
    }

    return status;
}

#endif