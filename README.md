# welcome to my OS Development Journey! 🚀

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

Exploring the world of operating systems is like a cool journey through the Matrix. Every driver created and every code written feels like the discovery of secrets in an as yet unknown digital world. Even in this small "toy kernel" is the magic that determines how our technical world works.

It's not just about typing in bytes and commands. It becomes the feeling of an architect of a small but exciting digital world 🌐 . Sometimes time passes unnoticed in the creation of this digital world. Oh yes, the development of operating systems is indeed very time-consuming. ⏳

# achievements 

- [x] Bootloader: Smooth system startup with a written from scratch bootloader

- [x] ATA Driver via polling: Successfully reads LBA sectors from the hard drive and loading the kernel into memory at 0x010000.

- [x] Protected Mode Transition: Seamlessly transitioned to Protected Mode (32-bit) with A20 Gate activation for full 32-bit memory utilization.

- [x] Interrupts: Implemented the Interrupt Descriptor Table (IDT) for handling interrupts.

- [x] PIC: Implemented the Programmable Interrupt Controller (PIC) to handle Interrupt Requests (IRQ).

- [x] Extended VGA Driver: Added functionality to scroll within the VGA text mode.

- [x] Heap - Memory Magic Unleashed: Introduced a function to dynamically allocate memory like "kheap(50)" for 50 bytes, aligning allocations in CPU cache-friendly 4096-byte blocks.

- [x] Logo: Integrated a function for printing initialization messages and unveiled an elegant logo, transforming system startup into a visual experience! :D

- [x] Heap - Deallocation: Allowing the release of memory with something like "kfree(ptr)."

- [x] Paging: Dynamically optimized virtual memory by efficiently managing pages between RAM and disk. Excited about unlocking its potential to create the illusion of each process having its own address space.

- [x] ATA Read from Disk in an interrupt driven manner: Successfully implemented interrupt-driven reading of data from the ATA disk because constantly polling the ATA controller to check if it's finished is too slow and blocks the CPU.

- [ ] Keyboard driver

## about

In the heart of crafting my own operating system kernel, I've reached some remarkable milestones. The bootloader is successfully implemented, and my ATA driver reads LBA sector 1 from the hard drive, cleverly placing the kernel in memory at 0x010000. After seamlessly transitioning to 'Protected Mode - 32 Bit' and activating the A20 gate to access the entire memory, I can now invoke the kmain() function in assembly. This allows me to output text and even display line breaks ('\n'). I've also implemented a scrolling effect.

I've successfully implemented the Interrupt Descriptor Table and integrated the ability to respond to hardware interrupts. The Programmable Interrupt Controller (PIC 1 & 2) is also implemented, enabling effective handling of various hardware interrupts.

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

Paging

I have successfully implemented paging in my 32-bit kernel. The PageDirectory consists of 1024 entries, and each entry points to a PageTable. Each PageTable manages 1024 entries, which in turn represent 4096 bytes - one page in the world of memory.

Index 0 of the PageDirectory starts at the physical address 0x0, while index 2 is at 0x400000 (0x400000 == decimal 1024*4096 = 4194304). This creates a linear assignment of the physical addresses to the PageTable entries.

In summary, the system consists of a PageDirectory with 1024 entries, with each entry pointing to a PageTable. Each PageTable has 1024 entries, each representing 4096 bytes. This results in a total memory of 4 GB or 4,294,967,296 bytes.

With the function 

```c
void page_init_directory(PageDirectory *self, const uint8_t flags);
```

I have initialized a linear PageDirectory with physical addresses. The linking of virtual addresses to physical addresses has been successfully implemented. For example, I can now map for example the virtual address 0x2000 to any physical equivalent.

```c
// Assume ptr_phy_addr = 0x1802000
char *ptr_phy_addr = kcalloc(4096);
virt_address_map(ptr_kpage_dir->directory, (void *)0x2000, (uint32_t)ptr_phy_addr | PAGE_ACCESSED | PAGE_PRESENT | PAGE_READ_WRITE);

asm_do_sti();
kprintf("Enable Interrupts...\n");

char *ptr_virt = (char *)0x2000;

// Change the virtual address to see reflected changes because we map 0x2000 => to a physical addr 0x1802000
ptr_virt[0] = 'A';
ptr_virt[1] = 'W';
ptr_virt[2] = 'E';
ptr_virt[3] = 'S';
ptr_virt[4] = 'O';
ptr_virt[5] = 'M';
ptr_virt[6] = 'E';
ptr_virt[7] = '!';

kprintf("Initializing Virtual Memory Paging...\n");
kprintf("Testing Paging...\nVirtual Address 0x2000 now points to Physical Address 0x1802000...\n");
kprintf("Virtual 0x2000 [");
kprintf(ptr_virt);
kprintf("] == ");
kprintf("Physical 0x1802000 [");
kprintf(ptr_phy_addr);
kprintf("]\n");
```

The ATA driver was successfully implemented and the data read from the ATA disk was stored in the ata_disk.buffer. This buffer contains the content retrieved from the ATA disk, which I could now access and use if needed. I have also implemented a way to display the buffer when the read operation is complete.

# build an cross compiler

```bash
./i686.sh
```

# build

```bash
mkdir bin
mkdir obj
./build.sh
```

# run

```bash
./build.sh && qemu-system-i386 -drive format=raw,file=./bin/os.bin
```

# debugging

```bash
clear && make clean && ./build.sh && gdb -x .gdbinit
``````

```bash
nm ./obj/loader.o && nm ./obj/kernel.o
```
