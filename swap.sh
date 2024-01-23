#!/bin/bash

./build.sh
KERNEL_PATH="./bin/ICARIUS.BIN"

LOOP1=$(sudo losetup -f)
sudo losetup $LOOP1 ./disk.img -o 1048576

MNT_DIR=$(mktemp -d)
sudo mount $LOOP1 $MNT_DIR

sudo cp $KERNEL_PATH $MNT_DIR/boot/ICARIUS.BIN

sudo umount $MNT_DIR
sudo losetup -d $LOOP1

rmdir $MNT_DIR
echo "KERNEL UPDATED!"