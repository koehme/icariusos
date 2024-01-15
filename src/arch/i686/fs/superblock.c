/**
 * @file superblock.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "superblock.h"

static Superblock *filesystems[MAX_FS] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
};

static VNodeDescriptor *file_descriptors[MAX_VNODE_DESCRIPTORS] = {};

static void vfs_load_default_fs(void)
{
    // TODO: Implement fat16 as a default fs
    return;
};

static void vfs_load(void)
{
    mset8(filesystems, 0x0, sizeof(Superblock) * MAX_FS);
    return;
};

void vfs_init(void)
{
    mset8(file_descriptors, 0x0, sizeof(VNodeDescriptor) * MAX_VNODE_DESCRIPTORS);
    vfs_load();
    return;
};

/**
 * @brief Gets a free slot in the file system array.
 * @return Pointer to a free Superblock pointer slot or 0x0 if no free slot is available.
 */
static Superblock **vfs_get_free_fs_slot(void)
{
    size_t i = 0;

    for (; i < MAX_FS; i++)
    {
        if (filesystems[i] == 0x0)
        {
            return &filesystems[i];
        };
    };
    return 0x0;
};

void vfs_insert(Superblock *fs)
{
    Superblock **vfs_free_slot = 0x0;

    if (!fs)
    {
        kpanic("VFS Layer needs a filesystem.\n");
        return;
    };
    vfs_free_slot = vfs_get_free_fs_slot();

    if (!vfs_free_slot)
    {
        kpanic("VFS free filesystem pool is exhausted.\n");
        return;
    };
    *vfs_free_slot = fs;
    return;
};
