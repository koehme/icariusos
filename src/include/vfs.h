/**
 * @file vfs.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VFS_H
#define VFS_H

#include <stdbool.h>
#include <stddef.h>

#include "kernel.h"
#include "pathparser.h"
#include "vnode.h"

typedef struct ata ata_t;

typedef int32_t (*resolve_fn)(ata_t* dev);
typedef void* (*open_fn)(ata_t* dev, pathnode_t* path, const uint8_t mode);
typedef size_t (*read_fn)(ata_t* dev, void* internal, uint8_t* buffer, size_t n_bytes, size_t n_blocks);
typedef int32_t (*close_fn)(void* internal);
typedef int32_t (*stat_fn)(ata_t* dev, void* internal, vstat_t* buffer);
typedef int32_t (*seek_fn)(void* internal, const uint32_t offset, const uint8_t whence);

typedef struct fs {
	resolve_fn resolve_cb;
	open_fn open_cb;
	read_fn read_cb;
	close_fn close_cb;
	stat_fn stat_cb;
	seek_fn seek_cb;
	char name[10];
} fs_t;

typedef struct fd_t {
	int32_t index;
	fs_t* fs;
	void* internal;
	ata_t* dev;
} fd_t;

void vfs_init();
void vfs_insert(fs_t* fs);
int32_t vfs_fopen(const char* filename, const char* mode);
size_t vfs_fread(void* buffer, size_t n_bytes, size_t n_blocks, const int32_t fd);
fs_t* vfs_resolve(ata_t* dev);
int32_t vfs_fclose(const int32_t fd);
int32_t vfs_fstat(const int32_t fd, vstat_t* buffer);
int32_t vfs_fseek(const int32_t fd, const uint32_t offset, const uint8_t whence);

#endif