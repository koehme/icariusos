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
void page_dump_dir(void);
uint32_t* page_create_dir(uint32_t flags);
void page_set_dir(uint32_t* self);
uint32_t* page_get_dir(void);
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_map_dir(uint32_t* dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_unmap(const uint32_t virt_addr);
uint32_t page_get_phys_addr(const uint32_t virt_addr);

void page_dump_dir(void)
{
	uint32_t* dir = page_get_dir();

	if (!dir) {
		printf("[ERROR] No Page Directory found!\n");
		return;
	};
	printf("\n");
	printf("====================================\n");
	printf("          PAGE DIR STATISTICS       \n");
	printf("====================================\n");
	printf("Physical Address: 0x%x\n", dir);

	for (uint32_t i = 0; i < 1024; i++) {
		if (dir[i] & PAGE_PRESENT) {
			const uint32_t phys_addr = dir[i] & 0xFFC00000;
			const uint32_t flags = dir[i] & 0xFFF;

			printf("Entry %d: PhysAddr=0x%x | Flags=0x%x | %s %s %s\n", i, phys_addr, flags, (flags & PAGE_PRESENT) ? "P" : "-",
			       (flags & PAGE_WRITABLE) ? "W" : "-", (flags & PAGE_USER) ? "U" : "K");
		};
	};
	printf("====================================\n\n");
	return;
};

uint32_t* page_create_dir(uint32_t flags)
{
	uint64_t phys_addr = pfa_alloc();

	if (!phys_addr) {
		return 0x0;
	};

	if (phys_addr & 0x3FFFFF) {
		printf("[ERROR] Page Directory phys_addr (0x%x) is not 4 MiB aligned!\n", phys_addr);
		return 0x0;
	};
	const uint32_t virt_addr = (uint32_t)p2v((uint32_t)phys_addr);
	uint32_t* new_page_dir = (uint32_t*)virt_addr;
	page_map_dir(new_page_dir, virt_addr, phys_addr, flags);
	memset((void*)virt_addr, 0, PAGE_SIZE);

	for (int32_t i = 768; i < 1024; i++) {
		if (kernel_directory[i] & PAGE_PRESENT) {
			new_page_dir[i] = (kernel_directory[i] & 0xFFFFF000) | (PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
		};
	};
	return new_page_dir;
};

void page_set_dir(uint32_t* self)
{
	asm volatile("mov %0, %%cr3" : : "r"(self));
	return;
};

uint32_t* page_get_dir(void)
{
	uint32_t cr3;
	asm volatile("mov %%cr3, %0" : "=r"(cr3));
	return (uint32_t*)cr3;
};

void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* dir = page_get_dir();
	dir[pd_index] = (phys_addr & 0xFFC00000) | (flags & 0xFFF);
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

void page_map_dir(uint32_t* dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
	const uint32_t pd_index = virt_addr >> 22;
	dir[pd_index] = (phys_addr & 0xFFC00000) | (flags & 0xFFF);
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

void page_unmap(const uint32_t virt_addr)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* dir = page_get_dir();
	dir[pd_index] = 0;
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

uint32_t page_get_phys_addr(const uint32_t virt_addr)
{
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* dir = page_get_dir();

	if (!(dir[pd_index] & PAGE_PRESENT)) {
		return 0x0;
	};
	return (dir[pd_index] & 0xFFC00000) + (virt_addr & 0x3FFFFF);
};

void page_restore_kernel_dir(void)
{
	const uint32_t phys_addr = (uint32_t)(v2p((void*)kernel_directory));
	asm volatile("mov %0, %%cr3" : : "r"(phys_addr));
	return;
};