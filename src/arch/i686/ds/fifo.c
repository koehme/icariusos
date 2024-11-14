/**
 * @file fifo.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief Circular FIFO buffer implementation.
 * @date 2024-11-14
 * @see https://en.wikipedia.org/wiki/FIFO_(computing_and_electronics)
 *
 * This file implements a Circular FIFO (First In, First Out) buffer, providing
 * efficient and reliable data storage for scenarios where data needs to be processed
 * in the order it was received.
 *
 * The FIFO buffer is particularly useful in systems where data arrives at varying rates,
 * such as keyboard input, mouse events, serial communication, or network packet processing.
 * By using a circular buffer structure, the FIFO ensures continuous use of memory,
 * minimizing overhead while maintaining performance.
 *
 * This implementation uses a fixed-size array, leveraging bitwise operations to
 * optimize index management. The buffer size must be a power of 2 to enable the
 * efficient masking technique used in this implementation.
 *
 * Implementation Notes:
 * - This implementation assumes a fixed-size buffer. If the buffer is full, new
 *   data will not be enqueued, preventing overwriting.
 * - Indexing within the buffer is managed using bitwise AND with a mask, optimizing
 *   performance for cyclic operations.
 * - All operations (enqueue, dequeue) run in constant time O(1), making the buffer
 *   suitable for high-throughput systems.
 *
 * Features:
 * - Efficient memory usage with a circular design.
 * - Fast bitwise index management for power-of-2 buffer sizes.
 * - Support for checking full and empty states.
 *
 * Usage Scenarios:
 * - Input device buffering (keyboard, mouse).
 * - Serial communication (UART).
 * - Networking (packet buffering).
 * - Logging or data streaming.
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