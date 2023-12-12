# about

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Exploring the world of operating systems is like a cool journey through the Matrix. Every driver created and every code written feels like the discovery of secrets in an as yet unknown digital world. Even in this small "toy kernel" is the magic that determines how our technical world works.

It's not just about typing in bytes and commands. It becomes the feeling of an architect of a small but exciting digital world 🌐 . Sometimes time passes unnoticed in the creation of this digital world. Oh yes, the development of operating systems is indeed very time-consuming. ⏳

## archievements 🚀

1. **bootloader:**
   Smooth system startup achieved with a functional bootloader

2. **ata driver:**
   Successfully reads LBA sector from the hard drive, loading the kernel into memory at 0x010000.

3. **protected mode transition:**
   Seamless transition to Protected Mode (32-bit) with A20 Gate activation for full 32bit memory utilization.

4. **interrupts:**
   Implemented the Interrupt Descriptor Table (IDT) for handling interrupts.

5. **pic:**
   Currently working on implementing the Programmable Interrupt Controller (PIC) to handle Interrupt Requests (IRQ).

6. **extend vga driver**
   Add a functionality to scroll

## next step 🛠️

7. **heap**
   Add a functionality to dynamically allocate memory like kheap(50) for 50 bytes

# build on macos

```bash
mkdir bin
mkdir obj
./macos.sh
```

# build on linux

```bash
mkdir bin
mkdir obj
./linux.sh
```

# run on macos

```bash
clear && make clean && ./macos.sh && qemu-system-i386 -drive format=raw,file=./bin/os.bin
```

# run on linux

```bash
clear && make clean && ./linux.sh && qemu-system-i386 -drive format=raw,file=./bin/os.bin
```

# debugging on mac

```bash
clear && make clean && ./macos.sh && /opt/homebrew/opt/i386-elf-gdb/bin/i386-elf-gdb -x .gdbinit
```

# debugging on linux

```bash
clear && make clean && ./linux.sh && gdb -x .gdbinit
```

```bash
nm ./obj/loader.o && nm ./obj/kernel.o
```