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
# 🔥 1️⃣ Partition mounten
sudo mount $LOOP1 /mnt
# 🔥 2️⃣ Bootloader und Kernel installieren
sudo mkdir -p /mnt/boot
sudo grub-install --target=i386-pc --root-directory=/mnt --no-floppy --modules="normal part_msdos ext2 fat multiboot" $LOOP0
sudo cp ./iso/boot/grub/grub.cfg /mnt/boot/grub/grub.cfg
sudo cp ./bin/ICARIUS.BIN /mnt/boot/ICARIUS.BIN
# 🔥 3️⃣ Alle FAT16-Dateien und -Verzeichnisse kopieren
sudo cp ./bin/TEST.BIN /mnt/  # Direkt ins Root Dir
sudo cp -r ./bin/BIN /mnt/    # Ganzes BIN-Verzeichnis kopieren
sudo cp -r ./bin/BLA /mnt/    # Ganzes BLA-Verzeichnis kopieren
# 🔥 4️⃣ Sicherstellen, dass alles geschrieben wird
sudo sync
ls -l /mnt/
# 🔥 5️⃣ Partition unmounten und Loopback-Devices freigeben
sudo umount /mnt
sudo losetup -D
echo "[INFO] FAT16-Dateisystem erfolgreich erstellt und Dateien kopiert!"