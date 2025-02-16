/**
 * @file syscall.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "syscall.h"
#include "errno.h"
#include "icarius.h"

typedef int32_t (*syscall_handler_t)(interrupt_frame_t*);

static void _run_syscall(const int32_t syscall_id, interrupt_frame_t* frame);
void syscall_dispatch(const int32_t syscall_id, interrupt_frame_t* frame);
void syscall_init(void);

int32_t _sys_exit(interrupt_frame_t* frame);
size_t _sys_write(interrupt_frame_t* frame);
static const char* _get_name(const int32_t syscall_id);

void* syscalls[MAX_SYSCALL] = {};

static const char* _get_name(const int32_t syscall_id)
{
	switch (syscall_id) {
	case SYS_EXIT:
		return "SYS_EXIT";
	case SYS_WRITE:
		return "SYS_WRITE";
	default:
		return "Unknown Syscall";
	};
	return 0x0;
};

int32_t _sys_exit(interrupt_frame_t* frame)
{
	const int32_t status = frame->ebx;
	printf("[INFO] Usermode Task EXITED with 0x%x! Back to Kernel-Land (Ring 0)\n", status);
	// TODO: Exit TASK!!
	return 0;
};

size_t _sys_write(interrupt_frame_t* frame) { return 1; };

static void _run_syscall(const int32_t syscall_id, interrupt_frame_t* frame)
{
	if (syscall_id < 0 || syscall_id >= MAX_SYSCALL || !syscalls[syscall_id]) {
		printf("[ERROR] Invalid Syscall %s - [%d]\n", _get_name(syscall_id), syscall_id);
		return;
	};
	syscall_handler_t handler = (syscall_handler_t)syscalls[syscall_id];
	int32_t result = handler(frame);

	printf("[INFO] Syscall [%d] Returned [EAX]: %d\n", syscall_id, result);
	kernel_shell();
	return;
};

void syscall_dispatch(const int32_t syscall_id, interrupt_frame_t* frame)
{
	printf("=====================================\n");
	printf("  Syscall Dispatcher: %s  - [%d]\n", _get_name(syscall_id), syscall_id);
	printf("=====================================\n");

	idt_dump_interrupt_frame(frame);
	page_restore_kernel_dir();
	asm_restore_kernel_segment();

	task_save(frame);

	_run_syscall(syscall_id, frame);
	return;
};

void syscall_init(void)
{
	for (int32_t id = 0; id < MAX_SYSCALL; id++) {
		syscalls[id] = 0x0;
	};
	syscalls[SYS_EXIT] = (void*)_sys_exit;
	syscalls[SYS_WRITE] = (void*)_sys_write;
	return;
};