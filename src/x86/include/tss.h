/**
 * @file tss.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TSS_H
#define TSS_H

#include <stdint.h>

#include "icarius.h"

typedef struct tss {
	uint32_t link;	 // Previous TSS link (not used, set to 0)
	uint32_t esp0;	 // Kernel stack pointer (used during ring 3 -> ring 0 transitions)
	uint32_t ss0;	 // Kernel stack segment
	uint32_t esp1;	 // Not used (set to 0)
	uint32_t ss1;	 // Not used (set to 0)
	uint32_t esp2;	 // Not used (set to 0)
	uint32_t ss2;	 // Not used (set to 0)
	uint32_t cr3;	 // Page directory base register (CR3)
	uint32_t eip;	 // Instruction pointer
	uint32_t eflags; // Flags register
	uint32_t eax;	 // General-purpose register
	uint32_t ecx;	 // General-purpose register
	uint32_t edx;	 // General-purpose register
	uint32_t ebx;	 // General-purpose register
	uint32_t esp;	 // Stack pointer
	uint32_t ebp;	 // Base pointer
	uint32_t esi;	 // Source index register
	uint32_t edi;	 // Destination index register
	uint32_t es;	 // Segment register
	uint32_t cs;	 // Code segment
	uint32_t ss;	 // Stack segment
	uint32_t ds;	 // Data segment
	uint32_t fs;	 // Additional segment register
	uint32_t gs;	 // Additional segment register
	uint32_t ldtr;	 // Local Descriptor Table Register
	uint32_t iopb;	 // I/O Map Base Address (offset into the I/O permission bitmap)
} __attribute__((packed)) tss_t;

void tss_init(tss_t* self, uint32_t esp0, uint16_t ss0);
void tss_load(uint16_t tss_selector);

#endif