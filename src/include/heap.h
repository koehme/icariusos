/**
 * @file heap.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

typedef enum HeapBlockStatus {
	HEAP_BLOCK_IS_FREE = 0b00000000,
	HEAP_BLOCK_IS_USED = 0b00000001,
	HEAP_BLOCK_IS_HEAD = 0b01000000,
	HEAP_BLOCK_HAS_NEXT = 0b10000000,
} HeapBlockStatus;

typedef enum HeapBlockFlags {
	HEAP_BLOCK_FLAG_USED = (1u << 0),
	HEAP_BLOCK_FLAG_HEAD = (1u << 6),
	HEAP_BLOCK_FLAG_NEXT = (1u << 7),
} HeapBlockFlags;

typedef struct HeapBitMap {
	uint8_t* saddr;	     // Pointer to the location for the marked map allocation data structure
	size_t total_blocks; // Assume block_size is a power of 2^12 (e.g., 4096)
} HeapBitMap;

typedef struct Heap {
	HeapBitMap* bitmap; // Pointer to the heap byte map data structure
	void* saddr;	    // Start address of the raw heap data pool
	size_t block_size;  // Size of each memory block in the heap
} Heap;

void heap_init(Heap* self, void* kheap_saddr, void* bitmap_saddr, const size_t n_bytes, const size_t block_size);
void* kmalloc(const size_t size);
void* kcalloc(const size_t size);
void kfree(void* ptr);
double kheap_info(Heap* self);

#endif