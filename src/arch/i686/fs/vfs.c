/**
 * @file vfs.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vfs.h"
#include "fat16.h"
#include "pathparser.h"

extern FileSystem fat16;

static FileSystem* filesystems[8] = {};
static FileDescriptor* fdescriptors[512] = {};

void vfs_init(void)
{
	FileSystem* fat16 = fat16_init();
	vfs_insert(fat16);
	return;
};

static FileSystem** vfs_find_empty_superblock(void)
{
	for (size_t i = 0; i < 8; i++) {
		if (filesystems[i] == 0x0) {
			return &filesystems[i];
		};
	};
	return 0x0;
};

void vfs_insert(FileSystem* fs)
{
	FileSystem** superblock = 0x0;

	if (!fs) {
		panic("Error: VFS needs a filesystem to insert.\n");
		return;
	};
	superblock = vfs_find_empty_superblock();

	if (!superblock) {
		panic("Error: VFS free superblock pool is exhausted.\n");
		return;
	};
	*superblock = fs;
	return;
};

static int32_t vfs_create_fd(FileDescriptor** ptr)
{
	int32_t res = -1;

	for (size_t i = 0; i < 512; i++) {
		if (fdescriptors[i] == 0x0) {
			FileDescriptor* fdescriptor = kcalloc(sizeof(FileDescriptor));
			fdescriptor->index = i + 1;
			fdescriptors[i] = fdescriptor;
			*ptr = fdescriptor;
			res = 0;
			break;
		};
	};
	return res;
};

static FileDescriptor* vfs_get_fd(const int32_t fd)
{
	if (fd <= 0 || fd >= 512) {
		return 0x0;
	};
	FileDescriptor* fdescriptor = fdescriptors[fd - 1];
	return fdescriptor;
};

FileSystem* vfs_resolve(ata_t* dev)
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

static VNODE_MODE vfs_get_vmode(const char* mode)
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
	const VNODE_MODE vmode = vfs_get_vmode(mode);

	if (vmode != V_READ) {
		res = -EINVAL;
		return res;
	};
	PathParser path_parser = {};
	PathRootNode* root = path_parser_parse(&path_parser, filename);

	if (!root->path->next) {
		printf("VFS Error: Files in '/' are prohibited\n");
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

	FileDescriptor* fdescriptor = 0x0;
	res = vfs_create_fd(&fdescriptor);

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
	FileDescriptor* fdescriptor = vfs_get_fd(fd);

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
	FileDescriptor* fdescriptor = vfs_get_fd(fd);

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
	FileDescriptor* fdescriptor = vfs_get_fd(fd);

	if (fdescriptor == 0x0) {
		res = -EBADF;
		return res;
	}
	res = fdescriptor->dev->fs->seek_cb(fdescriptor->internal, offset, whence);
	return res;
};

int32_t vfs_fstat(const int32_t fd, VStat* buffer)
{
	int32_t res = 0;

	if (fd < 1) {
		res = -EINVAL;
		return res;
	};
	FileDescriptor* fdescriptor = vfs_get_fd(fd);

	if (fdescriptor == 0x0) {
		res = -EBADF;
		return res;
	}
	fdescriptor->dev->fs->stat_cb(fdescriptor->dev, fdescriptor->internal, buffer);
	return res;
};