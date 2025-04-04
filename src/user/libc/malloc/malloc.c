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
#define MIN_BLOCK_SIZE 8

typedef struct heap_block {
	size_t size;		      // Size of user data
	int free;		      // 1 = free, 0 = used
	struct heap_block* next;      // next block in heap
	struct heap_block* next_free; // next free block in heap free list
} heap_block_t;

void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void heap_dump(void);

static heap_block_t* _find_reusable_block(const size_t size);

static heap_block_t* heap_head = NULL;
static heap_block_t* free_list = NULL;
static uint8_t* _heap_curr = (uint8_t*)USER_HEAP_START;
static uint8_t* _heap_end = (uint8_t*)USER_HEAP_END;

void heap_dump(void)
{
	int total_blocks = 0;
	int used_blocks = 0;
	int free_blocks = 0;
	size_t total_bytes = 0;

	printf("\n[HEAP BLOCKS]\n");
	heap_block_t* curr = heap_head;

	while (curr) {
		printf("  [0x%x] Size=%d - %s\n", (uint32_t)curr, (int)curr->size, curr->free ? "F" : "U");

		total_blocks++;
		total_bytes += curr->size;

		curr->free ? free_blocks++ : used_blocks++;
		curr = curr->next;
	};
	printf("\n[FREE LIST]\n");
	curr = free_list;

	while (curr) {
		printf("  [0x%x] Size=%d\n", (uint32_t)curr, (int)curr->size);
		curr = curr->next_free;
	};
	printf("\n[SUMMARY] Blocks=%d | Used=%d | Free=%d | Heap Total=%d Bytes\n", total_blocks, used_blocks, free_blocks, total_bytes);
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
	const size_t aligned_size = ALIGN8(size);

	if (requested_block->size >= size) {
		// Block size is fine, return ptr
		return ptr;
	};
	// Block is too small? Make a new one
	void* new_block = malloc(aligned_size);

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

static void _unlink_from_freelist(heap_block_t* prev, heap_block_t* block)
{
	if (prev) {
		prev->next_free = block->next_free;
	} else {
		free_list = block->next_free;
	};
	block->free = 0;
	block->next_free = NULL;
	return;
};

static heap_block_t* _find_reusable_block(const size_t size)
{
	if (!size) {
		return NULL;
	};
	heap_block_t* prev = NULL;
	heap_block_t* curr = free_list;

	while (curr) {
		if (curr->free && curr->size >= size) {
			const size_t remaining = curr->size - size;
			const bool can_be_split = remaining >= sizeof(heap_block_t) + MIN_BLOCK_SIZE;

			if (can_be_split) {
				// Take the curr ptr and calculate until we reach the end and split the rest and put it into the freelist
				heap_block_t* split = (heap_block_t*)((uint8_t*)(curr + 1) + size);

				split->size = remaining - sizeof(heap_block_t);
				split->free = 1;
				split->next = curr->next;
				split->next_free = curr->next_free;

				curr->size = size;
				curr->free = 0;
				curr->next = split;
				curr->next_free = NULL;

				// Insert the splittable block into the free list
				if (prev) {
					prev->next_free = split;
				} else {
					free_list = split;
				};
				return curr;
			} else {
				_unlink_from_freelist(prev, curr);
				return curr;
			};
		};
		prev = curr;
		curr = curr->next_free;
	};
	return NULL;
};

static void _append_block(heap_block_t* block)
{
	if (!block) {
		return;
	};

	if (!heap_head) {
		heap_head = block;
		return;
	};
	heap_block_t* iter = heap_head;

	while (iter && iter->next) {
		iter = iter->next;
	};
	iter->next = block;
	return;
};

void* malloc(size_t size)
{
	if (size == 0) {
		errno = EINVAL;
		return NULL;
	};
	const size_t aligned_size = ALIGN8(size);
	// Try to reuse a previously freed block
	heap_block_t* reused_block = _find_reusable_block(aligned_size);

	if (reused_block) {
		// return pointer after metadata
		return (void*)(reused_block + 1);
	};
	// No suitable block found → create new one
	const uint32_t total_size = sizeof(heap_block_t) + aligned_size;

	if ((_heap_curr + total_size) > _heap_end) {
		// TODO: Ask kernel to map new user heap pages (e.g. via syscall)
		errno = ENOMEM;
		return NULL;
	};
	// Init a new block at the current heap end
	heap_block_t* new_block = (heap_block_t*)_heap_curr;
	// Initialize block metadata
	new_block->size = aligned_size;
	new_block->free = 0;
	new_block->next = new_block->next_free = NULL;
	// Link the new block into the heap block chain
	_append_block(new_block);
	// Advance the heap ptr cursor past this block (header + data)
	_heap_curr += total_size;
	// Return pointer to usable memory (just after the header)
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

static void _insert_reusable(heap_block_t* reusable_block)
{
	if (!reusable_block) {
		return;
	};
	reusable_block->free = 1;
	reusable_block->next_free = free_list;
	free_list = reusable_block;
	return;
};

void free(void* ptr)
{
	if (!ptr) {
		return;
	};
	heap_block_t* iter_block = heap_head;
	// Traverse heap chain to find the block
	while (iter_block) {
		if ((void*)(iter_block + 1) == ptr) {
			_insert_reusable(iter_block);
			return;
		};
		iter_block = iter_block->next;
	};
	return;
};