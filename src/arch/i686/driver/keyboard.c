/**
 * @file keyboard.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>

#include "io.h"
#include "keyboard.h"
#include "string.h"
#include "idt.h"

extern void asm_interrupt_21h(void);

Keyboard keyboard = {
    .enabled = false,
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

// Checks if the keyboard controller is ready
int32_t keyboard_wait(void)
{
    // Wait until the keyboard controller is ready
    for (;;)
    {
        // Read the status register
        const uint8_t status = asm_inb(KEYBOARD_CTRL_STATS_REG);

        // Check if the Output Buffer Full bit is set
        if (status & KEYBOARD_CTRL_STATS_MASK_OUT_BUF)
        {
            // Keyboard controller is ready
            return 1;
        };
    };
    // This line should not be reached
    return 0;
};

void keyboard_enable(Keyboard *self)
{
    for (;;)
    {
        // Read the status register
        const uint8_t status = asm_inb(KEYBOARD_CTRL_STATS_REG);

        // Check if the Input Buffer Full bit is not set
        if ((status & KEYBOARD_CTRL_STATS_MASK_IN_BUF) == 0)
        {
            break;
        };
    };
    asm_outb(KEYBOARD_CTRL_CMD_REG, KEYBOARD_CTRL_ENABLE);
    self->enabled = true;
    return;
};

void keyboard_init(Keyboard *self)
{
    keyboard_enable(self);
    self->caps = false;
    self->caps_lock = false;
    self->alt_gr = false;
    idt_set(0x21, asm_interrupt_21h);
    return;
};

static void keyboard_update_keystroke(const uint8_t makecode, const uint8_t breakcode)
{
    switch (makecode)
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
        keyboard.alt_gr = !keyboard.alt_gr;
        break;
    };
    case KLEFT_SHIFT:
    {
        keyboard.caps = !keyboard.caps;
        break;
    };
    case KCAPS_LOCK:
    {
        if (breakcode == 0)
        {
            keyboard.caps_lock = !keyboard.caps_lock;
        };
        break;
    };
    default:
    {
        if (breakcode == 0)
        {
            if (keyboard.caps || keyboard.caps_lock)
            {
                kprtf("%c", qwertz_upper[makecode]);
            }
            else if (keyboard.alt_gr)
            {
                kprtf("%c", qwertz_altgr[makecode]);
            }
            else
            {
                kprtf("%c", qwertz_lower[makecode]);
            };
            break;
        };
        break;
    };
    };
    return;
};

void keyboard_read(Keyboard *self)
{
    const uint8_t key_state = asm_inb(KEYBOARD_ENC_INPUT_BUF);
    const uint8_t makecode = key_state & 0x7f;
    const uint8_t breakcode = key_state & 0x80; // released == 128 pressed == 0
    keyboard_update_keystroke(makecode, breakcode);
    return;
};