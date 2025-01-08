/**
 * @file gdt.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define NULL_SEG 0x00
#define KERNEL_CODE_SEG 0x9A
#define KERNEL_DATA_SEG 0x92
#define USER_CODE_SEG 0xFA
#define USER_DATA_SEG 0xF2
#define TSS_SEG 0x89

void gdt_init(void);
void gdt_set_entry(const uint32_t num, const uint32_t base, const uint32_t limit, const uint8_t access, const uint32_t flags);

#endif