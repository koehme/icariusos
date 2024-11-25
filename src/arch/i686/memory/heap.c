/**
 * @file heap.c
 * @author Kevin Oehme
 * @copyright MIT
 * @date 2024-11-11
 */

#include "heap.h"
#include "kernel.h"
#include "string.h"

heap_t heap = {
    .start_addr = 0x0,
    .next_addr = 0x0,
    .end_addr = 0x0,
};

/* PUBLIC API */
void heap_init(heap_t* self);
void* kmalloc(const size_t size);
void* kzalloc(const size_t size);
void kfree(void* ptr);

/* INTERNAL API */
static size_t _align_bytes(const size_t bytes);
static void* _malloc(heap_t* self, const size_t bytes);

void* kmalloc(const size_t size)
{
	void* ptr = 0x0;
	ptr = _malloc(&heap, size);
	return ptr;
};

void* kzalloc(const size_t size)
{
	void* ptr = 0x0;
	ptr = _malloc(&heap, size);
	memset(ptr, 0x0, size);
	return ptr;
};

void kfree(void* ptr)
{
	// _free(&heap, ptr);
	return;
};

static size_t _align_bytes(const size_t bytes)
{
	/*
	Check if the size is not already aligned to PAGE_SIZE
	Case already aligned
		4096	1000000000000	0x1000
	&	4095	0111111111111	0xfff
	=	0		0000000000000	0x0

	Case not aligned
		4097	1000000000001	0x1001
	&	4095	0111111111111	0xfff
	=	1		0000000000001	0x1
	*/
	if (bytes & (PAGE_SIZE - 1)) {
		// Round up to the next PAGE_SIZE boundary
		// eq == bytes - (bytes % PAGE_SIZE) + PAGE_SIZE
		return (bytes & ~(PAGE_SIZE - 1)) + PAGE_SIZE;
	};
	// If already aligned, return the original size
	return bytes;
};

static void* _malloc(heap_t* self, const size_t bytes)
{
	const size_t size = _align_bytes(bytes);
	const uint32_t start_addr = self->next_addr;
	const uint32_t end_addr = start_addr + size;

	if (end_addr > KERNEL_HEAP_MAX) {
		return 0x0;
	};
	for (uint32_t virt_addr = start_addr; virt_addr < end_addr; virt_addr += PAGE_SIZE) {
		const uint32_t phys_addr = pfa_alloc();

		if (!phys_addr) {
			return 0x0;
		};
		// Mapping from phy to virtual
		map_page(virt_addr, phys_addr, PAGE_PRESENT | PAGE_WRITABLE);
	};
	self->next_addr = end_addr;
	return (void*)start_addr;
};

void heap_init(heap_t* self)
{
	self->start_addr = self->next_addr = HEAP_START_ADDR;
	self->end_addr = KERNEL_HEAP_MAX;
	return;
};
