/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"
#include "page.h"

extern VGADisplay vga_display;
extern VBEDisplay vbe_display;
extern HeapByteMap heap_bytemap;
extern Heap heap;
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
	memset(ptr, 0x0, size);
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

static void kmotd()
{
	kspinner(20);
	const Date date = cmos_date(&cmos);
	printf(" _             _         _____ _____ \n");
	printf("|_|___ ___ ___|_|_ _ ___|     |   __|\n");
	printf("| |  _| .'|  _| | | |_ -|  |  |__   |\n");
	printf("|_|___|__,|_| |_|___|___|_____|_____|\n");
	printf("\nicariusOS is running on an i686 CPU.\n");
	printf("%s, %d %s %d\n", days[date.weekday - 1], date.day, months[date.month - 1], date.year);
	return;
};

static void kcheck_multiboot2_magic(const uint32_t magic)
{
	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printf("Invalid Magic Number: 0x%x\n", magic);
		return;
	};
	return;
};

static void kcheck_multiboot2_alignment(const uint32_t addr)
{
	if (addr & 7) {
		printf("Unaligned Mbi: 0x%x\n", addr);
		return;
	};
	return;
};

static void kread_multiboot2_fb(struct multiboot_tag_framebuffer* tagfb, VBEDisplay* vbe_display)
{
	const void* framebuffer_addr = (void*)(uintptr_t)(tagfb->common.framebuffer_addr & 0xFFFFFFFF);
	const unsigned int framebuffer_width = tagfb->common.framebuffer_width;
	const unsigned int framebuffer_height = tagfb->common.framebuffer_height;
	const unsigned int framebuffer_pitch = tagfb->common.framebuffer_pitch;
	const unsigned int framebuffer_bpp = tagfb->common.framebuffer_bpp;
	vbe_init(vbe_display, framebuffer_addr, framebuffer_width, framebuffer_height, framebuffer_pitch, framebuffer_bpp);
	return;
};

static void kread_multiboot2(uint32_t addr, VBEDisplay* vbe_display)
{
	struct multiboot_tag* tag = (struct multiboot_tag*)(addr + 8);

	while (tag->type != MULTIBOOT_TAG_TYPE_END) {
		if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
			kread_multiboot2_fb((struct multiboot_tag_framebuffer*)tag, vbe_display);
			break;
		};
		// Move the tag pointer to the next multiboot tag, aligning it to the next 8-byte boundary.
		// This ensures compliance with the Multiboot2 specification, which requires all tags to be 8-byte aligned.
		// Example:
		// If tag->size = 22:
		//    (22 + 7) = 29
		//    29 & ~0b00000111 = 24 (next multiple of 8)
		// The pointer will move by 24 bytes to the start of the next tag.
		tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~0b00000111));
	};
	return;
};


void kmain(const uint32_t magic, const uint32_t addr)
{
	kcheck_multiboot2_magic(magic);
	kcheck_multiboot2_alignment(addr);
	kread_multiboot2(addr, &vbe_display);

	page_paging_init();
	printf("Paging Initialization\n");

	heap_init(&heap, &heap_bytemap, (void*)0x01000000, (void*)0x00007e00, 1024 * 1024 * 100, 4096);
	vfs_init();
	idt_init();

	asm_do_sti();

	ATADev* ata_dev = ata_get("A");
	ata_init(ata_dev);
	ata_search_fs(ata_dev);

	keyboard_init(&keyboard);
	mouse_init(&mouse);
	timer_init(&timer, 100);

	kmotd();

	pci_devices_enumerate();

	// This code opens the file "A:/LEET/TEST.TXT" in read mode, seeks to byte offset 0x2300,
	// reads 10 bytes into the buffer twice (with the second read overwriting the first 10 bytes),
	// and then prints the contents of the buffer as a null-terminated string.
	// As a result, the `printf` statement will output the second set of 10 bytes read from
	// the file starting at offset 0x2300, effectively displaying the string formed by bytes
	// from offset 0x230A to 0x2313 in "TEST.TXT".
	const int32_t fd = vfs_fopen("A:/LEET/TEST.TXT", "r");
	char buffer[1024] = {};
	vfs_fseek(fd, 0x2300, SEEK_SET);
	vfs_fread(buffer, 10, 1, fd);
	printf("%s\n", buffer);

	while (true)
		;
};