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

// Mouse Command Definitions
#define MOUSE_ENABLE_AUX 0xA8	      // Enable auxiliary device (mouse)
#define MOUSE_DISABLE 0xA7	      // Disable mouse
#define MOUSE_GET_COMPAQ_STATUS 0x20  // Get Compaq status byte
#define MOUSE_SET_COMPAQ_STATUS 0x60  // Set Compaq status byte
#define MOUSE_SEND_COMMAND 0xD4	      // Send a command to the mouse
#define MOUSE_ENABLE_DATA_REPORT 0xF4 // Enable mouse data reporting
#define MOUSE_SET_DEFAULT 0xF6	      // Set mouse to default settings
#define MOUSE_DETECT_ACK 0xFA	      // Acknowledge received command
// Mouse flag masks
#define LEFT_BUTTON_MASK 0b00000001	// Left button pressed
#define RIGHT_BUTTON_MASK 0b00000010	// Right button pressed
#define MIDDLE_BUTTON_MASK 0b00000100	// Middle button pressed
#define ALIGNED_PACKET_MASK 0b00001000	// Packet alignment
#define Y_AXIS_OVERFLOW_MASK 0b01000000 // Y-axis overflow
#define X_AXIS_OVERFLOW_MASK 0b10000000 // X-axis overflow
#define SIGN_BIT_MASK 0b100000000	// Sign bit for movement

void mouse_handler(void* dev, const uint8_t data);
void mouse_init(mouse_t* self);

#endif