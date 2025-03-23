#!/bin/bash

dd if=/dev/zero of=ICARIUS.img bs=512 count=1048576
# FAT16-Image mit Partition ab Sektor 2048 für MBR & Bootloader (z. B. GRUB), Kernel wird darin gespeichert
sfdisk ICARIUS.img << EOF
label: dos
label-id: 0x513f54a3
device: ICARIUS.img 
unit: sectors
sector-size: 512
ICARIUS.img1 : start=2048, size=262110, type=06, bootable
EOF
# Sucht das nächste freie Loopback-Gerät und speichert den Gerätenamen in LOOP0
LOOP0=`sudo losetup -f`
# Verknüpft das Disk-Image ICARIUS.img mit dem Loopback-Gerät LOOP0
sudo losetup $LOOP0 ICARIUS.img
# Sucht erneut ein freies Loopback-Gerät und speichert den Gerätenamen in LOOP1
LOOP1=`sudo losetup -f`
# Verknüpft die Partition innerhalb des Images (Offset 1 MiB = 1048576 Bytes) mit LOOP1
sudo losetup $LOOP1 ICARIUS.img -o 1048576
# Formatiert die Partition als FAT16 mit zwei FAT-Kopien und setzt den Namen "ICADISK"
sudo mkfs.fat -F16 -f2 -n ICADISK $LOOP1
# 🔥 1️⃣ Partition mounten
sudo mount $LOOP1 /mnt
# 🔥 2️⃣ Bootloader und Kernel installieren
sudo mkdir -p /mnt/boot
sudo grub-install --target=i386-pc --root-directory=/mnt --no-floppy --modules="normal part_msdos ext2 fat multiboot" $LOOP0
sudo cp ./iso/boot/grub/grub.cfg /mnt/boot/grub/grub.cfg
sudo cp ./bin/ICARIUS.BIN /mnt/boot/ICARIUS.BIN
# 🔥 3️⃣ Alle FAT16-Dateien und -Verzeichnisse kopieren
sudo cp -r ./bin/BIN /mnt/
sudo cp -r ./bin/DEV /mnt/
sudo cp -r ./bin/ETC /mnt/    
sudo cp -r ./bin/TMP /mnt/  
# 🔥 4️⃣ Sicherstellen, dass alles geschrieben wird
sudo sync
ls -l /mnt/
# 🔥 5️⃣ Partition unmounten und Loopback-Devices freigeben
sudo umount /mnt
sudo losetup -D
echo "[INFO] FAT16-Dateisystem erfolgreich erstellt und Dateien kopiert!"