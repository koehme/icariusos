# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

```bash
- Booted with GRUB2 bootloader
- Runs on i686 architecture
```

# my journey into the world of tech 🌌

since childhood, technology has been my playground. the thrill of solving a tough problem after hours of trial and error is unmatched. and building an operating system? that's the ultimate challenge—a digital Everest. every step in creating **icariusOS** is pure, unfiltered joy. it's my drive, my obsession, and it fuels my relentless passion for innovation. 😊

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