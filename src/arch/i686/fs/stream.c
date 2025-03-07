/**
 * @file stream.c
 * @author Kevin
 * @copyright MIT
 * @brief Stream implementation for block device interaction
 * @date 2024-11-16
 */

#include "stream.h"
#include "string.h"

/* PUBLIC API */
void stream_init(stream_t* self, ata_t* dev);
void stream_seek(stream_t* self, const size_t pos);
int32_t stream_read(stream_t* self, uint8_t* buffer, const size_t n_bytes);

void stream_init(stream_t* self, ata_t* dev)
{
	self->pos = 0;
	self->dev = dev;
	return;
};

void stream_seek(stream_t* self, const size_t pos)
{
	self->pos = pos;
	return;
};

int32_t stream_read(stream_t* self, uint8_t* buffer, const size_t n_bytes)
{
	int32_t ata_status = -1;

	if (buffer == 0x0) {
		return -1;
	};
	const size_t block_size = self->dev->sector_size;
	size_t remaining_bytes = n_bytes;
	size_t read_size = n_bytes > block_size ? block_size : n_bytes;

	while (remaining_bytes) {
		const size_t lba_block = self->pos / block_size;
		const size_t offset = self->pos % block_size;
		ata_status = ata_read(self->dev, lba_block, 1);

		if (ata_status < 0) {
			return ata_status;
		};
		read_size = remaining_bytes > block_size ? block_size : remaining_bytes;

		for (size_t i = 0; i < read_size; i++) {
			*buffer++ = self->dev->buffer[offset + i];
		};
		self->pos += read_size;
		remaining_bytes -= read_size;
	};
	return ata_status;
};

int32_t stream_write(stream_t* self, const uint8_t* buffer, const size_t n_bytes)
{
	if (!self || !buffer) {
		return -1;
	};
	const size_t block_size = self->dev->sector_size;
	size_t remaining_bytes = n_bytes;
	size_t write_size = n_bytes > block_size ? block_size : n_bytes;

	while (remaining_bytes) {
		const size_t lba_block = self->pos / block_size;
		const size_t offset = self->pos % block_size;
		ata_read(self->dev, lba_block, 1);

		for (size_t i = 0; i < write_size; i++) {
			self->dev->buffer[offset + i] = *buffer++;
		};
		int32_t ata_status = ata_write(self->dev, lba_block, 1, self->dev->buffer);

		if (ata_status < 0) {
			return ata_status;
		};
		self->pos += write_size;
		remaining_bytes -= write_size;
	};
	return 0;
};