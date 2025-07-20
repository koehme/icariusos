/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
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
void panic(const char* fmt, ...) __attribute__((noreturn, format(printf, 1, 2)));
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
static void _remove_identity_mapping(void);

/* TEST API */
static void _test_vfs_read(const char file[]);
static void _test_heap(const int32_t size);
static void _test_page_dir_create(const uint32_t* pd);

void panic(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	printf("\n");
	printf("[KERNEL PANIC] ");
	vprintf(fmt, args);
	va_end(args);
	printf("\nSystem HALTED\n");
	asm volatile("cli; hlt");

	while (1) {
		;
	};
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
	time_t time = rtc_now();

	printf(" _             _         _____ _____ \n");
	printf("|_|___ ___ ___|_|_ _ ___|     |   __|\n");
	printf("| |  _| .'|  _| | | |_ -|  |  |__   |\n");
	printf("|_|___|__,|_| |_|___|___|_____|_____|\n");
	printf("\nicariusOS is running on an i686 CPU.\n");
	printf("Booted at %s, %d %s %d at ", days[date.weekday - 1], date.day, months[date.month - 1], date.year);
	printf("%d:%d:%d\n", time.hour, time.minute, time.second);
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
		printf("[STATUS] Kernel Memory Usage: %f \n", used_percentage);
	};
	return;
};

static void _remove_identity_mapping(void)
{
	printf("[INFO] Removing Identity Mapping...\n");
	page_unmap_between(page_get_dir(), 0x00000000, 0x00400000);
	printf("[SUCCESS] Identity Mapping removed!\n");
	return;
};

void kernel_shell(void)
{
	asm_do_sti();

	_render_spinner(32);
	_motd();

	// pfa_dump(&pfa, false);
	printf("icariusOS>");
	// process_list_dump();

	while (true) {
		uint8_t key;

		if (fifo_dequeue(&fifo_kbd, &key)) {
			printf("%c", key);
		};
		ps2_dispatch(&fifo_mouse, mouse_handler, &mouse);
	};

	return;
};

/*
############################
## Memory Layout Overview ##
############################

Formula: Entry Index * (1024*1024*4)
							^
							4 MiB

Example: 897 * 4194304 = 0xE0400000 - 0xE07FFFFF

| Virtual Address Range     | Physical Address Range    | Description                                | Page Directory Entry |
|---------------------------|---------------------------|--------------------------------------------|----------------------|
| 0x00000000 - 0x003FFFFF   | 0x00000000 - 0x003FFFFF   | Identity Mapping for Kernel Initialization | Entry 0              |
| 0x00400000 - 0x007FFFFF   | 0x00400000 - 0x007FFFFF   | Identity Mapping for Kernel Initialization | Entry 1              |
| 0x00800000 - 0x00BFFFFF   | 0x00800000 - 0x00BFFFFF   | Identity Mapping for Kernel Initialization | Entry 2              |
| 0x00C00000 - 0x00FFFFFF   | 0x00C00000 - 0x00FFFFFF   | Identity Mapping for Kernel Initialization | Entry 3              |
| 0x01000000 - 0x013FFFFF   | 0x01000000 - 0x013FFFFF   | Identity Mapping for Kernel Initialization | Entry 4              |
| 0x01400000 - 0x017FFFFF   | 0x01400000 - 0x017FFFFF   | Identity Mapping for Kernel Initialization | Entry 5              |
| 0x01800000 - 0x01BFFFFF   | 0x01800000 - 0x01BFFFFF   | Identity Mapping for Kernel Initialization | Entry 6              |
| 0x01C00000 - 0x01FFFFFF   | 0x01C00000 - 0x01FFFFFF   | Identity Mapping for Kernel Initialization | Entry 7              |
| 0x02000000 - 0x023FFFFF   | 0x02000000 - 0x023FFFFF   | Identity Mapping for Kernel Initialization | Entry 8              |
| 0x02400000 - 0x027FFFFF   | 0x02400000 - 0x027FFFFF   | Identity Mapping for Kernel Initialization | Entry 9              |
| 0x02800000 - 0x02BFFFFF   | 0x02800000 - 0x02BFFFFF   | Identity Mapping for Kernel Initialization | Entry 10             |
| 0x02C00000 - 0x02FFFFFF   | 0x02C00000 - 0x02FFFFFF   | Identity Mapping for Kernel Initialization | Entry 11             |
|---------------------------|---------------------------|--------------------------------------------|----------------------|
| 0x00000000 - 0x003FFFFF   | 0x00000000 - 0x003FFFFF   | _remove_identity_mapping()				 | Entry 1				|
|---------------------------|---------------------------|--------------------------------------------|----------------------|
| `0x00000000 - 0x3FFFFFFF` |        Dynamic            | User Program Code (Max. 1 GiB)             | Entry   0-255        |
| `0x40000000 - 0xBFBFFFFF` |        Dynamic            | User Heap (Grows Upwards, Max. 1,75 GiB)   | Entry 256-766        |
| `0xBFC00000 - 0xBFFFFFFF` |        Dynamic            | User Stack (Grows Downwards, 4 MiB)   	 | Entry 767            |
| `0xC0000000 - 0xC03FFFFF` | `0x00000000 - 0x003FFFFF` | Kernel Memory (| 	  4 MiB)                 | Entry 768            |
| `0xC0400000 - 0xC07FFFFF` | `0x00400000 - 0x007FFFFF` | Kernel Memory (->   4 MiB)                 | Entry 769            |
| `0xC0800000 - 0xC0BFFFFF` | `0x00800000 - 0x00BFFFFF` | Kernel Memory (->   4 MiB)                 | Entry 770            |
| `0xC0C00000 - 0xC0FFFFFF` | `0x00C00000 - 0x00FFFFFF` | Kernel Memory (->   4 MiB)                 | Entry 771            |
| `0xC1000000 - 0xC13FFFFF` | `0x01000000 - 0x013FFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 772            |
| `0xC1400000 - 0xC17FFFFF` | `0x01400000 - 0x017FFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 773            |
| `0xC1800000 - 0xC1BFFFFF` | `0x01800000 - 0x01BFFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 774            |
| `0xC1C00000 - 0xC1FFFFFF` | `0x01C00000 - 0x01FFFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 775            |
| `0xC2000000 - 0xC23FFFFF` | `0x02000000 - 0x023FFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 776            |
| `0xC2400000 - 0xC27FFFFF` | `0x02400000 - 0x027FFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 777            |
| `0xC2800000 - 0xC2BFFFFF` | `0x02800000 - 0x02BFFFFF` | Kernel Heap	(->   4 MiB) 			     | Entry 778            |
| `0xC2C00000 - 0xC2FFFFFF` | `0x02C00000 - 0x02FFFFFF` | Kernel Stack				                 | Entry 779            |
| `0xE0000000 - 0xE03FFFFF` | `0xFD000000 - 0xFD3FFFFF` | Framebuffer                                | Entry 896            |
| **Unmapped Addresses**    | -                         | Dynamic Allocation by PageFault Handler    | N/A                  |
|---------------------------|---------------------------|--------------------------------------------|----------------------|

################################################
## Stack Memory Layout for Higher-Half Kernel ##
################################################

| **Region**          | **Start Stack Address** | **End Stack Address**   | **Size**        | **Description**                               	|
|----------------------|------------------------|-------------------------|-----------------|-----------------------------------------------	|
| Kernel Stack         | `0xC2C00000`           | `0xC2C07FFF`            | 32 KiB          | Stack used by TSS for Ring 3 -> Ring 0       		|
| Reserved Stack Space | `0xC2C08000`           | `0xC2FFFFFF`            | 4064 KiB        | Reserved for Stack Expansion          			|
*/

void kmain(const uint32_t magic, const uint32_t addr)
{
	gdt_init();
	tss_init(&tss, KERNEL_STACK_TOP, GDT_KERNEL_DATA_SEGMENT);
	tss_load(GDT_TSS_SEGMENT);

	pic_init();
	idt_init();
	pfa_init(&pfa);

	page_set_dir(page_get_dir());
	page_restore_kernel_dir();

	_read_multiboot2(magic, addr, &vbe_display);
	_check_kernel_size(MAX_KERNEL_SIZE);

	heap_init(&heap);
	heap_dump(&heap);

	fifo_init(&fifo_kbd);
	fifo_init(&fifo_mouse);

	kbd_init(&kbd);
	mouse_init(&mouse);
	timer_init(&timer, 100);

	// pci_enumerate_bus();

	vfs_init();
	ata_t* ata_dev = ata_get("A");
	ata_init(ata_dev);
	ata_mount_fs(ata_dev);

	rtc_load_timezone();

	_remove_identity_mapping();
	syscall_init();

	scheduler_t* scheduler = scheduler_create(SCHED_ROUND_ROBIN);
	scheduler_select(scheduler);
	process_t* idle_proc = process_kspawn(kidle, "IDLE");
	scheduler->add_cb(idle_proc->tasks[0]);

	curr_process = idle_proc;

	process_t* icarsh = process_spawn("A:/BIN/ICARSH.BIN");

	if (!icarsh) {
		panic("Failed to initialize ICARSH");
	};
	tty_set_foreground(icarsh);
	scheduler->add_cb(icarsh->tasks[0]);

	asm_do_sti();

	// kernel_shell();
	return;
};