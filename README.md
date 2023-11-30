# about

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

# build

```bash
./build.sh
```

# run

```bash
clear && make clean && ./build.sh && qemu-system-x86_64 -hda ./bin/os.bin
```

# debugging loader

```bash
./build.sh
/opt/homebrew/opt/i386-elf-gdb/bin/i386-elf-gdb

add-symbol-file ./obj/loader.o 0x100000
break _start

target remote | qemu-system-x86_64 -S -gdb stdio -hda ./bin/os.bin
```

```bash
/opt/homebrew/opt/i386-elf-gdb/bin/i386-elf-gdb -x .gdbinit
```

```bash
nm ./obj/loader.o && nm ./obj/kernel.o
```
