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
#include <linux/sched.h>    /* Definition of struct clone_args */
#include <sched.h>          /* Definition of CLONE_* constants */
#include <sys/syscall.h>    /* Definition of SYS_* constants */
#include <unistd.h>
#include <signal.h>
#include <string.h>


#define STACK_SIZE (1024 * 1024)
#define MOUNT_DIR "."

struct child_config
{
    char *zip_path;
    char *hostname;
    char *mount_dir;
};

// Function to extract zip file
static int extract_zip(const char *zip_path, const char *target_dir)
{
    struct zip *za;
    struct zip_file *zf;
    struct zip_stat sb;
    char buf[4096];
    int err;

    if ((za = zip_open(zip_path, 0, &err)) == NULL)
    {
        fprintf(stderr, "Failed to open zip file\n");
        return -1;
    }

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

    zip_close(za);
    return 0;
}

void set_resource_limits()
{
}

int setup_mounts()
{
    setenv("capsule_env", "/lib", 0);
    return 1;
}

int child_function(void *arg)
{
    struct child_config * config = arg;

    if (sethostname(config->hostname, strlen(config->hostname)) == -1)
    {
        perror("sethostname");
        exit(EXIT_FAILURE);
    }

    // Setup mounts
    if (setup_mounts() == -1)
    {
        perror("setup mounts");
        exit(EXIT_FAILURE);
    }

    // Change root
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

    // Set resource limits
    set_resource_limits();

    // Execute shell
    char *args[] = {"/bin/bash", NULL};
    execv("/bin/bash", args);
    perror("execv");
    return EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./capsule <zipfile>\n");
        return 1;
    }

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

    // Allocate stack for child
    char *stack = malloc(STACK_SIZE);
    if (!stack)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Create child process with new namespaces
    int flags = CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWIPC |
                CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWUSER;

    pid_t child_pid = clone(child_function, stack+STACK_SIZE, flags | SIGCHLD, &ch_config);
    if (child_pid == -1)
    {
        perror("clone");
        exit(EXIT_FAILURE);
    }

    // Wait for child
    if (waitpid(child_pid, NULL, 0) == -1)
    {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    // Cleanup
    free(stack);

    return EXIT_SUCCESS;
}
