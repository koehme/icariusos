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

static FileDescriptor *file_descriptors[MAX_FILE_DESCRIPTORS] = {};

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
    mset8(file_descriptors, 0x0, sizeof(FileDescriptor) * MAX_FILE_DESCRIPTORS);
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

/**
 * @brief Creates a new FileDescriptor and assigns it to the provided pointer.
 * Creates a new FileDescriptor and assigns it to the provided pointer. It searches
 * for an available slot in the file_descriptors array and initializes the necessary fields.
 * @param ptr Pointer to a FileDescriptor pointer where the created FileDescriptor will be assigned.
 * @return 0 on success, -1 if no available slots are found or memory allocation fails.
 */
static int vfs_create_fd(FileDescriptor **ptr)
{
    int res = -1;

    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++)
    {
        FileDescriptor *curr_slot = file_descriptors[i];

        if (curr_slot == 0x0)
        {
            FileDescriptor *file_descriptor = kcalloc(sizeof(FileDescriptor));

            if (file_descriptor != 0x0)
            {
                file_descriptor->index = i + 1;
                file_descriptors[i] = file_descriptor;
                *ptr = file_descriptor;
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

/**
 * @brief Retrieves the FileDescriptor at the specified index.
 * Retrieves the FileDescriptor stored at the specified index in the
 * file_descriptors array. The index should be a positive integer within the valid range.
 * @param fd_index The index of the FileDescriptor to retrieve.
 * @return Pointer to the FileDescriptor at the specified index, or NULL if the index is out of bounds.
 */
static FileDescriptor *vfs_get_fd(const int fd_index)
{
    if (fd_index <= 0 || fd_index >= MAX_FILE_DESCRIPTORS)
    {
        return 0x0;
    };
    FileDescriptor *fd = file_descriptors[fd_index - 1];
    return fd;
};

/**
 * @brief Resolves the appropriate Superblock for the given ATADisk.
 * Iiterates through a list of Superblocks to find the one that matches
 * the filesystem on the provided ATADisk. It uses the resolve_cb function of each Superblock
 * to determine if there is a match.
 * @param disk Pointer to the ATADisk for which to resolve the Superblock.
 * @return Pointer to the resolved Superblock, or 0x0 if the disk parameter is 0x0 or no matching Superblock is found.
 */
Superblock *vfs_resolve(ATADisk *disk)
{
    Superblock *resolved_fs = 0x0;

    if (!disk)
    {
        return resolved_fs;
    };

    for (int i = 0; i < MAX_FS; i++)
    {
        const Superblock *curr_fs = filesystems[i];
        const bool has_fsheader = curr_fs != 0x0 && curr_fs->resolve_cb(disk) == 0;

        if (has_fsheader)
        {
            resolved_fs = curr_fs;
            break;
        };
    };
    return resolved_fs;
};