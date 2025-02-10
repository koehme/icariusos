# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

```bash
- Booted with GRUB2 bootloader
- Runs on i686 architecture
```

# features

# ⚙️ Icarius OS – Kernel Specifications 🚀  

## 1️⃣ Architecture 🏗️  
- **i686**, Higher Half (starting at **0xC0000000**)  
- **4 MiB Paging** (PAGE_PS) for efficient memory management  
- **Strict separation** between User and Kernel space 🛡️  
- **Supervisor mode** for Kernel – full control  

## 2️⃣ Boot Mechanism 🔥  
- **GRUB Multiboot2** for flexible kernel loading  
- **Initial stack** before enabling paging  
- **Identity mapping** only temporary during boot  

## 3️⃣ Paging & Memory Management 💾  
- **PFA (Page Frame Allocator):** 4 MiB pages for higher performance 🚀  
- **Page Directory:**  
  - Kernel pages start at entry **768**  
  - User-mode code pages from **0x00000000 – 0x400000**  
- **Stack grows downward** from **0xBFC00000**  
- **Heap:** Dynamic allocation for future memory management  

## 4️⃣ Process & Task Management 🏃‍♂️  
- Each task has its own **Page Directory**  
- **Task switching:** Changes **CR3** (Page Directory) on context switch  
- **Stack management:** User ESP set to **USER_STACK_END**  

## 5️⃣ Interrupt & Syscall Handling 🔄  
- **Syscalls via `int 0x80`** (Ring 3 → Ring 0)  
- **ASM wrapper + `pushad`** for saving register state  
- **Enhanced page fault handling** for user-mode errors  
- **TSS (Task State Segment):** Manages kernel stack on interrupts  

## 6️⃣ CR3 (Page Directory Base Register) 🔄  
- Always points to the **active task's Page Directory**  
- Kernel remains mapped in the **Kernel Directory**  
- Automatic switch on **task switch** or **syscall exit**  

## 7️⃣ Debugging & Error Handling 🛠️  
- **Tools:** `page_dump_dir()`, `p/x v2p(task->page_dir)`, stack & register dumps  
- **QEMU with GDB:** Low-level debugging  
- **Advanced page fault handlers** with detailed error codes  
- **`idt_dump_interrupt_frame()`** for precise error analysis  

## 8️⃣ Performance & Optimization ⚡  
- **8 MiB user space** per task (initial)  
- **Direct memory allocation** minimized for syscalls  
- **`page_map_between()`** optimized for larger memory regions  

## 9️⃣ Security & Protection Mechanisms 🔒  
- No **PAGE_USER** flags for kernel mappings  
- **User-mode processes isolated** – no direct kernel access  
- **Stack segment validation** during syscall handling  
- **Page fault handling** prevents unauthorized memory access  

## 🔟 User-Mode Implementation (NEW) 🖥️  
- Kernel can **jump into user mode** via `iret`  
- Proper syscall handling: `int 0x80` returns correctly to the kernel  
- After a syscall, the system **returns to the kernel shell** instead of an infinite loop  
- **Full support** for user programs with isolated memory space  

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