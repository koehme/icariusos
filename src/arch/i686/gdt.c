/**
 * @file gdt.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief
 * @date 2024-11-14
 * @see
 */

#include "gdt.h"
#include "tss.h"

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
extern tss_t tss;

/* PUBLIC API */
void gdt_init(void);
void gdt_set_entry(const uint32_t num, const uint32_t base, const uint32_t limit, const uint8_t access, const uint32_t flags);

gdt_entry_t gdt_entries[6];
gdt_t gdt;

void gdt_init(void)
{
	gdt.limit = (sizeof(gdt_entry_t) * 6) - 1;
	gdt.base = (uint32_t)&gdt_entries;
	gdt_set_entry(0, 0, 0, 0, 0);
	gdt_set_entry(1, 0, 0xFFFFFFFF, KERNEL_CODE_SEG, 0xCF);
	gdt_set_entry(2, 0, 0xFFFFFFFF, KERNEL_DATA_SEG, 0xCF);
	gdt_set_entry(3, 0, 0xFFFFFFFF, USER_CODE_SEG, 0xCF);
	gdt_set_entry(4, 0, 0xFFFFFFFF, USER_DATA_SEG, 0xCF);
	gdt_set_entry(5, (uint32_t)&tss, sizeof(tss_t) - 1, TSS_SEG, 0x00);
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