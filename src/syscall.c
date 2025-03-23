/**
 * @file syscall.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "syscall.h"
#include "dirent.h"
#include "errno.h"
#include "fifo.h"
#include "heap.h"
#include "icarius.h"
#include "task.h"

extern fifo_t fifo_kbd;
extern fifo_t fifo_mouse;

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
	case SYS_READ:
		return "SYS_READ";
	case SYS_WRITE:
		return "SYS_WRITE";
	case SYS_OPEN:
		return "SYS_OPEN";
	case SYS_CLOSE:
		return "SYS_CLOSE";
	case SYS_GETDENTS:
		return "SYS_GETDENTS";
	default:
		return "UNKNOWN Syscall";
	};
	return 0x0;
};

int32_t _sys_getdents(interrupt_frame_t* frame)
{
	const int fd = frame->ebx;
	struct dirent* user_buf = (struct dirent*)frame->ecx;
	const unsigned int count = frame->edx;

	if (fd < 1 || !user_buf || count < sizeof(struct dirent)) {
		return -EINVAL;
	};
	if ((uintptr_t)user_buf >= KERNEL_VIRTUAL_START) {
		return -EFAULT;
	};
	void* kernel_buf = kzalloc(count + 1);

	if (!kernel_buf) {
		return -ENOMEM;
	};
	task_restore_dir(curr_task);
	memcpy(kernel_buf, user_buf, count);
	page_restore_kernel_dir();

	vfs_dirent_t ventry = {};

	int32_t ret = vfs_readdir(fd, &ventry);

	if (ret != 1) {
		kfree(kernel_buf);
		return 0;
	};

	struct dirent dentry = {
	    .d_ino = 0,
	    .d_off = 0,
	    .d_reclen = sizeof(struct dirent),
	    .d_type = (ventry.type == 1) ? DT_DIR : DT_REG,
	};
	strncpy(dentry.d_name, ventry.name, sizeof(dentry.d_name));
	memcpy(kernel_buf, &dentry, sizeof(struct dirent));

	task_restore_dir(curr_task);
	memcpy(user_buf, kernel_buf, sizeof(struct dirent));
	page_restore_kernel_dir();

	kfree(kernel_buf);
	return sizeof(struct dirent);
};

int32_t _sys_close(interrupt_frame_t* frame)
{
	const int32_t fd = frame->ebx;

	if (fd < 1 || fd >= 512) {
		return -EBADF;
	};
	const int32_t res = vfs_fclose(fd);
	return res;
};

int32_t _sys_exit(interrupt_frame_t* frame)
{
	const int32_t status = frame->ebx;
	printf("\n[INFO] Usermode Task EXITED with 0x%x! Back to Kernel-Land (Ring 0)\n", status);

	task_t* task = curr_task;
	process_t* parent_process = task->parent;

	task_exit(task);

	if (parent_process->task_count == 0) {
		printf("[INFO] Process PID [%d] exit. Starting Kernel Shell..\n", parent_process->pid);
		curr_task = 0x0;
		process_exit(parent_process);
	};
	kernel_shell();
	return status;
};

int32_t _sys_open(interrupt_frame_t* frame)
{
	task_restore_dir(curr_task);
	const char* user_buf = (const char*)frame->ebx;
	const size_t count = strlen(user_buf);

	if ((uintptr_t)user_buf >= KERNEL_VIRTUAL_START) {
		return -EFAULT;
	};
	page_restore_kernel_dir();
	int flag = frame->ecx;
	void* kernel_buf = kzalloc(count + 1);

	if (!kernel_buf) {
		return -ENOMEM;
	};
	task_restore_dir(curr_task);
	strncpy(kernel_buf, user_buf, count);

	((uint8_t*)kernel_buf)[count] = '\0';
	page_restore_kernel_dir();

	char* mode;

	switch (flag) {
	case READ: {
		mode = "r";
		break;
	};
	case WRITE: {
		mode = "w";
		break;
	};
	default: {
		kfree(kernel_buf);
		return -EINVAL;
	};
	};
	const int32_t fd = vfs_fopen(kernel_buf, mode);
	kfree(kernel_buf);

	if (fd < 1) {
		return -ENOENT;
	};
	return fd;
};

size_t _sys_write(interrupt_frame_t* frame)
{
	const int32_t syscall_id = frame->eax;
	const int32_t fd = frame->ebx;
	const size_t count = frame->edx;
	void* kernel_buf = kzalloc(count);

	if (!kernel_buf) {
		return -ENOMEM;
	};
	task_restore_dir(curr_task);
	void* user_buf = (void*)frame->ecx;

	memcpy(kernel_buf, user_buf, count);
	((uint8_t*)kernel_buf)[count] = '\0';

	page_restore_kernel_dir();

	if (fd == FD_STDOUT || fd == FD_STDERR) {
		printf("%s", (uint8_t*)kernel_buf);
		kfree(kernel_buf);
		return count;
	};
	const size_t written = vfs_fwrite(kernel_buf, count, 1, fd);

	if (written < 0) {
		printf("[ERROR] Failed to write to file FD: %d\n", fd);
	};
	kfree(kernel_buf);
	return count;
};

int32_t _sys_read(interrupt_frame_t* frame)
{
	const int32_t syscall_id = frame->eax;
	const int32_t fd = frame->ebx;
	const size_t count = frame->edx;
	void* user_buf = (void*)frame->ecx;

	if (fd != 0 || user_buf == 0x0 || count == 0)
		return -1;

	asm_do_sti();
	void* kernel_buf = kzalloc(count);

	process_t* caller = curr_task->parent;

	if (!caller)
		return -1;

	for (size_t i = 0; i < count; i++) {
		while (fifo_is_empty(caller->keyboard_buffer)) {
			//	asm volatile("hlt");
		};

		if (!fifo_dequeue(caller->keyboard_buffer, (uint8_t*)kernel_buf + i)) {
			kfree(kernel_buf);
			return i;
		};
	};
	((char*)kernel_buf)[count] = '\0';

	task_restore_dir(curr_task);
	memcpy(user_buf, kernel_buf, count);

	page_restore_kernel_dir();

	kfree(kernel_buf);
	return count;
};

static void _run_syscall(const int32_t syscall_id, interrupt_frame_t* frame)
{
	if (syscall_id < 0 || syscall_id >= MAX_SYSCALL) {
		panic("Syscall ID [%d] out of range!\n", (int)syscall_id);
	};

	if (!syscalls[syscall_id]) {
		panic("Syscall [%s] (%d) not registered!\n", _get_name(syscall_id), (int)syscall_id);
	};
	const syscall_handler_t handler = (syscall_handler_t)syscalls[syscall_id];
	frame->eax = handler(frame);
	// printf("\n[DEBUG] Syscall [%d]\n", syscall_id);
	// kernel_shell();
	return;
};

void syscall_dispatch(const int32_t syscall_id, interrupt_frame_t* frame)
{
	// printf("=====================================\n");
	// printf("  Syscall Dispatcher: %s  - [%d]\n", _get_name(syscall_id), syscall_id);
	// printf("=====================================\n");

	// idt_dump_interrupt_frame(frame);
	page_restore_kernel_dir();
	asm_restore_kernel_segment();

	task_save(frame);

	_run_syscall(syscall_id, frame);

	task_restore_dir(curr_task);
	asm_restore_user_segment();
	return;
};

void syscall_init(void)
{
	for (int32_t id = 0; id < MAX_SYSCALL; id++) {
		syscalls[id] = 0x0;
	};
	syscalls[SYS_EXIT] = (void*)_sys_exit;
	syscalls[SYS_WRITE] = (void*)_sys_write;
	syscalls[SYS_READ] = (void*)_sys_read;
	syscalls[SYS_OPEN] = (void*)_sys_open;
	syscalls[SYS_CLOSE] = (void*)_sys_close;
	syscalls[SYS_GETDENTS] = (void*)_sys_getdents;
	return;
};