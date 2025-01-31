/**
 * @file heap.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef HEAP_H
#define HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "icarius.h"
#include "pfa.h"

typedef struct heap heap_t;

typedef struct heap_block {
	size_t size;
	bool is_free;
	size_t chunk_span;
	struct heap_block* prev;
	struct heap_block* next;
} heap_block_t;

void heap_init(heap_t* self);
void* kmalloc(size_t size);
void* kzalloc(size_t size);
void kfree(void* ptr);
void heap_dump(const heap_t* self);
void heap_trace(const heap_t* self);

#endif