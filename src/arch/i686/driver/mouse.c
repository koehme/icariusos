/**
 * @file mouse.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "mouse.h"
#include "idt.h"

extern void asm_interrupt_32h(void);

Mouse mouse = {
    .enabled = false,
    .rel_x = 0,
    .rel_y = 0,
    .abs_x = 0,
    .abs_y = 0,
    .mouse_cycle = 0,
    .byte0 = 0,
    .byte1 = 0,
    .byte2 = 0,
    .byte3 = 0,
    .byte4 = 0,
};

void mouse_handler(Mouse *self)
{
    switch (self->mouse_cycle)
    {
    case MOUSE_FIRST_CYCLE:
    {
        self->byte0 = asm_inb(MOUSE_DATA_PORT);
        self->mouse_cycle++;
        break;
    };
    case MOUSE_SECOND_CYCLE:
    {
        self->byte1 = asm_inb(MOUSE_DATA_PORT);
        self->mouse_cycle++;
        break;
    };
    case MOUSE_THIRD_CYCLE:
    {
        self->byte2 = asm_inb(MOUSE_DATA_PORT);
        // Update position
        self->rel_x = self->byte1;
        self->rel_y = self->byte2;
        // Reset cycles
        self->mouse_cycle = MOUSE_FIRST_CYCLE;
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
    uint8_t status;
    // Enable the auxiliary mouse device
    mouse_wait(MOUSE_INPUT_BUFFER);
    asm_outb(MOUSE_STATUS_PORT, MOUSE_ENABLE_AUX);
    // Enable the interrupts
    mouse_wait(MOUSE_INPUT_BUFFER);
    asm_outb(MOUSE_STATUS_PORT, MOUSE_GET_COMPAQ_STATUS);
    mouse_wait(MOUSE_OUTPUT_BUFFER);
    status = asm_inb(MOUSE_DATA_PORT) | 0b00000010;
    mouse_wait(MOUSE_INPUT_BUFFER);
    asm_outb(MOUSE_STATUS_PORT, MOUSE_SET_COMPAQ_STATUS);
    mouse_wait(MOUSE_INPUT_BUFFER);
    asm_outb(MOUSE_DATA_PORT, status);
    // Tell the mouse to use default settings
    mouse_write(MOUSE_SET_DEFAULT);
    mouse_read();
    // Enable data reporting
    mouse_write(MOUSE_ENABLE_DATA_REPORT);
    const uint8_t answer = mouse_read();

    if (answer == MOUSE_DETECT_ACK)
    {
        self->enabled = true;
        idt_set(0x2C, asm_interrupt_32h);
    };
    return;
};