# about

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Exploring the world of operating systems is like a cool journey through the Matrix. Every driver created and every code written feels like the discovery of secrets in an as yet unknown digital world. Even in this small "toy kernel" is the magic that determines how our technical world works.

It's not just about typing in bytes and commands. It becomes the feeling of an architect of a small but exciting digital world 🌐 . Sometimes time passes unnoticed in the creation of this digital world. Oh yes, the development of operating systems is indeed very time-consuming. ⏳

## archievements 🚀

1. **bootloader implementation:**
   Smooth system startup achieved with a functional bootloader

2. **ata driver:**
   Successfully reads LBA sector from the hard drive, loading the kernel into memory at 0x010000.

3. **protected mode transition:**
   Seamless transition to Protected Mode (32-bit) with A20 Gate activation for full memory utilization.

4. **interrupt handling:**
   Implemented the Interrupt Descriptor Table (IDT) for handling interrupts.

## next step 🛠️

1. **PIC implementation:**
   Currently working on implementing the Programmable Interrupt Controller (PIC) to handle Interrupt Requests (IRQ).

# build

```bash
./build.sh
```

# run

```bash
clear && make clean && ./build.sh && qemu-system-x86_64 -hda ./bin/os.bin
```

# debugging 

```bash
/opt/homebrew/opt/i386-elf-gdb/bin/i386-elf-gdb -x .gdbinit
```

```bash
./build.sh
/opt/homebrew/opt/i386-elf-gdb/bin/i386-elf-gdb

add-symbol-file ./obj/loader.o 0x100000
break _start

target remote | qemu-system-x86_64 -S -gdb stdio -hda ./bin/os.bin
```

```bash
nm ./obj/loader.o && nm ./obj/kernel.o
```
