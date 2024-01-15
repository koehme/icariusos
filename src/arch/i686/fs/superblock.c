/**
 * @file superblock.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "superblock.h"

static Superblock *filesystems[8] = {
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
    0x0,
};

/**
 * @brief Gets a free slot in the file system array.
 * @return Pointer to a free Superblock pointer slot or 0x0 if no free slot is available.
 */
static Superblock **vfs_get_free_fs_slot(void)
{
    size_t i = 0;

    for (; i < 8; i++)
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
    return;
};