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

#define ATA_DEBUG_DELAY 0

extern void asm_irq_14h(void);

ATADev ata_dev_0 = {
    .dev = 0x0,
    .sector_size = 0x0,
    .capacity = 0x0,
    .buffer = {},
    .fs = 0x0,
    .features = 0x0,
};

static void ata_read_into_buffer(uint16_t *buffer, const size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        buffer[i] = asm_inw(ATA_DATA_PORT);
    };
    return;
};

static uint8_t select_drive_ata_identify(const ATADriveType type)
{
    uint8_t drive_select = 0x0;

    switch (type)
    {
    case ATA_DRIVE_MASTER:
    {
        drive_select = 0xA0;
        break;
    };
    case ATA_DRIVE_SLAVE:
    {
        drive_select = 0xB0;
        break;
    }
    default:
    {
        break;
    };
    };
    return drive_select;
};

static void send_identify_cmd(const uint8_t target_drive)
{
    asm_outb(ATA_CONTROL_PORT, target_drive);
    asm_outb(ATA_SECTOR_COUNT_PORT, 0);
    asm_outb(ATA_LBA_LOW_PORT, 0);
    asm_outb(ATA_LBA_MID_PORT, 0);
    asm_outb(ATA_LBA_HIGH_PORT, 0);
    asm_outb(ATA_COMMAND_PORT, ATA_CMD_IDENTIFY);
    return;
};

static void ata_dump(ATADev *self, const int32_t delay)
{
    kprintf("Features: 0x%x\n", self->features);
    kprintf("Sector Size: %d\n", self->sector_size);
    kprintf("Total Sectors: %d\n", self->total_sectors);
    kprintf("Capacity: %d KiB\n", self->capacity / 1024);
    kprintf("Capacity: %d MiB\n", (self->capacity / 1024) / 1024);
    kprintf("Capacity: %d GiB\n", (((self->capacity / 1024) / 1024) / 1024));
    kdelay(delay);
    return;
};

// Set pio mode
static void ata_set_pio(ATADev *self, uint16_t *buffer)
{
    const uint16_t pio_flag = buffer[83];

    if (pio_flag & (1 << 10))
    {
        // PIO48 support
        const uint64_t high = (uint64_t)buffer[103] << 48;
        const uint64_t mid_high = (uint64_t)buffer[102] << 32;
        const uint64_t mid_low = (uint64_t)buffer[101] << 16;
        const uint64_t low = (uint64_t)buffer[100] << 0;
        const uint64_t total_sectors = high | mid_high | mid_low | low;
        const uint64_t capacity = total_sectors * self->sector_size;
        self->capacity = capacity;
        self->total_sectors = total_sectors;
        self->features |= (1 << 1);  // Set bit 1 to indicate pio48 support180268
        self->features &= ~(1 << 0); // Clear bit 0 to indicate no pio28 support
    }
    else
    {
        // Fallback PIO28
        const uint32_t high = (uint32_t)buffer[60] << 16;
        const uint32_t low = (uint32_t)buffer[61] << 0;
        const uint32_t total_sectors = high | low;
        const uint64_t capacity = total_sectors * self->sector_size;
        self->capacity = capacity;
        self->total_sectors = total_sectors;
        self->features |= (1 << 0);  // Bit 0 should set to 1 to indicate pio28 support
        self->features &= ~(1 << 1); // Clear bit 1 to indicate no pio48 support
    };
    return;
};

static void wait_until_not_busy(uint8_t *status)
{
    while (*status & ATA_STATUS_BSY)
    {
        *status = asm_inb(ATA_STATUS_REGISTER);
    };
    return;
};

static void wait_until_data_ready(uint8_t *status)
{
    while (!(*status & (ATA_STATUS_ERR | ATA_STATUS_DRQ)))
    {
        *status = asm_inb(ATA_STATUS_REGISTER);
    };
    return;
};

static int32_t ata_identify(ATADev *self, const ATADriveType type)
{
    const uint8_t target_drive = select_drive_ata_identify(type);
    send_identify_cmd(target_drive);
    // Read the status ports
    uint8_t status = asm_inb(ATA_STATUS_REGISTER);
    wait_until_not_busy(&status);
    // If it's zero, the drive does not exist
    if (status == 0)
    {
        return -EIO;
    };
    wait_until_not_busy(&status);
    const uint8_t lba_mid_byte = asm_inb(ATA_LBA_MID_PORT);
    const uint8_t lba_high_byte = asm_inb(ATA_LBA_HIGH_PORT);

    if (lba_mid_byte != 0 || lba_high_byte != 0)
    {
        return -EIO;
    };
    wait_until_data_ready(&status);
    // There was an error on the drive. Forget about it!
    if (status & ATA_STATUS_ERR)
    {
        return -EIO;
    };
    uint16_t buffer[256] = {};
    ata_read_into_buffer(buffer, self->sector_size / 2);

    ata_set_pio(self, buffer);

    ata_dump(self, ATA_DEBUG_DELAY);
    return 1;
};

// Initializes the ATA device
void ata_init(ATADev *self)
{
    idt_set(0x2E, asm_irq_14h);
    self->dev = ATA_DEV_0;
    self->sector_size = ATA_SECTOR_SIZE;
    self->total_sectors = 0x0;
    self->capacity = 0x0;
    self->features = 0b00000000;
    mset8(self->buffer, 0x0, sizeof(self->buffer));

    if (!ata_identify(self, ATA_DRIVE_MASTER))
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

static int32_t ata_read_pio_48(ATADev *self, const uint64_t lba, const uint16_t sectors)
{
    uint16_t *ptr_ata_buffer = (uint16_t *)self->buffer;

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
        ata_read_into_buffer(ptr_ata_buffer, self->sector_size / 2);
    };
    return 0;
};

static int32_t ata_read_pio_28(ATADev *self, const uint32_t lba, const uint8_t sectors)
{
    uint16_t *ptr_ata_buffer = (uint16_t *)self->buffer;
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
        ata_read_into_buffer(ptr_ata_buffer, self->sector_size / 2);
    };
    return 0;
};

// Gets the ATADev instance associated with the specified disk type
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

// Reads data from an ATA disk into the specified buffer
int32_t ata_read(ATADev *self, const size_t start_block, const size_t n_blocks)
{
    if (!self)
    {
        return -EIO;
    };
    const bool has_pio48_support = self->features & (1 << 1);

    if (has_pio48_support)
    {
        return ata_read_pio_48(self, start_block, n_blocks);
    };
    return ata_read_pio_28(self, start_block, n_blocks);
};