/**
 * @file task.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "task.h"
#include "errno.h"
#include "icarius.h"
#include "string.h"


/* PUBLIC API */
task_t* task_create(void (*entry_point)());

/* INTERNAL API */
static task_t* _init_task(void);
static void _init_task_register(task_t* task);

static task_t* _init_task(void)
{
	task_t* task = kzalloc(sizeof(task_t));

	if (!task) {
		errno = ENOMEM;
		kfree((void*)task);
		return 0x0;
	};
	memset(task, 0x0, sizeof(task_t));
	return task;
};

static void _init_task_register(task_t* task)
{
	if (!task) {
		printf("[ERROR] Invalid Task!\n");
		return;
	};
	task->registers.eip = USER_CODE_START;
	task->registers.cs = GDT_USER_CODE_SEGMENT;
	task->registers.eflags = 0x200;
	task->registers.esp = USER_STACK_END;
	task->registers.ss = GDT_USER_DATA_SEGMENT;
	printf("[DEBUG] Task Registers - CS: 0x%x, DS: 0x%x, EIP: 0x%x\n", task->registers.cs, task->registers.ss, task->registers.eip);
	return;
};

task_t* task_create(void (*entry_point)())
{
	task_t* task = _init_task();

	if (!task) {
		errno = ENOMEM;
		return 0x0;
	};
	const uint32_t flags = (PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
	task->page_dir = page_create_dir(flags, entry_point);
	_init_task_register(task);
	return task;
};