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

typedef struct HeapByteMap {
	uint8_t* saddr;	     // Pointer to the location for the marked map allocation data structure
	size_t total_blocks; // 25600 total blocks if heap_init called with n_bytes 1024 * 1024 * 100 - assume block_size is a power of 2^12 (e.g., 4096)
} HeapByteMap;

typedef struct Heap {
	HeapByteMap* bytemap; // Pointer to the heap byte map data structure
	void* saddr;	      // Start address of the raw heap data pool
	size_t block_size;    // Size of each memory block in the heap
} Heap;

void heap_init(Heap* self, HeapByteMap* bytemap, void* heap_saddr, void* bytemap_saddr, const size_t n_bytes, const size_t block_size);
void* heap_malloc(Heap* self, const size_t n_bytes);
void heap_free(Heap* self, void* ptr);
double heap_get_utilization(Heap* self);

#endif