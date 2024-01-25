/**
 * @file ata.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef ATA_H
#define ATA_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct Superblock Superblock;

typedef enum ATADiskType
{
    ATA_DISK_A = 0,
    ATA_DISK_B = 1,
    ATA_DISK_C = 2,
    ATA_SECTOR_SIZE = 512,
} ATADiskType;

typedef enum ATADrives
{
    ATA_DRIVE_MASTER = 0xe0
} ATADrives;

typedef enum ATACommands
{
    ATA_CMD_READ_SECTORS = 0x20,
} ATACommands;

typedef enum ATAStatus
{
    ATA_STATUS_BSY = 7,
    ATA_STATUS_DRQ = 3, // Data Request Bit
    ATA_STATUS_DF = 5,
    ATA_STATUS_ERR = 0, // Error Bit
} ATAStatus;

typedef enum ATAPorts
{
    ATA_DATA_PORT = 0x1f0,
    ATA_FEATURES_PORT = 0x1f1,
    ATA_SECTOR_COUNT_PORT = 0x1f2,
    ATA_LBA_LOW_PORT = 0x1f3,
    ATA_LBA_MID_PORT = 0x1f4,
    ATA_LBA_HIGH_PORT = 0x1f5,
    ATA_CONTROL_PORT = 0x1f6,
    ATA_COMMAND_PORT = 0x1f7,
    ATA_STATUS_REGISTER = 0x1f7,
} ATAPorts;

/**
 * @brief Represents an ATA Disk.
 */
typedef struct ATADisk
{
    ATADiskType disk_type; // Type of the ATA Disk
    int sector_size;       // Size of each sector in bytes
    uint8_t buffer[512];   // Data buffer for temporary storage
    Superblock *fs;
} ATADisk;

void ata_init(ATADisk *self);
void ata_search_fs(ATADisk *self);
ATADisk *ata_get_disk(const ATADiskType disk_type);
int ata_read(ATADisk *self, const size_t start_block, const size_t n_blocks, const bool sync);

#endif