# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

```bash
- Booted with GRUB2 bootloader
- Runs on i686 architecture
```

# my journey into the world of tech 🌌

since childhood, technology has been my playground. the thrill of solving a tough problem after hours of trial and error is unmatched. and building an operating system? that's the ultimate challenge—a digital Everest. every step in creating **icariusOS** is pure, unfiltered joy. it's my drive, my obsession, and it fuels my relentless passion for innovation. 😊

## core milestones 🏆

- ✨ **bootloader**: custom-built for a flawless system launch.  
- ✨ **ata driver**: loads kernel from hard drive into memory like a boss.  
- ✨ **protected mode**: unlocked the power of 32-bit computing with full memory utilization.  
- ✨ **interrupts**: implemented an elite IDT for superior interrupt handling.  
- ✨ **PIC**: tamed the Programmable Interrupt Controller for precise IRQ management.  
- ✨ **VGA driver**: smooth scrolling in text mode—because style matters.  
- ✨ **heap magic**: dynamic memory allocation, optimized for performance.  
- ✨ **logo**: startup is now a visual masterpiece with a sleek, custom logo. 😄  
- ✨ **paging**: efficient virtual memory management, unlocking limitless potential.  

## advanced warfare 🚀

- 🔥 **interrupt-driven ATA reads**: eliminated CPU blockages for lightning-fast disk reads.  
- 🔥 **printf 2.0**: upgraded to a pro-level print system for enhanced output.  
- 🔥 **CMOS date reader**: cracked time retrieval directly from CMOS.  
- 🔥 **real sleep function**: PIT-based sleep, because fake pauses are for amateurs.  
- 🔥 **Multiboot2 integration**: ditched the homemade bootloader for GRUB-level precision.  
- 🔥 **memory map**: harvested Multiboot2 data to conquer memory regions.  
- 🔥 **motd**: delivering daily doses of motivation with a killer message of the day. 😎  
- 🔥 **VFS layer**: file operations (`vfs_fopen`, `vfs_fread`) now operational.  
- 🔥 **framebuffer rendering**: graphics upgrade with sleek bitmap font support.  
- 🔥 **higher-half kernel**: kernel now thrives in upper memory realms.  

## current ops & next-gen goals 🔥

- 🚧 **FAT16 driver**: filesystem access up and running.  
- 🚧 **keyboard & mouse drivers**: taking input control to the next level (in progress).  

## page frame allocator 📦

- 🚧 **build a robust system** to manage physical memory frames.  
- 🚧 **efficient allocation and tracking** of free/used pages.  

### Multiboot2 Memory Map Integration  
- 🚧 **parse and interpret** the memory map provided by the Multiboot2 spec.  
- 🚧 **identify available physical memory regions** for allocation.  

### kzalloc Reimplementation  
- 🚧 **refactor `kzalloc`** to request physical pages directly from the page frame allocator.  
- 🚧 **maintain seamless dynamic memory allocation**, now with physical frame awareness.  

### Page Directory Integration  
- 🚧 **map physical pages into the page directory**.  
- 🚧 **establish a solid link** between virtual addresses and physical memory for ultimate control.  

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
./build.sh && qemu-system-i386 -m 256M -drive format=raw,file=./ICARIUS.img
```

# update

If you need to update the kernel, use the following command to swap the ICARIUS.BIN file in the FAT16 filesystem:

```bash
./swap.sh 
```

After updating the kernel, run the following command to launch icariusOS with the new kernel:

```bash
qemu-system-i386 -m 256M -drive format=raw,file=./ICARIUS.img
```

All-in-one for the lazy:

```bash
./swap.sh && qemu-system-i386 -m 256M -drive format=raw,file=./ICARIUS.img
```

# debug

```bash
./swap.sh && qemu-system-i386 -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
``````
