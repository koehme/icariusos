/**
 * @file icarius.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "icarius.h"

extern VGADisplay vga_display;
extern HeapDescriptor kheap_descriptor;
extern Heap kheap;
extern PageDirectory kpage_dir;
extern PLexer plexer;
extern PParser pparser;

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

void ksleep(const int n_times)
{
    for (int i = 0; i < n_times; i++)
    {
        for (int j = 0; j < 1000000; j++)
        {
        };
    };
    return;
};

void kspinner(const int frames, const int delay)
{
    const char *spinner_frames[4] = {"-", "\\", "|", "/"};

    for (int i = 0; i < frames; i++)
    {
        const char *curr_frame = spinner_frames[i % 4];
        kprintf(curr_frame);
        ksleep(50);
        kprintf("\b");
    };
    return;
};

void kmotd(void)
{
    kspinner(60, KDEBUG_SLOW_DOWN);
    kprintf("   \\    /   \n");
    kprintf("   (\\--/)   \n");
    kprintf("    /  \\    \n");
    kprintf("Welcome to icarius");
    kprintf("OS\n");
    return;
};

void kmain(void)
{
    vga_display_init(&vga_display, (uint16_t *)0xb8000, 80, 25);
    vga_display_clear(&vga_display);

    kprintf("Initializing Stack at 0x00200000...\n");
    kprintf("Initializing VGA Textmode Buffer at 0xb8000...\n");
    kprintf("Clearing VGA Textmode Buffer...\n");

    cursor_set(0, 0);
    kprintf("Initializing VGA Textmode Cursor at (0,0)...\n");

    heap_init(&kheap, &kheap_descriptor, (void *)0x01000000, (void *)0x00007e00, 1024 * 1024 * 100, 4096);
    kprintf("Initializing Kheap Datapool at 0x01000000...\n");
    kprintf("Initializing Kheap Descriptor at 0x00007e00...\n");

    idt_init();
    kprintf("Initializing Global Descriptor Table...\n");

    kprintf("Initializing Virtual Memory Paging...\n");
    PageDirectory *ptr_kpage_dir = &kpage_dir;
    page_init_directory(&kpage_dir, PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPERVISOR);
    page_switch(ptr_kpage_dir->directory);
    asm_page_enable();

    kprintf("Enable Interrupts...\n");
    asm_do_sti();

    kprintf("Initializing ATA Disk...\n");
    ATADisk *ptr_ata_disk = ata_get_disk(ATA_DISK_A);
    ata_init(ptr_ata_disk);
    kprintf("Initializing ATA Driver...\n");
    ata_read(ptr_ata_disk, 0, ptr_ata_disk->buffer, 1);

    plexer_init(&plexer, "A:/bin/cli.exe");
    PathRootNode *ptr_root_node = pparser_parse(&pparser, &plexer);

    kmotd();
    return;
};