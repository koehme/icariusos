# icariusOS

A 🗿 yet 🪶 **x86 Kernel**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
![Status](https://img.shields.io/badge/status-in_development-yellow)
![Architecture](https://img.shields.io/badge/arch-x86-blue)
![Language](https://img.shields.io/badge/language-C%20%26%20Assembly-blueviolet)
![Last Commit](https://img.shields.io/github/last-commit/koehme/icariusos)

> Minimal. Monolithic. Handcrafted

![Boot Demo](assets/icariusOS.gif)

## ✨ Features

### Architecture & Memory
- ✅ **HIGHER-HALF KERNEL**: Mapping at `0xC0000000`
- ✅ **PAGES**: 4 MiB Paging via **Page Size Extension (PSE)** 🔥
- ✅ **MEMORY ISOLATION**: User and Kernel **FULLY SEPARATED** using **CR3 Task Switching**
- ✅ **RING 3 TASKS**: Each Task has its **own Page Directory**

### Tasking & Scheduling
- ✅ **ROUND-ROBIN**: Custom Task Queue 
- ✅ **PREEMPTIVE**: Scheduler fires **AUTOMATICALLY** via `IRQ0 → scheduler_schedule()`
- ✅ **SYSCALL**: User Requests via `int 0x80`
- ✅ **STACK**: Supports up to **16 THREADS per Userspace Task** 

### Memory Management
- ✅ **KERNEL HEAP**: Chunking, Coalescing & **Detailed Stats**
- ✅ **USERSPACE ALLOCATOR**: Dynamic `malloc()` & `calloc()` (**NO MORE Bump Allocator!**)
- ✅ **PAGE FAULT HANDLER**: Diagnostics and Register Dumps

### 🖥️ Userspace Support
- ✅ **FULL USERSPACE ISOLATION**: 4 MiB for **CODE**, **BSS**, **HEAP**, **STACK** 
- ✅ **icarSH**:  
  `ls`, `cat`, `echo`, `exit`, `help`, `history`
- ✅ **USER HEAP SUPPORT**: Best-Fit Allocator

## 🧩 DEPENDENCIES

- ✅ DEBIAN or WINDOWS via WSL
- ✅ CROSS-COMPILER
- ⚠️ NOTE: ALWAYS RUN SCRIPTS FROM THE PROJECT ROOT DIR  

```bash
sudo ./scripts/build/dependencies.sh
./scripts/build/i686.sh
```

## 🛠️ BUILD

```bash
make icarsh && ./scripts/build/build.sh && sudo ./scripts/build/fat16.sh
```

## 🔁 KERNEL UPDATE && RUN

Replace the Kernel Binary in your FAT16 Disk Image with a new BUILD.

```bash
./scripts/tools/swap.sh && qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

## 🧠 Kernel Debugging (QEMU + GDB)

```bash
./scripts/tools/swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
```

## 🔍 Userspace Debugging

```bash
./scripts/tools/swap.sh && qemu-system-i386 -m 4G -s -S ./ICARIUS.img
gdb -x ./.gdbinit ./bin/ICARIUS.BIN
break *0x0
file ./src/user/icarsh/elf/icarsh.elf
```

## 🌲 Dir 

```bash
tree | xclip -selection clipboard
```