/**
 * @file ata.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef ATA_H
#define ATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct fs fs_t;

// ATA Limits
#define ATA_SECTOR_SIZE 512
// ATA Drive Types
#define ATA_DRIVE_MASTER 0xE0
#define ATA_DRIVE_SLAVE 0xF0
#define DRIVE_SELECT_MASTER 0xA0
#define DRIVE_SELECT_SLAVE 0xB0
#define DRIVE_SELECT_INVALID 0x0
// ATA Commands
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_IDENTIFY 0xEC
// ATA Status Flags
#define ATA_STATUS_ERR (1 << 0) // Indicates an error occurred. Send a new command to clear it
#define ATA_STATUS_NIEN (1 << 1)
#define ATA_STATUS_DRQ (1 << 3) // Set when the drive has PIO data to transfer or is ready to accept PIO data
#define ATA_STATUS_SRV (1 << 4) // Overlapped Mode Service Request
#define ATA_STATUS_DF (1 << 5)	// Drive Fault Error (does not set ERR)
#define ATA_STATUS_RDY (1 << 6) // Bit is clear when drive is spun down, or after an error. Set otherwise
#define ATA_STATUS_BSY (1 << 7) // Indicates the drive is preparing to send/receive data (wait for it to clear)
// ATA Ports
#define ATA_DATA_PORT 0x1F0
#define ATA_FEATURES_PORT 0x1F1
#define ATA_PRIMARY_ERROR 0x1F1
#define ATA_SECTOR_COUNT_PORT 0x1F2
#define ATA_LBA_LOW_PORT 0x1F3
#define ATA_LBA_MID_PORT 0x1F4
#define ATA_LBA_HIGH_PORT 0x1F5
#define ATA_CONTROL_PORT 0x1F6
#define ATA_COMMAND_PORT 0x1F7
#define ATA_STATUS_REGISTER 0x1F7

typedef struct ata {
	char dev[2];		// Type of the ATA device
	uint16_t sector_size;	// Size of a sector in bytes
	uint64_t total_sectors; // Total available sectors
	uint64_t capacity;	// Available device capacity in bytes
	uint8_t buffer[512];	// Data buffer for temporary storage
	fs_t* fs;		// fs_t mapped to the disk
	uint8_t features;
} ata_t;

void ata_init(ata_t* self);
void ata_mount_fs(ata_t* self);
ata_t* ata_get(const char dev[2]);
int32_t ata_read(ata_t* self, const size_t start_block, const size_t n_blocks);

#endif