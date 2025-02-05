/**
 * @file syscall.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include "task.h"

void syscall_dispatch(task_registers_t* regs);

#endif