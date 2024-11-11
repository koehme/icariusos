#include "gdt.h"


extern void gdt_flush(uint32_t);

struct GdtEntry gdt_entries[6];
struct Gdt gdt;

#define KERNEL_CODE_SEG 0x9A
#define KERNEL_DATA_SEG 0x92
#define USER_CODE_SEG 0xFA
#define USER_DATA_SEG 0xF2
#define TSS_SEG 0x89

void gdt_init()
{
	gdt.limit = (sizeof(struct GdtEntry) * 6) - 1;
	gdt.base = (uint32_t)&gdt_entries;

	gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment

	gdt_flush((uint32_t)&gdt);
	return;
};

void gdt_set_entry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags)
{
	// Base address
	gdt_entries[num].base_low = (base & 0xFFFF);	    // The low 16 bits
	gdt_entries[num].base_middle = (base >> 16) & 0xFF; // The middle 8 bits
	gdt_entries[num].base_high = (base >> 24) & 0xFF;   // The high 8 bits

	gdt_entries[num].limit = (limit & 0xFFFF);
	gdt_entries[num].flags = (limit >> 16) & 0x0F; // High 4 bits of the limit
	gdt_entries[num].flags |= (flags & 0xF0);      // Place flags on top of the limit

	gdt_entries[num].access = access;
	return;
};