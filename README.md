# about

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

# build

```bash
./build.sh
```

# run

```bash
clear && ./build.sh && qemu-system-x86_64 -hda ./bin/os.bin --display curses
```

# debugging

```bash
/opt/homebrew/opt/i386-elf-gdb/bin/i386-elf-gdb

add-symbol-file ./obj/loader.o 0x100000
break _start

target remote | qemu-system-x86_64 -S -gdb stdio -hda ./bin/os.bin
```

```bash
nm ./obj/loader.o && nm ./obj/kernel.o
```
