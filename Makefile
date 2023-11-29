DEPENDENCIES = ./obj/pm.o ./obj/kernel.o ./obj/vga.o ./obj/mem.o ./obj/idt.asm.o ./obj/idt.o
INCLUDES = -I./src/include
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/kernel.bin: $(DEPENDENCIES)
	$(LD) -g -relocatable $(DEPENDENCIES) -o ./obj/kernel.linked.o
	$(GCC) $(INCLUDES) $(FLAGS) -T ./linker.ld -o ./bin/kernel.bin ./obj/kernel.linked.o

./bin/boot.bin: ./src/bootloader/boot.asm
	nasm -f bin ./src/bootloader/boot.asm -o ./bin/boot.bin

./obj/pm.o: ./src/bootloader/pm.asm
	nasm -f elf -g ./src/bootloader/pm.asm -o ./obj/pm.o

./obj/kernel.o: ./src/kernel/kernel.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/kernel.c -o ./obj/kernel.o

./obj/vga.o: ./src/kernel/driver/vga.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/driver/vga.c -o ./obj/vga.o

./obj/mem.o: ./src/kernel/memory/mem.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/memory/mem.c -o ./obj/mem.o

./obj/idt.asm.o: ./src/kernel/interrupt/idt.asm
	nasm -f elf -g ./src/kernel/interrupt/idt.asm -o ./obj/idt.asm.o

./obj/idt.o: ./src/kernel/interrupt/idt.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/interrupt/idt.c -o ./obj/idt.o

clean:	
	rm -rf ./bin/*
	rm -rf ./obj/*