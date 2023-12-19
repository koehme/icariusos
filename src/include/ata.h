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
} ATAPorts;

typedef struct ATADisk
{
    uint8_t buffer[512];
    bool has_read;
} ATADisk;

void ata_read_sectors(const uint32_t lba, void *buffer, const size_t n_sectors);

#endif