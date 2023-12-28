/**
 * @file kbd.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "io.h"
#include "kbd.h"
#include "string.h"

#include <stdbool.h>

void kbd_read()
{
    const uint8_t kbd_data = asm_inb(KBD_DATA);
    const uint8_t scancode = kbd_data & 0x7f;
    const uint8_t presscode = kbd_data & 0x80; // released == 128, pressed == 0
    printf("(Scancode: %d, Pressed: %s)\n", scancode, presscode == 0 ? "true" : "false");
    return;
};