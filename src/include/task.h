/**
 * @file task.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct task_registers {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;
} task_registers_t;

typedef struct task {
	uint32_t* page_dir;
	task_registers_t registers;
	struct task* next;
	struct task* prev;
} task_t;

int32_t task_init(task_t* self);

#endif