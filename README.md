# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

```bash
- Booted with GRUB2 bootloader
- Runs on i686 architecture
```

# icariusOS – kernel specs

## architecture  
- i686, higher half (starting at 0xc0000000)  
- 4 mib paging (page_ps) for efficient memory management  
- strict separation between user and kernel space  
- supervisor mode for kernel – full control  

## boot mechanism  
- grub multiboot2 for flexible kernel loading  
- initial stack before enabling paging  
- identity mapping only temporary during boot  

## paging & memory management  
- pfa (page frame allocator): 4 mib pages for higher performance  
- page directory:  
  - kernel pages start at entry 768  
  - user-mode code pages from 0x00000000 – 0x400000  
- stack grows downward from 0xbfc00000  
- heap: dynamic allocation for future memory management  

## process & task management  
- each task has its own page directory  
- task switching: changes cr3 (page directory) on context switch  
- stack management: user esp set to user_stack_end  

## interrupt & syscall handling  
- syscalls via int 0x80 (ring 3 → ring 0)  
- asm wrapper + pushad for saving register state  
- enhanced page fault handling for user-mode errors  
- tss (task state segment): manages kernel stack on interrupts  

## cr3 (page directory base register)  
- always points to the active task's page directory  
- kernel remains mapped in the kernel directory  
- automatic switch on task switch or syscall exit  

## debugging & error handling  
- tools: page_dump_dir(), p/x v2p(task->page_dir), stack & register dumps  
- qemu with gdb: low-level debugging  
- advanced page fault handlers with detailed error codes  
- idt_dump_interrupt_frame() for precise error analysis  

## performance & optimization  
- 8 mib user space per task (initial)  
- direct memory allocation minimized for syscalls  
- page_map_between() optimized for larger memory regions  

## security & protection mechanisms  
- no page_user flags for kernel mappings  
- user-mode processes isolated – no direct kernel access  
- stack segment validation during syscall handling  
- page fault handling prevents unauthorized memory access  

## user-mode implementation  
- kernel can jump into user mode via iret  
- proper syscall handling: int 0x80 returns correctly to the kernel  
- after a syscall, the system returns to the kernel shell instead of an infinite loop  
- full support for user programs with isolated memory space  

## core milestones  
- bootloader: custom-built for a flawless system launch  
- ata driver: loads kernel from hard drive into memory  
- protected mode: unlocked the power of 32-bit computing with full memory utilization  
- interrupts: implemented an elite idt for superior interrupt handling  
- pic: tamed the programmable interrupt controller for precise irq management  
- vga driver: smooth scrolling in text mode  
- heap management: dynamic memory allocation optimized for performance  
- logo: startup features a sleek, custom logo  
- paging: efficient virtual memory management  

## advanced features  
- interrupt-driven ata reads: eliminated cpu blockages for fast disk reads  
- improved printf: upgraded to a pro-level print system  
- cmos date reader: time retrieval directly from cmos  
- real sleep function: pit-based sleep  
- multiboot2 integration: switched to grub for precision booting  
- memory map: harvested multiboot2 data to manage memory regions  
- motd: message of the day for system status or notifications  
- vfs layer: file operations (vfs_fopen, vfs_fread) now operational  
- framebuffer rendering: graphics upgrade with bitmap font support  
- higher-half kernel: kernel now operates in upper memory realms  

## current development  
- fat16 driver: filesystem in progress  
- keyboard & mouse drivers: advanced input handling under development  

## page frame allocator  
- build a robust system to manage physical memory frames  
- efficient allocation and tracking of free/used pages  

### multiboot2 memory map integration  
- parse and interpret the memory map provided by the multiboot2 spec  
- identify available physical memory regions for allocation  

### kzalloc reimplementation  
- refactor kzalloc to request physical pages directly from the page frame allocator  
- maintain seamless dynamic memory allocation with physical frame awareness  

### page directory integration  
- map physical pages into the page directory  
- establish a solid link between virtual addresses and physical memory  

# dependencies

Before building icariusOS, ensure that you have the following dependencies:

- **Ubuntu LTS**: I recommend using the latest Long Term Support (LTS) version of Ubuntu for development. 
You can download it from the [official Ubuntu website](https://ubuntu.com/download) and follow their installation instructions.

- **i686 Cross-Compiler**: Run the following commands to install the necessary dependencies

```bash
./i686.sh
```

# run

Before running icariusOS, ensure you have generated the FAT16 filesystem using the following command:

```bash
./build.sh
./fat16.sh
```

To rebuild and launch the kernel, use the following commands:

```bash
./build.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

# update

If you need to update the kernel, use the following command to swap the ICARIUS.BIN file in the FAT16 filesystem:

```bash
./swap.sh 
```

After updating the kernel, run the following command to launch icariusOS with the new kernel:

```bash
qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

All-in-one for the lazy:

```bash
./swap.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

# debug

```bash
./swap.sh && qemu-system-i386 -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
```

# tree

```bash
tree | xclip -selection clipboard
```