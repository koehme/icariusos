# icariusOS

A 🗿 yet 🪶 **x64 Kernel**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
![Status](https://img.shields.io/badge/status-in_development-yellow)
![Architecture](https://img.shields.io/badge/arch-x64-blue)
![Language](https://img.shields.io/badge/language-C%20%26%20Assembly-blueviolet)
![Last Commit](https://img.shields.io/github/last-commit/koehme/icariusos)

> Minimal. Monolithic. Handcrafted

 🆕 **Note:**  
ICARIUSOS AS A 64-BIT KERNEL IS BORN 🚀

BUILT ALONGSIDE CLASSIC ICARIUSOS — BOTH FLY TOGETHER. 🛠️🪶

---

## 🧩 INSTALL DEPENDENCIES

- ✅ DEBIAN or WINDOWS via WSL
- ✅ Limine (Bootloader)

```bash
./scripts/build/x86_64/limine.sh
```

- ✅ Emulator

```bash
./scripts/build/dependencies.sh
```

- ✅ CROSS-COMPILER

```bash
./scripts/build/x86_64/gcc.sh
```
- ⚠️ NOTE: ALWAYS RUN SCRIPTS FROM THE PROJECT ROOT DIR  

## 🛠️ BUILD

```bash
./scripts/build/x86_64/build64.sh
```

## 🔁 SWAP AND RUN AS UEFI

Replace the Kernel Binary in your FAT16 Disk Image with a new BUILD and RUN IT.

```bash
./scripts/build/x86_64/swap64.sh && qemu-system-x86_64 -cpu qemu64,+ssse3 -drive file=ICARIUS64.img,format=raw -drive if=pflash,format=raw,unit=0,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd -drive if=pflash,format=raw,unit=1,readonly=on,file=/usr/share/OVMF/OVMF_VARS_4M.fd

./scripts/build/x86_64/swap64.sh && qemu-system-x86_64 -cpu qemu64,+ssse3 -m 128M -drive file=ICARIUS64.img,format=raw -drive if=pflash,format=raw,unit=0,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd -drive if=pflash,format=raw,unit=1,readonly=on,file=/usr/share/OVMF/OVMF_VARS_4M.fd
```

## 🔁 SWAP AND RUN AS BIOS

Replace the Kernel Binary in your FAT16 Disk Image with a new BUILD and RUN IT.

```bash
./scripts/build/x86_64/swap64.sh && qemu-system-x86_64 -cpu qemu64,+ssse3 -drive file=ICARIUS64.img,format=raw
```

## 🧠 KERNEL DEBUG

```bash
qemu-system-x86_64 -cpu qemu64,+ssse3 -m 2048M -monitor stdio -drive file=ICARIUS64.img,format=raw -drive if=pflash,format=raw,unit=0,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd -drive if=pflash,format=raw,unit=1,readonly=on,file=/usr/share/OVMF/OVMF_VARS_4M.fd -s -S

qemu-system-x86_64 -m 2048M -monitor stdio -drive file=ICARIUS64.img,format=raw -drive if=pflash,format=raw,unit=0,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd -drive if=pflash,format=raw,unit=1,readonly=on,file=/usr/share/OVMF/OVMF_VARS_4M.fd -s -S

gdb -x ./.gdbinit64 ./bin/x86_64/ICARIUS.elf
```

## 🧠 QEMU MMAP

```bash
info mtree
```

## 🌲 PROJECT DIR

```bash
tree | xclip -selection clipboard
```

## UEFI

```bash
cp /usr/share/OVMF/OVMF_CODE_4M.fd ./OVMF_CODE.fd
```