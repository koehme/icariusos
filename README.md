# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

icariusOS is a handcrafted 32-bit operating system for i686, started in fall 2022.
I had built a few kernels before, but dropped those projects too early as other priorities took over.
With icariusOS, I committed myself to stick with it — and built everything entirely from scratch.

It began with a homemade bootloader, and now boots clean via GRUB.
Crafted by an IT'ler with a deep love for bare-metal hacking.

## ✨ Features

Paging & Virtual Memory
* ✅ Higher-half kernel mapped at 0xC0000000
* ✅ 4 MiB page support via Page Size Extension (PSE)
* ✅ Isolated user/kernel space with CR3-based task switching

Kernel Heap Allocator
* ✅ Chunk-based dynamic heap with coalescing
* ✅ Built-in stats & debug output for nerdy pleasure

Userspace Allocator
* ✅ malloc() and calloc()
* Just a bump allocator for now

Multitasking
* ❗ TODO: Preemptive Multitasking
* ✅ Per-process page directory setup
* ✅ Ring 3 usermode support via iret
* ✅ Thread stack layout for up to 16 threads per process

icarSH – the icarius Shell
* ✅ Lightweight custom userspace shell
* ✅ Built-in commands: ls, cat, echo, exit, history, help

Timezone & RTC
* ✅ CMOS clock readout for real-time freshness
* ✅ Auto UTC offset via ETC/TIMEZONE config

## 🧩 Dependencies

Before building icariusOS, make sure the following dependencies are installed

- **Ubuntu** – LTS version recommended (either on a VM or WSL)
- **Cross-Compiler** – You’ll need a freestanding i686 toolchain

To set it up, run

```bash
./i686.sh
```
🚀 Getting Started

Make sure the FAT16 file system is created before booting

```bash
./build.sh && ./fat16.sh
```
🛠️ Build & Run

Rebuild the kernel and run it via QEMU

```bash
./build.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

🔁 Kernel Updates

If you want to update the kernel binary inside the FAT16 image

```bash
./swap.sh
```
Then boot up with the fresh kernel

```bash
qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```
Shortcut for lazy

```bash
./swap.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```
🧠 Kernel Debugging (QEMU + GDB)

To debug the kernel at source level

```bash
./swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
```

🔍 Userspace Debugging

Debug the userspace too, with breakpoints and all the nerdy goodness

```bash
./swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
break *0x0
file ./src/user/icarsh/elf/icarsh.elf
```
🐚 Build the Userspace Shell ICARSH.BIN

This compiles the user shell and inserts it into the FAT16 image for execution

```bash
make icarsh && ./fat16.sh
```

🌲 Print Directory Structure

This command copies a visual tree of the project to your clipboard (Linux only)

```bash
tree | xclip -selection clipboard
```

🤖 YOLO DevOps Pipeline

YOLO

```bash
git checkout feature && git pull origin feature && git add . && git commit -m "Refactoring" && \
git checkout dev && git pull origin dev && git merge feature && git push origin dev && \
git checkout main && git pull origin main && git merge dev && git push origin main && \
git checkout feature
```