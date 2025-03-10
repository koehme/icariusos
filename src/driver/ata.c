/**
 * @file ata.c
 * @author Kevin Oehme
 * @copyright MIT
 * @date 2024-11-15
 */

#include "ata.h"
#include "io.h"
#include "kernel.h"
#include "string.h"

#define ATA_DEBUG_DELAY 0

ata_t ata_dev = {
    .dev = {"A"},
    .sector_size = 0x0,
    .capacity = 0x0,
    .buffer = {0},
    .fs = 0x0,
    .features = 0x0,
};

/* PUBLIC API */
void ata_init(ata_t* self);
void ata_mount_fs(ata_t* self);
ata_t* ata_get(const char dev[2]);
int32_t ata_read(ata_t* self, const size_t start_block, const size_t n_blocks);
int32_t ata_write(ata_t* self, const size_t start_block, const size_t n_blocks, const uint8_t* buffer);

/* INTERNAL API */
static void _load_into_buffer(uint16_t* buffer, const size_t size);
static uint8_t _get_drive_select_code(const uint8_t type);
static void _send_identify_req(const uint8_t target_drive);
static void _dump_ata(ata_t* self, const int32_t delay);
static void _detect_pio_mode(ata_t* self, uint16_t* buffer);
static void _check_bsy(uint8_t* status);
static void _check_drq_err(uint8_t* status);
static int32_t _init_identify(ata_t* self, const uint8_t type);
static int32_t _read_pio48(ata_t* self, const uint64_t lba, const uint16_t sectors);
static int32_t _read_pio28(ata_t* self, const uint32_t lba, const uint8_t sectors);
static uint64_t _calculate_total_sectors_pio48(uint16_t* buffer);
static uint64_t _calculate_total_sectors_pio28(uint16_t* buffer);
static inline uint64_t _calculate_capacity(const uint64_t total_sectors, const uint16_t sector_size);
static void set_pio_features(ata_t* self, const bool is_pio48);
static int32_t _write_pio28(ata_t* self, const uint32_t lba, const uint8_t sectors, const uint8_t* buffer);
static int32_t _write_pio48(ata_t* self, const uint64_t lba, const uint16_t sectors, const uint8_t* buffer);

static void _load_into_buffer(uint16_t* buffer, const size_t size)
{
	for (size_t i = 0; i < size; i++) {
		buffer[i] = inw(ATA_DATA_PORT);
	};
	return;
};

static uint8_t _get_drive_select_code(const uint8_t type)
{
	switch (type) {
	case ATA_DRIVE_MASTER:
		return DRIVE_SELECT_MASTER;
	case ATA_DRIVE_SLAVE:
		return DRIVE_SELECT_SLAVE;
	default:
		break;
	};
	return DRIVE_SELECT_INVALID;
};

static void _send_identify_req(const uint8_t target_drive)
{
	outb(ATA_CONTROL_PORT, target_drive);
	outb(ATA_SECTOR_COUNT_PORT, 0);
	outb(ATA_LBA_LOW_PORT, 0);
	outb(ATA_LBA_MID_PORT, 0);
	outb(ATA_LBA_HIGH_PORT, 0);
	outb(ATA_COMMAND_PORT, ATA_CMD_IDENTIFY);
	return;
};

static void _dump_ata(ata_t* self, const int32_t delay)
{
	printf("[INFO] ATA\n");
	printf("Sector Size: %d\n", self->sector_size);
	printf("Total Sectors: %d\n", self->total_sectors);
	printf("Capacity: %d KiB\n", self->capacity / 1024);
	printf("Capacity: %d MiB\n", (self->capacity / 1024) / 1024);
	printf("Capacity: %f GiB\n", ((double)self->capacity) / 1024 / 1024 / 1024);
	busy_wait(delay);
	return;
};

static uint64_t _calculate_total_sectors_pio48(uint16_t* buffer)
{
	const uint64_t high = (uint64_t)buffer[103] << 48;
	const uint64_t mid_high = (uint64_t)buffer[102] << 32;
	const uint64_t mid_low = (uint64_t)buffer[101] << 16;
	const uint64_t low = (uint64_t)buffer[100] << 0;
	return high | mid_high | mid_low | low;
};

static uint64_t _calculate_total_sectors_pio28(uint16_t* buffer)
{
	const uint32_t high = (uint32_t)buffer[60] << 16;
	const uint32_t low = (uint32_t)buffer[61] << 0;
	return high | low;
};

static inline uint64_t _calculate_capacity(const uint64_t total_sectors, const uint16_t sector_size) { return total_sectors * sector_size; }

static void set_pio_features(ata_t* self, const bool is_pio48)
{
	if (is_pio48) {
		self->features |= (1 << 1);  // Set bit 1 to indicate PIO48 support
		self->features &= ~(1 << 0); // Clear bit 0 to indicate no PIO28 support
	} else {
		self->features |= (1 << 0);  // Set bit 0 to indicate PIO28 support
		self->features &= ~(1 << 1); // Clear bit 1 to indicate no PIO48 support
	};
	return;
};

static void _detect_pio_mode(ata_t* self, uint16_t* buffer)
{
	const uint16_t pio_flag = buffer[83];

	if (pio_flag & (1 << 10)) {
		// PIO48 support
		const uint64_t total_sectors = _calculate_total_sectors_pio48(buffer);
		const uint64_t capacity = _calculate_capacity(total_sectors, self->sector_size);
		self->capacity = capacity;
		self->total_sectors = total_sectors;
		set_pio_features(self, true);
	} else {
		// Fallback PIO28
		const uint64_t total_sectors = _calculate_total_sectors_pio28(buffer);
		const uint64_t capacity = _calculate_capacity(total_sectors, self->sector_size);
		self->capacity = capacity;
		self->total_sectors = total_sectors;
		set_pio_features(self, false);
	};
	return;
};

static void _check_bsy(uint8_t* status)
{
	while (*status & ATA_STATUS_BSY) {
		*status = inb(ATA_STATUS_REGISTER);
	};
	return;
};

static void _check_drq_err(uint8_t* status)
{
	while (!(*status & (ATA_STATUS_ERR | ATA_STATUS_DRQ))) {
		*status = inb(ATA_STATUS_REGISTER);
	};
	return;
};

static int32_t _init_identify(ata_t* self, const uint8_t type)
{
	const uint8_t target_drive = _get_drive_select_code(type);
	_send_identify_req(target_drive);
	uint8_t status = inb(ATA_STATUS_REGISTER);
	_check_bsy(&status);

	if (status == 0) {
		return -EIO;
	};
	_check_bsy(&status);
	const uint8_t lba_mid_byte = inb(ATA_LBA_MID_PORT);
	const uint8_t lba_high_byte = inb(ATA_LBA_HIGH_PORT);

	if (lba_mid_byte != 0 || lba_high_byte != 0) {
		return -EIO;
	};
	_check_drq_err(&status);

	if (status & ATA_STATUS_ERR) {
		return -EIO;
	};
	uint16_t buffer[256] = {};
	_load_into_buffer(buffer, self->sector_size / 2);
	_detect_pio_mode(self, buffer);
	_dump_ata(self, ATA_DEBUG_DELAY);
	return 1;
};

// Initializes the default ATA device
void ata_init(ata_t* self)
{
	memcpy(self->dev, "A", sizeof(char) * 2);
	self->sector_size = ATA_SECTOR_SIZE;
	self->total_sectors = 0x0;
	self->capacity = 0x0;
	self->features = 0x0;
	memset(self->buffer, 0x0, sizeof(self->buffer));

	if (!_init_identify(self, ATA_DRIVE_MASTER)) {
		panic("[CRITICAL] Failed to Identify ATA.\n");
	};
	return;
};

void ata_mount_fs(ata_t* self)
{
	self->fs = vfs_resolve(self);
	return;
};

static int32_t _read_pio48(ata_t* self, const uint64_t lba, const uint16_t sectors)
{
	uint16_t* ptr_ata_buffer = (uint16_t*)self->buffer;

	outb(ATA_CONTROL_PORT, 0x40);			    // Select master
	outb(ATA_SECTOR_COUNT_PORT, (sectors >> 8) & 0xFF); // sectors high
	outb(ATA_LBA_LOW_PORT, (lba >> 24) & 0xFF);	    // LBA4
	outb(ATA_LBA_MID_PORT, (lba >> 32) & 0xFF);	    // LBA5
	outb(ATA_LBA_HIGH_PORT, (lba >> 40) & 0xFF);	    // LBA6
	outb(ATA_SECTOR_COUNT_PORT, sectors & 0xFF);	    // sectors low
	outb(ATA_LBA_LOW_PORT, lba & 0xFF);		    // LBA1
	outb(ATA_LBA_MID_PORT, (lba >> 8) & 0xFF);	    // LBA2
	outb(ATA_LBA_HIGH_PORT, (lba >> 16) & 0xFF);	    // LBA3
	outb(ATA_STATUS_REGISTER, 0x24);

	for (size_t i = 0; i < sectors; ++i) {
		for (;;) {
			const uint8_t status = inb(ATA_COMMAND_PORT);

			if (status & ATA_STATUS_DRQ) {
				break;
			};

			if ((status & ATA_STATUS_ERR) || (status & ATA_STATUS_DF)) {
				printf("[CRITICAL] ATA Read Error on LBA %d\n", lba);
				return -EIO;
			};
		};
		_load_into_buffer(ptr_ata_buffer, self->sector_size / 2);
	};
	return 0;
};

static int32_t _read_pio28(ata_t* self, const uint32_t lba, const uint8_t sectors)
{
	uint16_t* ptr_ata_buffer = (uint16_t*)self->buffer;
	outb(ATA_CONTROL_PORT, ATA_DRIVE_MASTER | ((lba >> 24) & 0x0F));
	outb(ATA_PRIMARY_ERROR, 0x00);
	outb(ATA_SECTOR_COUNT_PORT, sectors);
	outb(ATA_LBA_LOW_PORT, lba & 0xFF);
	outb(ATA_LBA_MID_PORT, (lba >> 8) & 0xFF);
	outb(ATA_LBA_HIGH_PORT, (lba >> 16) & 0xFF);
	outb(ATA_COMMAND_PORT, ATA_CMD_READ_SECTORS);

	for (size_t i = 0; i < sectors; ++i) {
		for (;;) {
			const uint8_t status = inb(ATA_COMMAND_PORT);

			if (status & ATA_STATUS_DRQ) {
				break;
			};

			if ((status & ATA_STATUS_ERR) || (status & ATA_STATUS_DF)) {
				printf("[CRITICAL] ATA Read Error on LBA %u\n", lba);
				return -EIO;
			};
		};
		_load_into_buffer(ptr_ata_buffer, self->sector_size / 2);
	};
	return 0;
};

ata_t* ata_get(const char dev[2])
{
	if (scmp(dev, "A")) {
		return &ata_dev;
	};
	return 0x0;
};

int32_t ata_read(ata_t* self, const size_t start_block, const size_t n_blocks)
{
	if (!self) {
		return -EIO;
	};
	const bool has_pio48 = self->features & (1 << 1);

	if (has_pio48) {
		return _read_pio48(self, start_block, n_blocks);
	};
	return _read_pio28(self, start_block, n_blocks);
};

int32_t ata_write(ata_t* self, const size_t start_block, const size_t n_blocks, const uint8_t* buffer)
{
	if (!self || !buffer) {
		return -EIO;
	};
	const bool has_pio48 = self->features & (1 << 1);

	if (has_pio48) {
		return _write_pio48(self, start_block, n_blocks, buffer);
	};
	return _write_pio28(self, start_block, n_blocks, buffer);
};

static int32_t _write_pio28(ata_t* self, const uint32_t lba, const uint8_t sectors, const uint8_t* buffer)
{
	if (!self || !buffer) {
		return -EIO;
	};
	// Select master/slave and set LBA mode
	outb(ATA_CONTROL_PORT, ATA_DRIVE_MASTER | ((lba >> 24) & 0x0F));
	// Set number of sectors & LBA address (28-bit)
	outb(ATA_SECTOR_COUNT_PORT, sectors);
	outb(ATA_LBA_LOW_PORT, lba & 0xFF);
	outb(ATA_LBA_MID_PORT, (lba >> 8) & 0xFF);
	outb(ATA_LBA_HIGH_PORT, (lba >> 16) & 0xFF);
	// Send “WRITE SECTORS” command (0x30)
	outb(ATA_COMMAND_PORT, ATA_CMD_WRITE_SECTORS);

	for (size_t i = 0; i < sectors; ++i) {
		// Wait for DRQ (Data Request)
		while (!(inb(ATA_COMMAND_PORT) & ATA_STATUS_DRQ))
			;
		// Write 512 bytes with delay between `outw()` and `outw()`
		for (size_t j = 0; j < 256; j++) {
			outw(ATA_DATA_PORT, ((uint16_t*)buffer)[j]);
			__asm__ __volatile__("jmp 1f\n1: jmp 1f\n1:");
		};
		buffer += 512;
	};
	// Cache flush (send ATA command `0xE7`)
	outb(ATA_COMMAND_PORT, ATA_CMD_CACHE_FLUSH);
	// Wait for BSY (Busy Flag) → Confirms that the flush is complete
	while (inb(ATA_COMMAND_PORT) & ATA_STATUS_BSY)
		;
	return 0;
};

static int32_t _write_pio48(ata_t* self, const uint64_t lba, const uint16_t sectors, const uint8_t* buffer)
{
	if (!self || !buffer) {
		return -EIO;
	};
	// Select master/slave and set LBA mode (bit 6 = 0x40 must be set)
	outb(ATA_CONTROL_PORT, 0x40 | ATA_DRIVE_MASTER);
	// Write the **upper bytes** of the number of sectors & LBA first
	outb(ATA_SECTOR_COUNT_PORT, (sectors >> 8) & 0xFF);
	outb(ATA_LBA_LOW_PORT, (lba >> 24) & 0xFF);
	outb(ATA_LBA_MID_PORT, (lba >> 32) & 0xFF);
	outb(ATA_LBA_HIGH_PORT, (lba >> 40) & 0xFF);
	// Then write the **lower bytes** of the number of sectors & LBA
	outb(ATA_SECTOR_COUNT_PORT, sectors & 0xFF);
	outb(ATA_LBA_LOW_PORT, lba & 0xFF);
	outb(ATA_LBA_MID_PORT, (lba >> 8) & 0xFF);
	outb(ATA_LBA_HIGH_PORT, (lba >> 16) & 0xFF);
	// Send “WRITE SECTORS EXT” command (0x34)
	outb(ATA_COMMAND_PORT, ATA_CMD_WRITE_SECTORS_EXT);

	for (size_t i = 0; i < sectors; ++i) {
		// Wait for DRQ (data request)
		while (!(inb(ATA_COMMAND_PORT) & ATA_STATUS_DRQ))
			;

		// Write 512 bytes to the ATA data port (no `rep outsw`)
		for (size_t j = 0; j < 256; j++) {
			outw(ATA_DATA_PORT, ((uint16_t*)buffer)[j]);
			__asm__ __volatile__("jmp 1f\n1: jmp 1f\n1:");
		};
		buffer += 512;
	};
	// Send cache flush (0xE7) to ensure that data is on the hard disk
	outb(ATA_COMMAND_PORT, ATA_CMD_CACHE_FLUSH);
	//  Wait until the write process is complete
	while (inb(ATA_COMMAND_PORT) & ATA_STATUS_BSY)
		;
	return 0;
};