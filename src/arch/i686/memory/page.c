/**
 * @file page.c
 * @author
 * @copyright MIT
 * @brief Paging 4 MiB Pages
 * @date 2024-11-14
 * @see
 */

#include "page.h"
#include "string.h"

/* EXTERNAL API */
extern uint32_t kernel_directory[1024];

/* PUBLIC API */
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_unmap(const uint32_t virt_addr);
uint32_t page_get_phys_addr(const uint32_t virt_addr);

void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* page_directory = kernel_directory;
	page_directory[pd_index] = (phys_addr & 0xFFC00000) | (flags & 0xFFF);
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

void page_unmap(const uint32_t virt_addr)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* page_directory = kernel_directory;
	page_directory[pd_index] = 0;
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

uint32_t page_get_phys_addr(const uint32_t virt_addr)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* page_directory = kernel_directory;


	if (!(page_directory[pd_index] & PAGE_PRESENT)) {
		return 0x0;
	};
	return (page_directory[pd_index] & 0xFFC00000) + (virt_addr & 0x3FFFFF);
};
