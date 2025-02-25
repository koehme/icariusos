/**
 * @file process.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <fifo.h>
#include <stdint.h>
#include <task.h>

typedef enum process_filetype { PROCESS_ELF, PROCESS_BINARY } process_filetype_t;

typedef struct process_arguments {
	int argc;
	char** argv;
} process_arguments_t;

typedef struct elf_file {
	uint32_t entry;
	uint32_t ph_offset;
	uint32_t ph_num;
	uint32_t ph_size;
	uint32_t sh_offset;
	uint32_t sh_num;
	uint32_t sh_size;
} elf_file_t;

typedef struct process_allocation {
	void* addr;
	uint32_t size;
} process_allocation_t;

typedef struct process {
	uint16_t pid;
	char filename[128];
	task_t* tasks[16];
	uint8_t task_count;
	process_allocation_t allocations[256];
	process_filetype_t filetype;
	union {
		void* ptr;
		elf_file_t* elf_file;
	};
	uint32_t size;
	fifo_t* keyboard_buffer;
	process_arguments_t arguments;
	struct process* prev;
	struct process* next;
} process_t;

extern process_t* curr_process;

process_t* process_spawn(const char* filepath);
void process_list_dump(void);

#endif