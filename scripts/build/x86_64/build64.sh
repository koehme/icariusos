#!/bin/bash

export GCC="x86_64-elf-gcc"
export LD="x86_64-elf-ld"
export ASSEMBLER="nasm"

make -f Makefile.64 all

if [ $? -ne 0 ]; then
    echo "Build ❌."
    exit 1
fi

echo "[✅] ELF"