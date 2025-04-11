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
#define IS_ALIGNED8(ptr) (((uintptr_t)(ptr) & 0x7) == 0)
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
	size_t total_bytes = 0;
	printf("\n+--[ HEAP LIST ]-----------------------------------------------\n");

	while (block) {
		const char* status = block->free ? "FREE" : "USED";
		printf("0x%08x | %4d | %4s | 0x%08x | 0x%08x\n", (uint32_t)(uintptr_t)block, (int)(block->size + sizeof(heap_block_t)), status,
		       (uint32_t)(uintptr_t)block->prev, (uint32_t)(uintptr_t)block->next);
		total_bytes += block->size + sizeof(heap_block_t);
		block = block->next;
		i++;
	};
	printf("\n+--[ FREE LIST ]-----------------------------------------------\n");
	heap_block_t* free = free_list;
	size_t j = 0;

	while (free) {
		const char* status = block->free ? "FREE" : "USED";
		printf("0x%08x | %4d | %4s | 0x%08x | 0x%08x\n", (uint32_t)(uintptr_t)free, (int)(free->size + sizeof(heap_block_t)), status,
		       (uint32_t)(uintptr_t)free->prev_free, (uint32_t)(uintptr_t)free->next_free);
		free = free->next_free;
		j++;
	};
	printf("\n+--[ HEAP SUMMARY ]--------------------------------------------\n");
	const size_t heap_used = (size_t)(_heap_curr - (uint8_t*)USER_HEAP_START);
	const size_t heap_total = (size_t)(_heap_end - (uint8_t*)USER_HEAP_START);
	const float percent = ((float)heap_used / heap_total) * 100.0f;
	printf(" Total Blocks       : %d\n", i);
	printf(" Total Bytes        : %d\n", total_bytes);
	printf(" Heap Capacity Used : %d / %d Bytes (%f %%)\n", heap_used, heap_total, percent);
	printf("+--------------------------------------------------------------\n");
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

static heap_block_t* _find_best_fit_free_block(const size_t aligned_size)
{
	if (aligned_size == 0) {
		return NULL;
	};
	// Let us check if we can find a reusable block
	heap_block_t* iter_block = free_list;
	heap_block_t* best_fit_block = 0x0;

	while (iter_block) {
		if ((uintptr_t)iter_block < USER_HEAP_START || (uintptr_t)iter_block >= (uintptr_t)_heap_end) {
			break;
		};
		// Searching for a best fit suitable free block
		// Loop through blocks and update best_fit if the iter block is smaller than the previous best_fit_block
		if (iter_block->size >= aligned_size && (!best_fit_block || (iter_block->size <= best_fit_block->size))) {
			best_fit_block = iter_block;
		};
		// Anti cyclic protection
		if (iter_block == iter_block->next_free) {
			break;
		};
		iter_block = iter_block->next_free;
	};
	return best_fit_block;
};

static void _split_block(heap_block_t* block, const size_t aligned_size)
{
	// Split the block after aligned_size, example new_block = (block + 1) + aligned_size is the starting point of the new
	// block?
	heap_block_t* new_block = (heap_block_t*)((uint8_t*)(block + 1) + aligned_size);

	if ((uint8_t*)new_block + sizeof(heap_block_t) > _heap_end) {
		return;
	};
	// Init new splitted block
	new_block->free = 1;
	new_block->size = block->size - aligned_size - sizeof(heap_block_t);
	// Update Free List Chaining at the head
	new_block->next_free = free_list;
	new_block->prev_free = NULL;
	// Is the freelist NOT empty? append the old head to the new head :D
	if (free_list) {
		free_list->prev_free = new_block;
	};
	free_list = new_block;
	// Update heap list
	new_block->next = block->next;
	new_block->prev = block;
	// If the allocated block has a next block we must insert the "splitted" block into the chain block --> new spliited block -->
	// original next block
	if (block->next) {
		block->next->prev = new_block;
	};
	// If the best_fit block has no next block, we can safley add the splitted block :D
	block->next = new_block;
	// Shrink old block
	block->size = aligned_size;

	if (!new_block->next) {
		heap_tail = new_block;
	};
	return;
};

static void _remove_from_free_list(heap_block_t* block)
{
	if (!block) {
		return;
	};
	heap_block_t* prev_block = block->prev_free;
	heap_block_t* next_block = block->next_free;

	if (!prev_block) {
		// No prev block? the next block is the new head
		free_list = next_block;
	} else {
		prev_block->next_free = next_block;
	};

	if (next_block) {
		next_block->prev_free = prev_block;
	};
	block->next_free = NULL;
	block->prev_free = NULL;
	return;
};

void* _make_new_block(const size_t aligned_size)
{
	const size_t new_block_size = aligned_size + sizeof(heap_block_t);

	if (_heap_curr + new_block_size >= _heap_end) {
		errno = ENOMEM;
		// TODO: Implement syscall to request more memory
		return 0x0;
	};
	heap_block_t* new_block = (heap_block_t*)_heap_curr;
	new_block->free = 0;
	new_block->size = aligned_size;

	new_block->next = NULL;
	new_block->prev = heap_tail;

	if (!heap_head) {
		heap_head = heap_tail = new_block;
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

void* malloc(size_t size)
{
	if (size == 0) {
		errno = EINVAL;
		return NULL;
	};
	const size_t aligned_size = ALIGN8(size);
	heap_block_t* best_fit_block = _find_best_fit_free_block(aligned_size);

	if (best_fit_block) {
		// Block splitting if the block is larger than the minimum block size and append the remaining part to the free list
		if (best_fit_block->size >= aligned_size + sizeof(heap_block_t) + MIN_BLOCK_SIZE) {
			_split_block(best_fit_block, aligned_size);
		};
		_remove_from_free_list(best_fit_block);
		best_fit_block->free = 0;
		return (void*)(best_fit_block + 1);
	};
	return _make_new_block(aligned_size);
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
	heap_block_t* block = ((heap_block_t*)ptr) - 1;

	if (block->size == 0 || (uintptr_t)block < USER_HEAP_START || (uintptr_t)block >= (uintptr_t)_heap_end) {
		return;
	};
	if (block->free) {
		return;
	};
	block->free = 1;
	// ===== Right merge =====
	heap_block_t* next_block = block->next;

	if (next_block && next_block->free) {
		// Unlink next_block from free_list
		if (next_block->prev_free) {
			// Bridge over next_block (backward link)
			next_block->prev_free->next_free = next_block->next_free;
		};
		if (next_block->next_free) {
			// Bridge over next_block (forward link)
			next_block->next_free->prev_free = next_block->prev_free;
		};
		if (free_list == next_block) {
			// Update head if necessary
			free_list = next_block->next_free;
		};
		// Merge next into block
		block->size += next_block->size + sizeof(heap_block_t);
		block->next = next_block->next;

		if (next_block->next) {
			next_block->next->prev = block;
		};
	};
	// ===== Left merge =====
	heap_block_t* prev_block = block->prev;

	if (prev_block && prev_block->free) {
		// Unlink prev_block from the free list
		if (prev_block->prev_free) {
			// Bridge over prev_block (backward link)
			prev_block->prev_free->next_free = prev_block->next_free;
		};
		if (prev_block->next_free) {
			// Bridge over prev_block (forward link)
			prev_block->next_free->prev_free = prev_block->prev_free;
		};
		if (free_list == prev_block) {
			// Update free list head if needed
			free_list = prev_block->next_free;
		};
		// Merge block into prev
		prev_block->size += block->size + sizeof(heap_block_t);
		prev_block->next = block->next;

		if (block->next) {
			block->next->prev = prev_block;
		};
		block = prev_block; // New final merged block, which will be inserted
	};
	// Insert at the head
	block->next_free = free_list;
	block->prev_free = NULL;

	if (free_list) {
		free_list->prev_free = block;
	};
	free_list = block;
	return;
};