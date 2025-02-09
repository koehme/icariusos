/**
 * @file syscall.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "syscall.h"


void syscall_dispatch(int32_t syscall_id, interrupt_frame_t* frame)
{
	page_restore_kernel_dir();

	switch (syscall_id) {
	case 1: {
		printf("[INFO] Usermode Task EXITED! Back to Kernel-Land (Ring 0)\n");
		break;
	};
	default: {
		printf("[INFO] Unknown Syscall: %d\n", syscall_id);
		break;
	};
	};
	return;
};