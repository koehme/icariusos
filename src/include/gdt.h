/**
 * @file gdt.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct gdt_entry_t {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_t {
	uint16_t limit;
	unsigned int base;
} __attribute__((packed)) gdt_t;

void gdt_init(void);
void gdt_set_entry(const uint32_t num, const uint32_t base, const uint32_t limit, const uint8_t access, const uint32_t flags);

#endif