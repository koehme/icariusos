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
    PIT_ACCESS_MODE = 0b00110000,    // Access mode Bit 4 - 5 => 1 1 = lo / hi byte 11
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
 * @details Holds information about the timer, including the number of ticks and the frequency.
 * @note The `frequency` member represents the frequency of the timer and is historically
 *       tied to the original PC's Programmable Interval Timer (PIT) chip, which used
 *       an oscillator running at approximately 1.193182 MHz. This frequency was derived
 *       from a 14.31818 MHz base oscillator, a method used for cost reduction in the
 *       late 1970s. The PIT chip serves as a reminder of the "good old times" when
 *       such techniques were employed due to cost constraints.
 */
typedef struct Timer
{
    uint64_t ticks;
    uint32_t frequency;
} Timer;

void timer_init(Timer *self, const uint32_t frequency);

#endif