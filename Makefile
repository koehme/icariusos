INCLUDES = -I./src/include
FLAGS = -std=gnu99 -g -ffreestanding -falign-jumps -falign-functions -falign-labels \
        -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions \
        -Wno-unused-function -Wno-unused-variable -fno-builtin -Werror -Wno-unused-label \
        -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0
ASSEMBLER = nasm
GCC = i686-elf-gcc
OBJCOPY = i686-elf-objcopy
AR = i686-elf-ar

OBJ_DIR = ./obj

SOURCES_C = \
    ./src/arch/i686/kernel.c \
    ./src/arch/i686/syscall.c \
    ./src/arch/i686/errno.c \
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
    ./src/arch/i686/gdt.c \
    ./src/arch/i686/pic.c \
    ./src/arch/i686/memory/heap.c \
    ./src/arch/i686/memory/page.c \
    ./src/arch/i686/memory/pfa.c \
    ./src/arch/i686/ds/fifo.c \
    ./src/arch/i686/process/tss.c \
    ./src/arch/i686/process/task.c \
    ./src/arch/i686/process/process.c \
    ./src/lib/stdlib.c \
    ./src/lib/stdio.c \
    ./src/lib/math.c \
    ./src/lib/string.c \
    ./src/lib/ctype.c

SOURCES_ASM = \
    ./src/arch/i686/gdt.asm \
    ./src/arch/i686/boot/loader.asm \
    ./src/arch/i686/idt.asm \
    ./src/arch/i686/io.asm \
    ./src/arch/i686/process/task.asm \

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

$(OBJ_DIR)/%.c.o: ./src/lib/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/process/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/driver/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/fs/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/ds/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/memory/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/arch/i686/string/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/kernel.c.o: ./src/arch/i686/kernel.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/idt.c.o: ./src/arch/i686/idt.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/syscall.c.o: ./src/arch/i686/syscall.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/errno.c.o: ./src/arch/i686/errno.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/gdt.c.o: ./src/arch/i686/gdt.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/pic.c.o: ./src/arch/i686/pic.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/loader.asm.o: ./src/arch/i686/boot/loader.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

$(OBJ_DIR)/gdt.asm.o: ./src/arch/i686/gdt.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

$(OBJ_DIR)/idt.asm.o: ./src/arch/i686/idt.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

$(OBJ_DIR)/io.asm.o: ./src/arch/i686/io.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

$(OBJ_DIR)/task.asm.o: ./src/arch/i686/process/task.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

image: $(OBJECTS)
	i686-elf-ld --no-warn-rwx-segments -n -T ./linker.ld $(OBJECTS) -o ./bin/ICARIUS.BIN
	cp ./bin/ICARIUS.BIN ./iso/boot/ICARIUS.BIN

clean:
	rm -rf $(OBJ_DIR)/*.o

shell:
	$(ASSEMBLER) -f elf32 ./src/arch/i686/user/shell.asm -o ./obj/shell.asm.o
	i686-elf-ld -T ./src/arch/i686/user/user.ld ./obj/shell.asm.o --oformat=binary -o ./bin/LEET/SHELL.BIN

icarsh:
	# Kompiliere die libc
	$(GCC) -I ./src/libc/include/ -ffreestanding -nostdlib -Wall -Wextra -O2 -c ./src/libc/stdio.c -o ./src/libc/obj/stdio.o
	$(GCC) -I ./src/libc/include/ -ffreestanding -nostdlib -Wall -Wextra -O2 -c ./src/libc/string.c -o ./src/libc/obj/string.o
	$(GCC) -I ./src/libc/include/ -ffreestanding -nostdlib -Wall -Wextra -O2 -c ./src/libc/syscall.c -o ./src/libc/obj/syscall.o

	# Erstelle libc.a
	$(AR) rcs ./src/libc/lib/libc.a ./src/libc/obj/stdio.o ./src/libc/obj/string.o ./src/libc/obj/syscall.o

	# Kompiliere den Assembler-Wrapper für `main()`
	nasm -f elf32 ./src/user/entry.asm -o ./src/user/obj/entry.o

	# Kompiliere die Usershell
	$(GCC) -I ./src/libc/include/ -ffreestanding -nostdlib -c ./src/user/icarsh.c -o ./src/user/obj/icarsh.o

	# Linke Usershell mit Assembler-Wrapper und libc.a
	$(GCC) -I ./src/libc/include/ -ffreestanding -nostdlib -T ./src/user/user.ld ./src/user/obj/entry.o ./src/user/obj/icarsh.o -o ./src/user/elf/icarsh.elf ./src/libc/lib/libc.a

	# Konvertiere ELF nach Flat Binary
	$(OBJCOPY) -O binary ./src/user/elf/icarsh.elf ./src/user/bin/ICARSH.BIN

	# Kopiere Usershell ins OS-Dateisystem
	cp ./src/user/bin/ICARSH.BIN ./bin/LEET/ICARSH.BIN