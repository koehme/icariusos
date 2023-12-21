/**
 * @file ata.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ata.h"
#include "io.h"
#include "mem.h"
#include "icarius.h"

ATADisk ata_disk_a = {
    .disk_type = 0,
    .sector_size = 0,
    .buffer = {},
};

/**
 * @brief Initializes the specified ATADisk instance by setting its
 * disk type to ATA_DISK_A, sector size to 512 bytes, and clearing its buffer.
 * @param self Pointer to the ATADisk instance to be initialized.
 */
void ata_init(ATADisk *self)
{
    self->disk_type = ATA_DISK_A;
    self->sector_size = ATA_SECTOR_SIZE;
    mset8(self->buffer, 0x0, sizeof(self->buffer));
    return;
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
static int ata_read_sector(const uint32_t lba, void *buffer, const size_t n_sectors)
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
    return 0;
};

/**
 * @brief Gets the ATADisk instance associated with the specified disk type.
 * Returns a pointer to the ATADisk instance corresponding to the
 * given disk type. If the specified disk type is not recognized, the function
 * returns a null pointer (NULL).
 * @param disk_type The type of ATA disk for which to retrieve the ATADisk instance.
 * @return
 *    - Returns a pointer to the ATADisk instance if the disk type is recognized.
 *    - Returns NULL if the disk type is not recognized.
 */
ATADisk *ata_get_disk(const ATADiskType disk_type)
{
    switch (disk_type)
    {
    case ATA_DISK_A:
    {
        return &ata_disk_a;
    };
    default:
    {
        break;
    };
    };
    return 0x0;
};

/**
 * @brief Reads data from an ATA disk into the specified buffer.
 * Initiates a read operation on the ATA disk by configuring
 * the necessary parameters such as the Logical Block Address (LBA), the number
 * of sectors to read, and the drive selection. The data is then read into
 * the provided buffer using ATA commands.
 * @param self Pointer to the ATADisk instance representing the ATA disk.
 * @param start_block The starting block (Logical Block Address) from which to read.
 * @param buffer A pointer to the buffer where the read data will be stored.
 * @param n_blocks The number of blocks (sectors) to read from the ATA disk.
 * @return
 *    - Returns 0 if the read operation is successful.
 *    - Returns -1 if the self parameter is NULL, indicating an invalid ATADisk instance.
 */
int ata_read(ATADisk *self, const size_t start_block, void *buffer, const size_t n_blocks)
{
    if (!self)
    {
        return -1;
    };
    const int res = ata_read_sector(start_block, buffer, n_blocks);
    return res;
};

/**
 * @brief Prints the content of the ATA buffer.
 * Prints the hexadecimal content of the ATA buffer
 * associated with the given ATADisk instance. It displays the content
 * in a readable format, showing each byte as a hexadecimal value.
 * @param self Pointer to the ATADisk instance whose buffer is to be printed.
 */
void ata_print_buffer(const ATADisk *self)
{
    kprint_color("ATA Buffer: \n", VGA_COLOR_LIGHT_GREEN);

    for (size_t i = 0; i < ATA_SECTOR_SIZE; ++i)
    {
        kprint_hex(self->buffer[i]);
        kprint(" ");
    };
    return;
};