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

#include "icarius.h"

typedef struct fs fs_t;

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
int32_t ata_write(ata_t* self, const size_t start_block, const size_t n_blocks, const uint8_t* buffer);

#endif