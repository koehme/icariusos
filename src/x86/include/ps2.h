
/**
 * @file ps2.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PS2_H
#define PS2_H

#include "fifo.h"
#include "icarius.h"

#include <stdint.h>

int ps2_wait(const uint8_t type);
void ps2_send(const uint8_t port, const uint8_t byte);
uint8_t ps2_receive(void);
void ps2_dispatch(fifo_t* fifo, void (*handler)(void*, uint8_t), void* device);

#endif