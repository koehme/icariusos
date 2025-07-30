/**
 * @file tty.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "tty.h"

/* PUBLIC API */
void tty_set_foreground(process_t* proc);
process_t* tty_get_foreground(void);

/* INTERNAL API */
process_t* _foreground_process = 0x0;

void tty_set_foreground(process_t* proc)
{
	_foreground_process = proc;
	return;
};

process_t* tty_get_foreground(void) { return _foreground_process; }
