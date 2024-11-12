/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"

#define p2v(addr) ((void*)((addr) + 0xC0000000))

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

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

void kmain(const uint32_t magic, const uint32_t addr)
{
	const void* kernel_start = _kernel_start;
	const void* kernel_end = p2v(_kernel_end);

	gdt_init();
	pic_init();
	idt_init();

	kread_multiboot2(addr, &vbe_display);

	printf("Hello World from Higher Half :-)\n");

	while (true) {
		;
	};
};