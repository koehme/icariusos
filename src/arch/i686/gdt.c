/**
 * @file gdt.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief
 * @date 2024-11-14
 * @see
 */

#include "gdt.h"

/* EXTERNAL API */
extern void gdt_flush(uint32_t);

/* PUBLIC API */
void gdt_init(void);
void gdt_set_entry(const uint32_t num, const uint32_t base, const uint32_t limit, const uint8_t access, const uint32_t flags);

gdt_entry_t gdt_entries[6];
gdt_t gdt;

#define KERNEL_CODE_SEG 0x9A
#define KERNEL_DATA_SEG 0x92
#define USER_CODE_SEG 0xFA
#define USER_DATA_SEG 0xF2
#define TSS_SEG 0x89

void gdt_init(void)
{
	// Set the GDT limit (size of all entries - 1) and base (address of gdt_entries array)
	gdt.limit = (sizeof(struct gdt_entry_t) * 6) - 1;
	gdt.base = (uint32_t)&gdt_entries;
	// Set up the individual GDT entries:
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment
	// Load the new GDT using an assembly function
	gdt_flush((uint32_t)&gdt);
	return;
};

void gdt_set_entry(const uint32_t num, const uint32_t base, const uint32_t limit, const uint8_t access, const uint32_t flags)
{
	// Set the base address of the segment
	gdt_entries[num].base_low = (base & 0xFFFF);	    // The low 16 bits
	gdt_entries[num].base_middle = (base >> 16) & 0xFF; // The middle 8 bits
	gdt_entries[num].base_high = (base >> 24) & 0xFF;   // The high 8 bits
	// Set the limit
	gdt_entries[num].limit = (limit & 0xFFFF);     //  Lower 16 bits of the limit
	gdt_entries[num].flags = (limit >> 16) & 0x0F; // High 4 bits of the limit
	gdt_entries[num].flags |= (flags & 0xF0);      // Combine flags with the limit
	// Set the access byte, which defines the segment type and privileges
	gdt_entries[num].access = access;
	return;
};