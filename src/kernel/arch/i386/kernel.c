/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"

extern VGADisplay vga_display;
extern HeapDescriptor kheap_descriptor;
extern Heap kheap;
extern PageDirectory kpage_dir;
extern PLexer plexer;
extern PParser pparser;
extern Keyboard keyboard;
extern Timer timer;
extern CMOS cmos;

void *kmalloc(const size_t size)
{
    void *ptr = 0x0;
    ptr = heap_malloc(&kheap, size);
    return ptr;
};

void *kcalloc(const size_t size)
{
    void *ptr = 0x0;
    ptr = heap_malloc(&kheap, size);
    mset8(ptr, 0x0, size);
    return ptr;
};

void kfree(void *ptr)
{
    heap_free(&kheap, ptr);
    return;
};

void kpanic(const char *str)
{
    kprintf(str);

    for (;;)
    {
    };
    return;
};

void ksleep(const uint32_t ms)
{
    const uint32_t ticks_to_wait = (ms * timer.hz) / 1000;
    const uint32_t end_ticks = timer.ticks + ticks_to_wait;

    while (timer.ticks < end_ticks)
    {
    };
    return;
};

void kdelay(const uint64_t delay)
{
    for (int i = 0; i < 1000; i++)
    {
        for (int j = 0; j < delay; j++)
        {
        };
    };
    return;
};

void kspinner(const int frames)
{
    const char *spinner_frames[4] = {"-", "\\", "|", "/"};

    for (int i = 0; i < frames; i++)
    {
        const char *curr_frame = spinner_frames[i % 4];
        kprintf(curr_frame);
        ksleep(500);
        kprintf("\b");
    };
    return;
};

void kmotd(multiboot_info_t *mbd)
{
    kprintf("Initializing Stack...\n");
    ksleep(500);
    kprintf("Initializing VGA Framebuffer %dx%d at 0xb8000...\n", mbd->framebuffer_width, mbd->framebuffer_height);
    ksleep(500);
    kprintf("Clearing VGA Textmode Buffer...\n");
    ksleep(500);
    kprintf("Initializing VGA Textmode Cursor at (0,0)...\n");
    ksleep(500);
    kprintf("Initializing Kheap Datapool at 0x01000000...\n");
    ksleep(500);
    kprintf("Initializing Kheap Descriptor at 0x00007e00...\n");
    ksleep(500);
    kprintf("Initializing Global Descriptor Table...\n");
    ksleep(500);
    kprintf("Initializing Virtual Memory Paging...\n");
    ksleep(500);
    kprintf("Enable Interrupts...\n");
    ksleep(500);
    kprintf("Initializing ATA Disk...\n");
    ksleep(500);
    kprintf("Initializing ATA Driver...\n");
    ksleep(500);
    kprintf("Initializing Keyboard Driver...\n");
    ksleep(500);
    kprintf("Initializing Timer...\n");
    ksleep(500);
    kprintf("Initializing CMOS Driver...\n");
    ksleep(500);
    kprintf("Initializing Disk Stream...\n");
    ksleep(500);

    kspinner(4);
    const Date date = cmos_date(&cmos);
    vga_display_clear(&vga_display);
    vga_display_set_cursor(&vga_display, 0, 0);

    kprintf(" _             _         _____ _____ \n");
    kprintf("|_|___ ___ ___|_|_ _ ___|     |   __|\n");
    kprintf("| |  _| .'|  _| | | |_ -|  |  |__   |\n");
    kprintf("|_|___|__,|_| |_|___|___|_____|_____|\n");

    vga_print(&vga_display, "                                     \n", VGA_COLOR_BLACK | (VGA_COLOR_LIGHT_GREEN << 4));
    kprintf("\nMessage: Welcome to icariusOS                                     \n");
    kprintf("Booted with the %s Bootloader.\n", mbd->boot_loader_name);
    kprintf("\n- Runs on an i686 CPU.\n");
    kprintf("Date: %s, %d %s %d                                                \n", days[date.weekday - 1], date.day, months[date.month + 1], date.year);
    return;
};

void kmain(uint32_t magic, multiboot_info_t *mbd)
{
    vga_display_init(&vga_display, (uint16_t *)0xb8000, 80, 25);
    vga_display_clear(&vga_display);
    cursor_set(0, 0);

    heap_init(&kheap, &kheap_descriptor, (void *)0x01000000, (void *)0x00007e00, 1024 * 1024 * 100, 4096);

    idt_init();

    PageDirectory *ptr_kpage_dir = &kpage_dir;
    page_init_directory(&kpage_dir, PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPERVISOR);
    page_switch(ptr_kpage_dir->directory);
    asm_page_enable();

    asm_do_sti();

    ATADisk *ata_disk = ata_get_disk(ATA_DISK_A);
    ata_init(ata_disk);
    plexer_init(&plexer, "A:/bin/cli.exe");
    PathRootNode *ptr_root_node = pparser_parse(&pparser, &plexer);

    keyboard_init(&keyboard);
    timer_init(&timer, 100);

    Stream stream = {};
    uint8_t stream_buffer[512];
    stream_init(&stream, ata_disk);
    stream_seek(&stream, 0x0);

    kmotd(mbd);
    return;
};