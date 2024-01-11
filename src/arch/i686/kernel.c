/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"

#define KMAIN_DEBUG_THROTTLE 5

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
        ksleep(KMAIN_DEBUG_THROTTLE);
        kprintf("\b");
    };
    return;
};

void kmotd(unsigned long addr)
{
    kprintf("Initializing Stack...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    // kprintf("Initializing VGA Framebuffer %dx%d at 0xb8000...\n", mbd->framebuffer_width, mbd->framebuffer_height);
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Clearing VGA Textmode Buffer...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing VGA Textmode Cursor at (0,0)...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing Kheap Datapool at 0x01000000...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing Kheap Descriptor at 0x00007e00...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing Global Descriptor Table...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing Virtual Memory Paging...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Enable Interrupts...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing ATA Disk...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing ATA Driver...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing Keyboard Driver...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing Timer...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing CMOS Driver...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);
    kprintf("Initializing Disk Stream...\n");
    ksleep(KMAIN_DEBUG_THROTTLE);

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

    struct multiboot_tag *tag;
    uint32_t size = *(uint32_t *)addr;

    for (tag = (struct multiboot_tag *)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        switch (tag->type)
        {
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
        {
            kprintf("Booted with the %s Bootloader.\n", ((struct multiboot_tag_string *)tag)->string);
            break;
        };
        case MULTIBOOT_TAG_TYPE_MMAP:
        {
            multiboot_memory_map_t *mmap;

            for (mmap = ((struct multiboot_tag_mmap *)tag)->entries;
                 (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
                 mmap = (multiboot_memory_map_t *)((uint32_t)mmap + ((struct multiboot_tag_mmap *)tag)->entry_size))
            {
                const uint64_t base_addr = (((uint64_t)mmap->addr_high << 32) | mmap->addr_low);
                kprintf("0x%x\n", base_addr);
            };
            break;
        };
        default:
        {
            break;
        };
        };
    };
    kprintf("\n- Runs on an i686 CPU.\n");
    kprintf("Date: %s, %d %s %d                                                \n", days[date.weekday - 1], date.day, months[date.month + 1], date.year);
    return;
};

void kmain(const uint32_t magic, const uint32_t addr)
{
    vga_display_init(&vga_display, (uint16_t *)0xb8000, 80, 25);
    vga_display_clear(&vga_display);
    cursor_set(0, 0);

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        kprintf("Invalid magic number: 0x%x\n", (unsigned)magic);
        return;
    };

    if (addr & 7)
    {
        kprintf("Unaligned mbi: 0x%x\n", addr);
        return;
    };
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

    kmotd(addr);
    return;
};