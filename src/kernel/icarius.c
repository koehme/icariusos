/**
 * @file icarius.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "icarius.h"
#include "vga.h"
#include "idt.h"
#include "io.h"

extern VGADisplay vga_display;

void kprint(const char *str)
{
    vga_print(&vga_display, str);
    return;
};

void kmain(void)
{
    vga_display_init(&vga_display, (volatile uint16_t *)0xb8000, 80, 25);
    vga_display_clear(&vga_display);
    idt_init();
    asm_do_sti();
    return;
};