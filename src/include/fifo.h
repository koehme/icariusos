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

#define FIFO_BUFFER_SIZE 128 // Must be a power of 2
#define FIFO_BUFFER_MASK (FIFO_BUFFER_SIZE - 1)

typedef struct fifo {
	uint8_t buffer[FIFO_BUFFER_SIZE]; // 'ptr' to the elements in memory
	uint8_t head;			  // Where to write (enqueue)
	uint8_t tail;			  // Where to read (dequeue)
	uint8_t count;			  // Number of elements in the buffer
} fifo_t;

void fifo_init(fifo_t* self);
bool fifo_enqueue(fifo_t* self, const uint8_t data);
bool fifo_dequeue(fifo_t* self, uint8_t* data);

#endif