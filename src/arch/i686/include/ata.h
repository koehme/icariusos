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

typedef enum ATA_ATTRIBUTES
{
    ATA_SECTOR_SIZE = 512,
} ATA_ATTRIBUTES;

typedef enum ATADeviceType
{
    ATA_DEV_0 = 0x0,
} ATADeviceType;

typedef enum ATADriveType
{
    ATA_DRIVE_MASTER = 0xE0,
    ATA_DRIVE_SLAVE = 0xF0,
} ATADriveType;

typedef enum ATACommands
{
    ATA_CMD_READ_SECTORS = 0x20,
    ATA_CMD_IDENTIFY = 0xEC,
} ATACommands;

typedef enum ATAStatus
{
    ATA_STATUS_ERR = (1 << 0), // Indicates an error occurred. Send a new command to clear it
    ATA_STATUS_NIEN = (1 << 1),
    ATA_STATUS_DRQ = (1 << 3), // Set when the drive has PIO data to transfer or is ready to accept PIO data
    ATA_STATUS_SRV = (1 << 4), // Overlapped Mode Service Request
    ATA_STATUS_DF = (1 << 5),  // Drive Fault Error (does not set ERR)
    ATA_STATUS_RDY = (1 << 6), // Bit is clear when drive is spun down, or after an error. Set otherwise
    ATA_STATUS_BSY = (1 << 7), // Indicates the drive is preparing to send/receive data (wait for it to clear)
} ATAStatus;

typedef enum ATAPorts
{
    ATA_DATA_PORT = 0x1F0,
    ATA_FEATURES_PORT = 0x1F1,
    ATA_PRIMARY_ERROR = 0x1F1,
    ATA_SECTOR_COUNT_PORT = 0x1F2,
    ATA_LBA_LOW_PORT = 0x1F3,
    ATA_LBA_MID_PORT = 0x1F4,
    ATA_LBA_HIGH_PORT = 0x1F5,
    ATA_CONTROL_PORT = 0x1F6,
    ATA_COMMAND_PORT = 0x1F7,
    ATA_STATUS_REGISTER = 0x1F7,
} ATAPorts;

/**
 * @brief Represents an ATA Disk.
 */
typedef struct ATADev
{
    ATADeviceType dev;      // Type of the ATA device
    uint16_t sector_size;   // Size of an sector in bytes
    uint64_t total_sectors; // Total available sectors
    uint64_t capacity;      // Available device capacity in bytes
    uint8_t buffer[512];    // Data buffer for temporary storage
    Superblock *fs;         // Filesystem mapped to the disk´
    uint8_t features;       // Bitmasks for ata device features if bit 0 is set pio28 is available. bit 1 is for pio48
} ATADev;

void ata_init(ATADev *self);
void ata_search_fs(ATADev *self);
ATADev *ata_get(const ATADeviceType dev);
int ata_read(ATADev *self, const size_t start_block, const size_t n_blocks);

#endif