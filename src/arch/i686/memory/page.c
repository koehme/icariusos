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
void page_dump_curr_directory(void);
uint32_t* page_create_directory(uint32_t flags);
void page_set_directory(uint32_t* new_page_dir);
uint32_t* page_get_directory(void);
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_unmap(const uint32_t virt_addr);
uint32_t page_get_phys_addr(const uint32_t virt_addr);

/* INTERNAL API */
static uint32_t* _curr_page_dir;

void page_dump_curr_directory(void)
{
	uint32_t* page_directory = page_get_directory();

	if (!page_directory) {
		printf("[ERROR] No active Page Directory found!\n");
		return;
	};
	printf("====================================\n");
	printf("     PAGE DIRECTORY DUMP (4 MiB)    \n");
	printf("====================================\n");

	printf("Physical Address: 0x%x\n", page_directory);

	for (uint32_t i = 0; i < 1024; i++) {
		if (page_directory[i] & PAGE_PRESENT) {
			const uint32_t phys_addr = page_directory[i] & 0xFFC00000;
			const uint32_t flags = page_directory[i] & 0xFFF;

			printf("Entry %d: PhysAddr=0x%x | Flags=0x%x | %s %s %s\n", i, phys_addr, flags, (flags & PAGE_PRESENT) ? "P" : "-",
			       (flags & PAGE_WRITABLE) ? "W" : "-", (flags & PAGE_USER) ? "U" : "K");
		};
	};
	printf("====================================\n\n");
	return;
};

uint32_t* page_create_directory(uint32_t flags)
{
	uint64_t phys_addr = pfa_alloc();

	if (!phys_addr) {
		return 0x0;
	};
	const uint32_t virt_addr = phys_addr + KERNEL_VIRTUAL_START;
	page_map(virt_addr, phys_addr, flags);

	memset((void*)virt_addr, 0, PAGE_SIZE);
	uint32_t* new_page_dir = (uint32_t*)virt_addr;

	for (int32_t i = 768; i < 1024; i++) {
		if (kernel_directory[i] & PAGE_PRESENT) {
			new_page_dir[i] = (kernel_directory[i] & 0xFFFFF000) | (PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE);
		};
	};
	return new_page_dir;
};

void page_set_directory(uint32_t* dir)
{
	_curr_page_dir = dir;
	asm volatile("mov %0, %%cr3" : : "r"(dir));
	return;
};

uint32_t* page_get_directory(void)
{
	uint32_t cr3;
	asm volatile("mov %%cr3, %0" : "=r"(cr3));
	return (uint32_t*)cr3;
};

void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* page_directory = page_get_directory();
	page_directory[pd_index] = (phys_addr & 0xFFC00000) | (flags & 0xFFF);
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

void page_unmap(const uint32_t virt_addr)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* page_directory = page_get_directory();
	page_directory[pd_index] = 0;
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

uint32_t page_get_phys_addr(const uint32_t virt_addr)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* page_directory = page_get_directory();

	if (!(page_directory[pd_index] & PAGE_PRESENT)) {
		return 0x0;
	};
	return (page_directory[pd_index] & 0xFFC00000) + (virt_addr & 0x3FFFFF);
};