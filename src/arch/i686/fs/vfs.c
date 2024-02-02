/**
 * @file vfs.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vfs.h"
#include "fat16.h"
#include "pathparser.h"

extern Superblock fat16;

static Superblock *vfs_superblocks[8] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
};

static FileDescriptor *file_descriptors[512] = {};

void vfs_init(void)
{
    mset8(file_descriptors, 0x0, sizeof(FileDescriptor) * 512);
    mset8(vfs_superblocks, 0x0, sizeof(Superblock) * 8);
    Superblock *fat16 = fat16_init();
    vfs_insert(fat16);
    return;
};

static Superblock **vfs_find_empty_superblock(void)
{
    size_t i = 0;

    for (; i < 8; i++)
    {
        if (vfs_superblocks[i] == 0x0)
        {
            return &vfs_superblocks[i];
        };
    };
    return 0x0;
};

void vfs_insert(Superblock *fs)
{
    Superblock **vfs_free_slot = 0x0;

    if (!fs)
    {
        kpanic("Error: VFS needs a filesystem to insert.\n");
        return;
    };
    vfs_free_slot = vfs_find_empty_superblock();

    if (!vfs_free_slot)
    {
        kpanic("Error: VFS free superblock pool is exhausted.\n");
        return;
    };
    *vfs_free_slot = fs;
    return;
};

static int vfs_create_fd(FileDescriptor **ptr)
{
    int res = -1;

    for (int i = 0; i < 512; i++)
    {
        if (file_descriptors[i] == 0x0)
        {
            FileDescriptor *descriptor = kcalloc(sizeof(FileDescriptor));

            if (descriptor != 0x0)
            {
                descriptor->index = i + 1;
                file_descriptors[i] = descriptor;
                *ptr = descriptor;
                res = 0;
                break;
            }
            else
            {
                res = -1;
                break;
            };
        };
    };
    return res;
};

static FileDescriptor *vfs_get_fd(const int fd_index)
{
    if (fd_index <= 0 || fd_index >= 512)
    {
        return 0x0;
    };
    FileDescriptor *fd = file_descriptors[fd_index - 1];
    return fd;
};

Superblock *vfs_resolve(ATADev *dev)
{
    Superblock *superblock = 0x0;

    if (!dev)
    {
        return superblock;
    };

    for (int i = 0; i < 8; i++)
    {
        const bool has_header = vfs_superblocks[i] != 0x0 && vfs_superblocks[i]->resolve_cb(dev) == 0;

        if (has_header)
        {
            superblock = vfs_superblocks[i];
            break;
        };
    };
    return superblock;
};

static VNODE_MODE vfs_get_vmode(const char *mode)
{
    switch (*mode)
    {
    case 'r':
    {
        return V_READ;
    };
    default:
        break;
    };
    return V_INVALID;
};

int vfs_fopen(const char *filename, const char *mode)
{
    int32_t res = 0;
    const VNODE_MODE vmode = vfs_get_vmode(mode);

    if (vmode != V_READ)
    {
        res = -EINVAL;
        return res;
    };
    PathParser path_parser = {};
    PathRootNode *root_path = path_parser_parse(&path_parser, filename);
    ATADev *dev = ata_get(ATA_DEV_0);

    if (!dev || !dev->fs)
    {
        res = -EIO;
        return res;
    };
    void *internal = dev->fs->open_cb(dev, root_path->path, vmode);
    FileDescriptor *fd = 0x0;
    res = vfs_create_fd(&fd);

    if (res < 0)
    {
        res = -ENOMEM;
        return res;
    };
    fd->dev = dev;
    fd->fs = dev->fs;
    fd->internal = internal;
    res = fd->index;
    return res;
};

size_t vfs_fread(void *buffer, size_t n_bytes, size_t n_blocks, int32_t fd_index)
{
    if (n_bytes == 0 || n_blocks == 0 || fd_index < 1)
    {
        return -EINVAL;
    };
    FileDescriptor *fd = vfs_get_fd(fd_index);

    if (!fd)
    {
        return -EINVAL;
    };
    const size_t total_read = fd->fs->read_cb(fd->dev, fd->internal, buffer, n_bytes, n_blocks);
    return total_read;
};