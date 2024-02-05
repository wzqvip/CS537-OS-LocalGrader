# XV6 Operating System

This directory contains a tarred and gziped version of the source code for the **XV6 Operating System**.  To get started on projects that use XV6, you will need to untar and unzip it to a working directory under your own home directory.  For example, you can run the following commands to create a new directory and expand the tarball into that directory:

```
> mkdir project2
> cd project2
> tar -xzf ~cs537-1/xv6/xv6.tar.gz -C .
```

This will extract the tarball into the current directory (-C .).  A subdirectory called xv6-public will be created containing the OSes source code.

To compile and run the code you can use the provided Makefile.  Try typing the following commands:

```
> cd xv6-public
> make
> make qemu-nox
```

This will change into the directory, compile the operating system, and then run the operating system in the virtual machine qemu (without graphical support (-nox)).  This is a very minimal operating system.  You can type `ls` to list the files.  You will see there are some user programs for doing very basic commands (e.g. cat, echo, etc.).

When you are done interacting with the operating system you can exit the OS and leave the qemu virtual machine by typing the command `<ctrl>-a x` ([qemu control keys](https://www.qemu.org/docs/master/system/mux-chardev.html)).

The other file in the directory, book-rev11.pdf, is the manual for this version of xv6.  Use it to gain a better understanding of this operating system.


