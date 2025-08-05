/**
 * @file idt.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdbool.h>
#include <stdint.h>

#include "ata.h"
#include "fifo.h"
#include "icarius.h"
#include "idt.h"
#include "io.h"
#include "kernel.h"
#include "keyboard.h"
#include "mouse.h"
#include "ps2.h"
#include "rr.h"
#include "scheduler.h"
#include "string.h"
#include "wq.h"

/* EXTERNAL API */
extern ata_t ata_dev;
extern timer_t timer;
extern kbd_t kbd;
extern mouse_t mouse;
extern fifo_t fifo_kbd;
extern fifo_t fifo_mouse;
extern uint32_t kernel_directory[1024];

extern void asm_idt_loader(idtr_t* ptr);
extern void asm_interrupt_default(void);
extern void asm_syscall(void);
extern void asm_isr0_wrapper(void);
extern void asm_isr1_wrapper(void);
extern void asm_isr2_wrapper(void);
extern void asm_isr6_wrapper(void);
extern void asm_isr8_wrapper(void);
extern void asm_isr12_wrapper(void);
extern void asm_isr13_wrapper(void);
extern void asm_isr14_wrapper(void);

/* PUBLIC API */
void idt_init(void);
void idt_set(const int32_t isr_num, void* isr, const uint8_t attributes);
void idt_dump_interrupt_frame(const interrupt_frame_t* frame);
void isr_1_handler(const uint32_t isr_num, interrupt_frame_t* frame);
void isr_2_handler(const uint32_t isr_num, interrupt_frame_t* frame);
void isr_6_handler(const uint32_t isr_num, interrupt_frame_t* frame);
void isr_8_handler(const uint32_t error_code, interrupt_frame_t* frame);
void isr_12_handler(const uint32_t error_code, interrupt_frame_t* frame);
void isr_13_handler(const uint32_t error_code, interrupt_frame_t* frame);
void isr_14_handler(const uint32_t fault_addr, const uint32_t error_code, interrupt_frame_t* frame);

void irq0_handler(interrupt_frame_t* frame);
void irq1_handler(interrupt_frame_t* frame);
void irq12_handler(void);
void isr_default_handler(interrupt_frame_t* frame);

/* INTERNAL API */
static void _pic1_send_eoi(void);
static void _pic2_send_eoi(void);
static void _init_isr(void);

/*
 * Interrupt Vector Table (IVT) Reference
 * Based on Intel 64 and IA-32 Architecture
 *
 * CPU Exceptions (0 - 31)
 * ---------------------------------------------
 * | INT  | Error Code? | Type     | Description                      |
 * |------|------------|----------|----------------------------------|
 * |  0   | No         | Fault    | Division by Zero (#DE)          |
 * |  1   | No         | Trap     | Debug Exception                 |
 * |  2   | No         | Interrupt| Non-Maskable Interrupt (NMI)    |
 * |  3   | No         | Trap     | Breakpoint Exception (INT3)     |
 * |  4   | No         | Trap     | Overflow Exception (#OF)        |
 * |  5   | No         | Fault    | BOUND Range Exceeded (#BR)      |
 * |  6   | No         | Fault    | Invalid Opcode (#UD)            |
 * |  7   | No         | Fault    | Device Not Available (#NM)      |
 * |  8   | Yes        | Abort    | Double Fault Exception          |
 * |  9   | No         | Fault    | Coprocessor Segment Overrun     |
 * | 10   | Yes        | Fault    | Invalid TSS Exception           |
 * | 11   | Yes        | Fault    | Segment Not Present (#NP)       |
 * | 12   | Yes        | Fault    | Stack-Segment Fault (#SS)       |
 * | 13   | Yes        | Fault    | General Protection Fault (#GP)  |
 * | 14   | Yes        | Fault    | Page Fault (#PF)                |
 * | 15   | No         | Reserved | (Reserved)                      |
 * | 16   | No         | Fault    | x87 FPU Floating Point Exception |
 * | 17   | Yes        | Fault    | Alignment Check Exception       |
 * | 18   | No         | Abort    | Machine Check Exception         |
 * | 19   | No         | Fault    | SIMD Floating-Point Exception   |
 * | 20   | No         | Fault    | Virtualization Exception        |
 * | 21   | No         | Fault    | Control Protection Exception (#CP) |
 * | 22-27| No         | Reserved | (Reserved)                      |
 * | 28   | No         | Reserved | (Reserved)                      |
 * | 29   | Yes        | Fault    | Security Exception              |
 * | 30   | No         | Reserved | (Reserved)                      |
 * | 31   | No         | Reserved | (Reserved)                      |
 * ---------------------------------------------
 *
 * Hardware Interrupts (IRQs, 32 - 47)
 * ---------------------------------------------------
 * | IRQ  | INT  | Description                         |
 * |------|------|-------------------------------------|
 * |  0   |  32  | Programmable Interval Timer (PIT)  |
 * |  1   |  33  | Keyboard (PS/2)                    |
 * |  2   |  34  | Cascade Interrupt (for PIC chaining) |
 * |  3   |  35  | Serial Port COM2                   |
 * |  4   |  36  | Serial Port COM1                   |
 * |  5   |  37  | LPT2 (Legacy, often unused)        |
 * |  6   |  38  | Floppy Disk Controller             |
 * |  7   |  39  | LPT1 / Spurious Interrupt          |
 * |  8   |  40  | CMOS Real-Time Clock (RTC)         |
 * |  9   |  41  | Free (often used for ACPI)         |
 * | 10   |  42  | Free (often used for SCSI/NICs)    |
 * | 11   |  43  | Free (often used for USB controllers) |
 * | 12   |  44  | PS/2 Mouse                         |
 * | 13   |  45  | FPU / Coprocessor (Legacy)         |
 * | 14   |  46  | Primary ATA Hard Disk Controller   |
 * | 15   |  47  | Secondary ATA Hard Disk Controller |
 * ---------------------------------------------------
 *
 * Legend:
 * - Fault    = Exception that may be recoverable (e.g., Page Fault)
 * - Trap     = Exception where execution continues after handling (e.g., Debug)
 * - Abort    = Critical failure, usually unrecoverable (e.g., Machine Check)
 * - Interrupt = Hardware-generated, typically from an external device (IRQ)
 */

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

void idt_dump_interrupt_frame(const interrupt_frame_t* frame)
{
	kprintf("\n");
	kprintf("==========================\n");
	kprintf("=   Register Dump		  \n");
	kprintf("==========================\n");
	kprintf("EAX     0x%x\n", frame->eax);
	kprintf("ECX     0x%x\n", frame->ecx);
	kprintf("EDX     0x%x\n", frame->edx);
	kprintf("EBX     0x%x\n", frame->ebx);
	kprintf("ESP     0x%x\n", frame->esp);
	kprintf("EBP     0x%x\n", frame->ebp);
	kprintf("ESI     0x%x\n", frame->esi);
	kprintf("EDI     0x%x\n", frame->edi);
	kprintf("EIP     0x%x\n", frame->eip);
	kprintf("CS      0x%x\n", frame->cs);
	kprintf("EFLAGS  0x%x\n", frame->eflags);
	return;
};

void isr_0_handler(const uint32_t isr_num, interrupt_frame_t* frame)
{
	kprintf("\n----------------------------------------------------\n");
	kprintf("[ERROR] Division by Zero (#DE) Exception\n");
	kprintf("----------------------------------------------------\n");
	switch (frame->cs) {
	case GDT_KERNEL_CODE_SEGMENT: {
		idt_dump_interrupt_frame(frame);
		panic(interrupt_messages[isr_num]);
		return;
	};
	case GDT_USER_CODE_SEGMENT: {
		kprintf(" - User Process triggered %s detected.", interrupt_messages[0]);
		break;
	};
	default: {
		kprintf(" - Unknown GDT Code Segment 0x%s detected.\n", frame->cs);
		panic(interrupt_messages[isr_num]);
		return;
	};
	};
	return;
};

void isr_1_handler(const uint32_t isr_num, interrupt_frame_t* frame)
{
	kprintf("\n----------------------------------------------------\n");
	kprintf("[ERROR] Debug Exception (#DB)\n");
	kprintf("----------------------------------------------------\n");
	switch (frame->cs) {
	case GDT_KERNEL_CODE_SEGMENT: {
		idt_dump_interrupt_frame(frame);
		panic(interrupt_messages[isr_num]);
		return;
	};
	case GDT_USER_CODE_SEGMENT: {
		break;
	};
	default: {
		kprintf(" - Unknown GDT Code Segment 0x%x detected.\n", frame->cs);
		panic(interrupt_messages[isr_num]);
		break;
	};
	};
	uint32_t dr6;
	asm volatile("mov %%dr6, %0" : "=r"(dr6));

	if (dr6 & (1 << 0)) {
		kprintf(" - Breakpoint 0 (DR0)\n");
	};
	if (dr6 & (1 << 1)) {
		kprintf(" - Breakpoint 1 (DR1)\n");
	};
	if (dr6 & (1 << 2)) {
		kprintf(" - Breakpoint 2 (DR2)\n");
	};
	if (dr6 & (1 << 3)) {
		kprintf(" - Breakpoint 3 (DR3)\n");
	};
	if (dr6 & (1 << 14)) {
		kprintf(" - Task-Switch Debug Exception\n");
	};
	asm volatile("mov %0, %%dr6" : : "r"(0x0));
	return;
};

void isr_2_handler(const uint32_t isr_num, interrupt_frame_t* frame)
{
	kprintf("\n----------------------------------------------------\n");
	kprintf("[ERROR] Non-Maskable Interrupt (NMI) Exception\n");
	kprintf("----------------------------------------------------\n");
	switch (frame->cs) {
	case GDT_KERNEL_CODE_SEGMENT: {
		idt_dump_interrupt_frame(frame);
		panic(interrupt_messages[isr_num]);
		return;
	};
	case GDT_USER_CODE_SEGMENT: {
		break;
	};
	default: {
		kprintf(" - Unknown GDT Code Segment 0x%x detected.\n", frame->cs);
		panic(interrupt_messages[isr_num]);
		break;
	};
	};
	const uint8_t status = inb(0x61);

	if (status & 0x80) {
		kprintf(" - Memory Parity Error!\n");
	};
	if (status & 0x40) {
		kprintf(" - I/O Channel Check Error!\n");
	};
	if (status & 0x80) {
		panic(" - Critical Memory Error - Halting System!\n");
	};
	kprintf("%s\n", interrupt_messages[isr_num]);
	return;
};

void isr_6_handler(const uint32_t isr_num, interrupt_frame_t* frame)
{
	kprintf("\n----------------------------------------------------\n");
	kprintf("[ERROR] Invalid Opcode (#UD) Exception\n");
	kprintf("----------------------------------------------------\n");

	switch (frame->cs) {
	case GDT_KERNEL_CODE_SEGMENT: {
		idt_dump_interrupt_frame(frame);
		panic(interrupt_messages[isr_num]);
		return;
	};
	case GDT_USER_CODE_SEGMENT: {
		kprintf(" - User Process triggered an invalid Opcode exception.\n");
		break;
	};
	default: {
		kprintf(" - Unknown GDT Code Segment 0x%x detected.\n", frame->cs);
		panic(interrupt_messages[isr_num]);
		return;
	};
	};
	return;
};

void isr_8_handler(const uint32_t error_code, interrupt_frame_t* frame)
{
	kprintf("\n----------------------------------------------------\n");
	kprintf("[ERROR] Double Fault (#DF) Exception\n");
	kprintf("----------------------------------------------------\n");
	kprintf(" - Error Code: 0x%x\n", error_code);
	kprintf(" - Faulting Address (EIP): 0x%x\n", frame->eip);
	kprintf(" - Code Segment (CS): 0x%x\n", frame->cs);
	idt_dump_interrupt_frame(frame);
	panic(interrupt_messages[0x8]);
	return;
};

void isr_12_handler(const uint32_t error_code, interrupt_frame_t* frame)
{
	kprintf("\n----------------------------------------------------\n");
	kprintf("[ERROR] Stack-Segment Fault (#SS) Exception\n");
	kprintf("----------------------------------------------------\n");
	kprintf(" - Error Code: 0x%x\n", error_code);
	kprintf(" - Faulting Address (EIP): 0x%x\n", frame->eip);
	kprintf(" - Code Segment (CS): 0x%x\n", frame->cs);

	if (error_code & 1) {
		kprintf(" - Protection Violation (Memory Access Violation)\n");
	} else {
		kprintf(" - Stack-Segment Not Present or Invalid Descriptor\n");
	};

	if (error_code & 2) {
		kprintf(" - Fault occurred in USER MODE (Ring 3)\n");
	} else {
		kprintf(" - Fault occurred in KERNEL MODE (Ring 0)\n");
	};
	idt_dump_interrupt_frame(frame);
	panic(interrupt_messages[0xC]);
	return;
};

void isr_13_handler(const uint32_t error_code, interrupt_frame_t* frame)
{
	kprintf("\n----------------------------------------------------\n");
	kprintf("[ERROR] General Protection Fault (#GP) Exception\n");
	kprintf("----------------------------------------------------\n");
	kprintf(" - Error Code: 0x%x\n", error_code);
	kprintf(" - Faulting Address (EIP): 0x%x\n", frame->eip);
	kprintf(" - Code Segment (CS): 0x%x\n", frame->cs);

	if (error_code & 1) {
		kprintf(" - Protection Violation (Memory Access Violation)\n");
	} else {
		kprintf(" - Segment Not Present or Invalid Descriptor\n");
	};

	if (error_code & 2) {
		kprintf(" - Fault occurred in USER MODE (Ring 3)\n");
	} else {
		kprintf(" - Fault occurred in KERNEL MODE (Ring 0)\n");
	};

	if (error_code & 4) {
		kprintf(" - LDT Segment Involved\n");
	} else {
		kprintf(" - GDT or IDT Segment Involved\n");
	};

	if (error_code & 8) {
		kprintf(" - Fault Caused by Instruction Fetch (CS Problem)\n");
	} else {
		kprintf(" - Fault Caused by Data Access (DS/SS Problem)\n");
	};
	const uint16_t selector_index = error_code >> 3;
	const uint8_t ext_bit = error_code & 1;
	const uint8_t idt_bit = (error_code >> 1) & 1;
	const uint8_t ti_bit = (error_code >> 2) & 1;
	kprintf("Faulty Segment Selector Index: %d (0x%x)\n", selector_index, selector_index);

	if (idt_bit) {
		kprintf(" - Fault caused by an IDT Entry (Interrupt Descriptor)\n");
	} else if (ti_bit) {
		kprintf(" - Faulty Segment is located in the LDT (Local Descriptor Table)\n");
	} else {
		kprintf(" - Faulty Segment is located in the GDT (Global Descriptor Table)\n");
	};

	if (ext_bit) {
		kprintf(" - Exception was caused by an External Event (e.g., CPU Violation)\n");
	} else {
		kprintf(" - Exception was caused by Software or invalid Segment\n");
	};
	kprintf("----------------------------------------------------\n");
	idt_dump_interrupt_frame(frame);

	// panic(interrupt_messages[0xD]);
	return;
};

void isr_14_handler(const uint32_t fault_addr, const uint32_t error_code, interrupt_frame_t* frame)
{
	kprintf("\n----------------------------------------------------\n");
	kprintf("[ERROR] Page Fault (#PF) Exception\n");
	kprintf("----------------------------------------------------\n");
	const int32_t present = error_code & PAGE_PRESENT;	  // Page Present
	const int32_t write = error_code & PAGE_WRITABLE;	  // Write Access
	const int32_t user_mode = error_code & PAGE_USER;	  // User-/Supervisor-Modus
	const int32_t reserved = error_code & PAGE_PWT;		  // Reserved Bits Overwritten
	const int32_t instruction_fetch = error_code & PAGE_PCD;  // Instruction Fetch
	const int32_t pk_flag = error_code & PAGE_ACCESSED;	  // PK flag (bit 5)
	const int32_t sgx_flag = error_code & 0b1000000000000000; // SGX flag (bit 15)
	kprintf("[INFO] Page Fault occured\n");

	if (!present) {
		kprintf(" - Page Not Present\n");
	} else {
		kprintf(" - Page Present\n");
	};

	if (write) {
		kprintf(" - Write Access\n");
	} else {
		kprintf(" - Read Access\n");
	};

	if (user_mode) {
		kprintf(" - User Mode\n");
	} else {
		kprintf(" - Supervisor Mode\n");
	};

	if (reserved) {
		kprintf(" - Reserved Bits\n");
	};

	if (instruction_fetch) {
		kprintf(" - Instruction Fetch\n");
	};

	if (pk_flag) {
		kprintf(" - Protection Key Violation (PK flag)\n");
	};

	if (sgx_flag) {
		kprintf(" - SGX-Specific Access Control Violation (SGX flag)\n");
	};

	if ((frame->cs & 0x3) == 3) {
		kprintf("[CONTEXT] Fault occurred in USER LAND RING 3.\n");
		kprintf("[SECURITY] Usermode attempted Kernel Write at 0x%x. Killing Process.\n", fault_addr);
		panic("[CRITICAL] System Halted!");
	} else {
		kprintf("[CONTEXT] Fault occurred in KERNEL Mode RING 0.\n");
	};
	/*
	uint32_t* target_dir = (user_mode) ? task_get_curr()->parent->page_dir : kernel_directory;

	kprintf("[DEBUG] Current Page Directory: 0x%x\n", (v2p((uint32_t)target_dir)));
	idt_dump_interrupt_frame(frame);

	// Align fault address to 4 MiB boundary by clearing lower 22 bits
	const uint32_t aligned_fault_addr = fault_addr & 0xFFC00000;
	kprintf("[DEBUG] Aligned Fault Address: 0x%x\n", aligned_fault_addr);

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
	page_map_dir(target_dir, aligned_fault_addr, phys_addr, page_flags);
	kprintf("[INFO] Mapped Virtual Address 0x%x to Physical Address 0x%x\n", aligned_fault_addr, phys_addr);
	*/
	return;
};

void irq0_handler(interrupt_frame_t* frame)
{
	timer.ticks++;
	_pic1_send_eoi();

	scheduler_schedule(frame);
	return;
};

void irq1_handler(interrupt_frame_t* frame)
{
	if (ps2_wait(PS2_BUFFER_OUTPUT) == 0) {
		const uint8_t scancode = inb(PS2_DATA_PORT);
		process_t* fg_proc = tty_get_foreground();

		if (fg_proc && fg_proc->keyboard_buffer) {
			fifo_enqueue(fg_proc->keyboard_buffer, scancode);
		};
		wq_wakeup(WAIT_KEYBOARD);
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

void isr_default_handler(interrupt_frame_t* frame)
{
	_pic1_send_eoi();
	return;
};

static void _init_isr(void)
{
	for (size_t i = 0; i < 256; i++) {
		idt_set(i, asm_interrupt_default, IDT_KERNEL_INT_GATE);
	};
	return;
};

void idt_init(void)
{
	idtr_descriptor.limit = (uint16_t)sizeof(idt_desc_t) * 256 - 1;
	idtr_descriptor.base = (uintptr_t)&idt[0];
	_init_isr();
	idt_set(0x0, asm_isr0_wrapper, IDT_KERNEL_INT_GATE);
	idt_set(0x1, asm_isr1_wrapper, IDT_KERNEL_INT_GATE);
	idt_set(0x2, asm_isr2_wrapper, IDT_KERNEL_INT_GATE);
	idt_set(0x6, asm_isr6_wrapper, IDT_KERNEL_INT_GATE);
	idt_set(0x8, asm_isr8_wrapper, IDT_KERNEL_INT_GATE);
	idt_set(0xC, asm_isr12_wrapper, IDT_KERNEL_INT_GATE);
	idt_set(0xD, asm_isr13_wrapper, IDT_KERNEL_INT_GATE);
	idt_set(0xE, asm_isr14_wrapper, IDT_KERNEL_INT_GATE);
	idt_set(0x80, asm_syscall, IDT_USER_INT_GATE);
	asm_idt_loader(&idtr_descriptor);
	return;
};

void idt_set(const int32_t isr_num, void* isr, const uint8_t attributes)
{
	if (isr_num < 0 || isr_num >= 256) {
		return;
	};
	idt_desc_t* descriptor = &idt[isr_num];
	descriptor->isr_low = (uintptr_t)isr & 0xffff;
	descriptor->kernel_cs = 0x08;
	descriptor->reserved = 0x00;
	descriptor->attributes = attributes;
	descriptor->isr_high = ((uintptr_t)isr >> 16) & 0xffff;
	return;
};
