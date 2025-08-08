/**
 * @file rr.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef RR_H
#define RR_H

#include "icarius.h"
#include "idt.h"
#include "kernel.h"
#include "scheduler.h"
#include "task.h"

void rr_add(task_t* task);
void rr_yield(interrupt_frame_t* frame);
void rr_dump(void);
task_t* rr_get(void);

void rr_init(scheduler_t* self);

extern scheduler_t round_robin;

#endif