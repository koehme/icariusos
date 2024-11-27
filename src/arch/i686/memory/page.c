/**
 * @file page.c
 * @author
 * @copyright MIT
 * @brief Implementierung von Paging mit 4 MiB Pages
 * @date 2024-11-14
 * @see
 */

#include "page.h"
#include "string.h"

extern uint32_t kernel_directory[1024];

void page_map(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags)
{
	/*
	4 MiB Paging:
	- Page directory index (higher bits, e.g., bits 22-31).
	- Offset (lower bits, e.g., bits 0-21).
	*/
	const uint32_t pd_index = virt_addr >> 22;
	uint32_t* page_directory = kernel_directory;
	page_directory[pd_index] = (phys_addr & 0xFFC00000) | (flags & 0xFFF);
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

void page_unmap(const uint32_t virt_addr)
{
	/*
	Entfernt eine 4 MiB Page-Mapping aus dem Page Directory.
	*/
	const uint32_t pd_index = virt_addr >> 22;

	uint32_t* page_directory = kernel_directory;

	// Entferne den Page Directory Entry
	page_directory[pd_index] = 0;

	// Synchronisiere TLB
	asm volatile("invlpg (%0)" ::"r"(virt_addr) : "memory");
	return;
};

uint32_t page_get_phys_addr(const uint32_t virt_addr)
{
	/*
	Ermittelt die physische Adresse aus einer virtuellen Adresse bei 4 MiB Pages.
	*/
	const uint32_t pd_index = virt_addr >> 22;

	uint32_t* page_directory = kernel_directory;

	// Überprüfe, ob die Seite im Page Directory vorhanden ist
	if (!(page_directory[pd_index] & PAGE_PRESENT)) {
		return 0x0; // Nicht gemappt
	}

	// Berechne die physische Adresse
	return (page_directory[pd_index] & 0xFFC00000) + (virt_addr & 0x3FFFFF);
	/*
	     ^                     ^             ^
	Base address       Remove flags       Offset innerhalb der 4 MiB Page
	*/
};
