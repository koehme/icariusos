
/**
 * @file ps2.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PS2_H
#define PS2_H

#include "fifo.h"

#include <stdint.h>

#define PS2_PACKAGE_DISPATCH 32

// PS/2 Buffer Status Flags
typedef enum ps2_buffer_t {
	PS2_BUFFER_OUTPUT = 0x01, // Output buffer
	PS2_BUFFER_INPUT = 0x02	  // Input buffer
} ps2_buffer_t;

// PS/2 Ports
#define PS2_DATA_PORT 0x60
#define PS2_STATUS_COMMAND_PORT 0x64

int ps2_wait(const ps2_buffer_t type);
void ps2_send(const uint8_t port, const uint8_t byte);
uint8_t ps2_receive(void);
void ps2_dispatch(fifo_t* fifo, void (*handler)(void*, uint8_t), void* device);

#endif