#!/bin/bash

./swap.sh && qemu-system-i386 -s -S ./ICARIUS.img &
sleep 1
gnome-terminal -- bash -c "gdb -x ./.gdbinit ./bin/ICARIUS.BIN; exec bash"