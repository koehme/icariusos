#include "tty.h"

process_t* foreground_process = 0x0;

void tty_set_foreground(process_t* proc)
{
	foreground_process = proc;
	return;
};

process_t* tty_get_foreground(void) { return foreground_process; }
