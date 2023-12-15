#!/bin/bash

sudo apt-get update -y
sudo apt-get upgrade -y

sudo apt-get install nasm -y
sudo apt-get install qemu -y
sudo apt-get install qemu-kvm -y
sudo apt-get install build-essential -y
sudo apt-get install bison -y
sudo apt-get install flex -y
sudo apt-get install libgmp3-dev -y
sudo apt-get install libmpc-dev -y
sudo apt-get install libmpfr-dev -y
sudo apt-get install texinfo -y
sudo apt-get install curl -y

export PREFIX="/usr/local/i686-elf-gcc"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir /tmp/src
cd /tmp/src

curl -O http://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.gz
tar xf binutils-2.41.tar.gz
mkdir binutils-build
cd binutils-build
../binutils-2.41/configure --target=$TARGET --enable-interwork --enable-multilib --disable-nls --disable-werror --prefix=$PREFIX 2>&1 | tee configure.log
sudo make all install 2>&1 | tee make.log

sudo rm -rf /tmp/src
mkdir /tmp/src

cd /tmp/src
curl -O https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz
tar xf gcc-13.2.0.tar.gz
mkdir gcc-build
cd gcc-build
echo Configure: . . . . . . .
../gcc-13.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-libssp --enable-language=c,c++ --without-headers
echo MAKE ALL-GCC:
sudo make all-gcc
echo MAKE ALL-TARGET-LIBGCC:
sudo make all-target-libgcc
echo MAKE INSTALL-GCC:
sudo make install-gcc
echo MAKE INSTALL-TARGET-LIBGCC:
sudo make install-target-libgcc


# sudo rm -rf /tmp/src
# mkdir /tmp/src

# cd /tmp/src
# curl -O https://ftp.gnu.org/gnu/gdb/gdb-14.1.tar.gz
# tar xf gdb-14.1.tar.gz
# mkdir gdb-build
# cd gdb-build
# ../gdb-14.1/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror
# sudo make
# sudo make install

# sudo rm -rf /tmp/src

export PATH="$PATH:/usr/local/i686-elf-gcc/bin"

ls /usr/local/i686-elf-gcc/bin

sudo chown $USER:$USER /usr/local/i686-elf-gcc/bin/*
echo 'export PATH="$PATH:/usr/local/i686-elf-gcc/bin"' >> ~/.bashrc
