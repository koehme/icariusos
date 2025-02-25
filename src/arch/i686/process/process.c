/**
 * @file process.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "process.h"
#include "stdlib.h"
#include "string.h"
#include "task.h"

/* PUBLIC API */
process_t* curr_process = 0x0;
process_t* processes = 0x0;
process_t* process_spawn(const char* filepath);
void process_list_dump(void);

/* INTERNAL API */
static uint16_t next_pid = 1;
static process_t* _process_alloc(const char* filepath);
static void _process_list_insert(process_t* new_process);
static uint32_t _process_get_filesize(const char* filename);

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
		printf("  Prev: 0x%x | Next: 0x%x\n", process->prev, process->next);
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
	task_t* initial_task = task_create((uint8_t*)new_process->filename);

	if (!initial_task) {
		kfree(new_process);
		return 0x0;
	};
	new_process->size = _process_get_filesize(new_process->filename);

	new_process->tasks[0] = initial_task;
	new_process->task_count++;
	_process_list_insert(new_process);
	return new_process;
};