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
    .dev = 0,
    .lba48 = false,
};

static int32_t ata_identify(ATADisk *self)
{
    // ATA detect commands
    asm_outb(ATA_CONTROL_PORT, 0xA0);
    asm_outb(ATA_SECTOR_COUNT_PORT, 0);
    asm_outb(ATA_LBA_LOW_PORT, 0);
    asm_outb(ATA_LBA_MID_PORT, 0);
    asm_outb(ATA_LBA_HIGH_PORT, 0);
    asm_outb(ATA_COMMAND_PORT, ATA_CMD_IDENTIFY);

    // Read the status ports
    uint8_t status = asm_inb(ATA_STATUS_REGISTER);

    while (status & ATA_STATUS_BSY)
    {
        status = asm_inb(ATA_STATUS_REGISTER);
    };
    // If it's zero, the drive does not exist
    if (status == 0)
    {
        return -EIO;
    };
    // Status indicates presence of a drive. Polling while STAT_BSY...
    while (status & ATA_STATUS_BSY)
    {
        status = asm_inb(ATA_STATUS_REGISTER);
    };
    const uint8_t mid_byte = asm_inb(ATA_LBA_MID_PORT);
    const uint8_t high_byte = asm_inb(ATA_LBA_HIGH_PORT);

    if (mid_byte || high_byte)
    {
        // The drive is not ATA
        return -EIO;
    };

    while (!(status & (ATA_STATUS_ERR | ATA_STATUS_DRQ)))
    {
        status = asm_inb(ATA_STATUS_REGISTER);
    };

    if (status & ATA_STATUS_ERR)
    {
        // There was an error on the drive. Forget about it.
        return -EIO;
    };
    uint16_t buffer[256] = {};

    for (size_t i = 0; i < self->sector_size / 2; i++)
    {
        buffer[i] = asm_inw(ATA_DATA_PORT);
    };
    const uint16_t pio48 = buffer[83];

    if (pio48 & (1 << 10))
    {
        kprintf("Disk => 48-Bit-PIO.\n");
        self->lba48 = true;
    };
    const uint64_t total_lba48_sectors = ((uint64_t)buffer[103] << 48) |
                                         ((uint64_t)buffer[102] << 32) |
                                         ((uint64_t)buffer[101] << 16) |
                                         buffer[100];

    kprintf("%d\n", total_lba48_sectors);
    const uint64_t sector_size = 512;
    const uint64_t max_disk_size_bytes = total_lba48_sectors * sector_size;

    // Drucken Sie die maximale Größe der Festplatte
    kprintf("Bytes: %d \n", max_disk_size_bytes);
    kprintf("KBytes: %d \n", max_disk_size_bytes / 1024);
    kprintf("MBytes: %d \n", (max_disk_size_bytes / 1024) / 1024);
    kdelay(500000);
    return 1;
};

/**
 * @brief Initializes the specified ATADisk instance by setting its
 * disk type to ATA_DISK_A, sector size to 512 bytes, and clearing its buffer.
 * @param self Pointer to the ATADisk instance to be initialized.
 */
void ata_init(ATADisk *self)
{
    idt_set(0x2E, asm_irq_14h);
    self->disk_type = ATA_DISK_A;
    self->sector_size = ATA_SECTOR_SIZE;
    self->lba48 = false;
    mset8(self->buffer, 0x0, sizeof(self->buffer));

    if (!ata_identify(self))
    {
        kpanic("Disk Error. IDENTIFY failed.\n");
        return;
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