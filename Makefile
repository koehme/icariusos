DEPENDENCIES = ./obj/loader.o ./obj/kernel.o
INCLUDES = -I./src/include
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/kernel.bin: $(DEPENDENCIES)
	$(LD) -g -relocatable $(DEPENDENCIES) -o ./obj/kernel.linked.o
	$(GCC) $(INCLUDES) $(FLAGS) -T ./linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./obj/kernel.linked.o

./bin/boot.bin: ./src/bootloader/boot.asm
	nasm -f bin ./src/bootloader/boot.asm -o ./bin/boot.bin

./obj/loader.o: ./src/bootloader/loader.asm
	nasm -f elf -g ./src/bootloader/loader.asm -o ./obj/loader.o

./obj/kernel.o: ./src/kernel/kernel.c
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/kernel/kernel.c -o ./obj/kernel.o

clean:	
	rm -rf ./bin/*
	rm -rf ./obj/*