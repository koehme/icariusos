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
        FileDescriptor *curr_descriptor_slot = file_descriptors[i];

        if (curr_descriptor_slot == 0x0)
        {
            FileDescriptor *new_descriptor = kcalloc(sizeof(FileDescriptor));

            if (new_descriptor != 0x0)
            {
                new_descriptor->index = i + 1;
                curr_descriptor_slot = new_descriptor;
                *ptr = new_descriptor;
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

int vfs_fopen(const char *file_name, const VNODE_MODE mode)
{
    int res = 0;

    if (mode != V_READ)
    {
        res = -EINVAL;
        return res;
    };
    PathParser path_parser = {};
    PathRootNode *root_path = path_parser_parse(&path_parser, file_name);
    ATADev *dev = ata_get(ATA_DEV_0);

    if (!dev || !dev->fs)
    {
        res = -EIO;
        return res;
    };
    void *internal_descriptor = dev->fs->open_cb(dev, root_path->path, mode);

    FileDescriptor *fd = 0x0;
    res = vfs_create_fd(&fd);

    if (res < 0)
    {
        res = -ENOMEM;
        return res;
    };
    fd->dev = dev;
    fd->fs = dev->fs;
    fd->internal = internal_descriptor;
    res = fd->index;
    return res;
};

size_t vfs_fread(const void *ptr, size_t n_bytes, size_t n_blocks, int32_t fd_index)
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
    const size_t readed_bytes = 0x0;

    if (readed_bytes < 0)
    {
        kprintf("Error reading from file. Bytes read: %d)\n", readed_bytes);
        return 0;
    };
    return readed_bytes;
};