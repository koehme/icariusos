/**
 * @file icarius.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "icarius.h"
#include "string.h"
#include "pathlexer.h"
#include "pathparser.h"

extern VGADisplay vga_display;
extern HeapDescriptor kheap_descriptor;
extern Heap kheap;
extern PageDirectory kpage_dir;
extern PathLexer plexer;
extern PathParser pparser;

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

void kprint_color(const char *str, const VGAColor color)
{
    vga_print(&vga_display, str, color);
    // ksleep(KDEBUG_SLOW_DOWN);
    return;
};

void kprint(const char *str)
{
    vga_print(&vga_display, str, VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    return;
};

void kprint_rainbow(const char *str)
{
    const VGAColor rainbow_colors[7] = {
        VGA_COLOR_LIGHT_RED,
        VGA_COLOR_BROWN,
        VGA_COLOR_YELLOW,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_LIGHT_BLUE,
        VGA_COLOR_LIGHT_MAGENTA,
    };
    const size_t len = slen(str);

    for (size_t i = 0; i < len; i++)
    {
        const VGAColor color = rainbow_colors[i % 7];
        const char ch = str[i];
        vga_print_ch(&vga_display, ch, color);
    };
    return;
};

void kprint_hex(const uint8_t value)
{
    char buffer[5] = {'0', 'x', 0x0, 0x0, '\0'};

    const char *hex_chars = "0123456789ABCDEF";
    const uint8_t upper_nibble = (value >> 4) & 0b00001111;
    const uint8_t lower_nibble = value & 0b00001111;

    buffer[2] = hex_chars[upper_nibble];
    buffer[3] = hex_chars[lower_nibble];
    kprint_color(buffer, VGA_COLOR_LIGHT_GREEN);
    return;
};

void kpanic(const char *str)
{
    vga_print(&vga_display, str, VGA_COLOR_LIGHT_RED | (VGA_COLOR_BLACK << 4));

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

void kascii_spinner(const int frames, const int delay)
{
    const char *spinner_frames[4] = {"-", "\\", "|", "/"};

    for (int i = 0; i < frames; i++)
    {
        const char *curr_frame = spinner_frames[i % 4];
        kprint(curr_frame);
        ksleep(50);
        kprint("\b");
    };
    return;
};

void kprint_logo(void)
{
    kprint_rainbow("   \\    /   \n");
    kprint_rainbow("   (\\--/)   \n");
    kprint_rainbow("    /  \\    \n");
    return;
};

void kprint_motd(void)
{
    kascii_spinner(60, 50);
    kprint_logo();
    kprint_color("Welcome to icarius", VGA_COLOR_GREEN);
    kprint_color("OS\n", VGA_COLOR_LIGHT_GREEN);
    return;
};

void kmain(void)
{
    vga_display_init(&vga_display, (uint16_t *)0xb8000, 80, 25);
    vga_display_clear(&vga_display);

    kprint_color("Initializing Stack at 0x00200000...\n", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Initializing VGA Textmode Buffer at 0xb8000...\n", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Clearing VGA Textmode Buffer...\n", VGA_COLOR_LIGHT_GREEN);

    cursor_set(0, 0);
    kprint_color("Initializing VGA Textmode Cursor at (0,0)...\n", VGA_COLOR_LIGHT_GREEN);

    heap_init(&kheap, &kheap_descriptor, (void *)0x01000000, (void *)0x00007e00, 1024 * 1024 * 100, 4096);
    kprint_color("Initializing Kheap Datapool at 0x01000000...\n", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Initializing Kheap Descriptor at 0x00007e00...\n", VGA_COLOR_LIGHT_GREEN);

    idt_init();
    kprint_color("Initializing Global Descriptor Table...\n", VGA_COLOR_LIGHT_GREEN);

    kprint_color("Initializing Virtual Memory Paging...\n", VGA_COLOR_LIGHT_GREEN);
    PageDirectory *ptr_kpage_dir = &kpage_dir;
    page_init_directory(&kpage_dir, PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPERVISOR);
    page_switch(ptr_kpage_dir->directory);
    asm_page_enable();

    kprint_color("Enable Interrupts...\n", VGA_COLOR_LIGHT_GREEN);
    asm_do_sti();

    kprint_color("Initializing ATA Disk...\n", VGA_COLOR_LIGHT_GREEN);
    ATADisk *ptr_ata_disk = ata_get_disk(ATA_DISK_A);
    ata_init(ptr_ata_disk);
    kprint_color("Initializing ATA Driver...\n", VGA_COLOR_LIGHT_GREEN);
    ata_read(ptr_ata_disk, 0, ptr_ata_disk->buffer, 1);

    // kprint_motd();
    // kprint(">");

    path_lexer_init(&plexer, "A:/bin/cli.exe");
    PathRootNode *ptr_root_node = path_parser_parse(&pparser, &plexer);
    return;
};