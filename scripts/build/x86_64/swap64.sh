#!/bin/bash

./scripts/build/x86_64/build64.sh
KERNEL="./bin/x86_64/ICARIUS.elf"
DISK="./ICARIUS64.img"

LOOP1=$(sudo losetup -f)
sudo losetup $LOOP1 $DISK -o 1048576

MNT_DIR=$(mktemp -d)
sudo mount $LOOP1 $MNT_DIR

sudo cp $KERNEL $MNT_DIR/boot/ICARIUS.elf

sudo umount $MNT_DIR
sudo losetup -d $LOOP1

rmdir $MNT_DIR

echo -e "\033[1;36m"
cat << "EOF"
.__                    .__               ________   _____  
|__| ____ _____ _______|__|__ __  ______/  _____/  /  |  | 
|  |/ ___\\__  \\_  __ \  |  |  \/  ___/   __  \  /   |  |_
|  \  \___ / __ \|  | \/  |  |  /\___ \\  |__\  \/    ^   /
|__|\___  >____  /__|  |__|____//____  >\_____  /\____   | 
        \/     \/                    \/       \/      |__| 
EOF
echo -e "\033[0m"