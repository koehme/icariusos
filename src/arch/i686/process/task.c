/**
 * @file task.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "task.h"

task_t* tasks = 0x0;
task_t* curr_task = 0x0;

int32_t task_init(task_t* self) { return 0; };

task_t* task_get(void) { return 0x0; };

task_t* task_create(void) { return 0x0; };

task_t* task_next(void) { return 0x0; };

int32_t task_free(task_t* self) { return 0; };