#!/bin/bash

# 153600000 Bytes (154 MB, 146 MiB) 
dd if=/dev/zero of=disk.img bs=512 count=300000
# Partitiontable
sfdisk disk.img < ./partition

LOOP0=`sudo losetup -f`
sudo losetup $LOOP0 disk.img

LOOP1=`sudo losetup -f`
sudo losetup $LOOP1 -o 524288 disk.img

sudo mkfs.fat -F16 -f2 -n ICARIUSDISK $LOOP1

sudo mount $LOOP1 /mnt
sudo grub-install --target=i386-pc --root-directory=/mnt --no-floppy --modules="normal part_msdos ext2 fat multiboot2" $LOOP0
sudo cp ./iso/boot/grub/grub.cfg /mnt/boot/grub/grub.cfg
sudo cp ./bin/kernel.bin /mnt/boot/kernel.bin

sudo umount /mnt
sudo losetup -D