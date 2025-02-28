/**
 * @file process.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <fifo.h>
#include <stdint.h>

struct task;
typedef struct task task_t;

#include <task.h>

#define PROCESS_MAX_FILENAME 128
#define PROCESS_MAX_THREAD 16
#define PROCESS_MAX_ALLOCATION 16

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
	char filename[PROCESS_MAX_FILENAME];
	task_t* tasks[PROCESS_MAX_THREAD];
	uint32_t* page_dir;
	uint8_t task_count;
	process_allocation_t allocations[PROCESS_MAX_ALLOCATION];
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

/**
 * Stack Allocation within a 4 MiB Page for up to 16 Threads.
 * Each Task Stack has a Size of 256 KiB (0x40000) and grows downwards
 * Stack Memory Layout
 * USER_STACK_END (0xBFFFFFFF)
 *     | Stack[0]  | 256 KiB  | 0xBFFFFFFF - (0x40000 - 0x1) → 0xBFFC0000
 *     | Stack[1]  | 256 KiB  | 0xBFFBFFFF - (0x40000 - 0x1) → 0xBFF80000
 *     | Stack[2]  | 256 KiB  | 0xBFF7FFFF - (0x40000 - 0x1) → 0xBFF40000
 *     | Stack[3]  | 256 KiB  | 0xBFF3FFFF - (0x40000 - 0x1) → 0xBFF00000
 *     | Stack[4]  | 256 KiB  | 0xBFEFFFFF - (0x40000 - 0x1) → 0xBFEC0000
 *     | Stack[5]  | 256 KiB  | 0xBFEBFFFF - (0x40000 - 0x1) → 0xBFE80000
 *     | Stack[6]  | 256 KiB  | 0xBFE7FFFF - (0x40000 - 0x1) → 0xBFE40000
 *     | Stack[7]  | 256 KiB  | 0xBFE3FFFF - (0x40000 - 0x1) → 0xBFE00000
 *     | Stack[8]  | 256 KiB  | 0xBFDFFFFF - (0x40000 - 0x1) → 0xBFDC0000
 *     | Stack[9]  | 256 KiB  | 0xBFDBFFFF - (0x40000 - 0x1) → 0xBFD80000
 *     | Stack[10] | 256 KiB  | 0xBFD7FFFF - (0x40000 - 0x1) → 0xBFD40000
 *     | Stack[11] | 256 KiB  | 0xBFD3FFFF - (0x40000 - 0x1) → 0xBFD00000
 *     | Stack[12] | 256 KiB  | 0xBFCFFFFF - (0x40000 - 0x1) → 0xBFCC0000
 *     | Stack[13] | 256 KiB  | 0xBFCBFFFF - (0x40000 - 0x1) → 0xBFC80000
 *     | Stack[14] | 256 KiB  | 0xBFC7FFFF - (0x40000 - 0x1) → 0xBFC40000
 *     | Stack[15] | 256 KiB  | 0xBFC3FFFF - (0x40000 - 0x1) → 0xBFC00000
 * USER_STACK_START (0xBFC00000)
 *
 * Stack Address Calculation:
 *
 * Stack_Top = USER_STACK_END - (Thread_ID * 0x40000)
 * Stack_Bottom = Stack_Top - (0x40000 - 0x1)
 */
extern process_t* curr_process;

process_t* process_spawn(const char* filepath);
void process_list_dump(void);

#endif