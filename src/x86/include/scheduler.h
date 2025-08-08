/**
 * @file scheduler.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "icarius.h"
#include "task.h"

typedef void (*add_fn)(task_t* task);
typedef void (*yield_fn)(interrupt_frame_t* frame);
typedef void (*dump_fn)(void);
typedef task_t* (*get_fn)(void);

typedef enum scheduler_type {
	SCHED_ROUND_ROBIN = 0x0,
} scheduler_type_t;

typedef struct scheduler {
	add_fn add_cb;
	yield_fn yield_cb;
	dump_fn dump_cb;
	get_fn get_cb;
	char name[11];
} scheduler_t;

scheduler_t* scheduler_get(void);
scheduler_t* scheduler_create(const scheduler_type_t type);
void scheduler_select(scheduler_t* self);
void scheduler_schedule(interrupt_frame_t* frame);

#endif