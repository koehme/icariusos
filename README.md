# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

icariusOS is my personal, experimental x86 kernel. I am building this operating system to better understand how an OS really works – from the ground up. It is a big challenge, because I have full control over the whole system – and that makes it very exciting!

Think about it: drawing to the framebuffer, managing memory, building a file system – doing all of this in your own kernel is not easy. In 32-bit mode, things are more detailed and maybe a bit old-school. You must work with limited memory and smaller resources – but that makes it even more interesting for me.

Why did I choose 32-bit instead of 64-bit? It was a practical decision. For 32-bit kernels, there are many more tutorials, documentation and example projects. That makes learning easier and helps me understand faster.

Of course, I know that a hobby kernel will never compete with Linux or Windows – one lifetime is not enough for that 😉. But that’s not my goal anyway.

I don’t want to create the next big Linux. This is about fun, challenge and learning. I am building my own operating system from scratch, step by step – and I enjoy writing every single line of code.

![Boot Demo](assets/icariusOS.gif)

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
- ✅ **PAGE FAULT HANDLER**: Diagnostics and Register Dumps

### 🖥️ Userspace Support
- ✅ **FULL USERSPACE ISOLATION**: 4 MiB for **CODE**, **BSS**, **HEAP**, **STACK** 
- ✅ **icarSH**:  
  `ls`, `cat`, `echo`, `exit`, `help`, `history`
- ✅ **DYNAMIC USER HEAP**: Best-Fit Allocator

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
