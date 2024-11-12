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

void timer_init(Timer* self, const uint32_t hz)
{
	self->ticks = 0;
	self->hz = hz;
	idt_set(0x20, asm_interrupt_20h);
	// Base frequency of the PIT (Programmable Interval Timer)
	const uint32_t pit_base_frequency = 1193180;
	// Divisor is required to configure the PIT so that it ticks at a specific interval corresponding to the desired hz
	const uint32_t divisor = pit_base_frequency / hz;
	// Merge all the necessary configuration settings for the PIT
	const uint8_t cmd = PIT_CHANNEL | PIT_ACCESS_MODE | PIT_OPERATING_MODE | PIT_BINARY_MODE;
	// Send the timer's configuration
	outb(PIT_MODE_COMMAND_REGISTER, cmd);
	// Extracts the low and high byte of the 16-bit divisor value
	const uint8_t divisor_high_byte = (uint8_t)(divisor >> 8) & 0xFF;
	const uint8_t divisor_low_byte = (uint8_t)divisor & 0xFF;
	// Send the timer's frequency
	outb(PIT_DATA_PORT_CHANNEL_0, divisor_low_byte);
	outb(PIT_DATA_PORT_CHANNEL_0, divisor_high_byte);
	return;
};