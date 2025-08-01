#!/bin/bash

if [[ ! -f ./bin/ICARIUS.BIN ]]; then
  echo "❌ RUN FROM PROJECT ROOT DIR!"
  exit 1
fi

export GCC="i686-elf-gcc"
export LD="i686-elf-ld"
export ASSEMBLER="nasm"

make all