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