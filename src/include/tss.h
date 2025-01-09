/**
 * @file tss.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TSS_H
#define TSS_H

#include <stdint.h>

/*
################################################
## Stack Memory Layout for Higher-Half Kernel ##
################################################

| **Region**          | **Start Stack Address** | **End Stack Address**   | **Size**        | **Description**
--------------------------------------------------------------------------------------------------------------------------------------
| Kernel Stack        | `0xC2C00000`            | `0xC2C07FFF`            | 32 KiB          | Stack used by TSS for Ring 3 -> Ring 0
| Reserved Stack Space| `0xC2C08000`            | `0xC2FFFFFF`            | 4064 KiB        | Reserved
--------------------------------------------------------------------------------------------------------------------------------------
*/

#define KERNEL_STACK_SIZE 0x8000				       // 32 KiB
#define KERNEL_STACK_BOTTOM (KERNEL_HEAP_MAX + 1)		       // 0xC2C00000
#define KERNEL_STACK_TOP (KERNEL_STACK_BOTTOM + KERNEL_STACK_SIZE - 1) // 0xC2C07FFF
#define RESERVED_STACK_START (KERNEL_STACK_TOP + 1)		       // 0xC2C08000
#define RESERVED_STACK_END 0xC2FFFFFF				       // Full 4 MiB Page Ends

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