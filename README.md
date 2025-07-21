# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## Why?

Born in November 2022, icariusOS is my personal deep dive into the fascinating world of operating system internals. Built entirely from scratch, it's not just a project—it’s a commitment.

I've written kernels before, but life had other plans—until now. With icariusOS, I promised myself I'd see it through to the end. Is there even an end to operating systems? It started as a homemade bootloader and has evolved into a clean GRUB-booting OS that runs flawlessly on real 32-bit hardware.

Crafted with love by an IT professional deeply passionate about bare-metal hacking. Paging, syscalls, context switching, interrupt handlers, memory allocators—you name it, I've built it. icariusOS is my way of truly understanding operating systems—not just reading about them, but actually making it happen.

This is OS development done the right way—by getting your hands dirty. 🛠️✨

## ✨ Features

🔧 ARCHITECTURE & MEMORY
✅ HIGHER-HALF KERNEL: Mapping at 0xC0000000 – KEEP IT SAFE!
✅ MEGA PAGES: 4 MiB Paging via Page Size Extension (PSE) 🔥
✅ MEMORY ISOLATION: User and Kernel FULLY SEPARATED using CR3 Task Switching
✅ RING 3 TASKS: Each Task has its own Page Directory—TOTAL CONTROL!

🧠 TASKING & SCHEDULING
✅ ROUND-ROBIN POWER: Custom Task Queue for FAIRNESS & SPEED :)
✅ PREEMPTIVE MULTITASKING: Scheduler fires AUTOMATICALLY via IRQ0 → scheduler_schedule()
✅ SYSCALL MAGIC: User Requests via int 0x80—CLEAN & FAST!
✅ STACK POWERHOUSE: Supports up to 16 THREADS per Userspace Task—GO WILD!

📦 MEMORY MANAGEMENT
✅ SMART KERNEL HEAP: Chunking, Coalescing, & Detailed Stats
✅ REAL USERSPACE ALLOCATOR: Dynamic malloc() & calloc() (NO MORE Bump Allocator!)
✅ PAGE FAULT HANDLER: Precision Diagnostics—FIND & FIX ISSUES QUICKLY + STACKDUMP!

🖥️ USERSPACE SUPPORT
✅ FULL USERSPACE ISOLATION: 4 MiB for CODE, BSS, HEAP, STACK—EVERY PROCESS ITS OWN KINGDOM 🏰
✅ icarSH: YOUR USERSHELL—Minimal, Sleek, Built-in Commands:
ls, cat, echo, exit, help, history
✅ DYNAMIC USER HEAP: Best-Fit Allocator—Memory managed

## 🧩 DEPENDENCIES

Before you dive into building icariusOS - Double-Check you've got:

✅ UBUNTU or WINDOWS via WSL – Latest LTS Recommended
✅ CROSS-COMPILER – You'll need a freestanding i686 toolchain:

```bash
./i686.sh
```

## 🚀 GETTING STARTED

```bash
./build.sh && ./fat16.sh

```
## 🛠️ BUILD & RUN

```bash
./build.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

## 🔁 KERNEL UPDATE

Replace the Kernel Binary in your FAT16 Disk Image with a new BUILD.

```bash
./swap.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

## 🧠 Kernel Debugging (QEMU + GDB)

```bash
./swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
```

## 🔍 Userspace Debugging

```bash
./swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
break *0x0
file ./src/user/icarsh/elf/icarsh.elf
```

## 🐚 Build the Userspace Shell ICARSH.BIN

```bash
make icarsh && ./fat16.sh
```

## 🌲 Dir Structure

```bash
tree | xclip -selection clipboard
```

## 🤖 YOLO DevOps Pipeline

```bash
git checkout feature && git pull origin feature && git add . && git commit -m "Refactoring" && \
git checkout dev && git pull origin dev && git merge feature && git push origin dev && \
git checkout main && git pull origin main && git merge dev && git push origin main && \
git checkout feature
```