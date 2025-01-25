/**
 * @file task.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "task.h"

task_t* task_head = 0x0;
task_t* task_tail = 0x0;
task_t* curr_task = 0x0;

int32_t task_init(task_t* self) { return 0; };