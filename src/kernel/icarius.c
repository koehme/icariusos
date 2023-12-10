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

extern VGADisplay vga_display;
extern HeapDescriptor kheap_descriptor;
extern Heap kheap;

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

    heap_init(&kheap, &kheap_descriptor, (void *)0x01000000, (void *)0x00007e00, 1024 * 1024 * 100, 4096);
    void *p1 = heap_malloc(&kheap, 4096);
    void *p2 = heap_malloc(&kheap, 50);

    if (p1 || p2)
    {
    };
    idt_init();
    asm_do_sti();
    return;
};