#include "page.h"


#include "string.h"

static page_t page_directory[PAGE_ENTRIES] __attribute__((aligned(4096)));
static page_t page_tables[PAGE_ENTRIES][PAGE_ENTRIES] __attribute__((aligned(4096)));

void page_paging_init()
{
	mset8(page_directory, 0, sizeof(page_directory));
	// Identity Mapping
	for (uint32_t i = 0; i < PAGE_ENTRIES; i++) {
		page_directory[i] = (uint32_t)page_tables[i] | PAGE_PRESENT | PAGE_WRITABLE;

		for (uint32_t j = 0; j < PAGE_ENTRIES; j++) {
			page_tables[i][j] = (i * PAGE_ENTRIES + j) * PAGE_SIZE | PAGE_PRESENT | PAGE_WRITABLE;
		};
	};
	asm volatile("mov %0, %%cr3" ::"r"(page_directory));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0" ::"r"(cr0));
	return;
};

void page_map_virt_to_phys(void* virt_addr, void* phys_addr, const uint32_t flags)
{
	// High 10 Bits
	const uint32_t dir_index = (uint32_t)virt_addr >> 22;
	// Middle 10 Bits
	const uint32_t table_index = ((uint32_t)virt_addr >> 12) & 0x03FF;
	page_tables[dir_index][table_index] = ((uint32_t)phys_addr & ~0xFFF) | (flags & 0xFFF);
	return;
}

void page_umap(void* virt_addr)
{
	const uint32_t dir_index = (uint32_t)virt_addr >> 22;
	const uint32_t table_index = ((uint32_t)virt_addr >> 12) & 0x03FF;
	page_tables[dir_index][table_index] = 0;
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

void* page_get_phys_addr(void* virt_addr)
{
	uint32_t dir_index = (uint32_t)virt_addr >> 22;
	uint32_t table_index = ((uint32_t)virt_addr >> 12) & 0x03FF;
	uint32_t offset = (uint32_t)virt_addr & 0xFFF;

	if (page_tables[dir_index][table_index] & PAGE_PRESENT) {
		return (void*)((page_tables[dir_index][table_index] & ~0xFFF) + offset);
	};
	return 0x0;
};