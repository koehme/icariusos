/**
 * @file gdt.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

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

void gdt_init(void);
void gdt_set_entry(const uint32_t num, const uint32_t base, const uint32_t limit, const uint8_t access, const uint32_t flags);

#endif