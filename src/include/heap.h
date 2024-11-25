/**
 * @file heap.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

#include "pfa.h"

typedef struct heap_t {
	uint64_t start_addr;
	uint64_t next_addr;
	uint64_t end_addr;
} heap_t;

void heap_init(heap_t* self);
void* kmalloc(const size_t size);
void* kzalloc(const size_t size);
void kfree(void* ptr);

#endif