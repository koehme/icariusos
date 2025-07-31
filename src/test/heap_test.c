/**
 * @file heap_test.c
 * @author Kevin Oehme
 * @copyright MIT
 */


#include "heap_test.h"

void test_heap(const int32_t size)
{
	kprintf("\n");
	kprintf("############################\n");
	kprintf("##       HEAP TEST        ##\n");
	kprintf("##------------------------##\n");

	void* ptr = kzalloc(size);
	if (!ptr) {
		kprintf("## [ERROR] Failed to Allocate %d Bytes.\n", size);
		kprintf("############################\n");
		return;
	};
	kprintf("##    ALLOCATED MEMORY    ##\n");
	kprintf("##   USER:  0x%x    ##\n", ptr);
	void* block_ptr = (void*)((uint8_t*)ptr - sizeof(heap_block_t));
	kprintf("##   BLOCK: 0x%x    ##\n", (uint32_t)block_ptr);
	kfree(ptr);
	kprintf("############################\n");
	return;
};