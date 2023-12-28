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
};

typedef enum KBDKeycodes
{
    KEY_NOT_SET = 0xFF,
    KEY_ESCAPE = 0x01,
    KEY_LEFT_SHIFT = 0x2A,
    KEY_CAPS_LOCK = 0x3A,
    KEY_ONE = '1',
    KEY_EXCLAMATION_MARK = '!',
} KBDKeycodes;

const uint32_t uppercase[] = {
    KEY_NOT_SET,
    KEY_ESCAPE,
    KEY_EXCLAMATION_MARK,
};

const uint32_t lowercase[] = {
    KEY_NOT_SET,
    KEY_ESCAPE,
    KEY_ONE,
};

void kbd_update(const uint8_t scancode, const uint8_t presscode)
{
    switch (scancode)
    {
    case KEY_LEFT_SHIFT:
    {
        const bool prev_caps_state = kbd_state.caps;
        kbd_state.caps = !prev_caps_state;
        break;
    };
    case KEY_CAPS_LOCK:
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
                printf("%c", uppercase[scancode]);
            }
            else
            {
                printf("%c", lowercase[scancode]);
            };
            break;
        };
    };
        return;
    };
};

void kbd_read()
{
    const uint8_t kbd_data = asm_inb(KBD_DATA);
    const uint8_t scancode = kbd_data & 0x7f;
    const uint8_t presscode = kbd_data & 0x80; // released == 128, pressed == 0
    kbd_update(scancode, presscode);
    return;
};