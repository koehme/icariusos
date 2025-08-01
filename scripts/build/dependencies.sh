#!/bin/bash

if [[ "$EUID" -ne 0 ]]; then
  echo "❌ RUN AS SUDO ROOT!"
  exit 1
fi

if [[ ! -f ./bin/ICARIUS.BIN ]]; then
  echo "❌ RUN FROM PROJECT ROOT DIR!"
  exit 1
fi

sudo apt update -y
sudo apt upgrade -y
sudo apt install -y \
  dosfstools nasm grub-pc gdb qemu-system-x86 build-essential \
  bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo \
  curl xorriso mtools fdisk