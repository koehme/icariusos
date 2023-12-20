/**
 * @file ata.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ata.h"
#include "io.h"
#include "mem.h"
#include "icarius.h"

ATADisk ata_disk = {
    .buffer = {},
    .buffer_size = 256,
};

/**
 * @brief Reads data from an ATA disk into the specified buffer.
 * Initiates a read operation on the ATA disk by configuring
 * the necessary parameters such as the Logical Block Address (LBA), the number
 * of sectors to read, and the drive selection. The data is then read into
 * the provided buffer using ATA commands.
 * @param lba The Logical Block Address (LBA) specifying the starting sector to read from.
 * @param buffer A pointer to the buffer where the read data will be stored.
 * @param n_sectors The number of sectors to read from the ATA disk.
 */
void ata_read(const uint32_t lba, void *buffer, const size_t n_sectors)
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
    return;
};

void ata_print_buffer(const ATADisk *disk)
{
    kprint_color("ATA Buffer Content: \n", VGA_COLOR_LIGHT_GREEN);

    for (size_t i = 0; i < disk->buffer_size; ++i)
    {
        kprint_hex(disk->buffer[i]);
        kprint(" ");
    };
    return;
};