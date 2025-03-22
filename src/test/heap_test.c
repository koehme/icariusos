#include "heap_test.h"

void test_heap(const int32_t size)
{
	printf("\n");
	printf("############################\n");
	printf("##       HEAP TEST        ##\n");
	printf("##------------------------##\n");

	void* ptr = kzalloc(size);
	if (!ptr) {
		printf("## [ERROR] Failed to Allocate %d Bytes.\n", size);
		printf("############################\n");
		return;
	};
	printf("##    ALLOCATED MEMORY    ##\n");
	printf("##   USER:  0x%x    ##\n", ptr);
	void* block_ptr = (void*)((uint8_t*)ptr - sizeof(heap_block_t));
	printf("##   BLOCK: 0x%x    ##\n", (uint32_t)block_ptr);
	kfree(ptr);
	printf("############################\n");
	return;
};