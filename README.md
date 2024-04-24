# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

```bash
- Booted with GRUB2 bootloader
- Runs on i686 architecture
```

I've loved technology and all its different parts since I was a kid. It makes me really happy when, after trying different things for a long time, I finally figure out a solution to a problem and see things that were made completely from the beginning. Making an operating system, especially, is likely one of the biggest challenges you can take on with a computer. For me, it's just pure joy in the world of IT, and that excitement always keeps me motivated to keep working on the icariusOS project. 😊

| Status | Task Description                                                                                                 |
|--------|--------------------------------------------------------------------------------------------------------------------|
| [x]    | Bootloader: Smooth system startup with a custom-written bootloader.                                              |
| [x]    | ATA Driver: Reads LBA sectors from the hard drive, loading the kernel into memory at 0x010000.                   |
| [x]    | Protected Mode: Seamlessly transitioned to Protected Mode (32-bit) with A20 Gate activation for full 32-bit memory utilization. |
| [x]    | Interrupts: Implemented the Interrupt Descriptor Table (IDT) for handling interrupts.                           |
| [x]    | PIC: Programmed the Programmable Interrupt Controller (PIC) to handle Interrupt Requests (IRQ).                 |
| [x]    | VGA Driver: Added scroll functionality within the VGA text mode.                                                |
| [x]    | Heap Magic: Introduced a dynamic memory allocation function like "kheap(50)," aligning allocations in CPU cache-friendly 4096-byte blocks. |
| [x]    | Logo: Integrated a function for printing initialization messages and unveiled an elegant logo, transforming system startup into a visual experience! 😄 |
| [x]    | Heap Deallocation: Enabled the release of memory with "kfree(ptr)."                                               |
| [x]    | Paging: Dynamically optimized virtual memory by efficiently managing pages between RAM and disk, unlocking the potential to create the illusion of each process having its own address space. |
| [x]    | ATA Read in Interrupt Mode: Successfully implemented interrupt-driven reading of data from the ATA disk, avoiding constant polling that slows down and blocks the CPU. |
| [x]    | printf Upgrade: Implemented a printing function, enhancing the system's ability to communicate and display information. |
| [x]    | CMOS Date Reading: Successfully implemented a date reading function from the CMOS.                               |
| [x]    | Real Ksleep with Timer: Used PIT to create a real ksleep function.                                                |
| [x]    | Multiboot2 Integration: Integrated the GRUB bootloader to support the Multiboot2 spec, removing the self-made bootloader. |
| [x]    | Memory Map: Successfully read the memory map from Multiboot2.                                                     |
| [x]    | Message of the Day: Created a captivating and sexy message of the day! 😎                                         |
| [x]    | printf Refinement: Improved printf and fixed bugs in %s and %d functionalities.                                |
| [x]    | Create fat16 disk: To implement next the fat16 driver to read clusters from the filesystem                        |
| [x]    | VFS layer vfs_fopen and vfs_fread implemented                                                                                              |
| [x]    | Ata driver using 48bit mode improved                                                                                          |
| [ ]    | FAT16 driver (In Progress)                                                                                      |
| [ ]    | Keyboard Driver (In Progress)                                                                                    |

# about

```bash
cat ./docs/start.md
```

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
