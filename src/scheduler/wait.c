/**
 * @file wait.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "wait.h"
#include "errno.h"
#include "string.h"

/* PUBLIC API */
void wait_push(task_t* task);
task_t* wait_pop(void);

/* INTERNAL API */
static void _wait_enqueue(task_t* task);
static task_t* _wait_dequeue(void);
static task_t* _wait_queue[WAIT_MAX];
static int32_t _head = 0; // Insert a task from
static int32_t _tail = 0; // Remove a task from
static int32_t _count = 0;

void wait_push(task_t* task)
{
	if (!task) {
		errno = EINVAL;
		return;
	};
	printf("[WAIT] Pushing TASK PID %d ('%s') to Wait Queue. Reason: %d\n", task->parent->pid, task->parent->filename, task->waiting_on);
	_wait_enqueue(task);
	return;
};

task_t* wait_pop(void)
{
	task_t* task = _wait_dequeue();
	return task;
};

static void _wait_enqueue(task_t* task)
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

static task_t* _wait_dequeue(void)
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