/**
 * @file page.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief Paging 4 MiB Pages
 * @date 2024-11-14
 * @see
 */

#include "page.h"


/* EXTERNAL API */
extern uint32_t kernel_directory[1024];

/* PUBLIC API */
void page_dump_dir(uint32_t* dir);
uint32_t* page_create_dir(uint32_t flags);
void page_set_dir(uint32_t* self);
uint32_t* page_get_dir(void);
void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_map_dir(uint32_t* dir, uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void page_unmap(const uint32_t virt_addr);
uint32_t page_get_phys_addr(uint32_t* dir, const uint32_t virt_addr);

void page_dump_dir(uint32_t* dir)
{
	if (!dir) {
		printf("[ERROR] Invalid Page Directory!\n");
		return;
	};
	printf("\n");
	printf("====================================\n");
	printf("          PAGE DIR STATISTICS       \n");
	printf("====================================\n");
	printf("Physical Address: 0x%x\n", v2p(dir));

	for (uint32_t i = 0; i < 1024; i++) {
		if (dir[i] & PAGE_PRESENT) {
			const uint32_t virt_addr = i * 0x400000;
			const uint32_t phys_addr = dir[i] & 0xFFC00000; // 4 MiB Page Mask
			const uint32_t flags = dir[i] & 0xFFF;

			printf("%d  | 0x%x | 0x%x | 0x%x  | %s %s %s %s\n", i, virt_addr, phys_addr, flags, (flags & PAGE_PRESENT) ? "P" : "-",
			       (flags & PAGE_WRITABLE) ? "W" : "-", (flags & PAGE_USER) ? "U" : "K", (flags & PAGE_PS) ? "4M" : "4K");
		};
	};
	printf("====================================\n");
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
	uint32_t* dir = (uint32_t*)virt_addr;
	page_map_dir(dir, virt_addr, phys_addr, flags);

	memset((void*)virt_addr, 0, PAGE_SIZE);

	for (int32_t i = 768; i < 1024; i++) {
		const uint32_t entry = kernel_directory[i];

		if (entry & (1 << 0)) {
			dir[i] = (entry & 0xFFFFF000) | (PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE);
		};
	};
	return dir;
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

uint32_t page_get_phys_addr(uint32_t* dir, const uint32_t virt_addr)
{
	const uint32_t pd_index = virt_addr >> 22;

	if (!(dir[pd_index] & PAGE_PRESENT)) {
		return 0x0;
	};
	return (dir[pd_index] & 0xFFC00000) + (virt_addr & 0x3FFFFF);
};

void page_restore_kernel_dir(void)
{
	const uint32_t phys_addr = (uint32_t)(v2p((void*)kernel_directory));
	asm volatile("mov %0, %%cr3" : : "r"(phys_addr));
	printf("[DEBUG] Switched to Kernel Page Directory at Phys: 0x%x\n", phys_addr);
	return;
};

void page_map_between(uint32_t* dir, uint32_t virt_start_addr, uint32_t virt_end_addr, uint32_t flags)
{
	if (!dir) {
		printf("[ERROR] Invalid Page Directory!\n");
		return;
	};
	virt_start_addr &= 0xFFFFF000;
	virt_end_addr = (virt_end_addr + 0xFFF) & 0xFFFFF000;

	for (uint32_t virt_curr_addr = virt_start_addr; virt_curr_addr < virt_end_addr; virt_curr_addr += PAGE_SIZE) {
		const uint32_t frame = pfa_alloc();

		if (!frame) {
			printf("[ERROR] Page Frame Allocator exhausted!\n");
			return;
		};
		page_map_dir(dir, virt_curr_addr, frame, flags);
		printf("[DEBUG] Mapped Virtual: 0x%x -> Physical: 0x%x (Flags: 0x%x)\n", virt_curr_addr, frame, flags);
	};
	printf("Mapped Virtual Memory: 0x%x - 0x%x\n", virt_start_addr, virt_end_addr);
	return;
};