/**
 * @file gdt.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief
 * @date 2024-11-14
 * @see
 */

#include "gdt.h"
#include "heap.h"
#include "tss.h"

// Access Flags for GDT Entries
#define GDT_ACCESS_NULL 0x00	    // Null-Segment
#define GDT_ACCESS_KERNEL_CODE 0x9A // Code-Segment Kernel (Ring 0), X, R, P
#define GDT_ACCESS_KERNEL_DATA 0x92 // Data-Segment Kernel (Ring 0), R/W, P
#define GDT_ACCESS_USER_CODE 0xFA   // Code-Segment User (Ring 3), X, R, P
#define GDT_ACCESS_USER_DATA 0xF2   // Data-Segment User (Ring 3), R/W, P
#define GDT_ACCESS_TSS 0x89	    // TSS-Segment (32-Bit TSS, Ring 0, P)

// Segment Offsets GDT
#define GDT_NULL_SEGMENT 0x00	     // Null-Segment Offset
#define GDT_KERNEL_CODE_SEGMENT 0x08 // Offset Kernel-Code-Segment
#define GDT_KERNEL_DATA_SEGMENT 0x10 // Offset Kernel-Data-Segment
#define GDT_USER_CODE_SEGMENT 0x18   // Offset User-Code-Segment
#define GDT_USER_DATA_SEGMENT 0x20   // Offset User-Data-Segment
#define GDT_TSS_SEGMENT 0x28	     // Offset TSS-Segment

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

/* INTERNAL API */
gdt_entry_t gdt_entries[6];
gdt_t gdt;

void gdt_init(void)
{
	gdt.limit = (sizeof(gdt_entry_t) * 6) - 1;
	gdt.base = (uint32_t)&gdt_entries;
	gdt_set_entry(0, 0, 0, GDT_ACCESS_NULL, 0);
	gdt_set_entry(1, 0, 0xFFFFFFFF, GDT_ACCESS_KERNEL_CODE, 0xCF);
	gdt_set_entry(2, 0, 0xFFFFFFFF, GDT_ACCESS_KERNEL_DATA, 0xCF);
	gdt_set_entry(3, 0, 0xFFFFFFFF, GDT_ACCESS_USER_CODE, 0xCF);
	gdt_set_entry(4, 0, 0xFFFFFFFF, GDT_ACCESS_USER_DATA, 0xCF);
	gdt_set_entry(5, (uint32_t)&tss, sizeof(tss_t) - 1, GDT_ACCESS_TSS, 0x00);
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