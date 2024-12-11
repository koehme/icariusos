/**
 * @file mouse.c
 * @brief PS/2 Mouse implementation
 * @author Kevin Oehme
 * @date 28.04.2024
 * @version 1.0
 * @copyright MIT
 */

#include "mouse.h"
#include "idt.h"
#include "ps2.h"

/* EXTERNAL API */
extern void asm_interrupt_32h(void);

/* PUBLIC API */
void mouse_handler(void* dev, const uint8_t data);
void mouse_init(mouse_t* self);

/* INTERNAL API */
static inline bool _match_mask(const mouse_t* self, const uint8_t mask);
static void _update_coordinates(mouse_t* self);

mouse_t mouse = {
    .x = 0,
    .y = 0,
    .prev_x = 0,
    .prev_y = 0,
    .cycle = 0,
    .flags = 0,
    .x_movement = 0,
    .y_movement = 0,
    .bytes = {0},
};

static inline bool _match_mask(const mouse_t* self, const uint8_t mask) { return (self->flags & mask) != 0; };

static void _update_coordinates(mouse_t* self)
{
	const int16_t delta_x = self->x_movement - ((self->flags << 3) & SIGN_BIT_MASK);
	const int16_t delta_y = self->y_movement - ((self->flags << 4) & SIGN_BIT_MASK);
	self->x += delta_x;
	self->y += delta_y;
	return;
};

void mouse_handler(void* dev, const uint8_t data)
{
	mouse_t* self = (mouse_t*)dev;

	switch (self->cycle) {
	case 0: {
		self->flags = data;

		if (!_match_mask(self, ALIGNED_PACKET_MASK)) {
			self->cycle = 0;
			break;
		};

		if (_match_mask(self, LEFT_BUTTON_MASK)) {
			printf("Left Btn\n");
		} else if (_match_mask(self, RIGHT_BUTTON_MASK)) {
			printf("Right Btn\n");
		} else if (_match_mask(self, MIDDLE_BUTTON_MASK)) {
			printf("Mid Btn\n");
		};
		self->cycle++;
		break;
	};
	case 1: {
		self->x_movement = data;
		self->cycle++;
		break;
	};
	case 2: {
		self->y_movement = data;

		if (!_match_mask(self, Y_AXIS_OVERFLOW_MASK) || !_match_mask(self, X_AXIS_OVERFLOW_MASK)) {
			_update_coordinates(self);
			printf("(%d,%d)\n", self->x, self->y);
		};
		self->cycle = 0;
		break;
	};
	};
	return;
};

void mouse_init(mouse_t* self)
{
	// Enable the auxiliary mouse device
	ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_ENABLE_AUX);
	// Enable the interrupts
	ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_GET_COMPAQ_STATUS);
	const uint8_t status = ps2_receive() | 0b00000010;
	ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SET_COMPAQ_STATUS);
	ps2_send(PS2_DATA_PORT, status);
	// Tell the mouse to use default settings
	ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SEND_COMMAND);
	ps2_send(PS2_DATA_PORT, MOUSE_SET_DEFAULT);
	ps2_receive();
	// Enable data reporting
	ps2_send(PS2_STATUS_COMMAND_PORT, MOUSE_SEND_COMMAND);
	ps2_send(PS2_DATA_PORT, MOUSE_ENABLE_DATA_REPORT);
	ps2_receive();
	idt_set(0x2C, asm_interrupt_32h);
	return;
};