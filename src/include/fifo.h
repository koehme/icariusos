/**
 * @file fifo.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef FIFO_H
#define FIFO_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "icarius.h"

typedef struct fifo {
	uint8_t buffer[FIFO_BUFFER_SIZE]; // 'ptr' to the elements in memory
	uint8_t head;			  // Where to write (enqueue)
	uint8_t tail;			  // Where to read (dequeue)
	uint8_t count;			  // Number of elements in the buffer
} fifo_t;

void fifo_init(fifo_t* self);
void fifo_dump(fifo_t* self);
bool fifo_enqueue(fifo_t* self, const uint8_t data);
bool fifo_dequeue(fifo_t* self, uint8_t* data);
bool fifo_is_empty(const fifo_t* self);

#endif