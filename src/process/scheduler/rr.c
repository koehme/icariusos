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
static task_t* _rr_queue[RR_MAX];
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
	if (!frame) {
		return;
	};
	// 1) Should save the actual task
	task_t* curr = task_get_curr();

	if (!curr) {
		return;
	};
	curr->state = TASK_READY;
	// 2) put the actual task in the queue back
	_rr_enqueue(curr);
	// 3) pickup the next ready task from the queue
	task_t* next = _rr_dequeue();

	if (!next) {
		return;
	};
	next->state = TASK_RUNNING;
	// 4) activate the new task
	task_start(next);
	return;
};

void rr_dump(void) { return; };

task_t* rr_get(void)
{
	if (_count <= 0) {
		return 0x0;
	};
	return _rr_queue[_tail];
};

static void _rr_enqueue(task_t* task)
{
	if (!task || _count >= RR_MAX) {
		return;
	};
	_rr_queue[_head] = task;
	_head = (_head + 1) % RR_MAX;
	_count++;
	return;
};

static task_t* _rr_dequeue(void)
{
	if (_count <= 0) {
		return 0x0;
	};

	size_t remaining = _count;

	while (remaining > 0) {
		task_t* task = _rr_queue[_tail];

		_tail = (_tail + 1) % RR_MAX;
		_count--;
		remaining--;

		if (!task) {
			continue;
		};

		if (task->state == TASK_READY) {
			return task;
		};
	};
	return 0x0;
};