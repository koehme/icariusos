/**
 * @file timer.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "timer.h"
#include "idt.h"
#include "io.h"

extern void asm_interrupt_20h(void);

Timer timer = {
    .ticks = 0,
    .hz = 0,
};

void timer_init(Timer *self, const uint32_t hz)
{
    self->ticks = 0;
    self->hz = hz;
    idt_set(0x20, asm_interrupt_20h);

    const uint32_t base_frequency = 1193180;
    const uint32_t divisor = base_frequency / hz;

    const uint8_t cmd = PIT_CHANNEL | PIT_ACCESS_MODE | PIT_OPERATING_MODE | PIT_BINARY_MODE;
    asm_outb(PIT_MODE_COMMAND_REGISTER, cmd);

    const uint8_t divisor_low_byte = (uint8_t)divisor & 0xFF;
    const uint8_t divisor_high_byte = (uint8_t)(divisor >> 8) & 0xFF;

    asm_outb(PIT_DATA_PORT_CHANNEL_0, divisor_low_byte);
    asm_outb(PIT_DATA_PORT_CHANNEL_0, divisor_high_byte);
    return;
};