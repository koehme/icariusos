/**
 * @file idt.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "idt.h"
#include "mem.h"
#include "kernel.h"

static IDTDescriptor idt[256];
static IDT_R idtr_descriptor;

extern void idt_loader(IDT_R *ptr);

void isr_division_by_zero(void)
{
    kprint("Interrupt INT 00h - Divide by 0\n");
    return;
};

void idt_set(const int vector, void *isr)
{
    if (vector < 0 || vector >= 256)
    {
        return;
    };
    IDTDescriptor *descriptor = &idt[vector];

    descriptor->isr_low = (uint32_t)isr & 0xffff;
    descriptor->kernel_cs = 0x08;
    descriptor->reserved = 0x00;
    descriptor->attributes = 0b11101110;
    descriptor->isr_high = (uint32_t)isr >> 16;
};

void idt_init(void)
{
    idtr_descriptor.limit = (uint16_t)sizeof(IDTDescriptor) * 256 - 1;
    idtr_descriptor.base = (uintptr_t)&idt[0];
    idt_set(0, isr_division_by_zero);

    // Pass the idt address in idtr_descriptor.base to assembly
    idt_loader(&idtr_descriptor);
    return;
};