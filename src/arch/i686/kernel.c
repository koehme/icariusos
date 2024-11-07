/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"

extern VGADisplay vga_display;
extern VBEDisplay vbe_display;
extern HeapByteMap heap_bytemap;
extern Heap heap;
extern PageDirectory kpage_dir;
extern Keyboard keyboard;
extern Mouse mouse;
extern Timer timer;
extern CMOS cmos;

void* kmalloc(const size_t size)
{
	void* ptr = 0x0;
	ptr = heap_malloc(&heap, size);
	return ptr;
};

void* kcalloc(const size_t size)
{
	void* ptr = 0x0;
	ptr = heap_malloc(&heap, size);
	mset8(ptr, 0x0, size);
	return ptr;
};

void kfree(void* ptr)
{
	heap_free(&heap, ptr);
	return;
};

void kpanic(const char* str)
{
	printf(str);

	for (;;) {
	};
	return;
};

void ksleep(const uint32_t ms)
{
	const uint32_t ticks_to_wait = (ms * timer.hz) / 1000;
	const uint32_t end_ticks = timer.ticks + ticks_to_wait;

	while (timer.ticks < end_ticks) {
	};
	return;
};

void kdelay(const uint64_t delay)
{
	for (size_t i = 0; i < 1000; i++) {
		for (size_t j = 0; j < delay; j++) {
		};
	};
	return;
};

void kspinner(const int32_t frames)
{
	const char* spinner_frames[4] = {"-", "\\", "|", "/"};

	for (size_t i = 0; i < frames; i++) {
		const char* curr_frame = spinner_frames[i % 4];
		printf(curr_frame);
		ksleep(60);
		printf("\b");
	};
	return;
};

static void kmotd(uint32_t addr)
{
	/*
	kspinner(20);
	const Date date = cmos_date(&cmos);
	vga_display_clear(&vga_display);
	vga_display_set_cursor(&vga_display, 0, 0);
	printf(" _             _         _____ _____ \n");
	printf("|_|___ ___ ___|_|_ _ ___|     |   __|\n");
	printf("| |  _| .'|  _| | | |_ -|  |  |__   |\n");
	printf("|_|___|__,|_| |_|___|___|_____|_____|\n");
	vga_print(&vga_display, "                                     \n", VGA_COLOR_BLACK | (VGA_COLOR_LIGHT_GREEN << 4));
	printf("\nicariusOS is running on an i686 CPU.\n");
	printf("%s, %d %s %d\n", days[date.weekday - 1], date.day, months[date.month - 1], date.year);
     */
	return;
};

static void run_vfs_test(const char** file_paths, const size_t num_files, size_t* buffer_sizes)
{
	for (size_t i = 0; i < num_files; ++i) {
		const char* file_path = file_paths[i];
		size_t buffer_size = buffer_sizes[i];

		uint8_t buffer[buffer_size];
		mset8(buffer, 0x0, buffer_size);

		const int32_t fd = vfs_fopen(file_path, "r");

		if (fd < 0) {
			printf("VFSError: Could not open file '%s'\n", file_path);
			continue;
		};
		const size_t bytes_read = vfs_fread(buffer, 1, sizeof(buffer), fd);

		if (bytes_read == 0) {
			printf("VFSError: Could not read from file '%s'\n", file_path);
			vfs_fclose(fd);
			continue;
		};
		buffer[bytes_read] = '\0';

		printf("\n--- File: %s ---\n", file_path);
		printf("===========================================\n");
		printf("%s", buffer);
		printf("\n===========================================\n");
		kdelay(KERNEL_DEBUG_DELAY);

		vfs_fclose(fd);
	};
	return;
};

void kmain(const uint32_t magic, const uint32_t addr)
{


	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printf("Invalid Magic Number: 0x%x\n", magic);
		return;
	};

	if (addr & 7) {
		printf("Unaligned Mbi: 0x%x\n", addr);
		return;
	};
	struct multiboot_tag* tag;

	for (tag = (struct multiboot_tag*)(addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag*)((multiboot_uint8_t*)tag + ((tag->size + 7) & ~7))) {

		if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
			struct multiboot_tag_framebuffer* tagfb = (struct multiboot_tag_framebuffer*)tag;
			void* framebuffer_addr = (void*)(unsigned long)tagfb->common.framebuffer_addr;
			void* framebuffer = (void*)(uintptr_t)(tagfb->common.framebuffer_addr & 0xFFFFFFFF);
			unsigned int framebuffer_width = tagfb->common.framebuffer_width;
			unsigned int framebuffer_height = tagfb->common.framebuffer_height;
			unsigned int framebuffer_pitch = tagfb->common.framebuffer_pitch;
			unsigned int framebuffer_bpp = tagfb->common.framebuffer_bpp;
			vbe_init(&vbe_display, framebuffer, framebuffer_width, framebuffer_height, framebuffer_pitch, framebuffer_bpp);
			break;
		};
	};
	// vbe_draw_horizontal_line(&vbe_display, 0, VBE_COLOR_YELLOW);
	vbe_draw_string(&vbe_display, "Hello World!\b\b", VBE_COLOR_RED);
	vbe_draw_string(&vbe_display, "from icariusOS", VBE_COLOR_RED);

	heap_init(&heap, &heap_bytemap, (void*)0x01000000, (void*)0x00007e00, 1024 * 1024 * 100, 4096);

	vfs_init();
	idt_init();

	PageDirectory* ptr_kpage_dir = &kpage_dir;
	page_init_directory(&kpage_dir, PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPERVISOR);
	page_switch(ptr_kpage_dir->directory);
	asm_page_enable();

	asm_do_sti();

	ATADev* ata_dev = ata_get("A");
	ata_init(ata_dev);
	ata_search_fs(ata_dev);

	keyboard_init(&keyboard);
	mouse_init(&mouse);
	timer_init(&timer, 100);
	pci_devices_enumerate();

	const int32_t fd = vfs_fopen("A:/LEET/TEST.TXT", "r");
	char buffer[1024] = {};
	vfs_fseek(fd, 0x2300, SEEK_SET);
	vfs_fread(buffer, 10, 1, fd);
	vfs_fread(buffer, 10, 1, fd);
	// printf("%s\n", buffer);
	// kmotd(addr);

	for (;;) {
	};
	return;
};