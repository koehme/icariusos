
/**
 * @file ps2.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PS2_H
#define PS2_H

#include <stdint.h>

typedef enum PS2Port
{
    PS2_DATA_PORT = 0x60,
    PS2_STATUS_COMMAND_PORT = 0x64,
} PS2Port;

void ps2_send(const uint8_t port, const uint8_t byte);
uint8_t ps2_receive(void);

#endif