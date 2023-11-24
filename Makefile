DEPENDENCIES=./obj/loader.o

all: ./bin/boot.bin ./bin/kernel.bin
	rm -rf ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin

./bin/kernel.bin: $(DEPENDENCIES)
	$(LD) -g -relocatable $(DEPENDENCIES) -o ./obj/kernel.o 
	$(GCC) -T ./linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./obj/kernel.o

./bin/boot.bin: ./src/bootloader/boot.asm
	nasm -f bin ./src/bootloader/boot.asm -o ./bin/boot.bin

./obj/loader.o: ./src/bootloader/loader.asm
	nasm -f elf -g ./src/bootloader/loader.asm -o ./obj/loader.o

clean:
	rm -rf ./bin/boot.bin