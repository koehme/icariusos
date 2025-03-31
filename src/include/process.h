/**
 * @file process.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PROCESS_H
#define PROCESS_H

#include "fifo.h"
#include "icarius.h"
#include "task.h"
#include <stdint.h>

struct task;
typedef struct task task_t;

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

typedef struct process {
	uint16_t pid;			     // Unique process ID
	char filename[PROCESS_MAX_FILENAME]; // For debugging & tracking
	task_t* tasks[PROCESS_MAX_THREAD];   // Thread table (one task = one thread)
	uint32_t* page_dir;		     // Own page directory (address space)
	uint8_t task_count;		     // Number of active threads
	process_filetype_t filetype;	     // ELF or BIN (determines loader logic)
	union {
		void* ptr;	      // Generic pointer access
		elf_file_t* elf_file; // Used if filetype == ELF
	};
	uint32_t size;		       // Size of loaded file
	fifo_t* keyboard_buffer;       // Keyboard input FIFO (per process)
	process_arguments_t arguments; // Command-line arguments
	struct process* prev;	       // Linked list (process chain)
	struct process* next;	       // Linked list (process chain)
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
void process_exit(process_t* self);

#endif