#ifndef __CGROUPS_H__
#define __CGROUPS_H__

#include "debug_print.h"


/* Utility macro to convert MB to bytes */
#define MB_TO_BYTES(x) (x * 1024 * 1024)



/**
 * @enum subsystems_t
 * @brief Enumeration of supported cgroup subsystems
 *
 * Defines the various cgroup controllers that can be used for
 * resource management
 */
typedef enum
{
    CPU,    /* CPU time and scheduling */
    CPUSET, /* CPU and memory node assignments */
    IO,     /* Block I/O control */
    MEMORY, /* Memory usage control */
} subsystems_t;

/**
 * @function subsystem_name
 * @brief Converts subsystem enumeration to string representation
 *
 * @param ss [in] subsystems_t - The subsystem enumeration value
 * @return const char* - String representation of the subsystem
 *
 * @note Returns "cpu" as default for unhandled cases
 * @example
 *    const char *name = subsystem_name(MEMORY); // Returns "memory"
 */
const char *subsystem_name(subsystems_t ss)
{
    switch (ss)
    {
    case CPU:
        return "cpu"; /* CPU controller */
    case CPUSET:
        return "cpuset"; /* CPU set controller */
    case MEMORY:
        return "memory"; /* Memory controller */
    default:
        return "cpu"; /* Default fallback */
    }
}

/**
 * @function: create_cgroup
 *
 * @purpose: Creates cgroup for resource control
 *
 * @param group: Cgroup name
 *
 * @returns: 0 on success, -1 on failure
 *
 * @note: Sets up CPU, memory, and I/O controllers
 */
// Function to create a new cgroup
int create_cgroup(const char *group)
{
    const char *controllers = "+cpuset +cpu +io +memory +pids +rdma";
    char path[1024];
    char pid_str[32];
    int fd;

    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/", group);
    DEBUG_PRINT("Creating cgroup: group=%s", path);

    if (mkdir(path, 0755) < 0)
    {
        // errno is global var // bad thing to use although
        if (errno == EEXIST)
        {
            WARN_PRINT("Group already exists: %s", path);
        }
        else
        {
            ERROR_PRINT("Failed to create group: %s (errno=%d: %s)",
                        path, errno, strerror(errno));
            return -1;
        }
    }
    INFO_PRINT("Successfully created cgroup: %s", path);
    // memory.max

    snprintf(path, sizeof(path), "/sys/fs/cgroup/cgroup.subtree_control");

    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        ERROR_PRINT("Failed to open tasks file");
        return -1;
    }

    if (write(fd, controllers, strlen(controllers)) == -1)
    {
        ERROR_PRINT("Failed to write controllers  process to cgroup");
        close(fd);
        return -1;
    }
    close(fd);

    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/cgroup.procs", group);
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());

    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        ERROR_PRINT("Failed to open tasks file");
        return -1;
    }

    if (write(fd, pid_str, strlen(pid_str)) == -1)
    {
        ERROR_PRINT("Failed to add process to cgroup");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

/**
 * @function remove_cgroup
 * @brief Removes a specified control group
 *
 * This function removes a cgroup directory from the cgroup filesystem.
 * The cgroup must be empty (no processes) before it can be removed.
 *
 * @param subsystem [in] const char* - The subsystem type (unused in this implementation)
 * @param group [in] const char* - Name of the cgroup to remove
 * @return int - 0 on success, -1 on failure
 *
 * @note Attempts to remove directory at /sys/fs/cgroup/<group>
 * @warning All processes must be removed from the cgroup before deletion
 */
int remove_cgroup(const char *subsystem, const char *group)
{
    char path[1024];

    /* Construct path to cgroup directory */
    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s", group);

    /* Attempt to remove the cgroup directory */
    if (rmdir(path) == -1)
    {
        perror("Failed to remove cgroup");
        return -1;
    }
    return 0;
}
// Function to set memory limit
int set_memory_limit(const char *group, unsigned long limit_in_bytes)
{
    char path[1024];
    char value[32];
    int fd;

    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/memory.max", group);
    snprintf(value, sizeof(value), "%lu", limit_in_bytes);
    printf("Memory limit in bytes set %lu in controller memory in file %s\n ", limit_in_bytes, path);

    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        perror("Failed to open memory limit file");
        return -1;
    }

    if (write(fd, value, strlen(value)) == -1)
    {
        perror("Failed to set memory limit");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

// Function to set memory limit
int set_cpu_limit(const char *group, unsigned long limit_in_percent)
{
    char path[1024];
    char value[64];
    int fd;
    //     # this allows the cgroup to only use 5% of a CPU
    // echo '5000 100000' > /sys/fs/cgroup/sandbox/cpu.max

    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/cpu.max", group);
    snprintf(value, sizeof(value), "%ld 100000", (limit_in_percent * 100000 / 100));
    printf("CPU limit set %lu, in controller cpu in file %s\n ", limit_in_percent, path);

    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        perror("Failed to open cpu limit file");
        return -1;
    }

    if (write(fd, value, strlen(value)) == -1)
    {
        perror("Failed to set cpu limit");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

// Function to set memory limit
int set_io_limit(const char *group, unsigned long limit_mb_per_sec)
{
    char path[1024];
    char value[32];
    int fd;
    int major = 259, minor = 0;

    // blkio.throttle.read_bps_device, blkio.throttle.write_bps_device
    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/io.max", group);
    snprintf(value, sizeof(value), "%d:%d rbps=%lu wbps=%lu", major, minor, (limit_mb_per_sec * 1024 * 1024), (limit_mb_per_sec * 1024 * 1024));

    printf("IO limit set %lu, in controller blkio in file %s\n ", limit_mb_per_sec, path);

    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        perror("Failed to open memory limit file");
        return -1;
    }

    if (write(fd, value, strlen(value)) == -1)
    {
        perror("Failed to set memory limit");
        close(fd);
        return -1;
    }
    close(fd);

    // snprintf(path, sizeof(path), "/sys/fs/cgroup/blkio/%s/blkio.throttle.write_bps_device", group);
    // snprintf(value, sizeof(value), "%lu", limit_mb_per_sec);
    // printf("IO limit set %lu, in controller blkio in file %s\n ", limit_mb_per_sec, path);
    // fd = open(path, O_WRONLY | O_CREAT);
    // if (fd == -1)
    // {
    //     perror("Failed to open memory limit file");
    //     return -1;
    // }

    // if (write(fd, value, strlen(value)) == -1)
    // {
    //     perror("Failed to set memory limit");
    //     close(fd);
    //     return -1;
    // }
    // close(fd);
    return 0;
}

void set_resource_limits()
{
    const char *group_name = "mygroup";

    DEBUG_PRINT("Setting up resource limits for group: %s", group_name);
    INFO_PRINT("Creating cgroup controllers");
    create_cgroup(group_name);
    DEBUG_PRINT("Setting CPU limit to 5%%");
    set_cpu_limit(group_name, 5);
    DEBUG_PRINT("Setting memory limit to 2MB");
    set_memory_limit(group_name, MB_TO_BYTES(2));
    DEBUG_PRINT("Setting I/O limit to 1MB/s");
    set_io_limit(group_name, 1);
    INFO_PRINT("Resource limits setup completed");
    INFO_PRINT("Cross checking resources \n");
}


#endif