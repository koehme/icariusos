#include "stream.h"
#include "string.h"
#include "mem.h"

void stream_init(Stream *self, ATADisk *disk)
{
    self->pos = 0;
    self->disk = disk;
    return;
};

void stream_seek(Stream *self, const size_t pos)
{
    self->pos = pos;
    return;
};

int stream_read(Stream *self, uint8_t *buffer, size_t total_bytes)
{
    int8_t ata_status = -1;

    if (buffer == 0x0)
    {
        return -1;
    };
    const size_t block_size = self->disk->sector_size;

    while (total_bytes > 0)
    {
        const size_t lba_block = self->pos / block_size;
        const size_t offset = self->pos % block_size;
        // Read the first sector/block of the calculated LBA block variable above using ata_read.
        // The data will be stored in sector_buffer.
        ata_status = ata_read(self->disk, lba_block, 1);

        while (!ata_is_buffer_ready(self->disk))
        {
        };
        if (ata_status < 0)
        {
            return ata_status;
        };
        // Calculate the remaining_bytes. If total_bytes is greater than block_size, set remaining_bytes to block_size; otherwise, set it to total_bytes
        const size_t remaining_bytes = total_bytes > block_size ? block_size : total_bytes;
        // Copy only the n remaining_bytes from the temporary buffer (sector_buffer) into the larger buffer provided by the parameter "buffer" in the function declaration
        for (size_t i = 0; i < remaining_bytes; i++)
        {
            *buffer++ = self->disk->buffer[offset + i];
        };
        // Adjust the self->pos in the disk stream to the actual remaining_bytes
        self->pos += remaining_bytes;
        // Update total_bytes for the next iteration
        total_bytes -= remaining_bytes;
    };
    return ata_status;
};

void stream_dump(const Stream *self, const uint8_t *buffer)
{
    for (size_t i = 0; i < self->disk->sector_size; ++i)
    {
        kprintf("0x%x ", buffer[i]);
        kdelay(10000);
    };
    return;
};