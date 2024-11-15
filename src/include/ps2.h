
/**
 * @file ps2.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PS2_H
#define PS2_H

#include "fifo.h"

#include <stdint.h>

#define PS2_PACKAGE_DISPATCH 5
// PS/2 Buffer Status Flags
#define PS2_BUFFER_OUTPUT 0x01
#define PS2_BUFFER_INPUT 0x02
// PS/2 Ports
#define PS2_DATA_PORT 0x60
#define PS2_STATUS_COMMAND_PORT 0x64

void ps2_send(const uint8_t port, const uint8_t byte);
uint8_t ps2_receive(void);
void ps2_dispatch(fifo_t* fifo, void (*handler)(void*, uint8_t), void* device);

#endif