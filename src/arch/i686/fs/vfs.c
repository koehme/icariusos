/**
 * @file vfs.c
 * @author Kevin Oehme
 * @copyright MIT
 * @brief Virtual File System (VFS) implementation.
 * @date 2024-11-16
 *
 * This file implements the Virtual File System (VFS), a layer that abstracts
 * file system operations and provides a unified interface for different file
 * systems. It allows seamless interaction with various file systems like FAT16
 * and others, supporting standard file operations such as opening, reading,
 * seeking, and closing files.
 *
 * The VFS plays a crucial role in the kernel by managing mounted file systems
 * and maintaining file descriptors. It enables processes to work with files
 * without needing to know the underlying file system details, ensuring
 * modularity and extensibility.
 *
 * Key VFS Components:
 * - Filesystem: Manages registered file systems.
 * - File Descriptor Table: Keeps track of open files.
 * - Path Parsing: Resolves file paths and interacts with mounted file systems.
 *
 * Complete Flow:
 *
 *    [Application]          [VFS Layer]             [File System]             [ATA Driver]
 *          |                     |                         |                          |
 *          | fopen("file.txt")   |                         |                          |
 *          +-------------------->| vfs_fopen()             |                          |
 *                                |------------------------>| fat16_open()             |
 *                                |                         |                          |
 *          | fread(buffer)       |                         |                          |
 *          +-------------------->| vfs_fread()             |                          |
 *                                |------------------------>| fat16_read()             |
 *                                |                         |------------------------->|
 *                                |                         |      ata_read()          |
 *                                |                         |<-------------------------|
 *          |                     |<------------------------|                          |
 *          | fclose(fd)          |                         |                          |
 *          +-------------------->| vfs_fclose()            |                          |
 *                                |------------------------>| fat16_close()            |
 *                                |                         |                          |
 */

#include "vfs.h"
#include "fat16.h"
#include "pathparser.h"

extern fs_t fat16;

static fs_t* filesystems[8] = {};
static fd_t* fdescriptors[512] = {};

/* PUBLIC API */
void vfs_init(void);
void vfs_insert(fs_t* fs);
fs_t* vfs_resolve(ata_t* dev);
int32_t vfs_fopen(const char* filename, const char* mode);
size_t vfs_fread(void* buffer, size_t n_bytes, size_t n_blocks, const int32_t fd);
int32_t vfs_fclose(const int32_t fd);
int32_t vfs_fseek(const int32_t fd, const uint32_t offset, const VNODE_SEEK_MODE whence);
int32_t vfs_fstat(const int32_t fd, vstat_t* buffer);

/* INTERNAL API */
static fs_t** _find_empty_fs(void);
static int32_t _create_fd(fd_t** ptr);
static fd_t* _get_fd(const int32_t fd);
static VNODE_MODE _get_vmode(const char* mode);

void vfs_init(void)
{
	fs_t* fat16 = fat16_init();
	vfs_insert(fat16);
	return;
};

static fs_t** _find_empty_fs(void)
{
	for (size_t i = 0; i < 8; i++) {
		if (filesystems[i] == 0x0) {
			return &filesystems[i];
		};
	};
	return 0x0;
};

void vfs_insert(fs_t* fs)
{
	fs_t** fs_slot = 0x0;

	if (!fs) {
		panic("[WARNING] VFS needs a fs to insert.\n");
		return;
	};
	fs_slot = _find_empty_fs();

	if (!fs_slot) {
		panic("[WARNING] VFS free fs_slot pool is exhausted.\n");
		return;
	};
	*fs_slot = fs;
	return;
};

static int32_t _create_fd(fd_t** ptr)
{
	int32_t res = -1;

	for (size_t i = 0; i < 512; i++) {
		if (fdescriptors[i] == 0x0) {
			fd_t* fdescriptor = kcalloc(sizeof(fd_t));
			fdescriptor->index = i + 1;
			fdescriptors[i] = fdescriptor;
			*ptr = fdescriptor;
			res = 0;
			break;
		};
	};
	return res;
};

static fd_t* _get_fd(const int32_t fd)
{
	if (fd <= 0 || fd >= 512) {
		return 0x0;
	};
	fd_t* fdescriptor = fdescriptors[fd - 1];
	return fdescriptor;
};

fs_t* vfs_resolve(ata_t* dev)
{
	if (!dev) {
		return 0x0;
	};

	for (size_t i = 0; i < 8; i++) {
		const bool has_header = filesystems[i] != 0x0 && filesystems[i]->resolve_cb(dev) == 0;

		if (has_header) {
			return filesystems[i];
		};
	};
	return 0x0;
};

static VNODE_MODE _get_vmode(const char* mode)
{
	switch (*mode) {
	case 'r': {
		return V_READ;
	};
	default:
		break;
	};
	return V_INVALID;
};

int32_t vfs_fopen(const char* filename, const char* mode)
{
	int32_t res = 0;
	const VNODE_MODE vmode = _get_vmode(mode);

	if (vmode != V_READ) {
		res = -EINVAL;
		return res;
	};
	pathparser_t path_parser = {};
	pathroot_node_t* root = path_parser_parse(&path_parser, filename);

	if (!root->path->next) {
		printf("[WARNING] Files in '/' are prohibited\n");
		res = -EINVAL;
		return res;
	};
	ata_t* dev = ata_get(root->drive);

	if (!dev || !dev->fs) {
		res = -EIO;
		return res;
	};
	void* internal = dev->fs->open_cb(dev, root->path, vmode);
	// Path analysis completed by the file system; only essential data extracted, safe to delete
	path_parser_free(root);

	fd_t* fdescriptor = 0x0;
	res = _create_fd(&fdescriptor);

	if (res < 0) {
		res = -ENOMEM;
		return res;
	};
	fdescriptor->dev = dev;
	fdescriptor->internal = internal;
	res = fdescriptor->index;
	return res;
};

size_t vfs_fread(void* buffer, size_t n_bytes, size_t n_blocks, const int32_t fd)
{
	if (n_bytes == 0 || n_blocks == 0 || fd < 1) {
		return -EINVAL;
	};
	fd_t* fdescriptor = _get_fd(fd);

	if (!fdescriptor) {
		return -EINVAL;
	};
	const size_t total_read = fdescriptor->dev->fs->read_cb(fdescriptor->dev, fdescriptor->internal, buffer, n_bytes, n_blocks);
	return total_read;
};

int32_t vfs_fclose(const int32_t fd)
{
	int32_t res = 0;

	if (fd < 1) {
		res = -EINVAL;
		return res;
	};
	fd_t* fdescriptor = _get_fd(fd);

	if (fdescriptor == 0x0) {
		res = -EBADF;
		return res;
	}
	fdescriptor->dev->fs->close_cb(fdescriptor->internal);
	kfree(fdescriptor);
	return res;
};

int32_t vfs_fseek(const int32_t fd, const uint32_t offset, const VNODE_SEEK_MODE whence)
{
	int32_t res = 0;

	if (fd < 1) {
		res = -EINVAL;
		return res;
	};
	fd_t* fdescriptor = _get_fd(fd);

	if (fdescriptor == 0x0) {
		res = -EBADF;
		return res;
	}
	res = fdescriptor->dev->fs->seek_cb(fdescriptor->internal, offset, whence);
	return res;
};

int32_t vfs_fstat(const int32_t fd, vstat_t* buffer)
{
	int32_t res = 0;

	if (fd < 1) {
		res = -EINVAL;
		return res;
	};
	fd_t* fdescriptor = _get_fd(fd);

	if (fdescriptor == 0x0) {
		res = -EBADF;
		return res;
	}
	fdescriptor->dev->fs->stat_cb(fdescriptor->dev, fdescriptor->internal, buffer);
	return res;
};