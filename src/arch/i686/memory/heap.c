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
static inline size_t align_to_page_size(size_t size);
static void* _malloc(heap_t* self, size_t size);
static void _free(heap_t* self, void* ptr);

static inline size_t align_to_page_size(size_t size) { return (size + sizeof(heap_block_t) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); };

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
	size = align_to_page_size(size);
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
