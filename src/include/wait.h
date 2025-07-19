/**
 * @file wait.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef WAIT_H
#define WAIT_H

#include "icarius.h"
#include "idt.h"
#include "kernel.h"
#include "scheduler.h"
#include "task.h"

void wait_push(task_t* task);
task_t* wait_pop(void);

#endif