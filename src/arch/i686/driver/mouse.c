/**
 * @file mouse.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "mouse.h"
#include "keyboard.h"

Mouse mouse = {
    .enabled = false,
    .x = 0,
    .y = 0,
};
// Waits until the specified condition in the mouse status register is met
void mouse_wait(const uint8_t a_type)
{
    uint32_t _time_out = 100000;

    if (a_type == 0)
    {
        // Wait until the output buffer of the mouse status register is ready
        while (_time_out--)
        {
            // Check if the output buffer is ready (bit 0 is set)
            if ((asm_inb(PS2_CTRL_PORT) & 1) == 1)
            {
                // Exit the function once the output buffer is ready
                return;
            };
        };
        // Return if the timeout limit is reached
        return;
    }
    else
    {
        // Wait until the input buffer of the mouse status register is ready
        while (_time_out--)
        {
            // Check if the input buffer is ready (bit 1 is clear)
            if ((asm_inb(PS2_CTRL_PORT) & 2) == 0)
            {
                // Exit the function once the input buffer is ready
                return;
            };
        };
        // Return if the timeout limit is reached
        return;
    };
    // Just to be explicit, though this point should not be reached
    return;
};

void mouse_init(void)
{
    uint8_t status;
    // Wait until the input buffer of the mouse status register is ready
    mouse_wait(1);
    // Enable interrupt
    asm_outb(PS2_DATA_PORT, MOUSE_ENABLE);
    // Wait until the input buffer of the mouse status register is ready
    mouse_wait(1);
    // Read command byte
    asm_outb(PS2_DATA_PORT, MOUSE_READ);
    // Wait until the output buffer of the mouse status register is ready
    mouse_wait(0);
    status = (asm_inb(PS2_DATA_PORT) | 2);
    kprtf("%d\n", status);
    return;
};