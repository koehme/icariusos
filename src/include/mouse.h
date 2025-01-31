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

#include "icarius.h"
#include "kernel.h"

typedef struct mouse {
	int16_t x;	// Current x-coordinate
	int16_t y;	// Current y-coordinate
	int16_t prev_x; // Previous x-coordinate
	int16_t prev_y; // Previous y-coordinate
	int8_t cycle;	// Current cycle in the mouse packet sequence
	union {
		struct {
			uint8_t flags;	    // Mouse flags (buttons, overflow, etc.)
			uint8_t x_movement; // Delta x from the mouse packet
			uint8_t y_movement; // Delta y from the mouse packet
		};
		uint8_t bytes[3];
	};
} mouse_t;

void mouse_handler(void* dev, const uint8_t data);
void mouse_init(mouse_t* self);

#endif