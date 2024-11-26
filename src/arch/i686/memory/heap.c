/**
 * @file heap.c
 * @author Kevin Oehme
 * @copyright MIT
 * @date 2024-11-11
 */

#include "heap.h"
#include "kernel.h"
#include "string.h"

extern pfa_t pfa;

heap_t heap = {
    .start_addr = 0x0,
    .next_addr = 0x0,
    .end_addr = 0x0,
    .head = 0x0,
    .tail = 0x0,
};

/* PUBLIC API */
void heap_init(heap_t* self);
void* kmalloc(size_t size);
void* kzalloc(size_t size);
void kfree(void* ptr);

/* INTERNAL API */
static inline size_t _align_to_next_page_boundary(size_t size);
static void* _malloc(heap_t* self, size_t size);
static void _free(heap_t* self, void* ptr);

static inline size_t _align_to_next_page_boundary(size_t size) { return (size + sizeof(heap_block_t) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); };

void* kmalloc(size_t size)
{
	void* ptr = 0x0;
	ptr = _malloc(&heap, size);
	return ptr;
};

void* kzalloc(size_t size)
{
	void* ptr = 0x0;
	ptr = _malloc(&heap, size);
	memset(ptr, 0x0, size);
	return ptr;
};

void kfree(void* ptr)
{
	_free(&heap, ptr);
	return;
};

static void _free(heap_t* self, void* ptr) { return; };

static void* _recycle_block(heap_t* self, size_t size)
{
	heap_block_t* curr = self->head;

	while (curr) {
		// Is the current block available and big enough?
		if (curr->is_free && curr->size >= size) {
			// TODO: Splitting if oversized, create a new block which is free for future allocations


			// If so, return the ptr to the requestor, but after the header, to save the header
			return (void*)(uint8_t*)curr + sizeof(heap_block_t);
		};
		curr = curr->next;
	};
	// No suitable block found ;-)
	return 0x0;
};

static void* _malloc(heap_t* self, size_t size)
{
	// Align the size to the next page boundary, including the header
	size = _align_to_next_page_boundary(size);
	// recycling previous allocations, to reuse blocks
	void* recycled = _recycle_block(self, size);

	if (!recycled) {
		// Create new blocks:
		// - Create a new block at the address `self->next_addr`.
		// - Calculate the number of required pages (frames) based on `size`.
		// - Call `pfa_alloc()` to obtain physical frames.
		// - Use `page_map` to map these frames to virtual addresses starting from `self->next_addr`.

		// Integrate the new block into the doubly linked list:
		// - If `self->tail` is not null:
		//   - Set `self->tail->next` to the new block.
		//   - Set `new_block->prev` to `self->tail`.
		//   - Update `self->tail` to point to the new block.
		// - If the list is empty (`self->head == NULL`):
		//   - Set both `self->head` and `self->tail` to the new block.

		// Update `self->next_addr`:
		// - Increment `self->next_addr` by the total allocated size (including the header).

		// Error handling:
		// - Check if `pfa_alloc()` fails (e.g., if physical memory is unavailable).
		// - Return `NULL` if allocation is not possible.
	};
	return recycled;
};

void heap_init(heap_t* self)
{
	self->start_addr = self->next_addr = KERNEL_HEAP_START;
	self->end_addr = KERNEL_HEAP_MAX;
	self->head = 0x0;
	self->tail = 0x0;
	return;
};
