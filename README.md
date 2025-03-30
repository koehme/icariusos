# icariusOS

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

A minimalist i686 kernel, handcrafted since early 2023, learning and complete control

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