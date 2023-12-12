/**
 * @file icarius.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "icarius.h"
#include "vga.h"
#include "idt.h"
#include "io.h"
#include "heap.h"
#include "cursor.h"

extern VGADisplay vga_display;
extern HeapDescriptor kheap_descriptor;
extern Heap kheap;

void *kmalloc(const size_t size)
{
    void *ptr = 0x0;
    ptr = heap_malloc(&kheap, size);
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
    return;
};

void kprint(const char *str)
{
    vga_print(&vga_display, str, VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    return;
};

void kpanic(const char *str)
{
    vga_print(&vga_display, str, VGA_COLOR_RED | (VGA_COLOR_BLACK << 4));

    for (;;)
    {
    };
    return;
};

void ksleep(const int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        for (int j = 0; j < 1000000; j++)
        {
        };
    };
    return;
};

void kascii_spinner(const int frames, const int delay)
{
    const char *spin_frames[] = {"-", "\\", "|", "/"};
    const int total_spin_frames = 4;

    for (int i = 0; i < frames; i++)
    {
        const char *curr_frame = spin_frames[i % total_spin_frames];
        kprint_color(curr_frame, VGA_COLOR_LIGHT_GREEN);
        ksleep(delay);
        kprint("\b");
    };
    return;
};

void kprint_logo(void)
{
    kprint("   \\    /   \n");
    kprint("   (\\--/)   \n");
    kprint("    /  \\    \n");
    return;
};

void kprint_motd()
{
    kascii_spinner(60, 50);
    vga_display_clear(&vga_display);
    vga_display_reset_cursor(&vga_display);
    kprint_logo();
    kprint_color("Welcome to icariusOS\n", VGA_COLOR_LIGHT_MAGENTA);
    return;
};

void kmain(void)
{
    vga_display_init(&vga_display, (volatile uint16_t *)0xb8000, 80, 25);
    vga_display_clear(&vga_display);

    kprint_color("Initializing Stack at 0x00200000...\n", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Initializing VGA Textmode Buffer at 0xb8000...\n", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Clearing VGA Textmode Buffer...\n", VGA_COLOR_LIGHT_GREEN);

    cursor_set(0, 0);
    kprint_color("Initializing VGA Textmode Cursor at (0,0)...\n", VGA_COLOR_LIGHT_GREEN);

    heap_init(&kheap, &kheap_descriptor, (void *)0x01000000, (void *)0x00007e00, 1024 * 1024 * 100, 4096);
    kprint_color("Initializing Kheap Datapool at 0x01000000...\n", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Initializing Kheap Descriptor at 0x00007e00...\n", VGA_COLOR_LIGHT_GREEN);

    void *p1 = kmalloc(1024); // Should be 0x01000000
    void *p2 = kmalloc(8192); // Should be 0x01001000
    void *p3 = kmalloc(1024); // Should be 0x01004000

    kfree(p1);

    idt_init();
    kprint_color("Initializing Global Descriptor Table...\n", VGA_COLOR_LIGHT_GREEN);

    asm_do_sti();
    kprint_color("Enable Interrupts...\n", VGA_COLOR_LIGHT_GREEN);

    kprint_motd();
    return;
};