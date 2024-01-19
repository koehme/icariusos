/**
 * @file fat16.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "fat16.h"
#include "stream.h"

Superblock fat16 = {
    .resolve_cb = 0x0,
    .open_cb = 0x0,
    .name = "FAT16",
};

Superblock *fat16_init(void)
{
    fat16.resolve_cb = fat16_resolve;
    fat16.open_cb = fat16_open;
    return &fat16;
};

int fat16_resolve(ATADisk *disk)
{
    FAT16BaseHeader bpb;
    Stream stream;
    stream_init(&stream, disk);
    stream_seek(&stream, 0x100000);
    stream_read(&stream, disk->buffer, 512);
    kprintf("FAT16 Resolved\n");

    // Implement logic to validate fat16 header
    // For now only printing...
    for (int i = 0; i < 512; i++)
    {
        const uint8_t byte = disk->buffer[i];
        kprintf("0x%x ", byte);
        kdelay(1000);
    };
    return 0;
};

void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode)
{
    return 0x0;
};