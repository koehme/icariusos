/**
 * @file vfs.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "vfs.h"
#include "fat16.h"
#include "pathparser.h"

extern Superblock fat16;

/**
 * @brief Array of Superblocks representing vfs_superblocks in the Virtual File System (VFS).
 * This static array serves as a container for Superblock pointers, each representing a
 * filesystem within the Virtual File System (VFS). The array allows the system to manage
 * and access different vfs_superblocks.
 */
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

/**
 * @brief Array of File Descriptors for file management in the system.
 * Represents the file descriptors used for managing files in the
 * system. Each element in the array corresponds to a file descriptor and the array
 * provides a mechanism for tracking and accessing open files within the system.
 */
static FileDescriptor *file_descriptors[512] = {};

/**
 * @brief Initializes the Virtual File System (VFS) subsystem.
 * Serves as the entry point for setting up the VFS subsystem. It
 * initializes file descriptors and loads essential components for the VFS to operate.
 * The primary purpose is to prepare the system for efficient file system management.
 */
void vfs_init(void)
{
    mset8(file_descriptors, 0x0, sizeof(FileDescriptor) * 512);
    mset8(vfs_superblocks, 0x0, sizeof(Superblock) * 8);
    vfs_insert(fat16_init());
    return;
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
        if (vfs_superblocks[i] == 0x0)
        {
            return &vfs_superblocks[i];
        };
    };
    return 0x0;
};
/**
 * @brief Inserts a filesystem into the Virtual File System (VFS) layer.
 * Responsible for adding a filesystem represented by the provided
 * Superblock to the VFS layer. The primary purpose is to manage and organize various
 * vfs_superblocks in a unified manner.
 * @param fs A pointer to the Superblock structure representing the filesystem to be inserted.
 * @note The function ensures that a valid filesystem is provided and that there is an
 * available slot in the VFS for insertion. If any of these conditions are not met,
 * a kernel panic is triggered with an appropriate error message.
 */
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

/**
 * @brief Retrieves the FileDescriptor at the specified index.
 * Retrieves the FileDescriptor stored at the specified index in the
 * file_descriptors array. The index should be a positive integer within the valid range.
 * @param fd_index The index of the FileDescriptor to retrieve.
 * @return Pointer to the FileDescriptor at the specified index, or NULL if the index is out of bounds.
 */
static FileDescriptor *vfs_get_fd(const int fd_index)
{
    if (fd_index <= 0 || fd_index >= 512)
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
    Superblock *superblock = 0x0;

    if (!disk)
    {
        return superblock;
    };

    for (int i = 0; i < 8; i++)
    {
        const bool has_header = vfs_superblocks[i] != 0x0 && vfs_superblocks[i]->resolve_cb(disk) == 0;

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
    ATADisk *disk = ata_get_disk(ATA_DISK_A);

    if (!disk || !disk->fs)
    {
        res = -EIO;
        return res;
    };
    void *internal_descriptor = disk->fs->open_cb(disk, root_path->path, mode);

    FileDescriptor *fd = 0x0;
    res = vfs_create_fd(&fd);

    if (res < 0)
    {
        res = -ENOMEM;
        return res;
    };
    fd->disk = disk;
    fd->fs = disk->fs;
    fd->internal = internal_descriptor;
    res = fd->index;
    return res;
};