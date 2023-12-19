/**
 * @file ata.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ata.h"
#include "io.h"

void ata_read_sectors(const uint32_t lba, uint8_t *buffer, const size_t n_sectors)
{
    // Select master drive and pass part of the lba
    asm_outb(ATA_CONTROL_PORT, (lba >> 24) | ATA_DRIVE_MASTER);
    // Send the total number of sectors we want to read
    asm_outb(ATA_SECTOR_COUNT_PORT, n_sectors);
    // Send more of the LBA
    // Send read command to ATA_COMMAND_PORT
    return;
};
