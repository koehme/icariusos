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
    FAT16BaseHeader base_header;
    Stream boot_stream;
    const int partition_offset = 0x100000;

    // Initialize the stream and read the first 512 byte of the boot Sector into the structure
    stream_init(&boot_stream, disk);
    stream_seek(&boot_stream, partition_offset);
    stream_read(&boot_stream, (uint8_t *)&base_header, sizeof(FAT16BaseHeader));

    // Validate FAT16 header
    if (!(base_header.jmp_short[0] == 0xEB && base_header.jmp_short[2] == 0x90) ||
        base_header.bytes_per_sector != 512 ||
        base_header.sectors_per_cluster < 1 ||
        base_header.fat_table_copies != 2 ||
        base_header.media_desc_type == 0)
    {
        kprintf("Error: Invalid FAT16 header.\n");
        return -1;
    };
    kprintf("FAT16 Header\n");
    kprintf("jmp_short: %d %d %d\n", base_header.jmp_short[0], base_header.jmp_short[1], base_header.jmp_short[2]);
    kprintf("oem_ident: %s\n", base_header.oem_ident);
    kprintf("bytes_per_sector: %d\n", base_header.bytes_per_sector);
    kprintf("sectors_per_cluster: %d\n", base_header.sectors_per_cluster);
    kprintf("reserved_sectors: %d\n", base_header.reserved_sectors);
    kprintf("fat_table_copies: %d\n", base_header.fat_table_copies);
    kprintf("root_directories: %d\n", base_header.root_directories);
    kprintf("total_sectors: %d\n", base_header.total_sectors);
    kprintf("media_desc_type: %d\n", base_header.media_desc_type);
    kprintf("sectors_per_fat: %d\n", base_header.sectors_per_fat);
    kprintf("sectors_per_track: %d\n", base_header.sectors_per_track);
    kprintf("heads: %d\n", base_header.heads);
    kprintf("hidden_sectors: %d\n", base_header.hidden_sectors);
    kprintf("large_total_sectors: %d\n", base_header.large_total_sectors);
    kprintf("----------------------------------\n");

    FAT16ExtendedHeader ext_header;
    stream_read(&boot_stream, (uint8_t *)&ext_header, sizeof(FAT16ExtendedHeader));

    if (ext_header.signature != 0x29 || ext_header.drive_number != 0x80)
    {
        kprintf("Warning: Invalid or missing FAT16 Extended Header. Continuing without extended information.\n");
    };
    kprintf("FAT16 Extended Header\n");
    kprintf("drive_number: %d\n", ext_header.drive_number);
    kprintf("nt_flags: %d\n", ext_header.nt_flags);
    kprintf("signature: %d\n", ext_header.signature);
    kprintf("volume_id: %d\n", ext_header.volume_id);
    kprintf("volume_label: %s\n", ext_header.volume_label);
    kprintf("system_ident: %s\n", ext_header.system_ident);
    kprintf("----------------------------------\n");

    const uint32_t root_directory_offset = base_header.bytes_per_sector * (base_header.reserved_sectors + base_header.fat_table_copies * base_header.sectors_per_fat);
    const uint32_t root_directory_absolute = partition_offset + root_directory_offset;
    kprintf("root_directory_offset: 0x%x\n", root_directory_offset);
    kprintf("root_directory_absolute: 0x%x\n", root_directory_absolute);
    return 0;
};

void *fat16_open(ATADisk *disk, PathNode *path, const VNODE_MODE mode)
{
    return 0x0;
};