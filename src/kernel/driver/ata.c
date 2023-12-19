/**
 * @file ata.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ata.h"
#include "io.h"
#include "mem.h"

ATADisk ata_disk = {
    .buffer = {},
    .has_read = false,
};

void ata_read_sectors(const uint32_t lba, void *buffer, const size_t n_sectors)
{
    // Select master drive and pass part of the lba
    asm_outb(ATA_CONTROL_PORT, (lba >> 24) | ATA_DRIVE_MASTER);
    // Send the total number of sectors we want to read
    asm_outb(ATA_SECTOR_COUNT_PORT, n_sectors);
    // Send more of the LBA
    asm_outb(ATA_LBA_LOW_PORT, lba & 0b11111111);
    asm_outb(ATA_LBA_MID_PORT, (lba >> 8) & 0b11111111);
    asm_outb(ATA_LBA_HIGH_PORT, (lba >> 16) & 0b11111111);
    // Send read command to ATA_COMMAND_PORT
    asm_outb(ATA_COMMAND_PORT, ATA_CMD_READ_SECTORS);
    // TODO
    ata_disk.has_read = false;
    return;
};
