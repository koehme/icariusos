/**
 * @file kbd.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "io.h"
#include "kbd.h"
#include "string.h"

KBD kbd_state = {
    .caps = false,
    .caps_lock = false,
    .alt_gr = false,
};

#define ESC 27
#define BACKSPACE '\b'
#define TAB '\t'
#define ENTER '\n'
#define RETURN '\r'
#define NEWLINE ENTER

typedef enum KeyCode
{
    KESC = 0x1,
    KLEFT_SHIFT = 0x2A,
    KRIGHT_SHIFT = 0x36,
    KCAPS_LOCK = 0x3A,
    KALTGR = 0x60,
} KeyCode;

const static uint8_t qwertz_lower[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 223, 180, 8,  // BACKSPACE
    9, 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 252, '+', 13, 0, // ENTER
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 246, 228, '#', 0, '<',
    'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-', 0, 0, 0, ' ', 0};

const static uint8_t qwertz_upper[] = {
    0, 0, '!', '"', '3', '$', '%', '&', '/', '(', ')', '=', '?', '`', 8,  // BACKSPACE
    9, 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 220, '*', 13, 0, // ENTER
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 214, 196, '\'', 0, '>',
    'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', 0, 0, 0, ' ', 0};

const static uint8_t qwertz_altgr[] = {0, 0, 0, 0, 0, 0, 0, 0, '{', '['};

void kbd_update(const uint8_t scancode, const uint8_t presscode)
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
        const bool prev_altgr_state = kbd_state.alt_gr;
        kbd_state.alt_gr = !prev_altgr_state;
        break;
    };
    case KLEFT_SHIFT:
    {
        const bool prev_caps_state = kbd_state.caps;
        kbd_state.caps = !prev_caps_state;
        break;
    };
    case KCAPS_LOCK:
    {
        const bool prev_caps_lock_state = kbd_state.caps_lock;

        if (prev_caps_lock_state == false && presscode == 0)
        {
            kbd_state.caps_lock = true;
        }
        else if (prev_caps_lock_state == true && presscode == 0)
        {
            kbd_state.caps_lock = false;
        };
        break;
    };
    default:
    {
        if (presscode == 0)
        {
            if (kbd_state.caps || kbd_state.caps_lock)
            {
                printf("%c", qwertz_upper[scancode]);
            }
            else if (kbd_state.alt_gr)
            {
                printf("%c", qwertz_altgr[scancode]);
            }
            else
            {
                printf("%c", qwertz_lower[scancode]);
            };
            break;
        };
    };
        return;
    };
    return;
};

void kbd_read()
{
    const uint8_t kbd_data = asm_inb(KBD_DATA);
    const uint8_t scancode = kbd_data & 0x7f;
    const uint8_t presscode = kbd_data & 0x80; // released == 128, pressed == 0
    kbd_update(scancode, presscode);
    return;
};