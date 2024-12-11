/**
 * @file gdt.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief
 * @date 2024-11-14
 * @see
 */

#include "gdt.h"

typedef struct gdt_entry {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt {
	uint16_t limit;
	unsigned int base;
} __attribute__((packed)) gdt_t;

/* EXTERNAL API */
extern void asm_gdt_flush(uint32_t);

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
	gdt.limit = (sizeof(gdt_entry_t) * 6) - 1;
	gdt.base = (uint32_t)&gdt_entries;
	gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
	asm_gdt_flush((uint32_t)&gdt);
	return;
};

void gdt_set_entry(const uint32_t num, const uint32_t base, const uint32_t limit, const uint8_t access, const uint32_t flags)
{
	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;
	gdt_entries[num].limit = (limit & 0xFFFF);
	gdt_entries[num].flags = (limit >> 16) & 0x0F;
	gdt_entries[num].flags |= (flags & 0xF0);
	gdt_entries[num].access = access;
	return;
};