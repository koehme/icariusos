/**
 * @file icarius.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "icarius.h"
#include "vga.h"
#include "idt.h"
#include "io.h"

void kprint(const char *str)
{
    vga_print(str);
    return;
};

void kmain(void)
{
    vga_display_init((volatile uint16_t *)0xb8000, 80, 25);
    vga_display_clear();

    kprint("Icarius Kernel!\n");

    idt_init();

    kprint("Test\n");

    outb(0x60, 0xFF); // Test for gdb (96,255)

    return;
};