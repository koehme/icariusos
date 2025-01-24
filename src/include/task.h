/**
 * @file task.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct registers {
	uint32_t eax, ebx, ecx, edx;	 // General-Purpose-Register
	uint32_t esi, edi, ebp, esp;	 // Stack-Pointer-Register
	uint32_t eip;			 // Instruction Pointer
	uint32_t eflags;		 // Flags
	uint32_t cr3;			 // Page Directory Base Register
	uint32_t cs, ds, es, fs, gs, ss; // Segment-Register
} registers_t;

typedef struct task {
	registers_t regs;	  // Registers to restore state
	uint32_t* page_directory; // Page directory of the task
	struct task* next;	  // Points to the next task
} task_t;

int32_t task_init(task_t* self);
task_t* task_get(void);
task_t* task_create(void);
task_t* task_next(void);
int32_t task_free(task_t* self);

#endif