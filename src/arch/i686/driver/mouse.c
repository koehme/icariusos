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
    .flags = 0,
    .x_movement = 0,
    .y_movement = 0,
    .bytes = {0},
};

static bool mouse_has_aligned_packet(const Mouse *self)
{
    return (self->flags & 0b00001000) != 0;
};

static bool mouse_has_x_overflow(const Mouse *self)
{
    return (self->flags & 0b10000000) != 0;
};

static bool mouse_has_y_overflow(const Mouse *self)
{
    return (self->flags & 0b01000000) != 0;
};

static void mouse_dump(const Mouse *self, const int16_t delta_x, const int16_t delta_y)
{
    printf("(%d,%d | %d,%d)\n", delta_x, delta_y, self->x, self->y);
    return;
};

void mouse_handler(Mouse *self)
{
    switch (self->cycle)
    {
    case 0:
    {
        self->flags = ps2_receive();

        if (!mouse_has_aligned_packet(self))
        {
            self->cycle = 0;
            break;
        };
        self->cycle++;
        break;
    };
    case 1:
    {
        self->x_movement = ps2_receive();
        self->cycle++;
        break;
    };
    case 2:
    {
        self->y_movement = ps2_receive();

        if (!mouse_has_x_overflow(self) || !mouse_has_y_overflow(self))
        {
            const int16_t delta_x = self->x_movement - ((self->flags << 3) & 0b100000000);
            const int16_t delta_y = self->y_movement - ((self->flags << 4) & 0b100000000);
            self->x += delta_x;
            self->y += delta_y;
            mouse_dump(self, delta_x, delta_y);
        };
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