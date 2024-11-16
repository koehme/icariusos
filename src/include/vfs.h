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

typedef struct ata_t ata_t;

typedef int32_t (*ResolveFunction)(ata_t* dev);
typedef void* (*OpenFunction)(ata_t* dev, PathNode* path, const VNODE_MODE mode);
typedef size_t (*ReadFunction)(ata_t* dev, void* internal, uint8_t* buffer, size_t n_bytes, size_t n_blocks);
typedef int32_t (*CloseFunction)(void* internal);
typedef int32_t (*StatFunction)(ata_t* dev, void* internal, VStat* buffer);
typedef int32_t (*SeekFunction)(void* internal, const uint32_t offset, const VNODE_SEEK_MODE whence);

typedef struct fs_t {
	ResolveFunction resolve_cb;
	OpenFunction open_cb;
	ReadFunction read_cb;
	CloseFunction close_cb;
	StatFunction stat_cb;
	SeekFunction seek_cb;
	char name[10];
} fs_t;

typedef struct FileDescriptor {
	int32_t index; // Serves as a rapid identifier for file descriptors within system data structures, facilitating efficient access and management
	fs_t* fs; // Is essential for user-space programs in the kernel to access the required function pointers of the specific filesystem implementation. It
		  // connects the virtual filesystem with the actual filesystem implementation, providing a unified interface for file operations
	void* internal; // Is used so that the specific filesystem implementation or driver can write private data specific to the filesystem, which will be
			// later required for reading or writing. From the VFS perspective, this is just a void pointer
	ata_t* dev; // Needed because it can indicate different devices on which the filesystem may need to access storage. Through this pointer, the 'dev' can
		    // utilize the specific filesystem on the device and perform various operations like read or write
} FileDescriptor;

void vfs_init();
void vfs_insert(fs_t* fs);
int32_t vfs_fopen(const char* filename, const char* mode);
size_t vfs_fread(void* buffer, size_t n_bytes, size_t n_blocks, const int32_t fd);
fs_t* vfs_resolve(ata_t* dev);
int32_t vfs_fclose(const int32_t fd);
int32_t vfs_fstat(const int32_t fd, VStat* buffer);
int32_t vfs_fseek(const int32_t fd, const uint32_t offset, const VNODE_SEEK_MODE whence);

#endif