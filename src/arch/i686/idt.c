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

typedef struct interrupt_frame interrupt_frame_t;

extern void asm_idt_loader(idtr_t* ptr);
extern void asm_interrupt_default(void);
extern void asm_isr0_divide_exception(void);
extern void asm_isr1_debug_exception(void);
extern void asm_isr14_page_fault(void);

/* PUBLIC API */
void idt_init(void);
void idt_set(const int32_t isr_num, void* isr);
void isr_0h_handler(interrupt_frame_t* regs);
void isr_1h_handler(interrupt_frame_t* regs);
void isr_14h_handler(void);
void irq0_handler(void);
void irq1_handler(void);
void irq12_handler(void);
void isr_default_handler(void);

/* INTERNAL API */
static void _pic1_send_eoi(void);
static void _pic2_send_eoi(void);
static void _init_isr(void);

typedef struct interrupt_frame {
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
} interrupt_frame_t;

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
	outb(PIC_2_CTRL, PIC_ACK);
	outb(PIC_1_CTRL, PIC_ACK);
	return;
};

static void _dump_register(const interrupt_frame_t* regs)
{
	printf("\n");
	printf("==========================\n");
	printf("=   Register Dump		  \n");
	printf("==========================\n");
	printf("EAX:  0x%x\n", regs->eax);
	printf("ECX:  0x%x\n", regs->ecx);
	printf("EDX:  0x%x\n", regs->edx);
	printf("EBX:  0x%x\n", regs->ebx);
	printf("ESP:  0x%x\n", regs->esp);
	printf("EBP:  0x%x\n", regs->ebp);
	printf("ESI:  0x%x\n", regs->esi);
	printf("EDI:  0x%x\n", regs->edi);
	printf("DS:   0x%x\n", regs->ds);
	printf("ES:   0x%x\n", regs->es);
	printf("FS:   0x%x\n", regs->fs);
	printf("GS:   0x%x\n", regs->gs);
	printf("EIP:  0x%x\n", regs->eip);
	printf("CS:   0x%x\n", regs->cs);
	printf("EFLAGS: 0x%x\n", regs->eflags);

	printf("==========================\n");
	printf("=   Stack Dump			  \n");
	printf("==========================\n");
	const uint32_t* stack = (uint32_t*)regs->esp;

	for (int32_t i = 3; i >= 1; i--) {
		const void* addr = stack + i;
		const uint32_t offset = i * 4;

		printf("\n--- [ESP + %d] ---\n", offset);
		printf(" Address  : 0x%x\n", addr);
		printf(" Value    : 0x%x\n", stack[i]);
		printf("-------------------------\n");
	};
	for (int32_t i = 0; i < 3; i++) {
		const void* addr = stack - i;
		const uint32_t offset = i * 4;

		printf("\n--- [ESP - %d] ---\n", offset);
		printf(" Address  : 0x%x\n", addr);
		printf(" Value    : 0x%x\n", stack[-i]);
		printf("-------------------------\n");
	};
	return;
};

void isr_0h_handler(interrupt_frame_t* regs)
{
	_dump_register(regs);
	panic(interrupt_messages[0]);
	return;
};

void isr_1h_handler(interrupt_frame_t* regs)
{
	_dump_register(regs);
	uint32_t dr6;
	asm volatile("mov %%dr6, %0" : "=r"(dr6));

	if (dr6 & (1 << 0))
		printf(" - Breakpoint 0 (DR0)\n");
	if (dr6 & (1 << 1))
		printf(" - Breakpoint 1 (DR1)\n");
	if (dr6 & (1 << 2))
		printf(" - Breakpoint 2 (DR2)\n");
	if (dr6 & (1 << 3))
		printf(" - Breakpoint 3 (DR3)\n");
	if (dr6 & (1 << 14))
		printf(" - Task-Switch Debug Exception\n");
	asm volatile("mov %0, %%dr6" : : "r"(0x0));
	printf("%s\n", interrupt_messages[1]);
	return;
};

void isr_14h_handler(void)
{
	uint32_t error_code;
	asm volatile("mov %%eax, %0" : "=r"(error_code) : : "eax");
	const int32_t present = error_code & 0b1;		  // Page Present
	const int32_t write = error_code & 0b10;		  // Write Access
	const int32_t user_mode = error_code & 0b100;		  // User-/Supervisor-Modus
	const int32_t reserved = error_code & 0b1000;		  // Reserved Bits Overwritten
	const int32_t instruction_fetch = error_code & 0b10000;	  // Instruction Fetch
	const int32_t pk_flag = error_code & 0b100000;		  // PK flag (bit 5)
	const int32_t sgx_flag = error_code & 0b1000000000000000; // SGX flag (bit 15)
	printf("[INFO] Page Fault occured\n");

	if (!present) {
		printf(" - Page Not Present\n");
	} else {
		printf(" - Page Present\n");
	};

	if (write) {
		printf(" - Write Access\n");
	} else {
		printf(" - Read Access\n");
	};

	if (user_mode) {
		printf(" - User Mode\n");
	} else {
		printf(" - Supervisor Mode\n");
	};

	if (reserved) {
		printf(" - Reserved Bits\n");
	};

	if (instruction_fetch) {
		printf(" - Instruction Fetch\n");
	};

	if (pk_flag) {
		printf(" - Protection Key Violation (PK flag)\n");
	};

	if (sgx_flag) {
		printf(" - SGX-Specific Access Control Violation (SGX flag)\n");
	};
	uint32_t fault_addr;
	asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
	printf("[WARNING] Unmapped Address Access Attempt Detected at: 0x%x\n", (uint32_t)fault_addr);
	// Align fault address to 4 MiB boundary by clearing lower 22 bits
	const uint32_t aligned_fault_addr = fault_addr & 0xFFC00000;
	const uint64_t phys_addr = pfa_alloc();

	if (!phys_addr) {
		panic("[CRITICAL] System Halted due to Unrecoverable Page Fault!");
		return;
	};
	uint32_t page_flags = PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE;

	if (user_mode) {
		page_flags |= PAGE_USER;
	} else {
		page_flags &= ~PAGE_USER;
	};
	page_map(aligned_fault_addr, phys_addr, page_flags);
	printf("[INFO] Mapped Virtual Address 0x%x to Physical Address 0x%x\n", aligned_fault_addr, phys_addr);
	return;
};

void irq0_handler(void)
{
	// printf("%d\n", timer.ticks);
	timer.ticks++;
	_pic1_send_eoi();
	return;
};

void irq1_handler(void)
{
	if (ps2_wait(PS2_BUFFER_OUTPUT) == 0) {
		const uint8_t data = inb(PS2_DATA_PORT);
		fifo_enqueue(&fifo_kbd, data);
	};
	_pic1_send_eoi();
	return;
};

void irq12_handler(void)
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
	idt_set(0, asm_isr0_divide_exception);
	idt_set(1, asm_isr1_debug_exception);
	idt_set(14, asm_isr14_page_fault);
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
	descriptor->attributes = 0b10001110;
	descriptor->isr_high = ((uintptr_t)isr >> 16) & 0xffff;
	return;
};
