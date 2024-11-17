/**
 * @file vnode.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VNODE_H
#define VNODE_H

#include "ata.h"

typedef enum VNODE_SEEK_MODE {
	SEEK_SET,
	SEEK_CUR,
	SEEK_END,
} VNODE_SEEK_MODE;

typedef enum VNODE_MODE {
	V_READ,
	V_WRITE,
	V_APPEND,
	V_INVALID,
} VNODE_MODE;

typedef struct vstat_t {
	char st_dev[2];
	VNODE_MODE st_mode;
	uint32_t st_size;
	uint32_t st_blksize;
	uint16_t st_blocks;
	uint16_t st_atime;
	uint16_t st_mtime;
	uint16_t st_ctime;
} vstat_t;

void vnode_dump(const vstat_t* self, const char name[]);

#endif