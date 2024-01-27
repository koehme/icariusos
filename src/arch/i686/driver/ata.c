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

#define ATA_DEBUG_DELAY 50000

extern void asm_irq_14h(void);

ATADev ata_dev_0 = {
    .dev = 0x0,
    .sector_size = 0x0,
    .capacity = 0x0,
    .buffer = {},
    .fs = 0x0,
    .features = 0x0,
};

static int32_t ata_identify(ATADev *self)
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
        // PIO48 support
        const uint64_t total_sectors = ((uint64_t)buffer[103] << 48) |
                                       ((uint64_t)buffer[102] << 32) |
                                       ((uint64_t)buffer[101] << 16) |
                                       buffer[100];
        const uint64_t capacity = total_sectors * self->sector_size;
        self->capacity = capacity;
        self->total_sectors = total_sectors;
        self->features |= (1 << 1);
    }
    else
    {
        // Fallback PIO28
        const uint32_t total_sectors = ((uint32_t)buffer[60] << 16) |
                                       buffer[61];
        const uint64_t capacity = total_sectors * self->sector_size;
        self->capacity = capacity;
        self->total_sectors = total_sectors;
        self->features |= (1 << 0);
    };
    kprintf("Features: 0x%x\n", self->features);
    kprintf("Sector Size: %d\n", self->sector_size);
    kprintf("Total Sectors: %d\n", self->total_sectors);
    kprintf("Capacity: %d KiB\n", self->capacity / 1024);
    kprintf("Capacity: %d MiB\n", (self->capacity / 1024) / 1024);
    return 1;
};

/**
 * @brief Initializes the specified ATADisk instance by setting its
 * disk type to ATA_DISK_A, sector size to 512 bytes, and clearing its buffer.
 * @param self Pointer to the ATADisk instance to be initialized.
 */
void ata_init(ATADev *self)
{
    idt_set(0x2E, asm_irq_14h);
    self->dev = ATA_DEV_0;
    self->sector_size = ATA_SECTOR_SIZE;
    self->total_sectors = 0x0;
    self->capacity = 0x0;
    self->features = 0b00000000;
    mset8(self->buffer, 0x0, sizeof(self->buffer));

    if (!ata_identify(self))
    {
        kpanic("ATA Error. Failed to identify.\n");
    };
    return;
};

void ata_search_fs(ATADev *self)
{
    self->fs = vfs_resolve(self);
    return;
};

static void ata_transfer(volatile uint16_t *buffer, const size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        buffer[i] = asm_inw(ATA_DATA_PORT);
    };
    return;
};

static int ata_read_pio_48(ATADev *self, uint64_t lba, const uint16_t sectors)
{
    volatile uint16_t *ptr_ata_buffer = (volatile uint16_t *)self->buffer;

    asm_outb(ATA_CONTROL_PORT, 0x40);                       // Select master
    asm_outb(ATA_SECTOR_COUNT_PORT, (sectors >> 8) & 0xFF); // sectors high

    asm_outb(ATA_LBA_LOW_PORT, (lba >> 24) & 0xFF);  // LBA4
    asm_outb(ATA_LBA_MID_PORT, (lba >> 32) & 0xFF);  // LBA5
    asm_outb(ATA_LBA_HIGH_PORT, (lba >> 40) & 0xFF); // LBA6

    asm_outb(ATA_SECTOR_COUNT_PORT, sectors & 0xFF); // sectors low

    asm_outb(ATA_LBA_LOW_PORT, lba & 0xFF);          // LBA1
    asm_outb(ATA_LBA_MID_PORT, (lba >> 8) & 0xFF);   // LBA2
    asm_outb(ATA_LBA_HIGH_PORT, (lba >> 16) & 0xFF); // LBA3

    asm_outb(ATA_STATUS_REGISTER, 0x24);

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
        // Copy from ata controller into buffer
        ata_transfer(ptr_ata_buffer, self->sector_size / 2);
    };
    return 0;
};

static int ata_read_pio_28(ATADev *self, uint32_t lba, const uint8_t sectors)
{
    volatile uint16_t *ptr_ata_buffer = (volatile uint16_t *)self->buffer;
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
        // Copy from ata controller into buffer
        ata_transfer(ptr_ata_buffer, self->sector_size / 2);
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
ATADev *ata_get(const ATADeviceType dev)
{
    switch (dev)
    {
    case ATA_DEV_0:
    {
        return &ata_dev_0;
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
 * @param n_blocks The number of blocks (sectors) to read from the ATA disk.
 * @return
 *    - Returns 0 if the read operation is successful.
 *    - Returns -1 if the self parameter is NULL, indicating an invalid ATADisk instance.
 */
int ata_read(ATADev *self, const size_t start_block, const size_t n_blocks)
{
    if (!self)
    {
        return -EIO;
    };
    const bool has_pio48 = self->features & (1 << 1);

    if (has_pio48)
    {
        return ata_read_pio_48(self, start_block, n_blocks);
    };
    return ata_read_pio_28(self, start_block, n_blocks);
};