/**
 * @file ps2.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ps2.h"
#include "io.h"

// Waits until the specified condition in the PS/2 status register is met
static void ps2_wait(const uint8_t mask)
{
    uint32_t timeout = 100000;

    while (timeout--)
    {
        const uint8_t status = asm_inb(PS2_STATUS_COMMAND_PORT) & mask;

        if (status)
        {
            return;
        };
    };
    return;
};

// Sends data to a PS/2 port after waiting for the port to be ready
void ps2_send(const uint8_t port, const uint8_t byte)
{
    // Wait until the PS/2 port is ready to accept data
    ps2_wait(0b00000010);
    // Send the data byte to the specified PS/2 port
    asm_outb(port, byte);
    return;
};

// Receives data from a PS/2 port after waiting for data to be available.
uint8_t ps2_receive(void)
{
    ps2_wait(0b00000001);
    return asm_inb(PS2_DATA_PORT);
};
