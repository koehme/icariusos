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
		// Check if the current block is free and large enough to satisfy the request
		if (curr->is_free && curr->size >= size) {
			// Mark the block as used
			curr->is_free = false;
			// If the block is larger than needed, split it into two:
			// The first part satisfies the current request
			// The second part remains free for future allocations
			if (curr->size >= size + sizeof(heap_block_t)) {
				heap_block_t* unused = (heap_block_t*)((uint8_t*)curr + sizeof(heap_block_t) + size);
				// Initialize the new unused block
				unused->is_free = true;
				unused->size = curr->size - size - sizeof(heap_block_t);
				// Insert the new unused block into the doubly linked list
				unused->next = curr->next;
				unused->prev = curr;

				if (curr->next) {
					// Update the previous pointer of the next block (curr <--> unused <--> curr->next)
					curr->next->prev = unused;
				} else {
					// If there is no next block, this unused block becomes the new tail
					self->tail = unused;
				};
				// Link the new unused block to the current block
				curr->next = unused;
				curr->size = size; // Resize the current block to the requested size
			};
			// Return a pointer to the usable memory area, after the header
			return (void*)((uint8_t*)curr + sizeof(heap_block_t));
		};
		curr = curr->next; // Move to the next block
	};
	// If no suitable block is found, return NULL
	return 0x0;
};

static void* _malloc(heap_t* self, size_t size)
{
	// Align the size to the next page boundary, including the header
	size = _align_to_next_page_boundary(size);
	// recycling previous allocations, to reuse blocks
	void* recycled = _recycle_block(self, size);

	if (!recycled) {
		// Request can't be perform, because no free virtual addresses are free
		if (self->next_addr + size >= self->end_addr) {
			return 0x0;
		};
		// - Calculate the number of required pages (frames) based on `size`
		const size_t pages = size / PAGE_SIZE;
		// - Call `pfa_alloc()` to obtain physical frames
		uintptr_t virt_addr = self->next_addr;

		// Map the required number of pages
		for (size_t page = 0; page < pages; ++page) {
			const uint64_t phys_addr = pfa_alloc();

			if (!phys_addr) {
				// Physical memory allocation failed
				return 0x0;
			};
			// - Use `page_map` to map these frames to virtual addresses starting from `self->next_addr`
			page_map(virt_addr, phys_addr, PAGE_PRESENT | PAGE_WRITABLE);
			virt_addr += PAGE_SIZE;
		};
		// Create new blocks:
		// - Create a new block at the address `self->next_addr`
		// Integrate the new block into the doubly linked list:
		// - If `self->tail` is not null:
		//   - Set `self->tail->next` to the new block
		//   - Set `new_block->prev` to `self->tail`
		//   - Update `self->tail` to point to the new block
		// - If the list is empty (`self->head == NULL`):
		//   - Set both `self->head` and `self->tail` to the new block

		// Update `self->next_addr`:
		// - Increment `self->next_addr` by the total allocated size (including the header)

		// Error handling:
		// - Check if `pfa_alloc()` fails (e.g., if physical memory is unavailable)
		// - Return `NULL` if allocation is not possible
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
