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
    .x = 0,
    .y = 0,
    .cycle = 0,
    .bytes = {0},
};

void mouse_handler(Mouse *self)
{
    switch (self->cycle)
    {
    case 0:
    {
        self->bytes[0] = ps2_receive();
        self->cycle++;
        break;
    };
    case 1:
    {
        self->bytes[1] = ps2_receive();
        self->cycle++;
        break;
    };
    case 2:
    {
        self->bytes[2] = ps2_receive();
        const int16_t rel_x = self->bytes[1] - ((self->bytes[0] << 3) & 0b100000000);
        const int16_t rel_y = self->bytes[2] - ((self->bytes[0] << 4) & 0b100000000);
        self->x += rel_x;
        self->y += rel_y;
        printf("(%d,%d)\n", self->x, self->y);
        self->cycle = 0;
        break;
    };
    };
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