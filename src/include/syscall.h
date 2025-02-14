/**
 * @file syscall.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

#include "icarius.h"
#include "idt.h"
#include "kernel.h"
#include "page.h"
#include "stdio.h"

void syscall_init(void);
void syscall_dispatch(const int32_t syscall_id, interrupt_frame_t* frame);

#endif