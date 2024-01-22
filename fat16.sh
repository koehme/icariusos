#!/bin/bash

# Create a disk image filled with zeros
dd if=/dev/zero of=disk.img bs=512 count=40000

# Partition the disk image using sfdisk
sfdisk disk.img << EOF
label: dos
label-id: 0x513f54a3
device: disk.img 
unit: sectors
sector-size: 512
disk.img1 : start=2048, size=30000, type=06, bootable
EOF

# Set up loop devices
LOOP0=`sudo losetup -f`
sudo losetup $LOOP0 disk.img

LOOP1=`sudo losetup -f`
# Start of the partition at 1 MB offset
sudo losetup $LOOP1 disk.img -o 1048576

# Format the partition with FAT16 and label it as "ICARIUSDISK"
sudo mkfs.fat -F16 -f2 -n ICARIUSDISK $LOOP1

# Mount the partition
sudo mount $LOOP1 /mnt

# Install GRUB on the disk image
sudo grub-install --target=i386-pc --root-directory=/mnt --no-floppy --modules="normal part_msdos ext2 fat multiboot" $LOOP0

# Copy GRUB configuration and kernel to the mounted partition
sudo cp ./iso/boot/grub/grub.cfg /mnt/boot/grub/grub.cfg
sudo cp ./bin/kernel.bin /mnt/boot/kernel.bin
sudo cp ./bin/hello.txt /mnt/hello.txt
# Unmount the partition and detach loop devices
sudo umount /mnt
sudo losetup -D