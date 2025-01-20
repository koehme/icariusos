/**
 * @file idt.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stdint.h>

#include "ata.h"
#include "fifo.h"
#include "idt.h"
#include "io.h"
#include "kernel.h"
#include "keyboard.h"
#include "mouse.h"
#include "ps2.h"
#include "string.h"

/* EXTERNAL API */
extern ata_t ata_dev;
extern timer_t timer;
extern kbd_t kbd;
extern mouse_t mouse;
extern fifo_t fifo_kbd;
extern fifo_t fifo_mouse;

extern void asm_idt_loader(idtr_t* ptr);
extern void asm_interrupt_default();
extern void asm_interrupt_14h(void);

/* PUBLIC API */
void idt_init(void);
void idt_set(const int32_t isr_num, void* isr);
void isr_14h_handler(void);
void isr_20h_handler(void);
void isr_21h_handler(void);
void isr_32h_handler(void);
void isr_default_handler(void);

/* INTERNAL API */
static void _pic1_send_eoi(void);
static void _pic2_send_eoi(void);
static void _init_isr(void);

static const char* interrupt_messages[] = {
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

static idt_desc_t idt[256];
static idtr_t idtr_descriptor;

static void _pic1_send_eoi(void)
{
	outb(PIC_1_CTRL, PIC_ACK);
	return;
};

static void _pic2_send_eoi(void)
{
	outb(PIC_1_CTRL, PIC_ACK);
	outb(PIC_2_CTRL, PIC_ACK);
	return;
};

void isr_14h_handler(void)
{
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r"(faulting_address));
	printf("[WARNING] Unmapped Address Access Attempt Detected at: 0x%x\n", (uint32_t)faulting_address);

	for (;;)
		;
	return;
};

void isr_20h_handler(void)
{
	// printf("%d\n", timer.ticks);
	timer.ticks++;
	_pic1_send_eoi();
	return;
};

void isr_21h_handler(void)
{
	if (ps2_wait(PS2_BUFFER_OUTPUT) == 0) {
		const uint8_t data = inb(PS2_DATA_PORT);
		fifo_enqueue(&fifo_kbd, data);
	};
	_pic1_send_eoi();
	return;
};

void isr_32h_handler(void)
{
	if (ps2_wait(PS2_BUFFER_OUTPUT) == 0) {
		const uint8_t data = inb(PS2_DATA_PORT);
		fifo_enqueue(&fifo_mouse, data);
	};
	_pic2_send_eoi();
	return;
};

void isr_default_handler(void)
{
	_pic1_send_eoi();
	return;
};

static void _init_isr(void)
{
	for (size_t i = 0; i < 256; i++) {
		idt_set(i, asm_interrupt_default);
	};
	return;
};

void idt_init(void)
{
	idtr_descriptor.limit = (uint16_t)sizeof(idt_desc_t) * 256 - 1;
	idtr_descriptor.base = (uintptr_t)&idt[0];
	_init_isr();
	idt_set(14, asm_interrupt_14h);
	asm_idt_loader(&idtr_descriptor);
	return;
};

void idt_set(const int32_t isr_num, void* isr)
{
	if (isr_num < 0 || isr_num >= 256) {
		return;
	};
	idt_desc_t* descriptor = &idt[isr_num];
	descriptor->isr_low = (uintptr_t)isr & 0xffff;
	descriptor->kernel_cs = 0x08;
	descriptor->reserved = 0x00;
	descriptor->attributes = 0b11101110;
	descriptor->isr_high = ((uintptr_t)isr >> 16) & 0xffff;
	return;
};
