/**
 * @file mouse.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef MOUSE_H
#define MOUSE_H

#define MOUSE_DEBUG 0

#include <stdbool.h>
#include <stdint.h>

#include "kernel.h"

typedef struct Mouse {
	int16_t x;
	int16_t y;
	int8_t cycle;
	union {
		struct {
			uint8_t flags;
			uint8_t x_movement;
			uint8_t y_movement;
		};
		uint8_t bytes[3];
	};
} Mouse;

typedef enum MouseCommand {
	MOUSE_ENABLE_AUX = 0xA8,
	MOUSE_DISABLE = 0xA7,
	MOUSE_GET_COMPAQ_STATUS = 0x20,
	MOUSE_SET_COMPAQ_STATUS = 0x60,
	MOUSE_SEND_COMMAND = 0xD4,
	MOUSE_ENABLE_DATA_REPORT = 0xF4,
	MOUSE_SET_DEFAULT = 0xF6,
	MOUSE_DETECT_ACK = 0xFA,
} MouseCommand;

void mouse_handler(void* dev, const uint8_t data);
void mouse_init(Mouse* self);

#endif