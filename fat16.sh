#!/bin/bash

dd if=/dev/zero of=disk.img bs=512 count=131072

sfdisk disk.img << EOF
label: dos
label-id: 0x513f54a3
device: disk.img 
unit: sectors
sector-size: 512
disk.img1 : start=2048, size=129024, type=06, bootable
EOF

LOOP0=`sudo losetup -f`
sudo losetup $LOOP0 disk.img

LOOP1=`sudo losetup -f`
sudo losetup $LOOP1 disk.img -o 1048576

sudo mkfs.fat -F16 -f2 -n ICADISK $LOOP1

sudo mount $LOOP1 /mnt

sudo grub-install --target=i386-pc --root-directory=/mnt --no-floppy --modules="normal part_msdos ext2 fat multiboot" $LOOP0

sudo cp ./iso/boot/grub/grub.cfg /mnt/boot/grub/grub.cfg
sudo cp ./bin/ICARIUS.BIN /mnt/boot/ICARIUS.BIN
sudo cp ./bin/TEST.TXT /mnt/TEST.TXT
sudo cp ./bin/ABC.TXT /mnt/ABC.TXT

sudo umount /mnt
sudo losetup -D