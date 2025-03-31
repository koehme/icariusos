#include "errno.h"
#include "icarius.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <stdint.h>

static unsigned int _heap_curr = USER_HEAP_START;
static unsigned int _heap_end = (USER_HEAP_START + PAGE_SIZE) - 1;

void* malloc(size_t size)
{
	if ((_heap_curr + size) > _heap_end) {
		errno = ENOMEM;
		printf("[ERROR] TODO -> Extend Heap Dynamically\n");
		return 0x0;
	};
	void* ptr = (void*)_heap_curr;
	printf("[DEBUG] malloc() → Addr=0x%x | Size=%d Bytes\n", (unsigned int)ptr, size);
	_heap_curr += size;
	return ptr;
};

void* calloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size == 0) {
		return malloc(0);
	};

	if (nmemb > (SIZE_MAX / size)) {
		errno = ENOMEM;
		return 0x0;
	};
	const size_t total_bytes = nmemb * size;
	void* ptr = malloc(total_bytes);

	if (!ptr) {
		errno = ENOMEM;
		return 0x0;
	};
	memset(ptr, 0, total_bytes);
	return ptr;
};