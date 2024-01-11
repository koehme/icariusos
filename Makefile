INCLUDES = -I./src/include
FLAGS = -std=gnu99 -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -Wno-unused-variable -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0

all: clean loader kernel image

clean:
	rm -rf ./obj/*.o

loader:
	$(ASSEMBLER) -f elf32 -g ./src/arch/i686/boot/multiboot.asm -o ./obj/multiboot.o
	$(ASSEMBLER) -f elf32 -g ./src/arch/i686/boot/loader.asm -o ./obj/loader.o

kernel:
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/kernel.c -o ./obj/kernel.o

	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/driver/ata.c -o ./obj/ata.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/driver/cmos.c -o ./obj/cmos.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/driver/cursor.c -o ./obj/cursor.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/driver/keyboard.c -o ./obj/keyboard.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/driver/timer.c -o ./obj/timer.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/driver/vga.c -o ./obj/vga.o

	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/fs/plexer.c -o ./obj/plexer.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/fs/pparser.c -o ./obj/pparser.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/fs/stream.c -o ./obj/stream.o

	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/idt.c -o ./obj/idt.o
	$(ASSEMBLER) -f elf32 -g ./src/arch/i686/idt.asm -o ./obj/idt.asm.o
	$(ASSEMBLER) -f elf32 -g ./src/arch/i686/io.asm -o ./obj/io.asm.o

	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/memory/heap.c -o ./obj/heap.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/memory/mem.c -o ./obj/mem.o
	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/memory/page.c -o ./obj/page.o
	$(ASSEMBLER) -f elf32 -g ./src/arch/i686/memory/page.asm -o ./obj/page.asm.o

	$(GCC) $(INCLUDES) $(FLAGS) -c ./src/arch/i686/string/string.c -o ./obj/string.o

image:
	i686-elf-ld -n -T ./linker.ld ./obj/multiboot.o ./obj/loader.o ./obj/kernel.o ./obj/ata.o ./obj/cmos.o ./obj/cursor.o ./obj/keyboard.o ./obj/timer.o ./obj/vga.o ./obj/plexer.o ./obj/pparser.o ./obj/stream.o ./obj/idt.o ./obj/idt.asm.o ./obj/io.asm.o ./obj/heap.o ./obj/mem.o ./obj/page.o ./obj/page.asm.o ./obj/string.o -o ./bin/kernel.bin
	cp ./bin/kernel.bin ./iso/boot/kernel.bin
	grub-mkrescue iso -o ./img/kernel.iso