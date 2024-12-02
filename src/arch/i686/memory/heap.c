/**
 * @file heap.c
 * @author Kevin Oehme
 * @copyright MIT
 * @date 2024-11-11
 */

#include "heap.h"
#include "kernel.h"
#include "pfa.h"
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
static void* _malloc(heap_t* self, size_t size);
static void _free(heap_t* self, void* ptr);

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
static void _free(heap_t* self, void* ptr)
{
	if (!ptr) {
		return;
	};
	heap_block_t* curr_block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
	curr_block->is_free = true;

	const size_t chunks_to_split = curr_block->chunk_span;

	heap_block_t* next_block = curr_block->next;
	heap_block_t* iter_block = curr_block;

	curr_block->chunk_span = 1;

	for (size_t i = 1; i < chunks_to_split; i++) {
		heap_block_t* new_block = (heap_block_t*)((uint8_t*)iter_block + 4096);

		new_block->chunk_span = 1;
		new_block->is_free = true;
		new_block->next = 0x0;
		new_block->prev = iter_block;

		iter_block->next = new_block;
		iter_block = new_block;
	};
	iter_block->next = next_block;

	if (next_block) {
		next_block->prev = iter_block;
	};
	return;
};

static void _init_heap_block(heap_block_t* self, size_t address, size_t size, heap_block_t* prev)
{
	self->chunk_span = 0; // Single chunk by default
	self->is_free = true; // Mark as free
	self->size = size;    // Set block size
	self->prev = prev;    // Previous block
	self->next = 0x0;     // Next will be set in the loop
	return;
};

static void _heap_grow(heap_t* self, uint64_t phys_addr)
{
	// Map the phys addr to virt_addr in self->next_addr
	page_map(self->next_addr, phys_addr, PAGE_PS | PAGE_WRITABLE | PAGE_PRESENT);
	const size_t page_size = 4 * 1024 * 1024; // 4 MiB
	const size_t chunk_size = 4096;		  // 4 KiB
	const size_t chunks = page_size / chunk_size;
	size_t virt_addr = self->next_addr;

	// Link the new blocks to the existing list, if it exists
	if (self->tail) {
		heap_block_t* first_new_block = (heap_block_t*)virt_addr;
		self->tail->next = first_new_block;
		first_new_block->prev = self->tail;
	};
	heap_block_t* prev_block = self->tail;

	for (size_t chunk = 0; chunk < chunks; ++chunk, virt_addr += chunk_size) {
		heap_block_t* block = (heap_block_t*)virt_addr;
		_init_heap_block(block, virt_addr, chunk_size, prev_block);

		if (prev_block) {
			prev_block->next = block;
		};

		if (chunk == 0 && !self->head) {
			self->head = block;
		};
		prev_block = block;
	};
	// Update self->tail and next_addr
	self->tail = prev_block;
	self->next_addr = virt_addr;
	return;
};

static void* _malloc(heap_t* self, size_t size)
{
	const size_t chunk_size = 4096;
	const size_t total_size_with_header = size + sizeof(heap_block_t);
	size_t chunks_needed = (total_size_with_header + chunk_size - 1) / chunk_size;
	heap_block_t* curr_block = (heap_block_t*)KERNEL_HEAP_START;

	while (curr_block) {
		if (curr_block->is_free) {
			heap_block_t* start_block = curr_block;
			size_t free_chunks = 0;

			while (curr_block && curr_block->is_free && free_chunks < chunks_needed) {
				free_chunks++;
				curr_block = curr_block->next;
			};
			if (free_chunks == chunks_needed) {
				start_block->is_free = false;
				start_block->chunk_span = chunks_needed;
				heap_block_t* block = start_block->next;

				for (size_t i = 1; i < chunks_needed; ++i) {
					block->is_free = false;
					block = block->next;
				};
				heap_block_t* next_free = block;

				if (next_free) {
					next_free->prev = start_block;
				}
				start_block->next = next_free;
				return (void*)((uint8_t*)start_block + sizeof(heap_block_t));
			};
			curr_block = start_block->next;
		} else {
			curr_block = curr_block->next;
		};
	};
	const uint64_t phys_addr = pfa_alloc();

	if (phys_addr == 0x0) {
		printf("[CRITICAL] Out of physical memory. Unable to allocate new page.\n");
		return NULL;
	};
	_heap_grow(self, phys_addr);
	return _malloc(self, size);
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
	heap_block_t* curr = (heap_block_t*)self->head;
	size_t block_count = 0;
	size_t total_used_memory = 0;
	size_t allocation_count = 0;
	const size_t total_heap_size = self->end_addr - self->start_addr;

	printf("\n====================================\n");
	printf("             HEAP DUMP              \n");
	printf("====================================\n");
	printf("Heap Start Address:       0x%x\n", self->start_addr);
	printf("Heap End Address:         0x%x\n", self->end_addr);
	printf("Heap Next Free Address:   0x%x\n", self->next_addr);

	while (curr) {
		if (!curr->is_free && curr->chunk_span > 0) {
			const size_t allocation_size = curr->chunk_span * 4096;
			total_used_memory += allocation_size;
			allocation_count++;
			printf("\n------------------------------------\n");
			printf("Allocation #%d\n", allocation_count);
			printf("Allocation Size:          %d Bytes\n", allocation_size);
			printf("Chunks Spanned:           %d\n", curr->chunk_span);
			printf("Previous Block Address:   0x%x\n", curr->prev);
			printf("Block Address:       	    0x%x\n", curr);
			printf("Next Block Address:       0x%x\n", curr->next);
			printf("------------------------------------\n");
		};
		curr = curr->next;
	};
	const double usage_percentage = ((double)total_used_memory / total_heap_size) * 100;
	printf("\n\n====================================\n");
	printf("             HEAP SUMMARY              \n");
	printf("====================================\n");
	printf("Total Used Allocations:   %d\n", allocation_count);
	printf("Total Used Memory:        %d Bytes\n", total_used_memory);
	printf("Kernel Heap Usage:        %f%%\n", usage_percentage);
	printf("Total Kernel Heap Size:   %d Bytes\n", total_heap_size);
	printf("====================================\n");
	return;
}
