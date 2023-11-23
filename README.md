# about

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

# macos

```bash
brew install qemu
brew install nasm
brew tap nativeos/i386-elf-toolchain
brew install nativeos/i386-elf-toolchain/i386-elf-binutils
brew install nativeos/i386-elf-toolchain/i386-elf-gcc
brew install i386-elf-gdb
```

# assembly

```bash
nasm -f bin ./src/bootloader/boot.asm -o ./bin/boot.bin && ndisasm ./bin/boot.bin && qemu-system-x86_64 -drive format=raw,file=./bin/boot.bin
```

# debugging

```bash
/opt/homebrew/opt/i386-elf-gdb/bin/i386-elf-gdb
target remote | qemu-system-x86_64 -drive format=raw,file=./bin/boot.bin -S -gdb stdio
```
