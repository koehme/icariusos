INCLUDES = -I./src/arch/i686/include
FLAGS = -std=gnu99 -g -ffreestanding -falign-jumps -falign-functions -falign-labels \
        -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions \
        -Wno-unused-function -Wno-unused-variable -fno-builtin -Werror -Wno-unused-label \
        -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0
ASSEMBLER = nasm
GCC = i686-elf-gcc

OBJ_DIR = ./obj

SOURCES_C = \
    ./src/arch/i686/kernel.c \
    ./src/arch/i686/driver/ata.c \
    ./src/arch/i686/driver/cmos.c \
    ./src/arch/i686/driver/cursor.c \
    ./src/arch/i686/driver/keyboard.c \
    ./src/arch/i686/driver/timer.c \
    ./src/arch/i686/driver/vga.c \
    ./src/arch/i686/driver/vbe.c \
    ./src/arch/i686/driver/pci.c \
    ./src/arch/i686/driver/mouse.c \
    ./src/arch/i686/driver/ps2.c \
    ./src/arch/i686/driver/fat16.c \
    ./src/arch/i686/fs/pathlexer.c \
    ./src/arch/i686/fs/pathparser.c \
    ./src/arch/i686/fs/stream.c \
    ./src/arch/i686/fs/vfs.c \
    ./src/arch/i686/fs/vnode.c \
    ./src/arch/i686/idt.c \
    ./src/arch/i686/memory/heap.c \
    ./src/arch/i686/memory/page.c \
    ./src/arch/i686/string/string.c

SOURCES_ASM = \
    ./src/arch/i686/boot/multiboot.asm \
    ./src/arch/i686/boot/loader.asm \
    ./src/arch/i686/idt.asm \
    ./src/arch/i686/io.asm \
    ./src/arch/i686/memory/page.asm

define obj_c
$(OBJ_DIR)/$(notdir $(1:.c=.c.o))
endef

define obj_asm
$(OBJ_DIR)/$(notdir $(1:.asm=.asm.o))
endef

OBJECTS_C = $(foreach src,$(SOURCES_C),$(call obj_c,$(src)))
OBJECTS_ASM = $(foreach src,$(SOURCES_ASM),$(call obj_asm,$(src)))
OBJECTS = $(OBJECTS_C) $(OBJECTS_ASM)

all: $(OBJECTS) image

$(OBJ_DIR)/%.c.o: ./src/arch/i686/driver/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/fs/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/memory/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/string/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/kernel.c.o: ./src/arch/i686/kernel.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/idt.c.o: ./src/arch/i686/idt.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/multiboot.asm.o: ./src/arch/i686/boot/multiboot.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

$(OBJ_DIR)/loader.asm.o: ./src/arch/i686/boot/loader.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

$(OBJ_DIR)/idt.asm.o: ./src/arch/i686/idt.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

$(OBJ_DIR)/io.asm.o: ./src/arch/i686/io.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

$(OBJ_DIR)/page.asm.o: ./src/arch/i686/memory/page.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

image: $(OBJECTS)
	i686-elf-ld --no-warn-rwx-segments -n -T ./linker.ld $(OBJECTS) -o ./bin/ICARIUS.BIN
	cp ./bin/ICARIUS.BIN ./iso/boot/ICARIUS.BIN

clean:
	rm -rf $(OBJ_DIR)/*.o
