target remote | qemu-system-i386 -S -gdb stdio -hda ./bin/os.bin
add-symbol-file ./obj/kernel.o 0x100000
break vga.c:25