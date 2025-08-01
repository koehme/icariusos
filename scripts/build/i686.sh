#!/bin/bash

set -e

export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir -p ~/build
cd ~/build

curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.44.tar.gz
tar xf binutils-2.44.tar.gz
mkdir -p binutils-build
cd binutils-build
../binutils-2.44/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --disable-werror
make -j$(nproc)
make install
cd ..

curl -O https://ftp.gnu.org/gnu/gcc/gcc-15.1.0/gcc-15.1.0.tar.gz
tar xf gcc-15.1.0.tar.gz
cd gcc-15.1.0
./contrib/download_prerequisites 
cd ..

mkdir -p gcc-build
cd gcc-build
../gcc-15.1.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make -j$(nproc) all-gcc
make -j$(nproc) all-target-libgcc
make install-gcc
make install-target-libgcc

if ! grep -q "$PREFIX/bin" ~/.bashrc; then
  echo 'export PATH="$PATH:'"$PREFIX"'/bin"' >> ~/.bashrc
fi