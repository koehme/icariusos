/**
 * @file mouse.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "mouse.h"
#include "ps2.h"
#include "idt.h"

extern void asm_interrupt_32h(void);

Mouse mouse = {
    .rel_x = 0,
    .rel_y = 0,
    .cycle = 0,
    .byte0 = 0,
    .byte1 = 0,
    .byte2 = 0,
    .byte3 = 0,
    .byte4 = 0,
};

void mouse_handler(Mouse *self)
{
    switch (self->cycle)
    {
    case MOUSE_BYTE_1:
    {
        self->byte0 = asm_inb(MOUSE_DATA_PORT);
        self->cycle++;
        break;
    };
    case MOUSE_BYTE_2:
    {
        self->byte1 = asm_inb(MOUSE_DATA_PORT);
        self->cycle++;
        break;
    };
    case MOUSE_BYTE_3:
    {
        self->byte2 = asm_inb(MOUSE_DATA_PORT);
        // Check axis overflow
        const bool has_y_overflow = (self->bytes[0] & 0b10000000);
        const bool has_x_overflow = (self->bytes[0] & 0b01000000);

        if (has_y_overflow || has_x_overflow)
        {
            break;
        };
        self->rel_x = self->byte1;
        self->rel_y = self->byte2;
        self->cycle = 0;
        break;
    };
    };
    return;
};

// Waits until the specified condition in the mouse status register is met
void mouse_wait(const MouseBufferType type)
{
    uint32_t timer = 100000;

    switch (type)
    {
    case MOUSE_OUTPUT_BUFFER:
    {
        while (timer--)
        {
            const uint8_t status = asm_inb(MOUSE_STATUS_PORT) & 0b00000001;

            if (status == 1)
            {
                return;
            };
        };
        return;
    };
    case MOUSE_INPUT_BUFFER:
    {
        while (timer--)
        {
            const uint8_t status = asm_inb(MOUSE_STATUS_PORT) & 0b00000010;

            if (status == 0)
            {
                return;
            };
        };
        return;
    };
    };
    return;
};

uint8_t mouse_read(void)
{
    mouse_wait(MOUSE_OUTPUT_BUFFER);
    const uint8_t data = asm_inb(MOUSE_DATA_PORT);
    return data;
};

void mouse_write(const uint8_t cmd)
{
    mouse_wait(MOUSE_INPUT_BUFFER);
    asm_outb(MOUSE_STATUS_PORT, MOUSE_SEND_COMMAND);
    mouse_wait(MOUSE_INPUT_BUFFER);
    asm_outb(MOUSE_DATA_PORT, cmd);
    return;
};

void mouse_init(Mouse *self)
{
    // Enable the auxiliary mouse device
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_ENABLE_AUX);
    // Enable the interrupts
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_GET_COMPAQ_STATUS);
    const uint8_t status = ps2_receive() | 0b00000010;
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SET_COMPAQ_STATUS);
    ps2_send(PS2_DATA_PORT, status);
    // Tell the mouse to use default settings
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SEND_COMMAND);
    ps2_send(PS2_DATA_PORT, MOUSE_SET_DEFAULT);
    ps2_receive();
    // Enable data reporting
    ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SEND_COMMAND);
    ps2_send(PS2_DATA_PORT, MOUSE_ENABLE_DATA_REPORT);
    ps2_receive();
    idt_set(0x2C, asm_interrupt_32h);
    return;
};