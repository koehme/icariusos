/**
 * @file heap.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief Kernel heap bitmap memory allocator.
 * @date 2024-11-11
 *
 * This file implements a basic heap memory allocator for the kernel.
 * The heap is divided into 4 KiB blocks, aligned with the paging system’s standard page size.
 * This ensures that each allocation fits perfectly within a page, reducing fragmentation and simplifying memory management.
 *
 * Using 4 KiB blocks makes memory allocation efficient by utilizing the entire space of each page.
 * This alignment minimizes wasted space and allows the Memory Management Unit (MMU) to quickly translate virtual addresses to physical addresses,
 * improving access speed and reducing page faults.
 *
 * The allocator includes initialization, allocation, and deallocation, with a bitmap tracking free and used blocks.
 * This approach balances simplicity and performance, making it ideal for kernel-level memory management.
 */

#include "heap.h"
#include "kernel.h"
#include "string.h"

HeapBitMap kheap_bitmap = {
    .saddr = 0x0,
    .total_blocks = 0,
};

Heap kheap = {
    .bitmap = 0x0,
    .saddr = 0x0,
    .block_size = 0,
};

/* PUBLIC API */
void kheap_init(Heap* self, void* heap_saddr, void* bytemap_saddr, const size_t n_bytes, const size_t block_size);
void* kmalloc(const size_t size);
void* kcalloc(const size_t size);
void kfree(void* ptr);
double kheap_info(Heap* self);

/* INTERNAL API */
static size_t _align(Heap* self, const size_t n_bytes);
static void _mark(Heap* self, const size_t start_block, const size_t end_block, const size_t blocks_needed);
static int32_t _demark(Heap* self, const size_t start_block);
static size_t _search(Heap* self, const size_t blocks_needed);
static void* _malloc(Heap* self, const size_t n_bytes);
static void _free(Heap* self, void* ptr);

void* kmalloc(const size_t size)
{
	void* ptr = 0x0;
	ptr = _malloc(&kheap, size);
	return ptr;
};

void* kcalloc(const size_t size)
{
	void* ptr = 0x0;
	ptr = _malloc(&kheap, size);
	memset(ptr, 0x0, size);
	return ptr;
};

void kfree(void* ptr)
{
	_free(&kheap, ptr);
	return;
};

void kheap_init(Heap* self, void* kheap_saddr, void* bitmap_saddr, const size_t n_bytes, const size_t block_size)
{
	self->bitmap->saddr = (uint8_t*)bitmap_saddr;
	self->bitmap->total_blocks = n_bytes / block_size;

	self->saddr = kheap_saddr;
	self->block_size = block_size;

	memset(self->bitmap->saddr, 0x0, self->bitmap->total_blocks * sizeof(uint8_t));
	return;
};

static size_t _align(Heap* self, const size_t n_bytes) { return (n_bytes + self->block_size - 1) & ~(self->block_size - 1); };

static void _mark(Heap* self, const size_t start_block, const size_t end_block, const size_t blocks_needed)
{
	// Mark the first block in the heap map_entry as head, next and used
	uint8_t map_entry = 0b00000000;
	// should be 65 0b01000001
	map_entry |= HEAP_BLOCK_FLAG_USED;
	map_entry |= HEAP_BLOCK_FLAG_HEAD;

	if (blocks_needed > 1) {
		// map_entry should be 193, if more than one block allocated
		map_entry |= HEAP_BLOCK_FLAG_NEXT;
	};

	for (size_t curr_block = start_block; curr_block <= end_block; curr_block++) {
		// Set the descriptor for the current block
		self->bitmap->saddr[curr_block] = map_entry;
		// Reset descriptor for a clean state
		map_entry = 0b00000000;
		// Mark the block as HEAP_BLOCK_FLAG_USED
		map_entry |= HEAP_BLOCK_FLAG_USED;
		// If the current block is not the last one, mark it as HEAP_BLOCK_FLAG_NEXT AND HEAP_BLOCK_FLAG_USED == 129
		const bool has_next_block = (curr_block < end_block - 1);

		if (has_next_block) {
			map_entry |= HEAP_BLOCK_FLAG_NEXT; // Mark block as HEAP_BLOCK_FLAG_NEXT AND HEAP_BLOCK_FLAG_USED == 129
		};
	};
	return;
};

static int32_t _demark(Heap* self, const size_t start_block)
{
	uint8_t map_entry = self->bitmap->saddr[start_block];
	const bool is_head = map_entry & HEAP_BLOCK_IS_HEAD;

	if (!is_head) {
		printf("HeapError: Wrong start block or miscalculated alignment.");
		return -1;
	};
	bool has_next_block = map_entry & HEAP_BLOCK_HAS_NEXT;
	size_t curr_block = start_block;
	self->bitmap->saddr[curr_block] = HEAP_BLOCK_IS_FREE;

	while (has_next_block) {
		curr_block++;
		map_entry = self->bitmap->saddr[curr_block];
		has_next_block = map_entry & HEAP_BLOCK_HAS_NEXT;
		self->bitmap->saddr[curr_block] = HEAP_BLOCK_IS_FREE;
	};
	return 1;
};

static size_t _search(Heap* self, const size_t blocks_needed)
{
	size_t start_block = -1;
	size_t contiguous_free_blocks = 0;

	// Go through all bytemap entries and search for a matching free block
	for (size_t i = 0; i < self->bitmap->total_blocks; i++) {
		const uint8_t curr_map_entry = self->bitmap->saddr[i];
		const bool is_block_free = !(curr_map_entry & HEAP_BLOCK_IS_USED);

		if (is_block_free) {
			// Set the start block
			if (start_block == -1) {
				start_block = i;
			};
			// Add a free block to the sum variable
			contiguous_free_blocks++;
		} else {
			start_block = -1;
			contiguous_free_blocks = 0;
			continue;
		};

		if (contiguous_free_blocks == blocks_needed) {
			break;
		};
	};

	if (start_block == -1) {
		return -1;
	};
	const size_t end_block = start_block + blocks_needed - 1;
	_mark(self, start_block, end_block, blocks_needed);
	return start_block;
};

static void* _malloc(Heap* self, const size_t n_bytes)
{
	// Number of blocks needed for the allocation
	const size_t n_bytes_aligned = _align(self, n_bytes);
	// Blocks in total we need for the allocation
	const size_t blocks_needed = n_bytes_aligned / self->block_size;

	if (blocks_needed > self->bitmap->total_blocks) {
		printf("HeapError: No additional blocks available for allocation\n");
	};
	size_t block = -1;
	block = _search(self, blocks_needed);

	if (block == -1) {
		printf("HeapError: No contiguous blocks available for allocation\n");
		return 0x0;
	};
	void* absolute_address = 0x0;
	absolute_address = self->saddr + (block * self->block_size);
	return absolute_address;
};

// Deallocate the memory block pointed to by 'ptr' to free up memory
void _free(Heap* self, void* ptr)
{
	if (ptr == 0x0) {
		return;
	};
	const size_t start_block = (((size_t)ptr) / self->block_size) % self->block_size;
	const int32_t res = _demark(self, start_block);

	if (res == -1) {
		printf("HeapError: Failed to deallocate memory\n");
		return;
	};
	return;
};

double kheap_info(Heap* self)
{
	const size_t total_blocks = self->bitmap->total_blocks;
	size_t used_blocks = 0;

	for (size_t i = 0; i < total_blocks; ++i) {
		uint8_t map_entry = self->bitmap->saddr[i];

		if (map_entry & HEAP_BLOCK_FLAG_USED) {
			used_blocks++;
		};
	};
	const double utilization = ((double)used_blocks / (double)total_blocks) * 100.0;
	return utilization;
};