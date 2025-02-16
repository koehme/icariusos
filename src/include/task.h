/**
 * @file task.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TASK_H
#define TASK_H

#include "idt.h"
#include <stdint.h>

typedef struct task_registers {
	uint32_t edi;	 // Offset +0   | General-purpose register EDI
	uint32_t esi;	 // Offset +4   | General-purpose register ESI
	uint32_t ebp;	 // Offset +8   | Base Pointer (stack frame)
	uint32_t ebx;	 // Offset +12  | General-purpose register EBX
	uint32_t edx;	 // Offset +16  | General-purpose register EDX
	uint32_t ecx;	 // Offset +20  | General-purpose register ECX
	uint32_t eax;	 // Offset +24  | General-purpose register EAX (Syscall Number)
	uint32_t eip;	 // Offset +28  | Instruction Pointer (Usermode Entry)
	uint32_t cs;	 // Offset +32  | Code Segment (Usermode CS)
	uint32_t eflags; // Offset +36  | EFLAGS Register (Interrupts, System Flags)
	uint32_t esp;	 // Offset +40  | Stack Pointer (Usermode ESP)
	uint32_t ss;	 // Offset +44  | Stack Segment (Usermode SS)
} task_registers_t;

typedef struct task {
	uint32_t* page_dir;
	task_registers_t registers;
} task_t;

extern task_t* curr_task;

extern void asm_enter_usermode(task_registers_t* regs);
extern void asm_restore_kernel_segment(void);
extern void asm_restore_user_segment(void);

task_t* task_create(void (*user_eip)());
task_t* task_get_curr(void);
void task_dump(task_t* self);
void task_save(interrupt_frame_t* frame);
void task_restore_dir(task_t* self);

#endif