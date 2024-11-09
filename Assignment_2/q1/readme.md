System Details
I am currently using ubuntu 16.04 (64 bit) system with dual cores. I currently have kernel 4.13.0–36-generic.

To know which kernel you have type on terminal uname -r which will give your kernel version.

1. Download the kernel source:
In your terminal type the following command:

wget https://www.kernel.org/pub/linux/kernel/v4.x/linux-4.17.4.tar.xz

As my kernel version was 4.13, I downloaded a kernel of higher version (4.17.4), this way the kernel gets automatically updated when you reboot the system after compiling.

Also, make sure to write appropriate v4.x or anything else based on the version series you type.

wget command : GNU Wget is a free utility for non-interactive download of files from the Web.

2. Extract the kernel source code
sudo tar -xvf linux-4.17.4.tar.xz -C/usr/src/

tar — Tar stores and extracts files from a tape or disk archive.

-x — extract files from an archive

-v — requested using the –verbose option, when extracting archives

-f — file archive; use archive file or device archive

-C — extract to the directory specified after it.(in this case /usr/src/)

Now, we’ll change the directory to where the files are extracted:

cd /usr/src/linux-4.17.4/

3. Define a new system call sys_parth( )
Create a directory named customsys/ and change the directory to customsys/:
Create a file sys_parth.c using your favourite text editor:
Create a “Makefile” in the customsys directory:
touch Makefile

and add the following line to it:

obj-y := sys_parth.o

This is to ensure that the sys_parth.c file is compiled and included in the kernel source code.

Note: There is no space in between“obj-y”.

4. Adding customsys/ to the kernel’s Makefile:
Go back to the parent dir i.e. cd ../ and open “Makefile”
search for core-y in the document, you’ll find this line as the second instance of your search:

core-y += kernel/ mm/ fs/ ipc/ security/ crypto/ block/

Add customsys/’ to the end of this line:

core-y += kernel/ mm/ fs/ ipc/ security/ crypto/ block/ customsys/


5. Add the new system call to the system call table:
If you are on a 32-bit system you’ll need to change ‘syscall_32.tbl’. For 64-bit, change ‘syscall_64.tbl’.

Run the following commands in your terminal from linux-4.17.4/ directory:

cd arch/x86/entry/syscalls/
vi syscall_64.tbl
You’ll get a file like the following in your editor:


syscalls_64.tbl
Go to the last of the document and add a new line like so:

333 64      parth           sys_parth

6. Add new system call to the system call header file:
Go to the linux-4.17.4/ directory and type the following commands:

cd include/linux/
vi syscalls.h
Add the following line to the end of the document before the #endif statement:

asmlinkage long sys_parth(void);

Save and exit.

This defines the prototype of the function of our system call. “asmlinkage” is a key word used to indicate that all parameters of the function would be available on the stack.

7. Compile the kernel:
Before starting to compile you need to install a few packages. Type the following commands in your terminal:

sudo apt-get install gcc
sudo apt-get install libncurses5-dev
sudo apt-get install bison
sudo apt-get install flex
sudo apt-get install libssl-dev
sudo apt-get install libelf-dev
sudo apt-get update
sudo apt-get upgrade
to configure your kernel use the following command in your linux-4.17.4/ directory:

sudo make menuconfig

Once the above command is used to configure the Linux kernel, you will get a pop up window with the list of menus and you can select the items for the new configuration. If your unfamiliar with the configuration just check for the file systems menu and check whether “ext4” is chosen or not, if not select it and save the configuration.

Now to compile the kernel you can use the make command:

sudo make -jn


8. Install / update Kernel:
Run the following command in your terminal:

sudo make modules_install install

It will create some files under /boot/ directory and it will automatically make a entry in your grub.cfg. To check whether it made correct entry, check the files under /boot/ directory . If you have followed the steps without any error you will find the following files in it in addition to others.

System.map-4.17.4
vmlinuz-4.17.4
initrd.img-4.17.4
config-4.17.4
Now to update the kernel in your system reboot the system . You can use the following command:

shutdown -r now

After rebooting you can verify the kernel version using the following command:

uname -r

It will display the kernel version like so:

4.17.4

9. Test system call:
Run userspace.c program in q2/cron