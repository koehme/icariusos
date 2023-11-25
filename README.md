# about

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

# build

```bash
./build.sh
```

# debugging

```bash
/opt/homebrew/opt/i386-elf-gdb/bin/i386-elf-gdb

add-symbol-file ./obj/kernel.o 0x100000
break _start

target remote | qemu-system-x86_64 -S -gdb stdio -hda ./bin/os.bin
```
