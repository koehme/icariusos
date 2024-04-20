/**
 * @file idt.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stdint.h>

#include "keyboard.h"
#include "mouse.h"
#include "idt.h"
#include "string.h"
#include "io.h"
#include "kernel.h"
#include "ata.h"

extern ATADev ata_dev_primary_master;
extern Timer timer;
extern Keyboard keyboard;
extern Mouse mouse;

extern void asm_idt_loader(IDT_R *ptr);
extern void asm_interrupt_default();

static void pic_send_eoi(void);
void isr_20h_handler(void);
void isr_21h_handler(void);
void isr_32h_handler(void);
void isr_default_handler(void);
static void idt_entries_init(void);

void idt_init(void);
void idt_set(const int32_t isr_num, void *isr);

static const char *interrupt_messages[] = {
    // CPU Interrupts
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
    // Reserved Interrupts
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
    // IRQ Interrupts
    "Timer (IRQ0)\n",
    "Keyboard (IRQ1)\n",
    "Cascade (IRQ2)\n",
    "COM2 (IRQ3)\n",
    "COM1 (IRQ4)\n",
    "LPT2 (IRQ5)\n",
    "Floppy (IRQ6)\n",
    "LPT1 (IRQ7)\n",
    "CMOS RTC (IRQ8)\n",
    "Free (IRQ9)\n",
    "Free (IRQ10)\n",
    "Free (IRQ11)\n",
    "PS2 Mouse (IRQ12)\n",
    "FPU / Coprocessor / Inter-Processor (IRQ13)\n",
    "Primary ATA Hard Disk (IRQ14)\n",
    "Secondary ATA Hard Disk (IRQ15)\n",
};

// Contains interrupt service routines that are required for handling interrupts
static IDTDescriptor idt[256];
// The location of the IDT is kept in the IDT_R (IDT register). This is loaded using the LIDT assembly instruction, whose argument is a pointer to an IDTDescriptor structure
static IDT_R idtr_descriptor;

// Sends an end-of-interrupt (EOI) signal to the PIC
static void pic_send_eoi(void)
{
    asm_outb(PIC_1_CTRL, PIC_ACK);
    return;
};

// IDE ATA dev
void irq_14h_handler(void)
{
    pic_send_eoi();
    return;
};

// It increments the tick count of the system timer and sends an end-of-interrupt (EOI) signal to the PIC
void isr_20h_handler(void)
{
    // kprtf("%d\n", timer.ticks);
    timer.ticks++;
    pic_send_eoi();
    return;
};

// Checks if a key is ready to be read from the keyboard, reads it if available and sends an end-of-interrupt (EOI) signal to the PIC
void isr_21h_handler(void)
{
    if (keyboard_wait())
    {
        keyboard_read(&keyboard);
    };
    pic_send_eoi();
    return;
};

// PS2 Mouse handler
void isr_32h_handler(void)
{
    mouse_handler(&mouse);
    kprtf("Mouse (rel_x: %d, rel_y: %d)\n", mouse.rel_x, mouse.rel_y);
    pic_send_eoi();
    return;
};

// Default ISR handler
void isr_default_handler(void)
{
    pic_send_eoi();
    return;
};

// Initializes the Interrupt Descriptor Table with default dummy interrupt handlers
static void idt_entries_init(void)
{
    // Set default interrupt handlers for all entries in the IDT
    for (size_t i = 0; i < 256; i++)
    {
        idt_set(i, asm_interrupt_default);
    };
    return;
};

// Initializes the Interrupt Descriptor Table (IDT).
void idt_init(void)
{
    // Set the limit and base address of the IDT descriptor
    idtr_descriptor.limit = (uint16_t)sizeof(IDTDescriptor) * 256 - 1;
    idtr_descriptor.base = (uintptr_t)&idt[0];
    idt_entries_init();
    // Load the idt in a specific 'lidt' register with the help of an assembly routine
    asm_idt_loader(&idtr_descriptor);
    return;
};

// Sets an entry in the Interrupt Descriptor Table with the provided interrupt service routine (ISR) function pointer
void idt_set(const int32_t isr_num, void *isr)
{
    if (isr_num < 0 || isr_num >= 256)
    {
        return;
    };
    IDTDescriptor *descriptor = &idt[isr_num];

    descriptor->isr_low = (uintptr_t)isr & 0xffff;
    descriptor->kernel_cs = 0x08;
    descriptor->reserved = 0x00;
    descriptor->attributes = 0b11101110;
    descriptor->isr_high = ((uintptr_t)isr >> 16) & 0xffff;
    return;
};
