/**
 * @file wq.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef WQ_H
#define WQ_H

#include "icarius.h"
#include "idt.h"
#include "kernel.h"
#include "scheduler.h"
#include "task.h"

void wq_wakeup(const wait_reason_t reason);
void wq_push(task_t* task);
task_t* wq_pop(void);

#endif