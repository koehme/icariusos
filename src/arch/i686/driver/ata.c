/**
 * @file ata.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ata.h"
#include "io.h"
#include "mem.h"
#include "kernel.h"
#include "string.h"

extern void asm_irq_14h(void);

ATADisk ata_disk = {
    .disk_type = 0,
    .sector_size = 0,
    .buffer = {},
    .fs = 0x0,
};

/**
 * @brief Initializes the specified ATADisk instance by setting its
 * disk type to ATA_DISK_A, sector size to 512 bytes, and clearing its buffer.
 * @param self Pointer to the ATADisk instance to be initialized.
 */
void ata_init(ATADisk *self)
{
    idt_set(0x2e, asm_irq_14h);
    self->disk_type = ATA_DISK_A;
    self->sector_size = ATA_SECTOR_SIZE;
    mset8(self->buffer, 0x0, sizeof(self->buffer));

    while ((asm_inb(ATA_STATUS_REGISTER) & 0x80) != 0)
    {
    };
    return;
};

void ata_search_fs(ATADisk *self)
{
    self->fs = vfs_resolve(self);
    return;
};

static int ata_read_synchronous(uint32_t lba, const uint8_t sectors)
{
    ATADisk *ptr_ata_disk = &ata_disk;
    volatile uint16_t *ptr_ata_buffer = (volatile uint16_t *)ptr_ata_disk->buffer;

    asm_outb(ATA_CONTROL_PORT, ATA_DRIVE_MASTER | ((lba >> 24) & 0x0F));
    asm_outb(ATA_PRIMARY_ERROR, 0x00);
    asm_outb(ATA_SECTOR_COUNT_PORT, sectors);
    asm_outb(ATA_LBA_LOW_PORT, lba & 0xFF);
    asm_outb(ATA_LBA_MID_PORT, (lba >> 8) & 0xFF);
    asm_outb(ATA_LBA_HIGH_PORT, (lba >> 16) & 0xFF);
    asm_outb(ATA_COMMAND_PORT, ATA_CMD_READ_SECTORS);

    for (size_t i = 0; i < sectors; ++i)
    {
        for (;;)
        {
            const uint8_t status = asm_inb(ATA_COMMAND_PORT);

            if (status & ATA_STATUS_DRQ)
            {
                break;
            };

            if ((status & ATA_STATUS_ERR) || (status & ATA_STATUS_DF))
            {
                kprintf("ATA: Read Error on LBA %u\n", lba);
                return -EIO;
            };
        };

        for (size_t j = 0; j < ptr_ata_disk->sector_size / 2; j++)
        {
            // Copy from disk into buffer
            *ptr_ata_buffer = asm_inw(ATA_DATA_PORT);
            ptr_ata_buffer++;
        };
    };
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
        return &ata_disk;
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
int ata_read(ATADisk *self, const size_t start_block, const size_t n_blocks)
{
    if (!self)
    {
        return -EIO;
    };
    return ata_read_synchronous(start_block, n_blocks);
};