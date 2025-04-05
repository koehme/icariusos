#include "errno.h"
#include "icarius.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Example: x = 14
// Step 1: 14 + 7 = 21       → binary:  0001 0101 (0x15)
// Step 2: ~7  = 0xFFFFFFF8  → mask to clear lower 3 bits (111)
// Step 3: 21 & ~7 = 16      → binary:  0001 0000 (0x10) = aligned
// ALIGN8(x) rounds up to the next multiple of 8
#define ALIGN8(x) (((x) + 7) & ~7)
#define MIN_BLOCK_SIZE 32

typedef struct heap_block {
	size_t size;
	int free;
	struct heap_block* next;
	struct heap_block* prev;
	struct heap_block* next_free;
	struct heap_block* prev_free;
} heap_block_t;

static heap_block_t* heap_head = NULL;
static heap_block_t* heap_tail = NULL;
static heap_block_t* free_list = NULL;

static uint8_t* _heap_curr = (uint8_t*)USER_HEAP_START;
static uint8_t* _heap_end = (uint8_t*)USER_HEAP_END;

void heap_dump(void)
{
	heap_block_t* block = heap_head;
	size_t i = 0;
	size_t total_user_bytes = 0;
	size_t total_bytes = 0;

	while (block) {
		void* block_addr = (void*)block;
		void* prev_addr = (void*)block->prev;
		void* next_addr = (void*)block->next;
		const char* status = block->free ? "FREE" : "USED";

		printf("[0x%x] ──┬─────────────────────────────────────────────\n", (uint32_t)(uintptr_t)block_addr);
		printf("           │ Block      : %d\n", i++);
		printf("           │ Size       : %d\n", (uint32_t)(block->size + sizeof(heap_block_t)));
		printf("           │ User Size  : %d\n", (uint32_t)block->size);
		printf("           │ Status     : %s\n", status);
		printf("           │ Prev       : 0x%x\n", (uint32_t)(uintptr_t)prev_addr);
		printf("           │ Next       : 0x%x\n", (uint32_t)(uintptr_t)next_addr);
		printf("           └─────────────────────────────────────────────\n");

		total_user_bytes += block->free ? 0 : block->size;
		total_bytes += block->size + sizeof(heap_block_t);

		block = block->next;
	};
	const size_t heap_used = (size_t)(_heap_curr - (uint8_t*)USER_HEAP_START);
	const size_t heap_total = (size_t)(_heap_end - (uint8_t*)USER_HEAP_START);
	float percent = ((float)heap_used / heap_total) * 100.0f;

	printf("----[ Heap ]----\n");
	printf(" Total Blocks       : %d\n", i);
	printf(" Total User Bytes   : %d\n", total_user_bytes);
	printf(" Total Bytes        : %d\n", total_bytes);
	printf(" Heap Capacity Used : %d / %d Bytes (%f)\n", heap_used, heap_total, percent);
	return;
};

void* realloc(void* ptr, size_t size)
{
	if (!ptr) {
		// realloc(NULL, size) == malloc(size)
		return malloc(size);
	};

	if (size == 0) {
		// realloc(ptr, 0) == free(ptr), return NULL
		free(ptr);
		return NULL;
	};
	heap_block_t* requested_block = (heap_block_t*)ptr - 1;

	if (requested_block->size >= size) {
		// Block size is fine, return ptr
		return ptr;
	};
	// Block is too small? Make a new one
	void* new_block = malloc(size);

	if (!new_block) {
		return NULL;
	};
	// copy payload
	const size_t bytes = requested_block->size;
	memcpy(new_block, ptr, bytes);
	// Free old block
	free(ptr);
	return new_block;
};

void* malloc(size_t size)
{
	if (size == 0) {
		errno = EINVAL;
		return NULL;
	};
	const size_t aligned_size = ALIGN8(size);
	// Let us check if we can find a reusable block
	heap_block_t* iter_block = free_list;
	heap_block_t* best_fit_block = 0x0;

	while (iter_block) {
		// Searching for a best fit suitable free block
		// Loop through blocks and update best_fit if the iter block is smaller than the previous best_fit_block
		if (iter_block->size >= aligned_size && (!best_fit_block || (iter_block->size <= best_fit_block->size))) {
			best_fit_block = iter_block;
		};
		iter_block = iter_block->next_free;
	};
	if (best_fit_block) {
		// TODO: Implement block splitting if the block is larger than the minimum block size and append the remaining part to the free list
		if (best_fit_block->size >= aligned_size + sizeof(heap_block_t) + MIN_BLOCK_SIZE) {
		};
		// TODO: Remove best_fit_block from free_list
		best_fit_block->free = 0;
		return (void*)(best_fit_block + 1);
	};
	const size_t new_block_size = aligned_size + sizeof(heap_block_t);

	if (_heap_curr + new_block_size >= _heap_end) {
		errno = ENOMEM;
		return 0x0;
	};
	heap_block_t* new_block = (heap_block_t*)_heap_curr;
	new_block->free = 0;
	new_block->size = aligned_size;

	if (!heap_head) {
		heap_head = heap_tail = new_block;
		new_block->next = NULL;
		new_block->prev = NULL;
	} else {
		heap_tail->next = new_block;
		new_block->prev = heap_tail;
		heap_tail = new_block;
	};
	new_block->prev_free = NULL;
	new_block->next_free = NULL;

	_heap_curr += new_block_size;
	return (void*)(new_block + 1);
};

void* calloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0) {
		return malloc(0);
	};

	if (nmemb > (SIZE_MAX / size)) {
		errno = ENOMEM;
		return NULL;
	};
	const size_t total_bytes = nmemb * size;
	void* ptr = malloc(total_bytes);

	if (!ptr) {
		errno = ENOMEM;
		return NULL;
	};
	memset(ptr, 0, total_bytes);
	return ptr;
};


void free(void* ptr)
{
	if (!ptr) {
		return;
	};
	return;
};