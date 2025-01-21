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

#define CHUNK_SIZE 4096

/* EXTERNAL API */
extern pfa_t pfa;

/* PUBLIC API */
void heap_init(heap_t* self);
void* kmalloc(size_t size);
void* kzalloc(size_t size);
void kfree(void* ptr);
void heap_dump(const heap_t* self);
void heap_trace(const heap_t* self);

/* INTERNAL API */
static void* _malloc(heap_t* self, size_t size);
static void _free(heap_t* self, void* ptr);
static void _init_heap_block(heap_block_t* self, size_t size, heap_block_t* prev);
static void _heap_grow(heap_t* self);

typedef struct heap {
	uintptr_t start_addr;
	uintptr_t next_addr;
	uintptr_t end_addr;
	heap_block_t* last_block;
	size_t used_chunks;
	size_t free_chunks;
} heap_t;

heap_t heap = {
    .start_addr = 0x0,
    .next_addr = 0x0,
    .end_addr = 0x0,
    .last_block = 0x0,
    .used_chunks = 0,
    .free_chunks = 0,
};

void* kmalloc(size_t size)
{
	void* ptr = 0x0;
	ptr = _malloc(&heap, size);

	if (!ptr) {
		printf("[ERROR] Memory Allocation failed for Size: %d\n", size);
		return 0x0;
	};
	return ptr;
};

void* kzalloc(size_t size)
{
	void* ptr = 0x0;
	ptr = _malloc(&heap, size);

	if (!ptr) {
		printf("[ERROR] Memory Allocation failed for Size: %d\n", size);
		return 0x0;
	};
	memset(ptr, 0x0, size);
	return ptr;
};

void kfree(void* ptr)
{
	if (!ptr) {
		return;
	};
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

	const size_t chunks = curr_block->chunk_span;

	heap_block_t* next_block = curr_block->next;
	heap_block_t* iter_block = curr_block;

	curr_block->chunk_span = 1;

	for (size_t i = 1; i < chunks; i++) {
		heap_block_t* new_block = (heap_block_t*)((uint8_t*)iter_block + 4096);
		_init_heap_block(new_block, 4096, iter_block);
		iter_block->next = new_block;
		iter_block = new_block;
	};
	iter_block->next = next_block;

	if (next_block) {
		next_block->prev = iter_block;
	};
	self->free_chunks += chunks;
	self->used_chunks -= chunks;
	return;
};

static void _init_heap_block(heap_block_t* self, size_t size, heap_block_t* prev)
{
	self->chunk_span = 1;
	self->is_free = true;
	self->size = size;
	self->prev = prev;
	self->next = 0x0;
	return;
};

static void _heap_grow(heap_t* self)
{
	const uint64_t phys_addr = pfa_alloc();

	if (!phys_addr) {
		panic("[CRITICAL] Out of Physical Memory. Unable to Allocate more Mem.\n");
		return;
	};
	page_map(self->next_addr, phys_addr, PAGE_PS | PAGE_WRITABLE | PAGE_PRESENT);
	const size_t chunks = PAGE_SIZE / CHUNK_SIZE;
	size_t virt_addr = self->next_addr;

	if (self->last_block) {
		heap_block_t* first_new_block = (heap_block_t*)virt_addr;
		self->last_block->next = first_new_block;
		first_new_block->prev = self->last_block;
	};
	heap_block_t* prev_block = self->last_block;

	for (size_t chunk = 0; chunk < chunks; ++chunk, virt_addr += CHUNK_SIZE) {
		heap_block_t* block = (heap_block_t*)virt_addr;
		_init_heap_block(block, CHUNK_SIZE, prev_block);

		if (prev_block) {
			prev_block->next = block;
		};
		prev_block = block;
	};
	self->last_block = prev_block;
	self->next_addr = virt_addr;
	self->free_chunks += chunks;
	return;
};

static void* _malloc(heap_t* self, size_t size)
{
	const size_t total_size_with_header = size + sizeof(heap_block_t);

	if ((uintptr_t)self->next_addr + total_size_with_header > KERNEL_HEAP_MAX) {
		return 0x0;
	};
	const size_t chunks_needed = (total_size_with_header + CHUNK_SIZE - 1) / CHUNK_SIZE;

	const size_t total_chunks = self->free_chunks + self->used_chunks;

	if (total_chunks == 0) {
		_heap_grow(self);
	} else if (((self->free_chunks * 100) / total_chunks) < 20) {
		_heap_grow(self);
	};

	while (true) {
		heap_block_t* curr_block = (heap_block_t*)self->start_addr;

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

						if (!block->next) {
							_heap_grow(self);
						};
						block = block->next;
					};
					heap_block_t* next_free = block;

					if (next_free) {
						next_free->prev = start_block;
					};
					start_block->next = next_free;

					if (!next_free) {
						_heap_grow(self);
					};
					self->free_chunks -= chunks_needed;
					self->used_chunks += chunks_needed;
					return (void*)((uint8_t*)start_block + sizeof(heap_block_t));
				};
				curr_block = start_block->next;
			} else {
				curr_block = curr_block->next;
			};
		};
		_heap_grow(self);
	};
	return 0x0;
};

void heap_init(heap_t* self)
{
	self->start_addr = self->next_addr = KERNEL_HEAP_START;
	self->end_addr = KERNEL_HEAP_MAX;
	self->last_block = 0x0;
	return;
};

void heap_dump(const heap_t* self)
{
	heap_block_t* curr = (heap_block_t*)self->start_addr;
	size_t block_count = 0;
	size_t total_used_memory = 0;
	size_t allocation_count = 0;
	const size_t total_heap_size = (self->end_addr - self->start_addr) + 1;

	printf("\n====================================\n");
	printf("           KERNEL HEAP DUMP         \n");
	printf("====================================\n");
	printf("Heap Start Address:       0x%x\n", self->start_addr);
	printf("Heap Last Block Address:  0x%x\n", self->last_block);
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
	printf("           KERNEL HEAP SUMMARY              \n");
	printf("====================================\n");
	printf("Total Used Allocations:   %d\n", allocation_count);
	printf("Total Used Memory:        %d Bytes\n", total_used_memory);
	printf("Kernel Heap Usage:        %f%%\n", usage_percentage);
	printf("Total Kernel Heap Size:   %d Bytes\n", total_heap_size);
	printf("Heap Free Chunks:         %d\n", self->free_chunks);
	printf("Heap Used Chunks:         %d\n", self->used_chunks);
	printf("====================================\n");
	return;
};

void heap_trace(const heap_t* self)
{
	heap_block_t* curr = (heap_block_t*)self->start_addr;
	printf("\n\n====================================\n");
	printf("           KERNEL HEAP TRACE              \n");
	printf("====================================\n");

	while (curr) {
		printf("<- 0x%x | 0x%x | -> 0x%x | %d] \n", curr->prev ? (unsigned int)curr->prev : 0, (unsigned int)curr,
		       curr->next ? (unsigned int)curr->next : 0, curr->chunk_span * 4096);
		curr = curr->next;
		busy_wait(50000);
	};
	printf("==========================\n");
	return;
};