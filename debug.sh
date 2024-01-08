#!/bin/bash

./build.sh && qemu-system-i386 -s -S ./img/kernel.iso &
sleep 1
gnome-terminal -- bash -c "gdb -x ./.gdbinit ./bin/kernel.bin; exec bash"