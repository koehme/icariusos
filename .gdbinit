target remote | qemu-system-x86_64 -S -gdb stdio -hda ./bin/os.bin
add-symbol-file ./obj/kernel.o 0x100000