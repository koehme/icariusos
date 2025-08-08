/**
 * @file task.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TASK_H
#define TASK_H

#include "idt.h"
#include "process.h"
#include <stdint.h>

struct process;
typedef struct process process_t;

typedef enum task_state {
	TASK_STATE_READY = 0x0,	    // schedulable task, i.e. waits to be picked up by the scheduler to be executed on the cpu
	TASK_STATE_RUN = 0x1,	    // is running on the cpu
	TASK_STATE_BLOCK = 0x2,	    // waiting on an event i/o.. wait()?
	TASK_STATE_TERMINATE = 0x3, // exited
} task_state_t;

typedef enum wait_reason {
	WAIT_NONE,
	WAIT_KEYBOARD,
	WAIT_NOUSE,
} wait_reason_t;

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
} __attribute__((packed)) task_registers_t;

typedef struct task {
	uint32_t stack_top;
	uint32_t stack_bottom;
	task_registers_t registers;
	process_t* parent;
	task_state_t state;
	wait_reason_t waiting_on;
} task_t;

extern void asm_enter_task(task_registers_t* frame);
extern void asm_restore_kernel_segment(void);
extern void asm_restore_user_segment(void);

void task_block_on(task_t* self, const wait_reason_t reason);
void task_exit(task_t* self);
task_t* task_create(process_t* parent, const uint8_t* file);
task_t* task_kcreate(process_t* parent, void (*entry)());
process_t* process_kspawn(void (*entry)(), const char* name);
task_t* task_get_curr(void);
void task_set_curr(task_t* self);
void task_dump(task_t* self);
void task_save(interrupt_frame_t* frame);
void task_restore_dir(task_t* self);
void task_set_block(task_t* self);
void task_set_unblock(task_t* self);
void task_switch(task_t* next);

#endif