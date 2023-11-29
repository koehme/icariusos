/**
 * @file idt.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "idt.h"
#include "mem.h"
#include "icarius.h"

extern void idt_loader(IDT_R *ptr);

static IDTDescriptor idt[256];
static IDT_R idtr_descriptor;

static const char interrupt_messages[][256] = {
    "Division by Zero (INT 0)\n",
    "Debug Exception (INT 1)\n",
    "Non-Maskable Interrupt (NMI - INT 2)\n",
    "Breakpoint Exception (INT 3)\n",
    "Overflow (INT 4)\n",
    "BOUND Range Exceeded (INT 5)\n",
    "Invalid Opcode (INT 6)\n",
    "Device Not Available (INT 7)\n",
    "Double Fault (INT 8)\n",
    "Coprocessor Segment Overrun (INT 9)\n",
    "Invalid TSS (INT 10)\n",
    "Segment Not Present (INT 11)\n",
    "Stack-Segment Fault (INT 12)\n",
    "General Protection Fault (INT 13)\n",
    "Page Fault (INT 14)\n",
    "Reserved (INT 15)\n",
    "x87 Floating-Point Exception (INT 16)\n",
    "Alignment Check (INT 17)\n",
    "Machine Check (INT 18)\n",
    "SIMD Floating-Point Exception (INT 19)\n",
    "Virtualization Exception (INT 20)\n",
    "Control Protection Exception (INT 21)\n",
    "Reserved (INT 22)\n",
    "Reserved (INT 23)\n",
    "Reserved (INT 24)\n",
    "Reserved (INT 25)\n",
    "Reserved (INT 26)\n",
    "Reserved (INT 27)\n",
    "Reserved (INT 28)\n",
    "Reserved (INT 29)\n",
    "Security Exception (INT 30)\n",
    "Reserved (INT 31)\n",
};

/**
 * @brief Interrupt Service Routine (ISR) for the "Division by Zero"
 */
void isr0(void)
{
    const char *message = interrupt_messages[0];
    kprint(message);
    return;
};

/**
 * @brief Sets up an entry in the Interrupt Descriptor Table (IDT).
 * Configures the specified entry in the IDT with the given vector and ISR (Interrupt Service Routine).
 * @param vector The interrupt number.
 * @param isr A pointer to the ISR (Interrupt Service Routine) function.
 * @return void
 */
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
    descriptor->isr_high = ((uint32_t)isr >> 16) & 0xffff;
};

/**
 * @brief Initializes the Interrupt Descriptor Table Register (IDTR).
 * Set up the IDTR descriptor and initializes the first entry
 * It then passes the IDT address to the assembly routine for further loading.
 * @return void
 */
void idt_init(void)
{
    // Set the limit and base address of the IDT descriptor
    idtr_descriptor.limit = (uint16_t)sizeof(IDTDescriptor) * 256 - 1;
    idtr_descriptor.base = (uintptr_t)&idt[0];
    // Initialize the first IDT entry for the "division by zero" interrupt
    idt_set(0, isr0);
    // Pass the IDT address in idtr_descriptor.base to the assembly routine
    idt_loader(&idtr_descriptor);
    return;
};