/**
 * @file page.c
 * @author Kevin Oehme
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
uint32_t* page_create_directory(uint32_t flags);
void page_set_directory(uint32_t* new_page_dir);
uint32_t* page_get_directory(void);
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_unmap(const uint32_t virt_addr);
uint32_t page_get_phys_addr(const uint32_t virt_addr);

/* INTERNAL API */
static uint32_t* _curr_page_dir;

uint32_t* page_create_directory(uint32_t flags)
{
	uint64_t phys_addr = pfa_alloc();

	if (!phys_addr) {
		panic("[ERROR] Out of Memory\n");
	};
	const uint32_t virt_addr = phys_addr + KERNEL_VIRTUAL_START;
	page_map(virt_addr, phys_addr, flags);

	memset((void*)virt_addr, 0, PAGE_SIZE);
	uint32_t* pd = (uint32_t*)virt_addr;

	for (int32_t i = 768; i < 1024; i++) {
		pd[i] = kernel_directory[i];
	};
	return pd;
};

void page_set_directory(uint32_t* new_page_dir)
{
	_curr_page_dir = new_page_dir;
	asm volatile("mov %0, %%cr3" : : "r"(_curr_page_dir));
	return;
};

uint32_t* page_get_directory(void) { return _curr_page_dir; };

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