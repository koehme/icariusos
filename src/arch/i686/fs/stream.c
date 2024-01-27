#include "stream.h"
#include "string.h"
#include "mem.h"

void stream_init(Stream *self, ATADev *dev)
{
    self->pos = 0;
    self->dev = dev;
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
    const size_t block_size = self->dev->sector_size;

    while (total_bytes > 0)
    {
        const size_t lba_block = self->pos / block_size;
        const size_t offset = self->pos % block_size;
        ata_status = ata_read(self->dev, lba_block, 1);

        if (ata_status < 0)
        {
            return ata_status;
        };
        const size_t remaining_bytes = total_bytes > block_size ? block_size : total_bytes;

        for (size_t i = 0; i < remaining_bytes; i++)
        {
            *buffer++ = self->dev->buffer[offset + i];
        };
        self->pos += remaining_bytes;
        total_bytes -= remaining_bytes;
    };
    return ata_status;
};