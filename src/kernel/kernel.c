/**
 * @file kernel.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kernel.h"
#include "vga.h"

void kmain(void)
{
    VgaDisplay display;
    vga_display_init(&display, (uint16_t *)0xb8000, 80, 25);
    vga_display_clear(&display);

    vga_print(&display, "Hello World!\n");
    vga_print(&display, "Icarius Kernel!");
    return;
};