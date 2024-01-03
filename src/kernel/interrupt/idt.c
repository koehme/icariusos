/**
 * @file idt.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stdint.h>

#include "keyboard.h"
#include "idt.h"
#include "string.h"
#include "io.h"
#include "timer.h"
#include "stream.h"
#include "icarius.h"
#include "ata.h"

extern ATADisk ata_disk_a;
extern Timer timer;

extern void asm_irq_14h(void);
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

void pic_send_eoi(void)
{
    asm_outb(PIC_1_CTRL, PIC_ACK);
    return;
};

/**
 * @brief IRQ handler for ATA Disk Read completion (IRQ 14).
 * Handles the interrupt generated when an ATA Disk Read operation
 * is completed. It prints a message indicating the successful transfer of 512 bytes
 * from the ATA Disk to the Buffer and acknowledges the interrupt to the PIC (Programmable Interrupt Controller).
 * @details
 * Begins by printing an interrupt message in light magenta color.
 * It then initiates the data transfer from the ATA Disk to the Buffer in an interrupt-driven manner.
 * After the transfer is completed, a success message is printed in light green color,
 * and the contents of the ATA Buffer are printed using `ata_print_buffer`.
 * Finally, the interrupt is acknowledged to the PIC.
 */
void irq_14h_handler(void)
{
    ATADisk *ptr_ata_disk = &ata_disk_a;
    ptr_ata_disk->buffer_state = BUFFER_BUSY;

    volatile uint16_t *ptr_ata_buffer = (volatile uint16_t *)ptr_ata_disk->buffer;
    const char *message = interrupt_messages[46];
    kprintf(message);

    for (size_t i = 0; i < ptr_ata_disk->sector_size / 2; ++i)
    {
        // Copy from disk into buffer
        *ptr_ata_buffer = asm_inw(ATA_DATA_PORT);
        ptr_ata_buffer++;
    };
    ptr_ata_disk->buffer_state = BUFFER_READY;
    kprintf("ATA Disk Read Successful: 512 Bytes transferred into ATA Buffer\n");
    // ata_print_buffer(ptr_ata_disk);
    pic_send_eoi();
    return;
};

/**
 * @brief Timer ISR handler.
 * @return void
 */
void isr_20h_handler(void)
{
    // kprintf("%d\n", timer.ticks);
    timer.ticks++;
    pic_send_eoi();
    return;
};

/**
 * @brief Keyboard ISR handler.
 * @return void
 */
void isr_21h_handler(void)
{
    if (keyboard_controller_is_ready())
    {
        keyboard_read();
    };
    pic_send_eoi();
    return;
};

/**
 * @brief Default ISR handler.
 * @return void
 */
void isr_default_handler(void)
{
    pic_send_eoi();
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
    return;
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
    // Assign specific handlers for interrupts
    idt_set(0x2e, asm_irq_14h);
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