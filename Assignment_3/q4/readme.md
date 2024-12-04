parth@rog:~$ sudo bpftrace -l | grep sys_clone
kfunc:vmlinux:__ia32_sys_clone
kfunc:vmlinux:__ia32_sys_clone3
kfunc:vmlinux:__x64_sys_clone
kfunc:vmlinux:__x64_sys_clone3
kprobe:__ia32_sys_clone
kprobe:__ia32_sys_clone3
kprobe:__x64_sys_clone
kprobe:__x64_sys_clone3


bpftrace -l 'tracepoint:syscalls:sys_enter_*'


perl -nE '
  BEGIN { say "const char *syscallnames[] = {" }
  if (/__NR_(\w+) (\d+)/) { say qq/\t[$2] = "$1",/ }
  END { say "};" }' /usr/include/x86_64-linux-gnu/asm/unistd_64.h > syscallnames.h
