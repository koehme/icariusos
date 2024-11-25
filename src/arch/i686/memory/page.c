/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief
 * @date 2024-11-14
 * @see
 */

#include "page.h"
#include "string.h"

extern uint32_t kernel_directory[1024];

/*
Page directory index (higher bits, e.g., bits 22-31).
Page table index (middle bits, e.g., bits 12-21).
Page offset (lower bits, e.g., bits 0-11).
*/

void map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
	/*
    Page directory index (higher bits, e.g., bits 22-31).
    Page table index (middle bits, e.g., bits 12-21).
    Page offset (lower bits, e.g., bits 0-11).
    */
	const uint32_t pd_index = virt_addr >> 22;
	const uint32_t pt_index = (virt_addr >> 12) & 0x03FF;

	uint32_t* page_directory = kernel_directory;

	if (!(page_directory[pd_index] & PAGE_PRESENT)) {
		const uint32_t page_table = pfa_alloc();
		memset((void*)(KERNEL_VIRT_BASE + page_table), 0, PAGE_SIZE);
		page_directory[pd_index] = page_table | PAGE_PRESENT | PAGE_WRITABLE;
	};
	uint32_t* page_table = (uint32_t*)((page_directory[pd_index] & ~0xFFF) + KERNEL_VIRT_BASE);
	page_table[pt_index] = phys_addr | flags;
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};