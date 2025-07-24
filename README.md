# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

icariusOS is a HAND-BUILT x86 KERNEL from SCRATCH — NO SHORTCUTS.
Started in 2022 — Now with INTERRUPTS, VFS, FAT16 FS, PAGING, SYSCALLS a SCHEDULER and much MORE! 👾
Pure BARE-METAL Hacking. READY TO DIVE IN? WANNA GET YOUR HANDS DIRTY? CONTRIBUTE AND LET’S BUILD TOGETHER!

  <img src="assets/icariusOS.gif" alt="Boot Demo" width="500" heigt="auto"/>

## ✨ Features

### 🔧 Architecture & Memory
- ✅ **HIGHER-HALF KERNEL**: Mapping at `0xC0000000`
- ✅ **MEGA PAGES**: 4 MiB Paging via **Page Size Extension (PSE)** 🔥
- ✅ **MEMORY ISOLATION**: User and Kernel **FULLY SEPARATED** using **CR3 Task Switching**
- ✅ **RING 3 TASKS**: Each Task has its **own Page Directory**

### 🧠 Tasking & Scheduling
- ✅ **ROUND-ROBIN POWER**: Custom Task Queue 
- ✅ **PREEMPTIVE MULTITASKING**: Scheduler fires **AUTOMATICALLY** via `IRQ0 → scheduler_schedule()`
- ✅ **SYSCALL MAGIC**: User Requests via `int 0x80`
- ✅ **STACK POWERHOUSE**: Supports up to **16 THREADS per Userspace Task** 

### 📦 Memory Management
- ✅ **SMART KERNEL HEAP**: Chunking, Coalescing & **Detailed Stats**
- ✅ **REAL USERSPACE ALLOCATOR**: Dynamic `malloc()` & `calloc()` (**NO MORE Bump Allocator!**)
- ✅ **PAGE FAULT HANDLER**: Diagnostics – **FIND & FIX ISSUES QUICKLY** + **STACKDUMP!**

### 🖥️ Userspace Support
- ✅ **FULL USERSPACE ISOLATION**: 4 MiB for **CODE**, **BSS**, **HEAP**, **STACK** 
- ✅ **icarSH**:  
  `ls`, `cat`, `echo`, `exit`, `help`, `history`
- ✅ **DYNAMIC USER HEAP**: Best-Fit Allocator – **Memory managed RIGHT**

## 🧩 DEPENDENCIES

- ✅ UBUNTU or WINDOWS via WSL – Latest LTS Recommended
- ✅ CROSS-COMPILER – You'll need a Freestanding i686 Toolchain:

- ⚠️ NOTE: ALWAYS RUN SCRIPTS FROM THE ROOT DIR (`./`)  

```bash
./scripts/build/i686.sh
```

## 🛠️ BUILD & RUN

```bash
make icarsh && ./scripts/build/build.sh && ./scripts/build/fat16.sh
```

```bash
qemu-system-i386 -m 4G -drive format=raw,file=./ICARIUS.img
```

## 🔁 KERNEL UPDATE

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
