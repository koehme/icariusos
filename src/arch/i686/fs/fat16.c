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
    Stream boot_stream;
    const int partition_offset = 0x100000;

    // Initialize the stream and read the first 512 byte of the boot Sector into the structure
    stream_init(&boot_stream, disk);
    stream_seek(&boot_stream, partition_offset);
    stream_read(&boot_stream, (uint8_t *)&bpb, sizeof(FAT16BaseHeader));

    // Validate FAT16 header
    if (!(bpb.jmp_short[0] == 0xEB && bpb.jmp_short[2] == 0x90) ||
        bpb.bytes_per_sector != 512 ||
        bpb.sectors_per_cluster < 1 ||
        bpb.fat_table_copies != 2 ||
        bpb.media_desc_type == 0)
    {
        kprintf("Error: Invalid FAT16 header.\n");
        return -1;
    };
    kprintf("FAT16 Header\n");
    kprintf("jmp_short: %d %d %d\n", bpb.jmp_short[0], bpb.jmp_short[1], bpb.jmp_short[2]);
    kprintf("oem_ident: %s\n", bpb.oem_ident);
    kprintf("bytes_per_sector: %d\n", bpb.bytes_per_sector);
    kprintf("sectors_per_cluster: %d\n", bpb.sectors_per_cluster);
    kprintf("reserved_sectors: %d\n", bpb.reserved_sectors);
    kprintf("fat_table_copies: %d\n", bpb.fat_table_copies);
    kprintf("root_directories: %d\n", bpb.root_directories);
    kprintf("total_sectors: %d\n", bpb.total_sectors);
    kprintf("media_desc_type: %d\n", bpb.media_desc_type);
    kprintf("sectors_per_fat: %d\n", bpb.sectors_per_fat);
    kprintf("sectors_per_track: %d\n", bpb.sectors_per_track);
    kprintf("heads: %d\n", bpb.heads);
    kprintf("hidden_sectors: %d\n", bpb.hidden_sectors);
    kprintf("large_total_sectors: %d\n", bpb.large_total_sectors);
    kprintf("----------------------------------\n");
    const uint32_t root_directory_offset = bpb.bytes_per_sector * (bpb.reserved_sectors + bpb.fat_table_copies * bpb.sectors_per_fat);
    const uint32_t root_directory_absolute = partition_offset + root_directory_offset;
    kprintf("root_directory_offset: 0x%x\n", root_directory_offset);
    kprintf("root_directory_absolute: 0x%x\n", root_directory_absolute);
    return 0;
};

void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode)
{
    return 0x0;
};