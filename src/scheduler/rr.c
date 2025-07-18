/**
 * @file rr.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "rr.h"
#include "errno.h"
#include "string.h"

/* PUBLIC API */
void rr_init(scheduler_t* self);
void rr_add(task_t* task);
void rr_yield(interrupt_frame_t* frame);
void rr_dump(void);
task_t* rr_get(void);

/* INTERNAL API */
static void _rr_enqueue(task_t* task);
static task_t* _rr_dequeue(void);
static task_t* _ready_queue[RR_MAX];
static int32_t _head = 0; // Insert a task from
static int32_t _tail = 0; // Remove a task from
static int32_t _count = 0;

scheduler_t round_robin = {
    .add_cb = 0x0,
    .yield_cb = 0x0,
    .dump_cb = 0x0,
    .get_cb = 0x0,
    .name = {0x0},
};

void rr_init(scheduler_t* self)
{
	self->add_cb = rr_add;
	self->yield_cb = rr_yield;
	self->dump_cb = rr_dump;
	self->get_cb = rr_get;
	memcpy(self->name, "RoundRobin", 11);
	return;
};

void rr_add(task_t* task)
{
	if (!task) {
		errno = EINVAL;
		return;
	};
	_rr_enqueue(task);
	return;
};

void rr_yield(interrupt_frame_t* frame)
{
	if (!curr_task) {
		curr_task = curr_process->tasks[0];
	} else if (curr_task && frame) {
		task_save(frame);
	};

	if (curr_task->state == TASK_STATE_RUN) {
		curr_task->state = TASK_STATE_READY;
		_rr_enqueue(curr_task);
	};
	task_t* next = _rr_dequeue();
	printf("[SCHEDULER] Yield: Task from PID %d ('%s') -> Task from PID %d ('%s')\n", curr_task->parent->pid, curr_task->parent->filename,
	       next->parent->pid, next->parent->filename);
	// If no page directory exists, it is a kernel task (e.g. idle task)
	if (!next->parent->page_dir) {
		printf("[RR] No READY Tasks found – Entering Idle Task\n");
	};
	task_switch(next);
	return;
};

void rr_dump(void) { return; };

task_t* rr_get(void)
{
	if (_count <= 0) {
		return 0x0;
	};
	return _ready_queue[_head];
};

static void _rr_enqueue(task_t* task)
{
	if (!task || _count >= RR_MAX) {
		return;
	};
	if (task->state != TASK_STATE_READY) {
		return;
	};
	_ready_queue[_head] = task;
	_head = (_head + 1) % RR_MAX;
	_count++;
	return;
};

static task_t* _rr_dequeue(void)
{
	if (_count <= 0 || _count > RR_MAX) {
		return 0x0;
	};
	task_t* task = _ready_queue[_tail];

	if (!task) {
		return 0x0;
	};
	_ready_queue[_tail] = 0x0;
	_tail = (_tail + 1) % RR_MAX;
	_count--;
	return task;
};