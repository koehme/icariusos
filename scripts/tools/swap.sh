#!/bin/bash

./scripts/build/build.sh
KERNEL_PATH="./bin/ICARIUS.BIN"

LOOP1=$(sudo losetup -f)
sudo losetup $LOOP1 ./ICARIUS.img -o 1048576

MNT_DIR=$(mktemp -d)
sudo mount $LOOP1 $MNT_DIR

sudo cp $KERNEL_PATH $MNT_DIR/boot/ICARIUS.BIN

sudo umount $MNT_DIR
sudo losetup -d $LOOP1

rmdir $MNT_DIR

echo -e "\033[1;36m"
cat << "EOF"
   _              _          ____  ____      ___ ___ 
  (_)______ _____(_)_ _____ / __ \/ __/ _  _<  // _ \
 / / __/ _ `/ __/ / // (_-</ /_/ /\ \  | |/ / // // /
/_/\__/\_,_/_/ /_/\_,_/___/\____/___/  |___/_(_)___/                                                                                                                                    
EOF
echo -e "\033[0m"