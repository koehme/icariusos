/**
 * @file gdt.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

void gdt_init(void);
void gdt_set_entry(const uint32_t num, const uint32_t base, const uint32_t limit, const uint8_t access, const uint32_t flags);

#endif