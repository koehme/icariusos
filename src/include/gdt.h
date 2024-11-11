#ifndef GDT_H
#define GDT_H

typedef struct GdtEntry {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} GdtEntry __attribute__((packed));

typedef struct Gdt {
	uint16_t limit;
	unsigned int base;
} Gdt __attribute__((packed));

void gdt_init();
void gdt_set_entry(uint32_t num, uint32_t base, uint32_t limit, uint8_t access, uint32_t flags);

#endif