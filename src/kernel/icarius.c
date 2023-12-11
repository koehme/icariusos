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

void kprint(const char *str)
{
    vga_print(&vga_display, str);
    return;
};

void kpanic(const char *str)
{
    vga_print(&vga_display, str);

    for (;;)
    {
    };
    return;
};

void kmain(void)
{
    vga_display_init(&vga_display, (volatile uint16_t *)0xb8000, 80, 25);
    vga_display_clear(&vga_display);
    cursor_set(0, 0);

    heap_init(&kheap, &kheap_descriptor, (void *)0x01000000, (void *)0x00007e00, 1024 * 1024 * 100, 4096);

    void *p1 = kmalloc(1024); // Should be 0x01000000
    void *p2 = kmalloc(8192); // Should be 0x01001000
    void *p3 = kmalloc(1024); // Should be 0x01004000

    idt_init();
    asm_do_sti();
    return;
};