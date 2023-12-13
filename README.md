# welcome

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
   Implementing the Programmable Interrupt Controller (PIC) to handle Interrupt Requests (IRQ).

6. **extend vga driver**
   Add a functionality to scroll.

7. **heap - memory magic unleashed**
   Introducing a function to dynamically allocate memory with flair, something like "kheap(50)" for 50 bytes, aligning the allocation in cpu cache friendly 4096-byte blocks.

8. **logo**
   Integrate a function for printing initialization messages and unveil an elegant logo. This makes starting the system a visual experience! :D

## next step 🛠️

9. **heap - deallocation**
   The next step is to implement a deallocation function. To give the spirit of liberation a spell with something like "kfree(ptr)" to free the memory.

## about

In the heart of crafting my own operating system kernel, I've reached some remarkable milestones. The bootloader is successfully implemented, and my ATA driver reads LBA sector 1 from the hard drive, cleverly placing the kernel in memory at 0x010000. After seamlessly transitioning to 'Protected Mode - 32 Bit' and activating the A20 gate to access the entire memory, I can now invoke the kmain() function in assembly. This allows me to output text and even display line breaks ('\n'). I've also implemented a scrolling effect.

Currently, I've successfully implemented the Interrupt Descriptor Table and integrated the ability to respond to hardware interrupts. The Programmable Interrupt Controller (PIC) is also implemented, enabling effective handling of various hardware interrupts.

VGA Text Mode

I'm utilizing a VGA text mode driver that directly manipulates memory at 0xb8000. I've already implemented scrolling downwards, and the small blinking cursor is appropriately updated.

System Memory Manager

My system memory manager heap is implemented as an array, not a linked list, for speed and CPU cache-friendliness.

This initial structure maintains a table or descriptor for the heap. In this table, each byte represents a real memory block in the data pool of the other structure.

My heap data pool starts at memory address 0x01000000, which corresponds to 16777216 in decimal. The first block in the data pool also starts at 0x01000000, and regardless of the desired allocation size, it will be aligned to 4096 bytes.

Here's an example of what my allocator supports:

Here, 50 bytes are requested and aligned to 4096, considered as a single allocation block.

```c
kmalloc(50);
```

In the second call, 5000 bytes are requested, which will consume 2 blocks (4096 + 4096 = 8192 bytes) for allocation.

```c
kmalloc(5000);
```

My HeapDescriptor works with status flags to determine when a block is no longer free, whether it utilizes another block in the allocation, and marks the end of an allocation.

For now, my heap data pool supports 104857600 bytes (10241024100). When divided by the block size of 4096 bytes, I get 25600 bytes or 25 kilobytes required for the HeapDescriptor.

HeapDescriptor operates in real mode memory 0x00007E00 to 0x0007FFFF (480.5 KiB) since I'm already in Protected Mode. The starting address of the heap data pool is 0x01000000 (16777216 in decimal) + (104857600 in decimal) = End address 0x73FFFFF.

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