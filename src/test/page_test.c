/**
 * @file page_test.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "page_test.h"

void test_page_dir_create(const uint32_t* pd)
{
	if (!pd) {
		printf("[ERROR] Failed to create Page Directory\n");
		return;
	};
	// Debug information about the page directory
	printf("[DEBUG INFO] New Page Directory created at Virtual Address: 0x%x\n", pd);
	printf("[DEBUG INFO] First Kernel Mapping (768): 0x%x\n", pd[768]);
	printf("[DEBUG INFO] Last Kernel Mapping (1023): 0x%x\n", pd[1023]);

	// Iterate through kernel mappings
	for (int32_t i = 768; i < 1024; i++) {
		if (pd[i] & PAGE_PRESENT) {
			printf(" - Entry %d: PhysAddr=0x%x | Flags=0x%x\n", i, pd[i] & 0xFFFFF000, pd[i] & 0xFFF);
		};
	};
	return;
};
