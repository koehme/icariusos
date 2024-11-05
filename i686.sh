#!/bin/bash

sudo apt update -y
sudo apt upgrade -y

sudo apt install dosfstools -y
sudo apt install nasm -y
sudo apt install grub-pc -y
sudo apt install gdb -y
sudo apt install qemu -y
sudo apt install qemu-system-x86 -y
sudo apt install build-essential -y
sudo apt install bison -y
sudo apt install flex -y
sudo apt install libgmp3-dev -y
sudo apt install libmpc-dev -y
sudo apt install libmpfr-dev -y
sudo apt install texinfo -y
sudo apt install curl -y
sudo apt install xorriso -y
sudo apt install mtools -y
sudo apt install fdisk -y

export PREFIX="/usr/local/i686-elf-gcc"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir /tmp/src
cd /tmp/src

curl -O http://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz
tar xf binutils-2.42.tar.gz
mkdir binutils-build
cd binutils-build
../binutils-2.42/configure --target=$TARGET --enable-interwork --enable-multilib --disable-nls --disable-werror --prefix=$PREFIX 2>&1 | tee configure.log
sudo make all install 2>&1 | tee make.log

sudo rm -rf /tmp/src
mkdir /tmp/src

cd /tmp/src
curl -O https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
tar xf gcc-13.2.0.tar.gz
mkdir gcc-build
cd gcc-build
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-libssp --enable-language=c,c++ --without-headers
sudo make all-gcc
sudo make all-target-libgcc
sudo make install-gcc
sudo make install-target-libgcc

export PATH="$PATH:/usr/local/i686-elf-gcc/bin"

ls /usr/local/i686-elf-gcc/bin

sudo chown $USER:$USER /usr/local/i686-elf-gcc/bin/*
echo 'export PATH="$PATH:/usr/local/i686-elf-gcc/bin"' >> ~/.bashrc
