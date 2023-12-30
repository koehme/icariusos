target remote | qemu-system-i386 -S -gdb stdio -drive format=raw,file=./bin/os.bin
add-symbol-file ./obj/kernel.o 0x100000

break keyboard.c:keyboard_ctrl_is_ready

layout split
focus cmd
set print pretty on