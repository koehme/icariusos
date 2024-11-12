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
	const void* framebuffer_v_addr = (void*)(0xE0000000);
	const unsigned int framebuffer_width = tagfb->common.framebuffer_width;
	const unsigned int framebuffer_height = tagfb->common.framebuffer_height;
	const unsigned int framebuffer_pitch = tagfb->common.framebuffer_pitch;
	const unsigned int framebuffer_bpp = tagfb->common.framebuffer_bpp;
	vbe_init(vbe_display, framebuffer_v_addr, framebuffer_width, framebuffer_height, framebuffer_pitch, framebuffer_bpp);
	return;
};

static void kread_multiboot2(uint32_t addr, VBEDisplay* vbe_display)
{
	const uint32_t v_addr = (uint32_t)p2v(addr);
	struct multiboot_tag* tag = (struct multiboot_tag*)(v_addr + 8);

	while (tag->type != MULTIBOOT_TAG_TYPE_END) {
		if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
			kread_multiboot2_fb((struct multiboot_tag_framebuffer*)tag, vbe_display);
			break;
		};
		tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~0b00000111));
	};
	return;
};

static void kvalidate_size(void)
{
	const uint32_t max_kernel_size = 16 * 1024 * 1024; // 16 MiB Bytes
	// const uint32_t multiboot_size = (uint32_t)_multiboot_end - (uint32_t)_multiboot_start;
	// const uint32_t boot_size = (uint32_t)_boot_end - (uint32_t)_boot_start;
	const uint32_t text_size = (uint32_t)_text_end - (uint32_t)_text_start;
	const uint32_t rodata_size = (uint32_t)_rodata_end - (uint32_t)_rodata_start;
	const uint32_t data_size = (uint32_t)_data_end - (uint32_t)_data_start;
	const uint32_t bss_size = (uint32_t)_bss_end - (uint32_t)_bss_start;
	const uint32_t total_size = text_size + rodata_size + data_size + bss_size;
	const float used_percentage = ((float)total_size / max_kernel_size) * 100.0;

	// printf("Multiboot: %d Bytes\n", multiboot_size);
	// printf("Boot: %d Bytes\n", boot_size);
	printf("Text: %d Bytes\n", text_size);
	printf("Rodata: %d Bytes\n", rodata_size);
	printf("Data: %d Bytes\n", data_size);
	printf("BSS: %d Bytes\n", bss_size);

	if (used_percentage >= 100.0) {
		kpanic("[CRITICAL] Kernel size exceeds 16 MiB!");
	} else if (used_percentage >= 90.0) {
		printf("[WARNING] Kernel is nearing critical size: %f%% used.\n", used_percentage);
	} else if (used_percentage >= 75.0) {
		printf("[CAUTION] Kernel is heavily: %f%% used. Consider optimizing.\n", used_percentage);
	} else if (used_percentage >= 50.0) {
		printf("[NOTICE] Kernel is halfway full: %f%% used. Keep an eye on usage.\n", used_percentage);
	} else if (used_percentage >= 25.0) {
		printf("[INFO] Kernel is using %f%% of available space. Plenty of room left.\n", used_percentage);
	} else {
		printf("[OK] Kernel is running efficiently: %f%% used.\n", used_percentage);
	};
	return;
};

void kmain(const uint32_t magic, const uint32_t addr)
{
	gdt_init();
	pic_init();
	idt_init();

	kread_multiboot2(addr, &vbe_display);
	kvalidate_size();

	printf("Hello World from Higher Half :-)\n");

	while (true) {
		;
	};
};