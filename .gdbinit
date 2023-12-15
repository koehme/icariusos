target remote | qemu-system-i386 -S -gdb stdio -drive format=raw,file=./bin/os.bin
add-symbol-file ./obj/kernel.o 0x100000

break icarius.c:121
break icarius.c:123
break page.c:22
break page.c:25
break page.c:45