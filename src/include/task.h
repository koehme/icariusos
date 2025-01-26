/**
 * @file task.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct registers {
	uint32_t eax, ebx, ecx, edx;
	uint32_t esi, edi, ebp, esp;
	uint32_t eip;
	uint32_t eflags;
	uint32_t cr3;
	uint32_t cs, ds, es, fs, gs, ss;
} registers_t;

typedef struct task {
	registers_t reg;
	uint32_t* page_dir;
	struct task* next;
	struct task* prev;
} task_t;

int32_t task_init(task_t* self);

#endif