/**
 * @file timer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// PIT Commands
#define PIT_BINARY_MODE 0b00000000    // 16-Bit binary mode (Bit 0)
#define PIT_OPERATING_MODE 0b00000110 // Square wave generator (Bit 1-3: 011)
#define PIT_ACCESS_MODE 0b00110000    // Access mode lo/hi byte (Bit 4-5: 11)
#define PIT_CHANNEL 0b00000000	      // Select channel 0 (Bit 6-7: 00)
// PIT Ports
#define PIT_DATA_PORT_CHANNEL_0 0x40   // Channel 0 data port (read/write)
#define PIT_MODE_COMMAND_REGISTER 0x43 // Mode/Command register (write only)

// Holds information about the number of ticks and the hz
typedef struct timer {
	uint64_t ticks; // Number of ticks since system booted
	uint32_t hz;	// Frequency (in Hertz) at which the timer_t operates
} timer_t;

void timer_init(timer_t* self, const uint32_t hz);

#endif