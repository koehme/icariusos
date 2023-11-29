/**
 * @file idt.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct IDTDescriptor
{
    uint16_t isr_low;   // The lower 16 bits of the ISR's address
    uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t reserved;   // Set to zero
    uint8_t attributes; // Type and attributes
    uint16_t isr_high;  // The higher 16 bits of the ISR's address
} __attribute__((packed)) IDTDescriptor;

typedef struct IDT_R
{
    uint16_t limit; // Specifying the size of the interrupt descriptor table - 1
    uint32_t base;  // Pointing to the start address of the interrupt descriptor table
} __attribute__((packed)) IDT_R;

void idt_init(void);

#endif