/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"

extern VBEDisplay vbe_display;
extern Heap kheap;
extern Keyboard keyboard;
extern Mouse mouse;
extern Timer timer;
extern CMOS cmos;

void kpanic(const char* str)
{
	printf(str);

	while (true) {
		;
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

static void kspinner(const int32_t frames)
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

static void kmotd(void)
{
	const Date date = cmos_date(&cmos);
	printf(" _             _         _____ _____ \n");
	printf("|_|___ ___ ___|_|_ _ ___|     |   __|\n");
	printf("| |  _| .'|  _| | | |_ -|  |  |__   |\n");
	printf("|_|___|__,|_| |_|___|___|_____|_____|\n");
	printf("\nicariusOS is running on an i686 CPU.\n");
	printf("%s, %d %s %d\n", days[date.weekday - 1], date.day, months[date.month - 1], date.year);

	printf("\nicariusOS - Summary of Memory Regions:\n\n"
	       "------------------------------------------------------------------------------------\n"
	       "Region                          Start Address    End Address      Size\n"
	       "------------------------------------------------------------------------------------\n"
	       "Kernel (Code, Data, BSS)        0xC0000000      0xC1000000        16 MiB\n"
	       "Heap                            0xC1000000      0xC1400000        4 MiB\n"
	       "Heap Bytemap                    0xC1400000      0xC1400400        1 KiB\n"
	       "Free Memory                     0xC1400400      0xC2FFFFFF        Remaining MiB\n\n");
	return;
};

static void kcheck_multiboot2_magic(const uint32_t magic)
{
	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printf("[INFO] Invalid Magic Number: 0x%x\n", magic);
		return;
	};
	return;
};

static void kcheck_multiboot2_alignment(const uint32_t addr)
{
	if (addr & 7) {
		printf("[INFO] Unaligned Mbi: 0x%x\n", addr);
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
	const uint32_t text_size = (uint32_t)_text_end - (uint32_t)_text_start;
	const uint32_t rodata_size = (uint32_t)_rodata_end - (uint32_t)_rodata_start;
	const uint32_t data_size = (uint32_t)_data_end - (uint32_t)_data_start;
	const uint32_t bss_size = (uint32_t)_bss_end - (uint32_t)_bss_start;
	const uint32_t total_size = text_size + rodata_size + data_size + bss_size;
	const float used_percentage = ((float)total_size / max_kernel_size) * 100.0;

	if (used_percentage >= 100.0) {
		kpanic("[CRITICAL] Kernel Memory SIZE EXCEEDS 16 MiB!");
	} else if (used_percentage >= 90.0) {
		printf("[WARNING] Kernel Memory NEARING MAX SIZE: %f%% USED.\n", used_percentage);
	} else if (used_percentage >= 75.0) {
		printf("[CAUTION] Kernel Memory IS HEAVY: %f%% USED.\n", used_percentage);
	} else if (used_percentage >= 50.0) {
		printf("[NOTICE] Kernel Memory HALF FULL: %f%% USED.\n", used_percentage);
	} else if (used_percentage >= 25.0) {
		printf("[INFO] Kernel Memory IS FINE: %f%% USED.\n", used_percentage);
	} else {
		printf("[OK] Kernel Memory IS EFFICIENT: %f%% USED.\n", used_percentage);
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

	kheap_init(&kheap, (void*)0xC1000000, (void*)0xC1400000, 4 * 1024 * 1024, 4096);
	printf("[INFO] Kernel Heap: %f%%\n", kheap_info(&kheap));

	asm_do_sti();

	keyboard_init(&keyboard);
	mouse_init(&mouse);
	timer_init(&timer, 100);

	kspinner(64);
	kmotd();

	ATADev* ata_dev = ata_get("A");
	ata_init(ata_dev);
	ata_search_fs(ata_dev);

	while (true) {
		;
	};
};