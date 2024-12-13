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
	int32_t index; // Serves as a rapid identifier for file descriptors within system data structures, facilitating efficient access and management
	fs_t* fs; // Is essential for user-space programs in the kernel to access the required function pointers of the specific filesystem implementation. It
		  // connects the virtual filesystem with the actual filesystem implementation, providing a unified interface for file operations
	void* internal; // Is used so that the specific filesystem implementation or driver can write private data specific to the filesystem, which will be
			// later required for reading or writing. From the VFS perspective, this is just a void pointer
	ata_t* dev; // Needed because it can indicate different devices on which the filesystem may need to access storage. Through this pointer, the 'dev' can
		    // utilize the specific filesystem on the device and perform various operations like read or write
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