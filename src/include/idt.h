/**
 * @file idt.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#include "icarius.h"

extern void asm_do_nop(void);
extern void asm_do_sti(void);
extern void asm_do_cli(void);

typedef struct idt_desc {
	uint16_t isr_low;   // The lower 16 bits of the ISR's address
	uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t reserved;   // Set to zero
	uint8_t attributes; // Type and attributes
	uint16_t isr_high;  // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_desc_t;

typedef struct idtr {
	uint16_t limit; // Specifying the size of the interrupt descriptor table - 1
	uint32_t base;	// Pointing to the start address of the interrupt descriptor table
} __attribute__((packed)) idtr_t;

typedef struct interrupt_frame {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t reserved;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;
} __attribute__((packed)) interrupt_frame_t;

void idt_init(void);
void idt_set(const int32_t isr_num, void* isr, const uint8_t attributes);
void idt_dump_interrupt_frame(const interrupt_frame_t* regs);

void isr_0_handler(const uint32_t isr_num, interrupt_frame_t* regs);
void isr_1_handler(const uint32_t isr_num, interrupt_frame_t* regs);
void isr_2_handler(const uint32_t isr_num, interrupt_frame_t* regs);
void isr_13_handler(const uint32_t error_code, interrupt_frame_t* regs);
void isr_14_handler(uint32_t fault_addr, uint32_t error_code, interrupt_frame_t* regs);
void irq0_handler(interrupt_frame_t* regs);
void irq1_handler(interrupt_frame_t* regs);
void irq12_handler(void);
void isr_default_handler(interrupt_frame_t* regs);

#endif