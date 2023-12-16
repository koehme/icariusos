target remote | qemu-system-i386 -S -gdb stdio -drive format=raw,file=./bin/os.bin
add-symbol-file ./obj/kernel.o 0x100000

break icarius.c:121
break icarius.c:123
break page.c:20
break page.c:42