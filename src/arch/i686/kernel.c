/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief The heart of the operating system. This is where the magic happens.
 * @version 1.0
 * @date 2024-11-15
 */

#include "kernel.h"

/* EXTERNAL API */
extern vbe_t vbe_display;
extern pfa_t pfa;
extern heap_t heap;
extern kbd_t kbd;
extern mouse_t mouse;
extern timer_t timer;
extern cmos_t cmos;
extern fifo_t fifo_kbd;
extern fifo_t fifo_mouse;
extern tss_t tss;

/* PUBLIC API */
void kmain(const uint32_t magic, const uint32_t addr);
void panic(const char* str);
void sleep(const uint32_t ms);
void busy_wait(const uint64_t delay);

/* INTERNAL API */
static void _render_spinner(const int32_t frames);
static void _motd(void);
static void _check_multiboot2_magic(const uint32_t magic);
static void _check_multiboot2_alignment(const uint32_t addr);
static void _read_multiboot2(const uint32_t magic, const uint32_t addr, vbe_t* vbe_display);
static void _check_kernel_size(const uint32_t max_kernel_size);
static void _init_fb(struct multiboot_tag* tag);
static void _init_mmap(struct multiboot_tag* tag);

void panic(const char* str)
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
		;
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
	const date_t date = cmos_date(&cmos);
	printf(" _             _         _____ _____ \n");
	printf("|_|___ ___ ___|_|_ _ ___|     |   __|\n");
	printf("| |  _| .'|  _| | | |_ -|  |  |__   |\n");
	printf("|_|___|__,|_| |_|___|___|_____|_____|\n");
	printf("\nicariusOS is running on an i686 CPU.\n");
	printf("%s, %d %s %d\n", days[date.weekday - 1], date.day, months[date.month - 1], date.year);
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

void _mark_kernel(void)
{
	const uint64_t kernel_start_frame = KERNEL_PHYS_BASE / PAGE_SIZE;
	const uint64_t kernel_end_frame = KERNEL_PHYS_END / PAGE_SIZE;

	for (uint64_t frame = kernel_start_frame; frame <= kernel_end_frame && frame < MAX_FRAMES; frame++) {
		pfa_set(&pfa, frame);
	};
	return;
};

void _mark_fb(void)
{
	const uint64_t fb_start_frame = FRAMEBUFFER_PHYS_BASE / PAGE_SIZE;
	const uint64_t fb_end_frame = (FRAMEBUFFER_PHYS_BASE + FRAMEBUFFER_SIZE - 1) / PAGE_SIZE;

	for (uint64_t frame = fb_start_frame; frame <= fb_end_frame && frame < MAX_FRAMES; frame++) {
		pfa_set(&pfa, frame);
	};
	return;
};

void _init_mmap(struct multiboot_tag* tag)
{
	multiboot_memory_map_t* mmap;
	struct multiboot_tag_mmap* tag_mmap = (struct multiboot_tag_mmap*)tag;

	for (mmap = tag_mmap->entries; (multiboot_uint8_t*)mmap < (multiboot_uint8_t*)tag + tag->size;
	     mmap = (multiboot_memory_map_t*)((unsigned long)mmap + tag_mmap->entry_size)) {
		const uint64_t base_addr = ((uint64_t)mmap->addr_high << 32) | mmap->addr_low;
		const uint64_t length = ((uint64_t)mmap->len_high << 32) | mmap->len_low;
		const uint64_t end_addr = base_addr + length;

		const uint64_t first_frame = base_addr / PAGE_SIZE;
		const uint64_t last_frame = end_addr / PAGE_SIZE;

		switch (mmap->type) {
		case MULTIBOOT_MEMORY_AVAILABLE: {
			for (uint64_t frame = first_frame; frame < last_frame && frame < MAX_FRAMES; frame++) {
				pfa_clear(&pfa, frame);
			};
			break;
		};
		case MULTIBOOT_MEMORY_RESERVED:
		case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
		case MULTIBOOT_MEMORY_NVS:
		case MULTIBOOT_MEMORY_BADRAM: {
			for (uint64_t frame = first_frame; frame < last_frame && frame < MAX_FRAMES; frame++) {
				pfa_set(&pfa, frame);
			};
			break;
		};
		default: {
			break;
		};
		};
	};
	_mark_kernel();
	_mark_fb();
	return;
};

static void _init_fb(struct multiboot_tag* tag)
{
	struct multiboot_tag_framebuffer* tag_fb = (struct multiboot_tag_framebuffer*)tag;
	const void* framebuffer_addr = (void*)(uintptr_t)(tag_fb->common.framebuffer_addr & 0xFFFFFFFF);
	const void* framebuffer_v_addr = (void*)(FRAMEBUFFER_VIRT_BASE);
	const unsigned int framebuffer_width = tag_fb->common.framebuffer_width;
	const unsigned int framebuffer_height = tag_fb->common.framebuffer_height;
	const unsigned int framebuffer_pitch = tag_fb->common.framebuffer_pitch;
	const unsigned int framebuffer_bpp = tag_fb->common.framebuffer_bpp;
	vbe_init(&vbe_display, framebuffer_v_addr, framebuffer_width, framebuffer_height, framebuffer_pitch, framebuffer_bpp);
	return;
};

static void _read_multiboot2(const uint32_t magic, const uint32_t addr, vbe_t* vbe_display)
{
	_check_multiboot2_magic(magic);
	_check_multiboot2_alignment(addr);
	const uint32_t vaddr = (uint32_t)p2v(addr);
	struct multiboot_tag* tag = (struct multiboot_tag*)(vaddr + 8);

	while (tag->type != MULTIBOOT_TAG_TYPE_END) {
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			_init_fb(tag);
			break;
		};
		case MULTIBOOT_TAG_TYPE_MMAP: {
			_init_mmap(tag);
			break;
		};
		default: {
			break;
		};
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
		panic("[CRITICAL] Kernel Memory > 16 MiB");
	} else {
		printf("[STATUS] Kernel Memory Usage: %f%%\n", used_percentage);
	};
	return;
};

void kmain(const uint32_t magic, const uint32_t addr)
{
	gdt_init();
	tss_init(&tss, KERNEL_STACK_TOP, GDT_KERNEL_DATA_SEGMENT);
	tss_load(GDT_TSS_SEGMENT);

	pic_init();
	idt_init();
	pfa_init(&pfa);

	_read_multiboot2(magic, addr, &vbe_display);
	_check_kernel_size(MAX_KERNEL_SIZE);

	pfa_dump(&pfa, false);

	heap_init(&heap);

	void* ptr1 = kzalloc(4096 * 1023);
	void* ptr2 = kzalloc(4096 * 1023);
	kfree(ptr2);
	kfree(ptr1);
	kzalloc(1024);
	heap_dump(&heap);

	fifo_init(&fifo_kbd);
	fifo_init(&fifo_mouse);

	kbd_init(&kbd);
	mouse_init(&mouse);
	timer_init(&timer, 100);
	asm_do_sti();
	/*
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

	pci_enumerate_bus();
	*/

	while (true) {
		ps2_dispatch(&fifo_kbd, kbd_handler, &kbd);
		ps2_dispatch(&fifo_mouse, mouse_handler, &mouse);
	};
};