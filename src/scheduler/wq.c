/**
 * @file wq.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "wq.h"
#include "errno.h"
#include "scheduler.h"
#include "string.h"
#include "task.h"

/* PUBLIC API */
void wq_push(task_t* task);
task_t* wq_pop(void);
int32_t wq_size(void);

/* INTERNAL API */
static void _wq_enqueue(task_t* task);
static task_t* _wq_dequeue(void);
static task_t* _wait_queue[WAIT_MAX];
static int32_t _head = 0; // Insert a task from
static int32_t _tail = 0; // Remove a task from
static int32_t _count = 0;

int32_t wq_size(void) { return _count; };

void wq_wakeup(const wait_reason_t reason)
{
	size_t i = wq_size();

	while (i--) {
		task_t* task = _wq_dequeue();

		if (task->waiting_on == reason) {
			task_set_unblock(task);
			scheduler_get()->add_cb(task);
		} else {
			_wq_enqueue(task);
		};
	};
	return;
};

void wq_push(task_t* task)
{
	if (!task) {
		errno = EINVAL;
		return;
	};
	// printf("[WAIT] Pushing TASK PID %d ('%s') to Wait Queue. Reason: %d\n", task->parent->pid, task->parent->filename, task->waiting_on);
	_wq_enqueue(task);
	return;
};

task_t* wq_pop(void)
{
	task_t* task = _wq_dequeue();
	return task;
};

static void _wq_enqueue(task_t* task)
{
	if (!task || _count >= WAIT_MAX) {
		return;
	};
	if (task->state != TASK_STATE_BLOCK) {
		return;
	};
	_wait_queue[_head] = task;
	_head = (_head + 1) % WAIT_MAX;
	_count++;
	return;
};

static task_t* _wq_dequeue(void)
{
	if (_count <= 0 || _count > WAIT_MAX) {
		return 0x0;
	};
	task_t* task = _wait_queue[_tail];

	if (!task) {
		return 0x0;
	};
	_wait_queue[_tail] = 0x0;
	_tail = (_tail + 1) % WAIT_MAX;
	_count--;
	return task;
};