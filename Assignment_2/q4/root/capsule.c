#define _GNU_SOURCE
#include <stdio.h>
#include <linux/seccomp.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <zip.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/sched.h> /* Definition of struct clone_args */
#include <sched.h>       /* Definition of CLONE_* constants */
#include <sys/syscall.h> /* Definition of SYS_* constants */
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <errno.h> // for errno

#define STACK_SIZE (1024 * 1024)
#define MOUNT_DIR "."

#define MB_TO_BYTES(x) (x * 1024 * 1024)



#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"


// Debug print macros
#define DEBUG_PRINT(fmt, ...) \
    printf(BLUE "[DEBUG][%d] " fmt RESET "\n", getpid(), ##__VA_ARGS__)

#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, RED "[ERROR][%d] " fmt RESET "\n", getpid(), ##__VA_ARGS__)

#define INFO_PRINT(fmt, ...) \
    printf(GREEN "[INFO][%d] " fmt RESET "\n", getpid(), ##__VA_ARGS__)

#define WARN_PRINT(fmt, ...) \
    printf(YELLOW "[WARN][%d] " fmt RESET "\n", getpid(), ##__VA_ARGS__)

struct child_config
{
    char *zip_path;
    char *hostname;
    char *mount_dir;
};

typedef enum
{
    CPU,
    CPUSET,
    IO,
    MEMORY,
} subsystems_t;

const char *subsystem_name(subsystems_t ss)
{
    switch (ss)
    {
    case CPU:
        return "cpu";
    case CPUSET:
        return "cpuset";
    case MEMORY:
        return "memory";
    default:
        return "cpu";
    }
}




// Function to extract zip file
static int extract_zip(const char *zip_path, const char *target_dir)
{
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[4096];
    int err;

     DEBUG_PRINT("Starting zip extraction process");
    DEBUG_PRINT("Source: %s", zip_path);
    DEBUG_PRINT("Target: %s", target_dir);

    if ((za = zip_open(zip_path, 0, &err)) == NULL)
    {
        ERROR_PRINT("Failed to open zip file: %s (error code: %d)", zip_path, err);
        return -1;
    }

    INFO_PRINT("Successfully opened zip file with %d entries", zip_get_num_entries(za, 0));

    for (int i = 0; i < zip_get_num_entries(za, 0); i++)
    {
        // int zip_stat(zip_t *archive, const char *fname, zip_flags_t flags, zip_stat_t *sb);
        // int zip_stat_index(zip_t *archive, zip_uint64_t index, zip_flags_t flags, zip_stat_t *sb);
        // The zip_stat() function obtains information about the file named fname in archive. The flags argument specifies how the name lookup should be done. Its values are described in zip_name_locate(3). Also, ZIP_FL_UNCHANGED may be or'ed to it to request information about the original file in the archive, ignoring any changes made.
        // The zip_stat_index() function obtains information about the file at position index.
        // The sb argument is a pointer to a struct zip_stat (shown below), into which information about the file is placed.
        // struct zip_stat {
        //     zip_uint64_t valid;                 /* which fields have valid values */
        //     const char *name;                   /* name of the file */
        //     zip_uint64_t index;                 /* index within archive */
        //     zip_uint64_t size;                  /* size of file (uncompressed) */
        //     zip_uint64_t comp_size;             /* size of file (compressed) */
        //     time_t mtime;                       /* modification time */
        //     zip_uint32_t crc;                   /* crc of file data */
        //     zip_uint16_t comp_method;           /* compression method used */
        //     zip_uint16_t encryption_method;     /* encryption method used */
        //     zip_uint32_t flags;                 /* reserved for future use */
        // };
        // The structure pointed to by sb must be allocated before calling zip_stat() or zip_stat_index().
        // The valid field of the structure specifies which other fields are valid. Check if the flag defined by the following defines are in valid before accessing the fields:

        // ZIP_STAT_NAME
        // name
        // ZIP_STAT_INDEX
        // index
        // ZIP_STAT_SIZE
        // size
        // ZIP_STAT_COMP_SIZE
        // comp_size
        // ZIP_STAT_MTIME
        // mtime
        // ZIP_STAT_CRC
        // crc
        // ZIP_STAT_COMP_METHOD
        // comp_method
        // ZIP_STAT_ENCRYPTION_METHOD
        // encryption_method
        // ZIP_STAT_FLAGS
        // flags
        // RETURN VALUES
        // Upon successful completion 0 is returned. Otherwise, -1 is returned and the error information in archive is set to indicate the error.

        if (zip_stat_index(za, i, 0, &sb) == 0)
        {
            char path[100];
            snprintf(path, sizeof(path), "%s/%s", target_dir, sb.name);

            DEBUG_PRINT("Processing file %d/%d: %s", 
                       i + 1, zip_get_num_entries(za, 0), sb.name);
            DEBUG_PRINT("File size: %lu bytes", sb.size);
            // to make directory
            if (sb.name[strlen(sb.name) - 1] == '/')
            {
                mkdir(path, 0755);
                continue;
            }

            // zip_file_t * zip_fopen(zip_t *archive, const char *fname, zip_flags_t flags);
            // zip_file_t * zip_fopen_index(zip_t *archive, zip_uint64_t index, zip_flags_t flags);
            // DESCRIPTION
            // The zip_fopen() function opens the file name fname in archive. The flags argument specifies how the name lookup should be done, according to the values are described in zip_name_locate(3). Also, the following values may be or'ed to it.
            // ZIP_FL_COMPRESSED
            // Read the compressed data. Otherwise the data is uncompressed by zip_fread().
            // ZIP_FL_UNCHANGED
            // Read the original data from the zip archive, ignoring any changes made to the file; this is not supported by all data sources.
            // The zip_fopen_index() function opens the file at position index.
            // If encrypted data is encountered, the functions call zip_fopen_encrypted(3) or zip_fopen_index_encrypted(3) respectively, using the default password set with zip_set_default_password(3).
            // RETURN VALUES
            // Upon successful completion, a struct zip_file pointer is returned. Otherwise, NULL is returned and the error code in archive is set to indicate the error.
            // ERRORS
            // [ZIP_ER_CHANGED]
            // The file data has been changed and the data source does not support rereading data.
            // [ZIP_ER_COMPNOTSUPP]
            // The compression method used is not supported.
            // [ZIP_ER_ENCRNOTSUPP]
            // The encryption method used is not supported.
            // [ZIP_ER_MEMORY]
            // Required memory could not be allocated.
            // [ZIP_ER_NOPASSWD]
            // The file is encrypted, but no password has been provided.
            // [ZIP_ER_READ]
            // A file read error occurred.
            // [ZIP_ER_SEEK]
            // A file seek error occurred.
            // [ZIP_ER_WRONGPASSWD]
            // The provided password does not match the password used for encryption. Note that some incorrect passwords are not detected by the check done by zip_fopen().
            // [ZIP_ER_ZLIB]
            // Initializing the zlib stream failed.
            // The function zip_fopen() may also fail and set zip_err for any of the errors specified for the routine zip_name_locate(3).
            // The function zip_fopen_index() may also fail with ZIP_ER_INVAL if index is invalid.
            zf = zip_fopen_index(za, i, 0);
            if (!zf)
            {
                fprintf(stderr, "Failed to open zip entry\n");
                continue;
            }

            // create or open file at the path %s/%s, target_dir, file_name
            int fd = open(path, O_CREAT | O_WRONLY, 0755);
            if (fd < 0)
            {
                zip_fclose(zf);
                continue;
            }

            // read all the contents.
            zip_int64_t sum = 0;
            while (sum != sb.size)
            {
                zip_int64_t len = zip_fread(zf, buf, sizeof(buf));
                if (len < 0)
                    break;
                // writing to the created destination file
                write(fd, buf, len);
                // append whatever we have written
                sum += len;
            }

            // Closing file
            close(fd);
            zip_fclose(zf);
        }
    }
    INFO_PRINT("Zip extraction completed");
    zip_close(za);
    return 0;
}


// Function to create a new cgroup
int create_cgroup(const char *group)
{
    const char * controllers = "+cpuset +cpu +io +memory +io +pids +rdma";
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
        perror("Failed to open tasks file");
        return -1;
    }

    if (write(fd, controllers, strlen(controllers)) == -1)
    {
        perror("Failed to write controllers  process to cgroup");
        close(fd);
        return -1;
    }
    close(fd);

    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s/cgroup.procs", group);
    snprintf(pid_str, sizeof(pid_str), "%d", getpid());

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

    return 0;
}


// Function to remove cgroup
int remove_cgroup(const char *subsystem, const char *group)
{
    char path[1024];
    snprintf(path, sizeof(path), "/sys/fs/cgroup/%s", group);

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
    snprintf(value, sizeof(value), "%d 100000", (limit_in_percent * 100000 / 100));
    printf("CPU limit set %lu, in controller cpu in file %s\n ", limit_in_percent, path);

    fd = open(path, O_WRONLY );
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

    

    fd = open(path, O_WRONLY );
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







int setup_mounts()
{
    DEBUG_PRINT("Initializing mount namespace setup");
    if (mount("proc", "/proc", "proc", 0, NULL) == -1)
    {
        perror("mount proc");
        exit(EXIT_FAILURE);
    }
    INFO_PRINT("Successfully mounted proc filesystem");
    
    // if (mount("sysfs", "/sys", "sysfs", 0, NULL) == -1) {
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
    DEBUG_PRINT("Configuring mount namespace");
    if (setup_mounts() == -1)
    {
        perror("setup mounts");
        exit(EXIT_FAILURE);
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

    

    // Execute shell
    INFO_PRINT("Launching shell");
    char *args[] = {"/bin/bash", NULL};
    execv("/bin/bash", args);
    perror("execv");
    return EXIT_FAILURE;
}

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
    set_resource_limits();

    // Allocate stack for child
    DEBUG_PRINT("Allocating stack (size: %d bytes)", STACK_SIZE);
    char *stack = malloc(STACK_SIZE);
    if (!stack)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

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

    return EXIT_SUCCESS;
}
