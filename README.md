# Custom Operating System Prototype
This project is about creating operating system from scratch.
Right now it is a simple bootloader, that after loading itself, tries to load the kernel code into memory.

## Requirements
~~~
makefile
gcc
nasm
~~~
System image is created using mkfs tool (Linux).
Building was not tested on windows so you need to troubleshoot it yourself or use WSL.

## How to run?
1. Clone the repository
2. Open command line in project directory
3. Use makefile to build the system image
~~~
make
~~~
4. Test the image in emulator (bochs, qemu)
