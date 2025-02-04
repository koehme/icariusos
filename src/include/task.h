/**
 * @file task.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TASK_H
#define TASK_H

#include <stdint.h>

typedef struct task_register {
	uint32_t eip;
	uint32_t esp;
	uint32_t ss;
} task_register_t;

typedef struct task {
	task_register_t reg;
	uint32_t* page_dir;
	struct task* next;
	struct task* prev;
} task_t;

int32_t task_init(task_t* self);

#endif