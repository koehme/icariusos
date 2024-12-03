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

#include "pfa.h"

#define CHUNK_SIZE 4096

#define KERNEL_HEAP_START 0xC1000000
#define KERNEL_HEAP_MAX 0xC2FFFFFF

typedef struct heap_block {
	size_t size;		 // Size of the memory block in bytes (usually 4096 bytes for chunks)
	bool is_free;		 // Indicates whether this block is free (true) or allocated (false)
	size_t chunk_span;	 // Number of contiguous chunks allocated for this block (0 for single chunk and n for > 0)
	struct heap_block* prev; // Pointer to the previous block
	struct heap_block* next; // Pointer to the next block
} heap_block_t;

typedef struct heap {
	uintptr_t start_addr;	 // Start address of the heap memory region
	uintptr_t next_addr;	 // Next address to be allocated by pfa_alloc (physical frame allocator)
	uintptr_t end_addr;	 // End address of the heap memory region
	heap_block_t* prev_head; // Pointer to the prev first block
	heap_block_t* head;	 // Pointer to the first block
	heap_block_t* tail;	 // Pointer to the last block
} heap_t;

void heap_init(heap_t* self);
void* kmalloc(size_t size);
void* kzalloc(size_t size);
void kfree(void* ptr);
void heap_dump(const heap_t* self);
void heap_trace(const heap_t* self);

#endif