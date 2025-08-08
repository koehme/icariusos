#!/bin/bash
set -e

KERNEL=./bin/x86_64/ICARIUS.elf
DISK=ICARIUS64.img
MNT=/mnt
LABEL=ICADISK
DISK_SIZE=64M
LIMINE_DIR=./limine
LIMINE_BIN=$LIMINE_DIR/bin

if [[ ! -f $LIMINE_BIN/limine-bios.sys ]]; then
    echo "[❌] limine-bios.sys "
    exit 1
fi

[ -f $DISK ] && rm -f $DISK

dd if=/dev/zero of=$DISK bs=1M count=64

sfdisk $DISK << EOF
label: dos
label-id: 0x1337CAFE
unit: sectors
sector-size: 512
${DISK}1 : start=2048, size=126976, type=06, bootable
EOF

LOOP0=$(sudo losetup -f)
sudo losetup $LOOP0 $DISK

LOOP1=$(sudo losetup -f)
sudo losetup $LOOP1 $DISK -o $((2048 * 512)) --sizelimit $((126976 * 512))

sudo mkfs.fat -F16 -n $LABEL $LOOP1

sudo mount $LOOP1 $MNT

sudo mkdir -p $MNT/boot
sudo cp $KERNEL $MNT/boot/

cat << EOF | sudo tee $MNT/limine.conf > /dev/null
timeout: 5
/ICARIUS
    protocol: limine
    path: boot():/boot/ICARIUS.elf
EOF

sudo mkdir -p $MNT/EFI/BOOT
sudo cp $LIMINE_BIN/limine-bios.sys $MNT/
sudo cp $LIMINE_BIN/BOOTX64.EFI $MNT/EFI/BOOT/
sudo cp $MNT/limine.conf $MNT/boot
sync

sudo umount $MNT

sudo losetup -D
$LIMINE_BIN/limine bios-install $DISK