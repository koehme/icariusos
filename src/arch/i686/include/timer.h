/**
 * @file timer.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef enum PitCommands
{
    PIT_BINARY_MODE = 0b00000000,    // Bit 0 => 16-Bit binary mode 0
    PIT_OPERATING_MODE = 0b00000110, // Bit 1-3 => Square wave generator 011
    PIT_ACCESS_MODE = 0b00110000,    // Access mode Bit 4 - 5 => 11 = lo / hi byte
    PIT_CHANNEL = 0b00000000,        // Select channel Bit 6 - 7 00
} PitCommands;

typedef enum PitPorts
{
    PIT_DATA_PORT_CHANNEL_0 = 0x40,  // Channel 0 data port (read/write)
    PIT_MODE_COMMAND_REGISTER = 0x43 // Mode/Command register (write only, a read is ignored)
} PITPorts;

/**
 * @struct Timer
 * @brief Represent a timer.
 * @details Holds information about the timer, including the number of ticks and the hz.
 */
typedef struct Timer
{
    uint64_t ticks; // Number of ticks since system booted
    uint8_t subticks;
    uint8_t max_subticks;
    uint32_t hz;
} Timer;

void timer_init(Timer *self, const uint32_t hz);

#endif