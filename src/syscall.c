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
#include "unistd.h"
#include "wq.h"

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

	task_t* task = task_get_curr();
	process_t* parent = task->parent;

	task_exit(task);

	if (parent->task_count == 0) {
		process_exit(parent);
	};
	task->state = TASK_STATE_TERMINATE;
	scheduler_schedule(frame);
	__builtin_unreachable();
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

/**
 * @brief Handles the `read` syscall (int 0x80) for user processes.
 *
 * Blocks the task if reading from FD_STDIN and the keyboard buffer is empty.
 * Ensures kernel/user memory isolation by copying data through an internal kernel buffer.
 * Prevents invalid memory access by validating user buffer and allocation results.
 * Resumes the kernel's page directory after user-specific VFS operations.
 * Only FD_STDIN and valid file descriptors are supported; STDOUT/STDERR are rejected.
 */
int32_t _sys_read(interrupt_frame_t* frame)
{
	const int32_t fd = frame->ebx;
	const size_t count = frame->edx;
	int32_t n_read = 0;
	void* user_buf = (void*)frame->ecx;

	if (!user_buf || !count) {
		return -1;
	};
	void* kernel_buf = kzalloc(count);

	if (!kernel_buf) {
		return -1;
	};
	((char*)kernel_buf)[count] = '\0';

	process_t* caller = curr_task->parent;

	if (!caller) {
		kfree(kernel_buf);
		return -1;
	};

	switch (fd) {
	case FD_STDIN: {
		for (size_t i = 0; i < count; i++) {
			if (fifo_is_empty(caller->keyboard_buffer)) {
				task_t* task = caller->tasks[0];
				task_set_block(task);
				task_block_on(task, WAIT_KEYBOARD);
				wq_push(task);
				break;
			};

			if (!fifo_dequeue(caller->keyboard_buffer, (uint8_t*)kernel_buf + i)) {
				kfree(kernel_buf);
				return i;
			};
		};
		break;
	};
	case FD_STDOUT:
	case FD_STDERR: {
		kfree(kernel_buf);
		return -1;
	};
	default: {
		n_read = vfs_fread(kernel_buf, count, 1, fd);

		if (n_read < 0) {
			n_read = -1;
		};
		break;
	};
	};
	task_restore_dir(curr_task);
	memcpy(user_buf, kernel_buf, count);

	page_restore_kernel_dir();

	kfree(kernel_buf);
	return n_read;
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