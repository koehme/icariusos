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
    kprintf("=== FAT16 Filesystem Header Detected ===\n");
    kprintf("----------------------------------\n");
    kprintf("Jump Short: %d %d %d\n", bpb.jmp_short[0], bpb.jmp_short[1], bpb.jmp_short[2]);
    kprintf("OEM Identifier: %s\n", bpb.oem_ident);
    kprintf("Reserved Sectors: %d\n", bpb.reserved_sectors);
    kprintf("FAT Table Copies: %d\n", bpb.fat_table_copies);
    kprintf("Root Directories: %d\n", bpb.root_directories);
    kprintf("Total Sectors: %d\n", bpb.total_sectors);
    kprintf("Media Descriptor Type: %d\n", bpb.media_desc_type);
    kprintf("Sectors per FAT: %d\n", bpb.sectors_per_fat);
    kprintf("Sectors per Track: %d\n", bpb.sectors_per_track);
    kprintf("Heads: %d\n", bpb.heads);
    kprintf("Hidden Sectors: %d\n", bpb.hidden_sectors);
    kprintf("Large Total Sectors: %d\n", bpb.large_total_sectors);
    kprintf("----------------------------------\n");

    const uint32_t total_sectors = (bpb.total_sectors == 0) ? bpb.large_total_sectors : bpb.total_sectors;
    const uint16_t fat_size = (bpb.sectors_per_fat == 0) ? bpb.large_total_sectors / bpb.sectors_per_cluster : bpb.sectors_per_fat;
    const uint32_t root_dir_sectors = ((bpb.root_directories * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
    const uint32_t first_data_sector = bpb.reserved_sectors + (bpb.fat_table_copies * fat_size) + root_dir_sectors;
    const uint32_t first_fat_sector = bpb.reserved_sectors;
    const uint32_t data_sectors = total_sectors - (bpb.reserved_sectors + (bpb.fat_table_copies * fat_size) + root_dir_sectors);
    const uint32_t total_clusters = data_sectors / bpb.sectors_per_cluster;

    kprintf("Total Sectors (including VBR): %d\n", total_sectors);
    kprintf("FAT Size in Sectors: %d\n", fat_size);
    kprintf("Size of Root Directory in Sectors: %d\n", root_dir_sectors);
    kprintf("First Data Sector: %d\n", first_data_sector);
    kprintf("First Sector in the File Allocation Table: %d\n", first_fat_sector);
    kprintf("Total Number of Data Sectors: %d\n", data_sectors);
    kprintf("Total Number of Clusters: %d\n", total_clusters);
    // The first step in reading directories is finding and reading the root directory.
    // On a FAT 16 volume the root directory is at a fixed position immediately after the File Allocation Tables
    const uint32_t first_root_dir_sector = first_data_sector - root_dir_sectors;
    kprintf("First root dir sector: %d\n", first_root_dir_sector);
    return 0;
};

void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode)
{
    return 0x0;
};