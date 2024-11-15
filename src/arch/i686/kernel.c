/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"

extern VBEDisplay vbe_display;
extern Heap kheap;
extern kbd_t kbd;
extern Mouse mouse;
extern Timer timer;
extern CMOS cmos;
extern fifo_t fifo_kbd;
extern fifo_t fifo_mouse;

void kpanic(const char* str)
{
	printf(str);

	while (true) {
		;
	};
	return;
};

void sleep(const uint32_t ms)
{
	const uint32_t ticks_to_wait = (ms * timer.hz) / 1000;
	const uint32_t end_ticks = timer.ticks + ticks_to_wait;

	while (timer.ticks < end_ticks) {
	};
	return;
};

void busy_wait(const uint64_t delay)
{
	for (size_t i = 0; i < 1000; i++) {
		for (size_t j = 0; j < delay; j++) {
		};
	};
	return;
};

static void _render_spinner(const int32_t frames)
{
	const char* spinner_frames[4] = {"-", "\\", "|", "/"};

	for (size_t i = 0; i < frames; i++) {
		const char* curr_frame = spinner_frames[i % 4];
		printf(curr_frame);
		sleep(60);
		printf("\b");
	};
	return;
};

static void _motd(void)
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

static void _check_multiboot2_magic(const uint32_t magic)
{
	if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
		printf("[INFO] Invalid Magic Number: 0x%x\n", magic);
		return;
	};
	return;
};

static void _check_multiboot2_alignment(const uint32_t addr)
{
	if (addr & 7) {
		printf("[INFO] Unaligned Mbi: 0x%x\n", addr);
		return;
	};
	return;
};

static void _init_framebuffer(struct multiboot_tag_framebuffer* tagfb, VBEDisplay* vbe_display)
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

static void _read_multiboot2(const uint32_t magic, const uint32_t addr, VBEDisplay* vbe_display)
{
	_check_multiboot2_magic(magic);
	_check_multiboot2_alignment(addr);
	// Read multiboot2 tags
	const uint32_t vaddr = (uint32_t)p2v(addr);
	struct multiboot_tag* tag = (struct multiboot_tag*)(vaddr + 8);

	while (tag->type != MULTIBOOT_TAG_TYPE_END) {
		if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER) {
			_init_framebuffer((struct multiboot_tag_framebuffer*)tag, vbe_display);
			break;
		};
		tag = (struct multiboot_tag*)((uint8_t*)tag + ((tag->size + 7) & ~0b00000111));
	};
	return;
};

static void _check_kernel_size(const uint32_t max_kernel_size)
{
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

	_read_multiboot2(magic, addr, &vbe_display);
	_check_kernel_size(16 * 1024 * 1024); // 16 MiB max kernel size

	heap_init(&kheap, (void*)0xC1000000, (void*)0xC1400000, 4 * 1024 * 1024, 4096); // 4 MiB max heap size
	printf("[INFO] Kernel Heap: %f%%\n", kheap_info(&kheap));

	fifo_init(&fifo_kbd);
	fifo_init(&fifo_mouse);

	kbd_init(&kbd);
	mouse_init(&mouse);
	timer_init(&timer, 100);
	asm_do_sti();

	pci_devices_enumerate();
	_render_spinner(64);
	_motd();

	vfs_init();
	ata_t* ata_dev = ata_get("A");
	ata_init(ata_dev);
	ata_mount_fs(ata_dev);

	const int32_t fd = vfs_fopen("A:/LEET/TEST.TXT", "r");
	char buffer[1024] = {};
	vfs_fseek(fd, 0x2300, SEEK_SET);
	vfs_fread(buffer, 10, 1, fd);
	vfs_fread(buffer, 10, 1, fd);
	printf("%s\n", buffer);

	while (true) {
		ps2_process_dev(&fifo_kbd, kbd_handler, &kbd);
		ps2_process_dev(&fifo_mouse, mouse_handler, &mouse);
	};
};