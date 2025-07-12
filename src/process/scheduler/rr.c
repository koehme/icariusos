/**
 * @file rr.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "rr.h"
#include "errno.h"
#include "idle.h"
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
	// 1. Sicherstellen, dass es überhaupt einen aktiven Task gibt
	if (!curr_task) {
		return;
	}

	// 2. Wenn der Aufruf aus einem Interrupt kommt (z. B. Timer), sichern wir den CPU-Zustand
	if (frame) {
		task_save(frame);
	}

	// 3. Wenn der aktuelle Task noch regulär läuft (nicht BLOCKED oder TERMINATED),
	//    dann markieren wir ihn als READY und reihen ihn wieder in die Scheduler-Queue ein.
	if (curr_task->state == TASK_RUNNING) {
		curr_task->state = TASK_READY;
		_rr_enqueue(curr_task); // ➜ Task ist wieder READY, wird später erneut geplant
	}

	// 4. Wähle den nächsten READY-Task aus der Warteschlange
	task_t* next = _rr_dequeue();

	if (!next) {
		return;
	}

	curr_task = next; // globales `curr_task` aktualisieren

	// 5. Unterscheiden: neuer Task oder bereits laufender
	if (!next->started) {
		next->started = true;
		task_start(next); // ➜ Erststart: springt zu entry point
	} else {
		task_switch(next); // ➜ Wiederaufnahme: springt zu gespeichertem EIP
	}
}

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