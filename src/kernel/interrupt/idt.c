/**
 * @file idt.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "idt.h"
#include "string.h"
#include "io.h"
#include "icarius.h"

extern void asm_interrupt_20h(void);
extern void asm_interrupt_21h(void);
extern void asm_idt_loader(IDT_R *ptr);
extern void asm_interrupt_default();

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
    "Secondary ATA Hard Disk (IRQ15)\n"};

/**
 * Represents the IDT and is used to define various interrupts.
 * Each element in this array corresponds to an interrupt vector,
 * holding specific information such as the address of the Interrupt Service Routine (ISR) handler.
 */
static IDTDescriptor idt[256];
/**
 * Represents the IDTR register value.
 * The IDTR contains information about the size and starting point of the IDT.
 * Defining an IDTR allows efficient initialization and loading of the IDT.
 */
static IDT_R idtr_descriptor;

static char counter = 'A'; // Initial character

/**
 * @brief Timer ISR handler.
 * @return void
 */
void isr_20h_handler(void)
{
    // Temporary test routine for VGA scrolling - to be deleted
    kprint(&counter);
    counter++;

    if (counter > 'Z')
    {
        counter = 'A';
    };
    asm_outb(PIC_1_CTRL, PIC_ACK);
    return;
};

/**
 * @brief Keyboard ISR handler.
 * @return void
 */
void isr_21h_handler(void)
{
    const char *message = interrupt_messages[33];
    kprint(message);
    asm_outb(PIC_1_CTRL, PIC_ACK);
    return;
};

/**
 * @brief Default ISR handler.
 * @return void
 */
void isr_default_handler(void)
{
    asm_outb(PIC_1_CTRL, PIC_ACK);
    return;
};

/**
 * @brief Sets up an entry in the Interrupt Descriptor Table (IDT).
 * Configures the specified entry in the IDT with the given vector and ISR (Interrupt Service Routine).
 * @param interrupt_n The interrupt number.
 * @param isr A pointer to the ISR (Interrupt Service Routine) function.
 * @return void
 */
void idt_set(const int interrupt_n, void *isr)
{
    if (interrupt_n < 0 || interrupt_n >= 256)
    {
        return;
    };
    IDTDescriptor *descriptor = &idt[interrupt_n];

    descriptor->isr_low = (uintptr_t)isr & 0xffff;
    descriptor->kernel_cs = 0x08;
    descriptor->reserved = 0x00;
    descriptor->attributes = 0b11101110;
    descriptor->isr_high = ((uintptr_t)isr >> 16) & 0xffff;
};

/**
 * @brief Initializes the IDT entries with interrupt handlers.
 * @return void
 */
void idt_entries_init(void)
{
    // Set default interrupt handlers for all entries in the IDT
    for (int interrupt_n = 0; interrupt_n < 256; interrupt_n++)
    {
        idt_set(interrupt_n, asm_interrupt_default);
    };
    // Assign specific handlers for interrupts 0x20 and 0x21
    idt_set(0x20, asm_interrupt_20h);
    idt_set(0x21, asm_interrupt_21h);
    return;
};

/**
 * @brief Initializes the Interrupt Descriptor Table (IDT).
 * Sets the limit and base address of the IDT descriptor and
 * initializes the IDT entries using the separate function initialize_idt_entries.
 * Finally, it passes the IDT address to the assembly routine for loading.
 * @return void
 */
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