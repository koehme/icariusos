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

void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
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

void page_unmap(const uint32_t virt_addr)
{
	const uint32_t pd_index = virt_addr >> 22;
	const uint32_t pt_index = (virt_addr >> 12) & 0x03FF;

	uint32_t* page_directory = kernel_directory;

	if (!(page_directory[pd_index] & PAGE_PRESENT)) {
		return;
	};
	uint32_t* page_table = (uint32_t*)((page_directory[pd_index] & ~0xFFF) + KERNEL_VIRT_BASE);
	page_table[pt_index] = 0;
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

uint32_t page_get_phys_addr(const uint32_t virt_addr)
{
	/*
	Example for virt_addr: 0xC1000000
	- Page Directory Index (pd_index): 768 (highest 10 bits).
	- Page Table Index (pt_index): 0x0 (next 10 bits).
	- Offset: 0x0 (lowest 12 bits, page-aligned).
	*/
	// Extract Page Directory and Page Table indices
	const uint32_t pd_index = virt_addr >> 22;
	const uint32_t pt_index = (virt_addr >> 12) & 0x03FF;
	uint32_t* page_directory = kernel_directory;
	// Page Directory check
	if (!(page_directory[pd_index] & PAGE_PRESENT)) {
		// Not mapped in Page Directory
		return 0x0;
	};
	// Get the Page Table and convert physical to virtual address
	const uint32_t* page_table = (uint32_t*)((kernel_directory[pd_index] & ~0xFFF) + KERNEL_VIRT_BASE);
	// Page Table check
	if (!(page_table[pt_index] & PAGE_PRESENT)) {
		return 0x0; // Not mapped in Page Table
	};
	// Compute physical address
	return (page_table[pt_index] & ~0xFFF) + (virt_addr & 0xFFF);
	/*
		    ^                     ^           ^
		Base address       Remove flags       Extract the offset
		of the page        (lower 12 bits)    (lower 12 bits)
	*/
};