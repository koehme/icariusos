/**
 * @file syscall.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#include "idt.h"
#include "page.h"
#include "stdio.h"

void syscall_dispatch(int32_t syscall_id, interrupt_frame_t* frame);

#endif