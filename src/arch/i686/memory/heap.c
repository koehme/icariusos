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

static void _free(heap_t* self, void* ptr) { return; };

static void _init_heap_block(heap_block_t* block, size_t address, size_t size, heap_block_t* prev)
{
	block->chunk_span = 0;	  // Single chunk by default
	block->is_free = true;	  // Mark as free
	block->size = size;	  // Set block size
	block->address = address; // Virtual address
	block->prev = prev;	  // Previous block
	block->next = 0x0;	  // Next will be set in the loop
	return;
};

static void _heap_split_page_to_chunks(heap_t* self, uint64_t phys_addr)
{
	// Map the phys addr to virt_addr in self->next_addr
	page_map(self->next_addr, phys_addr, PAGE_PS | PAGE_WRITABLE | PAGE_PRESENT);
	// Define page and chunk sizes
	const size_t page_size = 4 * 1024 * 1024; // 4 MiB
	const size_t chunk_size = 4096;		  // 4 KiB
	const size_t chunks = page_size / chunk_size;
	// Start from the next available virtual address
	size_t virt_addr = self->next_addr;
	heap_block_t* prev_block = NULL;

	// Loop through all chunks in the 4 MiB block
	for (size_t chunk = 0; chunk < chunks; ++chunk, virt_addr += chunk_size) {
		// Interpret the current virtual address as a heap_block_t pointer
		heap_block_t* block = (heap_block_t*)virt_addr;
		// Initialize the heap block metadata
		_init_heap_block(block, virt_addr, chunk_size, prev_block);

		if (prev_block) {
			prev_block->next = block;
		};
		// Set self->head if this is the first block
		if (chunk == 0) {
			self->head = block;
		};
		prev_block = block;
	};
	// Set self->tail to the last block
	self->tail = prev_block;
	// Update the heap's next available virtual address. Points to the next available 4 MiB region
	self->next_addr = virt_addr;
	return;
};

static void* _malloc(heap_t* self, size_t size)
{
	// Step 1: Check if the allocation list in self->head is initialized
	// Case 1: If the list is empty
	//    - Request a new memory block of 4 MiB from the physical frame allocator (pfa_alloc).
	//    - Use the address stored in self->next_addr to map this memory into the heap's virtual address space.
	//    - Divide the entire 4 MiB block, represented by the virtual address in self->next_addr, into 4096-byte chunks.
	//    - Create heap_block_t structures for each 4 KiB chunk and set their metadata, such as address and is_free flag.
	//    - Append all 1024 newly created heap_block_t chunks to the doubly linked list that begins with self->head.

	if (!self->head || (self->head == self->tail)) {
		uint64_t phys_addr = pfa_alloc();
		_heap_split_page_to_chunks(self, phys_addr);
	};
	// Case 2: If the list is not empty
	//    - Traverse the doubly linked list starting from self->head.
	//    - Search for a contiguous sequence of free blocks (as indicated by the is_free flag) that satisfies the requested size.
	//    - Consider that larger size requests may span multiple 4 KiB chunks, so count adjacent free blocks to determine eligibility.

	// Step 2: If a suitable memory region is found, mark the selected blocks as allocated and return the address of the first block.
	// Step 3: If no suitable region is found in the existing list, repeat Case 1 to request and initialize a new 4 MiB block.
	return 0x0;
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
		printf("Heap is EMPTY.\n");
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