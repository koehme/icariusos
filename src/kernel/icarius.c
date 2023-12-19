/**
 * @file icarius.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "icarius.h"
#include "mem.h"
#include "page.h"

extern VGADisplay vga_display;
extern HeapDescriptor kheap_descriptor;
extern Heap kheap;
extern PageDirectory kpage_dir;

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
    return;
};

void kprint(const char *str)
{
    vga_print(&vga_display, str, VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
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
    const char *spin_frames[4] = {"-", "\\", "|", "/"};

    for (int i = 0; i < frames; i++)
    {
        const char *curr_frame = spin_frames[i % 4];
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

void kprint_motd(void)
{
    kascii_spinner(60, 50);
    vga_display_clear(&vga_display);
    vga_display_set_cursor(&vga_display, 0, 0);
    kprint_logo();
    kprint_color("Welcome to icariusOS\n", VGA_COLOR_LIGHT_MAGENTA);
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

    PageDirectory *ptr_kpage_dir = &kpage_dir;
    page_init_directory(&kpage_dir, PAGE_PRESENT | PAGE_READ_WRITE | PAGE_USER_SUPERVISOR);
    page_switch(ptr_kpage_dir->directory);
    asm_page_enable();

    // Assume ptr_phy_addr = 0x1802000
    char *ptr_phy_addr = kcalloc(4096);
    virt_address_map(ptr_kpage_dir->directory, (void *)0x2000, (uint32_t)ptr_phy_addr | PAGE_ACCESSED | PAGE_PRESENT | PAGE_READ_WRITE);

    asm_do_sti();
    kprint_color("Enable Interrupts...\n", VGA_COLOR_LIGHT_GREEN);

    char *ptr_virt = (char *)0x2000;

    // Change the virtual address to see reflected changes because we map 0x2000 => to a physical addr 0x1802000
    ptr_virt[0] = 'A';
    ptr_virt[1] = 'W';
    ptr_virt[2] = 'E';
    ptr_virt[3] = 'S';
    ptr_virt[4] = 'O';
    ptr_virt[5] = 'M';
    ptr_virt[6] = 'E';
    ptr_virt[7] = '!';

    kprint_color("Initializing Virtual Memory Paging...\n", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Testing Paging...\nVirtual Address 0x2000 now points to Physical Address 0x1802000...\n", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Virtual 0x2000 [", VGA_COLOR_LIGHT_GREEN);
    kprint_color(ptr_virt, VGA_COLOR_LIGHT_GREEN);
    kprint_color("] == ", VGA_COLOR_LIGHT_GREEN);
    kprint_color("Physical 0x1802000 [", VGA_COLOR_LIGHT_GREEN);
    kprint_color(ptr_phy_addr, VGA_COLOR_LIGHT_GREEN);
    kprint_color("]\n", VGA_COLOR_LIGHT_GREEN);

    kprint_motd();
    return;
};