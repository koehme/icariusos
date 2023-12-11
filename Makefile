DEPENDENCIES = ./obj/pm.o ./obj/icarius.o ./obj/vga.o ./obj/idt.asm.o ./obj/idt.o ./obj/io.asm.o ./obj/string.o ./obj/mem.o ./obj/heap.o ./obj/cursor.o
INCLUDES = -I./src/include
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -Wno-unused-variable -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/kernel.bin: $(DEPENDENCIES)
	$(LD) -g -relocatable $(DEPENDENCIES) -o ./obj/kernel.o
	$(GCC) $(INCLUDES) $(FLAGS) -T ./linker.ld ./obj/kernel.o -o ./bin/kernel.bin

./bin/boot.bin: ./src/bootloader/boot.asm
	nasm -f bin ./src/bootloader/boot.asm -o ./bin/boot.bin

./obj/pm.o: ./src/bootloader/pm.asm
	nasm -f elf -g ./src/bootloader/pm.asm -o ./obj/pm.o

./obj/icarius.o: ./src/kernel/icarius.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/icarius.c -o ./obj/icarius.o

./obj/vga.o: ./src/kernel/driver/vga.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/driver/vga.c -o ./obj/vga.o

./obj/idt.asm.o: ./src/kernel/interrupt/idt.asm
	nasm -f elf -g ./src/kernel/interrupt/idt.asm -o ./obj/idt.asm.o

./obj/idt.o: ./src/kernel/interrupt/idt.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/interrupt/idt.c -o ./obj/idt.o

./obj/io.asm.o: ./src/kernel/io/io.asm
	nasm -f elf -g ./src/kernel/io/io.asm -o ./obj/io.asm.o

./obj/string.o: ./src/kernel/string/string.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/string/string.c -o ./obj/string.o

./obj/mem.o: ./src/kernel/memory/mem.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/memory/mem.c -o ./obj/mem.o

./obj/heap.o: ./src/kernel/memory/heap.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/memory/heap.c -o ./obj/heap.o

./obj/cursor.o: ./src/kernel/driver/cursor.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/driver/cursor.c -o ./obj/cursor.o

clean:	
	rm -rf ./bin/*
	rm -rf ./obj/*