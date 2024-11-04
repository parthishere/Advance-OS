#include <stdio.h>
#include <linux/seccomp.h>  /* Definition of PR_* constants */
#include <seccomp.h>
#include <linux/prctl.h>  /* Definition of PR_* constants */
#include <sys/prctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int part_to_run;
scmp_filter_ctx ctx;

void graceful_exit(int rc)
{
    seccomp_release(ctx);
    exit(rc);
}

int main(int argc, char *argv[])
{
    int rc;

    if(argc != 2){
        printf("Usage : ./sandboxed_program <part-of-program-to-run 1,2,3> %d\n", argc);
        return -1;
    }
    else{
        printf("hellow %s\n", argv[1]);
        part_to_run = atoi(argv[1]);
    }
    int test_file_fd = open("test.txt", 0666);

    if ((ctx = seccomp_init(SCMP_ACT_KILL)) == NULL) {
            graceful_exit(1);
    }
    printf("Strict mode enabled - only read/write/exit allowed\n");

    /* Add allowed system calls to the BPF program */
    if ((rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0)) != 0) {
            graceful_exit(1);
    }
    if ((rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0)) != 0) {
            graceful_exit(1);
    }
    if ((rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0)) != 0) {
            graceful_exit(1);
    }
    if ((rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0)) != 0) {
            graceful_exit(1);
    }
    if ((rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0)) != 0) {
            graceful_exit(1);
    }
    if ((rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0)) != 0) {
            graceful_exit(1);
    }

    /* Load the BPF program for the current context into the kernel */
    if ((rc = seccomp_load(ctx)) != 0) {
            graceful_exit(1);
    }

    // if (test_file == NULL)
    // {
    //     printf("Error opening file");
    //     return 1;
    // }

    char ch;
    while (read(test_file_fd, &ch, 1) != 0)
    {
        printf("%c", ch);
    }

    // fclose(test_file);
    lseek(test_file_fd, 0, SEEK_END);
    const char *log_entry = "simple append to a file \n";
    char buffer_to_write[100];
    int how_many_bytes_written = snprintf(buffer_to_write, "%s", log_entry);
    printf("%s -- %d \n", buffer_to_write, how_many_bytes_written);
    write(test_file_fd, buffer_to_write, how_many_bytes_written);

    // unauthorize syscall
    if(argv[3] == "2"){
        
    }

// fstat
    // creat a new file


    // open file not included in your jailed environmnet

    close(test_file_fd);
    return 0;
}