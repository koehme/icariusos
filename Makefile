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
    ./src/kernel.c \
    ./src/syscall.c \
    ./src/errno.c \
    ./src/driver/ata.c \
    ./src/driver/cmos.c \
    ./src/driver/cursor.c \
    ./src/driver/keyboard.c \
    ./src/driver/timer.c \
    ./src/driver/vga.c \
    ./src/driver/vbe.c \
    ./src/driver/pci.c \
    ./src/driver/mouse.c \
    ./src/driver/rtc.c \
    ./src/driver/ps2.c \
    ./src/driver/fat16/fat16.c \
    ./src/fs/pathlexer.c \
    ./src/fs/pathparser.c \
    ./src/fs/stream.c \
    ./src/fs/vfs.c \
    ./src/fs/vnode.c \
    ./src/arch/i686/idt.c \
    ./src/arch/i686/gdt.c \
    ./src/arch/i686/pic.c \
    ./src/memory/heap.c \
    ./src/memory/page.c \
    ./src/memory/pfa.c \
    ./src/ds/fifo.c \
    ./src/process/tss.c \
    ./src/process/task.c \
    ./src/process/process.c \
    ./src/process/idle.c \
    ./src/process/tty.c \
    ./src/scheduler/scheduler.c \
    ./src/scheduler/rr.c \
    ./src/scheduler/wq.c \
    ./src/lib/stdlib.c \
    ./src/lib/stdio.c \
    ./src/lib/math.c \
    ./src/lib/string.c \
    ./src/lib/ctype.c \
    ./src/test/ata_test.c \
    ./src/test/heap_test.c \
    ./src/test/isr_test.c \
    ./src/test/page_test.c \
    ./src/test/vfs_test.c \

SOURCES_ASM = \
    ./src/arch/i686/gdt.asm \
    ./src/arch/i686/boot/loader.asm \
    ./src/arch/i686/idt.asm \
    ./src/arch/i686/io.asm \
    ./src/process/task.asm \

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

$(OBJ_DIR)/%.c.o: ./src/test/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@
    
$(OBJ_DIR)/%.c.o: ./src/fs/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/driver/fat16/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/driver/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/process/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/scheduler/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/ds/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.c.o: ./src/memory/%.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/kernel.c.o: ./src/kernel.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/idt.c.o: ./src/arch/i686/idt.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/syscall.c.o: ./src/syscall.c
	$(GCC) $(INCLUDES) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/errno.c.o: ./src/errno.c
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

$(OBJ_DIR)/task.asm.o: ./src/process/task.asm
	$(ASSEMBLER) -f elf32 -g $< -o $@

image: $(OBJECTS)
	i686-elf-ld --no-warn-rwx-segments -n -T ./linker.ld $(OBJECTS) -o ./bin/ICARIUS.BIN
	cp ./bin/ICARIUS.BIN ./iso/boot/ICARIUS.BIN

clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -rf ./src/user/obj/*.o
	rm -rf ./src/user/libc/obj/*.o

icarsh:
	# Kompiliere die libc
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/stdio.c -o ./src/user/libc/obj/stdio.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/malloc/malloc.c -o ./src/user/libc/obj/malloc.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/history.c -o ./src/user/libc/obj/history.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/kbd.c -o ./src/user/libc/obj/kbd.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/stdlib.c -o ./src/user/libc/obj/stdlib.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/string.c -o ./src/user/libc/obj/string.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/readline.c -o ./src/user/libc/obj/readline.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/syscall.c -o ./src/user/libc/obj/syscall.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/errno.c -o ./src/user/libc/obj/errno.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/dirent.c -o ./src/user/libc/obj/dirent.o
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -c ./src/user/libc/string/strerror.c -o ./src/user/libc/obj/strerror.o

	# Erstelle libc.a
	$(AR) rcs ./src/user/libc/lib/libc.a ./src/user/libc/obj/stdio.o ./src/user/libc/obj/malloc.o ./src/user/libc/obj/history.o ./src/user/libc/obj/kbd.o ./src/user/libc/obj/strerror.o ./src/user/libc/obj/dirent.o ./src/user/libc/obj/errno.o ./src/user/libc/obj/stdlib.o ./src/user/libc/obj/readline.o ./src/user/libc/obj/string.o ./src/user/libc/obj/syscall.o

	# Kompiliere den Assembler-Wrapper für `main()`
	$(ASSEMBLER) -f elf32 -g ./src/user/icarsh/entry.asm -o ./src/user/icarsh/obj/entry.o

	# Kompiliere die Usershell
	$(GCC) -I ./src/user/libc/include/ -I ./src/user/icarsh/include/  $(FLAGS) -c ./src/user/icarsh/icarsh.c -o ./src/user/icarsh/obj/icarsh.o
	$(GCC) -I ./src/user/libc/include/ -I ./src/user/icarsh/include/ $(FLAGS) -c ./src/user/icarsh/builtin.c -o ./src/user/icarsh/obj/builtin.o

	# Linke Usershell mit Assembler-Wrapper und libc.a
	$(GCC) -I ./src/user/libc/include/ $(FLAGS) -T ./src/user/icarsh/user.ld ./src/user/icarsh/obj/entry.o ./src/user/icarsh/obj/icarsh.o ./src/user/icarsh/obj/builtin.o -o ./src/user/icarsh/elf/icarsh.elf ./src/user/libc/lib/libc.a

	# Konvertiere ELF nach Flat Binary
	$(OBJCOPY) -O binary ./src/user/icarsh/elf/icarsh.elf ./src/user/icarsh/bin/ICARSH.BIN

	# Kopiere Usershell ins OS-Dateisystem
	cp ./src/user/icarsh/bin/ICARSH.BIN ./bin/BIN/ICARSH.BIN
