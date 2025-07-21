/**
 * @file scheduler.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "scheduler.h"
#include "errno.h"
#include "heap.h"
#include "rr.h"

static scheduler_t* curr_scheduler = 0x0;

scheduler_t* scheduler_get(void) { return curr_scheduler; };

scheduler_t* scheduler_create(const scheduler_type_t type)
{
	switch (type) {
	case SCHED_ROUND_ROBIN: {
		scheduler_t* scheduler = kmalloc(sizeof(scheduler_t));

		if (!scheduler) {
			errno = ENOMEM;
			return 0x0;
		};
		rr_init(scheduler);
		return scheduler;
	};
	default: {
		errno = EINVAL;
		return 0x0;
	};
	};
	return 0x0;
};

void scheduler_select(scheduler_t* self)
{
	if (!self) {
		errno = EINVAL;
		printf("[SCHEDULER] Invalid Scheduler Pointer\n");
		return;
	};
	curr_scheduler = self;
	printf("[SCHEDULER] SCHEDULER Activated: %s\n", self->name);
	return;
};

void scheduler_schedule(interrupt_frame_t* frame)
{
	if (!curr_scheduler || !curr_scheduler->yield_cb) {
		printf("[SCHEDULER] No SCHEDULER. No Yield. No Service.\n");
		return;
	};
	curr_scheduler->yield_cb(frame);
	return;
};