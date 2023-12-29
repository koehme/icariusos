/**
 * @file kbd.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "io.h"
#include "keyboard.h"
#include "string.h"

Keyboard keyboard_state = {
    .caps = false,
    .caps_lock = false,
    .alt_gr = false,
};

typedef enum KeyCode
{
    KESC = 0x1,
    KLEFT_SHIFT = 0x2A,
    KRIGHT_SHIFT = 0x36,
    KCAPS_LOCK = 0x3A,
    KALTGR = 0x60,
} KeyCode;

const static uint8_t qwertz_lower[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 223, 180, '\b',
    9, 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 252, '+', 13, 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 246, 228, '^', 0, '<',
    'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, 0, 0, ' ', 0};

const static uint8_t qwertz_upper[] = {
    0, 0, '!', '"', '3', '$', '%', '&', '/', '(', ')', '=', '?', '`', 8,
    9, 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 220, '*', 13, 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 214, 196, '^', 0, '>',
    'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 0, 0, 0, ' ', 0};

const static uint8_t qwertz_altgr[] = {0, 0, 0, 0, 0, 0, 0, 0, '{', '['};

static void keyboard_update_keystroke(const uint8_t scancode, const uint8_t presscode)
{
    switch (scancode)
    {
    case 1:
    case 29:
    case 56:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 87:
    case 88:
        break;
    case KALTGR:
    {
        keyboard_state.alt_gr = !keyboard_state.alt_gr;
        break;
    };
    case KLEFT_SHIFT:
    {
        keyboard_state.caps = !keyboard_state.caps;
        break;
    };
    case KCAPS_LOCK:
    {
        if (presscode == 0)
        {
            keyboard_state.caps_lock = !keyboard_state.caps_lock;
        };
        break;
    };
    default:
    {
        if (presscode == 0)
        {
            if (keyboard_state.caps || keyboard_state.caps_lock)
            {
                kprintf("%c", qwertz_upper[scancode]);
            }
            else if (keyboard_state.alt_gr)
            {
                kprintf("%c", qwertz_altgr[scancode]);
            }
            else
            {
                kprintf("%c", qwertz_lower[scancode]);
            };
            break;
        };
    };
        return;
    };
    return;
};

static void keyboard_update(const uint8_t scancode, const uint8_t presscode)
{
    keyboard_update_keystroke(scancode, presscode);
    return;
};

void keyboard_read()
{
    const uint8_t kb_data = asm_inb(KB_DATA);
    const uint8_t scancode = kb_data & 0x7f;
    const uint8_t presscode = kb_data & 0x80; // released == 128 pressed == 0
    keyboard_update(scancode, presscode);
    return;
};