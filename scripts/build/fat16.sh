#!/bin/bash

dd if=/dev/zero of=ICARIUS.img bs=512 count=1048576

sfdisk ICARIUS.img << EOF
label: dos
label-id: 0x513f54a3
device: ICARIUS.img 
unit: sectors
sector-size: 512
ICARIUS.img1 : start=2048, size=262110, type=06, bootable
EOF

LOOP0=`sudo losetup -f`

sudo losetup $LOOP0 ICARIUS.img

LOOP1=`sudo losetup -f`

sudo losetup $LOOP1 ICARIUS.img -o 1048576

sudo mkfs.fat -F16 -f2 -n ICADISK $LOOP1

sudo mount $LOOP1 /mnt
sudo mkdir -p /mnt/boot
sudo grub-install --target=i386-pc --root-directory=/mnt --no-floppy --modules="normal part_msdos ext2 fat multiboot" $LOOP0
sudo cp ./iso/boot/grub/grub.cfg /mnt/boot/grub/grub.cfg
sudo cp ./bin/ICARIUS.BIN /mnt/boot/ICARIUS.BIN

sudo cp -r ./bin/BIN /mnt/
sudo cp -r ./bin/DEV /mnt/
sudo cp -r ./bin/ETC /mnt/    
sudo cp -r ./bin/TMP /mnt/  
sudo cp ./LICENSE /mnt/LICENSE.TXT

sudo sync
ls -l /mnt/

sudo umount /mnt
sudo losetup -D