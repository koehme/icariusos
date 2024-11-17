/**
 * @file vnode.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VNODE_H
#define VNODE_H

#include "ata.h"

// VNODE_SEEK_MODE
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
// VNODE_MODE
#define READ 0
#define WRITE 1
#define APPEND 2
#define INVALID 3

typedef struct vstat_t {
	char st_dev[2];	     // Device ID (e.g., filesystem identifier)
	uint8_t st_mode;     // File mode (e.g., permissions)
	uint32_t st_size;    // File size in bytes
	uint32_t st_blksize; // Block size for I/O operations
	uint16_t st_blocks;  // Number of blocks
	uint32_t st_atime;   // Last access time (timestamp)
	uint32_t st_mtime;   // Last modification time (timestamp)
	uint32_t st_ctime;   // Last status change time (timestamp)
} vstat_t;

void vnode_dump(const vstat_t* self, const char name[]);

#endif