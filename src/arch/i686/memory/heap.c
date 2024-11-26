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
	// Try to recycle a previously freed block that is large enough
	void* recycled = _recycle_block(self, size);

	if (recycled) {
		// If a suitable recycled block was found, return it
		return recycled;
	};
	// Request can't be perform, because no free virtual addresses are free
	if (self->next_addr + size >= self->end_addr) {
		return 0x0;
	};
	// Calculate the number of required pages (frames) based on `size`
	const size_t pages = size / PAGE_SIZE;
	uintptr_t virt_addr = self->next_addr;
	// Allocate and map the required number of pages
	for (size_t page = 0; page < pages; ++page) {
		const uint64_t phys_addr = pfa_alloc();

		if (!phys_addr) {
			// Physical memory allocation failed
			return 0x0;
		};
		// Map these frames to virtual addresses starting from `self->next_addr`
		page_map(virt_addr, phys_addr, PAGE_PRESENT | PAGE_WRITABLE);
		virt_addr += PAGE_SIZE;
	};
	// Create the new block at the current `next_addr`
	heap_block_t* new_block = (heap_block_t*)self->next_addr;
	new_block->size = size - sizeof(heap_block_t);
	new_block->is_free = false;
	new_block->next = 0x0;

	// Link the new block into the allocation list
	if (self->tail) {
		// Append to the end of the list
		self->tail->next = new_block;
		new_block->prev = self->tail;
	} else {
		// List is empty
		self->head = new_block;
		new_block->prev = 0x0;
	};
	self->tail = new_block;
	// Update `next_addr` to point to the next available address
	self->next_addr += size;
	// Return a pointer to the usable memory area, after the header
	return (void*)((uint8_t*)new_block + sizeof(heap_block_t));
};

void heap_init(heap_t* self)
{
	self->start_addr = self->next_addr = KERNEL_HEAP_START;
	self->end_addr = KERNEL_HEAP_MAX;
	self->head = 0x0;
	self->tail = 0x0;
	return;
};

void heap_dump(const heap_t* self)
{
	heap_block_t* curr = self->head;
	size_t block_count = 0;

	printf("\n\n====================================\n");
	printf("             HEAP DUMP              \n");
	printf("====================================\n");
	printf("Heap Start Address:       0x%x\n", self->start_addr);
	printf("Heap End Address:         0x%x\n", self->end_addr);
	printf("Heap Next Free Address:   0x%x\n", self->next_addr);

	if (!curr) {
		printf("[ ].\n");
		printf("====================================\n");
		return;
	};

	while (curr) {
		printf("\n------------------------------------\n");
		printf("Block #%d\n", block_count++);
		printf("Block Address:            0x%x\n", (uintptr_t)curr);
		printf("Block Size:               %d Bytes\n", curr->size);
		printf("Block Status:             %s\n", curr->is_free ? "Free" : "Used");
		printf("Next Block Address:       0x%x\n", (uintptr_t)curr->next);
		printf("Previous Block Address:   0x%x\n", (uintptr_t)curr->prev);
		printf("------------------------------------\n");
		curr = curr->next;
	};
	printf("====================================\n");
	return;
};