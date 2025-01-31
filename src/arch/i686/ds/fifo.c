/**
 * @file fifo.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief Circular FIFO buffer implementation
 * @date 2024-11-14
 * @see https://en.wikipedia.org/wiki/FIFO_(computing_and_electronics)
 */

#include "fifo.h"

/* PUBLIC API */
void fifo_init(fifo_t* self);
bool fifo_enqueue(fifo_t* self, const uint8_t data);
bool fifo_dequeue(fifo_t* self, uint8_t* data);

/* INTERNAL API */
static inline bool _is_empty(const fifo_t* self);
static inline bool _is_full(const fifo_t* self);

fifo_t fifo_kbd = {
    .buffer = {0},
    .head = 0,
    .tail = 0,
    .count = 0,
};

fifo_t fifo_mouse = {
    .buffer = {0},
    .head = 0,
    .tail = 0,
    .count = 0,
};

void fifo_init(fifo_t* self)
{
	if (self == NULL) {
		return;
	};
	self->head = 0;
	self->tail = 0;
	self->count = 0;
	return;
};

bool fifo_enqueue(fifo_t* self, const uint8_t data)
{
	if (!self || _is_full(self)) {
		return false;
	};
	self->buffer[self->head] = data;
	self->head = (self->head + 1) & FIFO_BUFFER_MASK;
	self->count++;
	return true;
};

bool fifo_dequeue(fifo_t* self, uint8_t* data)
{
	if (!self || _is_empty(self)) {
		return false;
	};
	*data = self->buffer[self->tail];
	self->tail = (self->tail + 1) & FIFO_BUFFER_MASK;
	self->count--;
	return true;
};

static inline bool _is_empty(const fifo_t* self) { return self->count == 0; };

static inline bool _is_full(const fifo_t* self) { return self->count == FIFO_BUFFER_SIZE; }