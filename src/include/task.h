/**
 * @file task.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct task_registers {
	uint32_t edi; // Offset +0   | General-purpose register EDI
	uint32_t esi; // Offset +4   | General-purpose register ESI
	uint32_t ebp; // Offset +8   | Base Pointer (stack frame)
	uint32_t ebx; // Offset +12  | General-purpose register EBX
	uint32_t edx; // Offset +16  | General-purpose register EDX
	uint32_t ecx; // Offset +20  | General-purpose register ECX
	uint32_t eax; // Offset +24  | General-purpose register EAX (Syscall Number)

	uint32_t eip;	 // Offset +28  | Instruction Pointer (Usermode Entry)
	uint32_t cs;	 // Offset +32  | Code Segment (Usermode CS)
	uint32_t eflags; // Offset +36  | EFLAGS Register (Interrupts, System Flags)

	uint32_t esp; // Offset +40  | Stack Pointer (Usermode ESP)
	uint32_t ss;  // Offset +44  | Stack Segment (Usermode SS)
} task_registers_t;

typedef struct task {
	uint32_t* page_dir;
	task_registers_t registers;
	struct task* next;
	struct task* prev;
} task_t;

extern void asm_task_switch_to_userland(task_registers_t* regs);

int32_t task_init(task_t* self);
task_t* task_create_user(void (*entry_point)());
void task_restore_kernel_directory(void);
void task_switch_page_directory(task_t* task);

#endif