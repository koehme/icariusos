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

static void* _malloc(heap_t* self, size_t size)
{
	// Step 1: Check if the allocation list in self->head is initialized
	// Case 1: If the list is empty
	//    - Request a new memory block of 4 MiB from the physical frame allocator (pfa_alloc).
	//    - Use the address stored in self->next_addr to map this memory into the heap's virtual address space.
	//    - Divide the entire 4 MiB block, represented by the virtual address in self->next_addr, into 4096-byte chunks.
	//    - Create heap_block_t structures for each 4 KiB chunk and set their metadata, such as address and is_free flag.
	//    - Append all 1024 newly created heap_block_t chunks to the doubly linked list that begins with self->head.

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