/**
 * @file tty.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef TTY_H
#define TTY_H

#include "process.h"

extern process_t* foreground_process;

void tty_set_foreground(process_t* proc);
process_t* tty_get_foreground(void);

#endif