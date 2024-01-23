#!/bin/bash

# Compile the kernel using build.sh
./build.sh

# Path to the compiled kernel.bin
KERNEL_PATH="./bin/kernel.bin"

# Mount the partition using loop devices
LOOP1=$(sudo losetup -f)
sudo losetup $LOOP1 ./disk.img -o 1048576

# Mount the partition to a temporary directory
mnt_dir=$(mktemp -d)
sudo mount $LOOP1 $mnt_dir

# Copy the updated kernel to the mounted partition
sudo cp $KERNEL_PATH $mnt_dir/boot/ICARIUS.BIN

# Unmount the partition and detach loop devices
sudo umount $mnt_dir
sudo losetup -d $LOOP1

# Clean up temporary directory
rmdir $mnt_dir

echo "ICARIUS.BIN swapped!"
