/**
 * @file process.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "process.h"
#include "errno.h"
#include "stdlib.h"
#include "string.h"
#include "task.h"

extern pfa_t pfa;
extern uint32_t kernel_directory[1024];

/* PUBLIC API */
void process_set_curr(process_t* self);
process_t* process_get_curr(void);
process_t* process_spawn(const char* filepath);
process_t* process_kspawn(void (*entry)(), const char* name);
void process_list_dump(void);
void process_exit(process_t* self);

/* INTERNAL API */
process_t* curr_process = 0x0;
process_t* processes = 0x0;
static uint16_t next_pid = 1;
static process_t* _process_alloc(const char* filepath);
static void _process_list_insert(process_t* new_process);
static uint32_t _process_get_filesize(const char* filename);

void process_set_curr(process_t* self)
{
	if (!self) {
		errno = EINVAL;
		return;
	}
	curr_process = self;
	return;
};

process_t* process_get_curr(void)
{
	if (!curr_process) {
		errno = ESRCH;
		return 0x0;
	};
	return curr_process;
};

void process_list_dump(void)
{
	printf("\n====================================\n");
	printf("         PROCESS LIST DUMP          \n");
	printf("====================================\n");

	process_t* process = processes;

	if (!process) {
		printf("[INFO] No Processes are currently running\n");
		return;
	};

	do {
		printf("PID: %d \n", process->pid);
		printf("  Addr: 0x%x \n", process);
		printf("  Name: %s\n", process->filename);
		printf("  Tasks: %d | File Type: %s\n", process->task_count, process->filetype == PROCESS_ELF ? "ELF" : "BINARY");
		printf("  Filesize: %d Bytes\n", process->size);
		printf("  Page Directory: 0x%x\n", process->page_dir);
		printf("  Keyboard Buffer: 0x%x\n", process->keyboard_buffer);
		printf("  Prev: 0x%x | Next: 0x%x\n", process->prev, process->next);

		printf("  Task List:\n");
		for (size_t i = 0; i < PROCESS_MAX_THREAD; i++) {
			const task_t* task = process->tasks[i];

			if (task) {
				printf("    - Task[%d]: 0x%x (Stack: 0x%x - 0x%x)\n", i, task, task->stack_top, task->stack_bottom);
			};
		};

		if (process->filetype == PROCESS_ELF && process->elf_file) {
			printf("  ELF File:\n");
			printf("    - Entry Point: 0x%x\n", process->elf_file->entry);
			printf("    - PH Offset: 0x%x | PH Num: %d | PH Size: %d\n", process->elf_file->ph_offset, process->elf_file->ph_num,
			       process->elf_file->ph_size);
			printf("    - SH Offset: 0x%x | SH Num: %d | SH Size: %d\n", process->elf_file->sh_offset, process->elf_file->sh_num,
			       process->elf_file->sh_size);
		};

		printf("  Arguments (argc=%d):\n", process->arguments.argc);
		for (size_t i = 0; i < process->arguments.argc; i++) {
			printf("    - argv[%d]: %s\n", i, process->arguments.argv[i]);
		};
		printf("------------------------------------\n");
		process = process->next;
	} while (process && process != processes);
	printf("====================================\n");
	return;
};

static process_t* _process_alloc(const char* filepath)
{
	process_t* new_process = kzalloc(sizeof(process_t));

	if (!new_process) {
		return 0x0;
	};
	memset(new_process, 0, sizeof(process_t));
	new_process->pid = next_pid++;
	strncpy(new_process->filename, filepath, sizeof(new_process->filename) - 1);
	new_process->filetype = PROCESS_BINARY;
	new_process->keyboard_buffer = kzalloc(sizeof(fifo_t));
	fifo_init(new_process->keyboard_buffer);
	return new_process;
};

static void _process_list_insert(process_t* new_process)
{
	if (!curr_process) {
		new_process->prev = new_process->next = 0x0;
		curr_process = new_process;
	} else {
		new_process->next = processes;
		new_process->prev = 0x0;

		if (processes) {
			processes->prev = new_process;
		};
	};
	processes = new_process;
	return;
};

static uint32_t _process_get_filesize(const char* filename)
{
	vstat_t stat_buf = {};
	const int32_t fd = vfs_fopen(filename, "r");

	if (fd < 0) {
		return 0;
	};

	if (vfs_fstat(fd, &stat_buf) < 0) {
		vfs_fclose(fd);
		return 0;
	};
	vfs_fclose(fd);
	return stat_buf.st_size;
};

process_t* process_spawn(const char* filepath)
{
	process_t* new_process = _process_alloc(filepath);

	if (!new_process) {
		errno = -ENOMEM;
		return 0x0;
	};
	const uint32_t flags = (PAGE_PS | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
	new_process->page_dir = page_create_dir(flags);
	page_map_between(new_process->page_dir, USER_CODE_START, USER_BSS_END, flags);
	page_map_between(new_process->page_dir, USER_HEAP_START, USER_HEAP_END, flags);

	task_t* task = task_create(new_process, (uint8_t*)new_process->filename);

	if (!task) {
		kfree(new_process);
		return 0x0;
	};
	new_process->size = _process_get_filesize(new_process->filename);

	new_process->tasks[new_process->task_count] = task;
	new_process->task_count++;

	_process_list_insert(new_process);
	return new_process;
};

process_t* process_kspawn(void (*entry)(), const char* name)
{
	process_t* proc = kzalloc(sizeof(process_t));

	if (!proc) {
		errno = -ENOMEM;
		return 0x0;
	};
	proc->pid = next_pid++;
	strncpy(proc->filename, name, sizeof(proc->filename) - 1);
	proc->filetype = PROCESS_KERNEL_THREAD;
	proc->keyboard_buffer = 0x0;

	proc->page_dir = kernel_directory;

	if (proc->task_count >= PROCESS_MAX_THREAD) {
		kfree(proc);
		errno = -E2BIG;
		return 0x0;
	};
	task_t* task = task_kcreate(proc, entry);

	if (!task) {
		errno = -ENOMEM;
		kfree(proc);
		return 0x0;
	};
	proc->tasks[proc->task_count++] = task;
	_process_list_insert(proc);
	return proc;
};

void process_exit(process_t* self)
{
	uint32_t* dir = self->page_dir;

	if (self->prev) {
		self->prev->next = self->next;
	}
	if (self->next) {
		self->next->prev = self->prev;
	}

	if (processes == self) {
		processes = self->next;
	};

	if (curr_process == self) {
		curr_process = 0x0;
	};

	for (uint32_t i = 0; i < 768; i++) {
		if (dir[i] & PAGE_PRESENT) {
			const uint32_t virt_addr = i * 0x400000;
			const uint32_t phys_addr = page_get_phys_addr(dir, virt_addr);
			page_unmap_dir(dir, virt_addr);
			const uint32_t frame = phys_addr / PAGE_SIZE;
			pfa_clear(&pfa, frame);
		};
	};
	const uint32_t phys_addr = (uint32_t)v2p((void*)self->page_dir);
	const uint32_t frame = phys_addr / PAGE_SIZE;
	page_unmap_dir(page_get_dir(), (uint32_t)self->page_dir);
	pfa_clear(&pfa, frame);

	kfree(self->keyboard_buffer);
	kfree(self);
	pfa_dump(&pfa, false);
	return;
};