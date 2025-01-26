/**
 * @file task.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "task.h"
#include "errno.h"
#include "string.h"
#include "umode.h"

task_t* task_head = 0x0;
task_t* task_tail = 0x0;
task_t* curr_task = 0x0;

int32_t task_init(task_t* self)
{
	memset(self, 0x0, sizeof(task_t));
	self->page_dir = page_create_directory(PAGE_PS | PAGE_USER | PAGE_PRESENT);

	if (!self->page_dir) {
		return -EIO;
	};
	self->reg.eip = USER_CODE_START;      // 0x00000000
	self->reg.ss = GDT_USER_DATA_SEGMENT; // 0x20
	self->reg.esp = USER_STACK_END;	      // start: 0xBFC00000 - end: 0xBFFFFFFF
	return 0;
};

task_t* task_new(void)
{
	task_t* new_task = kzalloc(sizeof(task_t));

	if (!new_task) {
		errno = ENOMEM;
		return 0x0;
	};
	int32_t res = task_init(new_task);

	if (res != 0) {
		errno = -res;
		kfree((void*)new_task);
		return 0x0;
	};

	if (!task_head) {
		task_head = task_tail = new_task;
	} else {
		task_tail->next = new_task;
		new_task->prev = task_tail;
		task_tail = new_task;
	};
	return new_task;
};

task_t* task_get_curr(void) { return curr_task; };