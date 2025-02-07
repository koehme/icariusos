/**
 * @file syscall.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "syscall.h"

#include "page.h"
#include "stdio.h"
#include "task.h"

void syscall_dispatch(task_registers_t* regs)
{
	page_restore_kernel_dir();

	switch (regs->eax) {
	case 1: {
		printf("[INFO] Usermode Task EXITED! Back to Kernel-Land (Ring 0)\n");
		break;
	};
	default: {
		printf("[INFO] Unknown Syscall: %d\n", regs->eax);
		break;
	};
	};
	return;
};