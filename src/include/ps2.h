
/**
 * @file ps2.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PS2_H
#define PS2_H

#include "fifo.h"

#include <stdint.h>

typedef enum PS2Buffer {
	PS2_BUFFER_OUTPUT = 0x01, // Represents the status of the output buffer
	PS2_BUFFER_INPUT = 0x02,  // Represents the status of the input buffer
} PS2Buffer;

typedef enum PS2Port {
	PS2_DATA_PORT = 0x60,
	PS2_STATUS_COMMAND_PORT = 0x64,
} PS2Port;

void ps2_send(const uint8_t port, const uint8_t byte);
uint8_t ps2_receive(void);
void ps2_process_dev(fifo_t* fifo, void (*handler)(void*, uint8_t), void* device);

#endif