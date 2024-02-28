#include "stream.h"
#include "string.h"

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

int32_t stream_read(Stream *self, uint8_t *buffer, const size_t n_bytes)
{
    int32_t ata_status = -1;

    if (buffer == 0x0)
    {
        return -1;
    };
    const size_t block_size = self->dev->sector_size;
    size_t remaining_bytes = n_bytes;
    size_t read_size = n_bytes > block_size ? block_size : n_bytes;

    while (remaining_bytes)
    {
        const size_t lba_block = self->pos / block_size;
        const size_t offset = self->pos % block_size;
        ata_status = ata_read(self->dev, lba_block, 1);

        if (ata_status < 0)
        {
            return ata_status;
        };
        read_size = remaining_bytes > block_size ? block_size : remaining_bytes;

        for (size_t i = 0; i < read_size; i++)
        {
            *buffer++ = self->dev->buffer[offset + i];
        };
        self->pos += read_size;
        remaining_bytes -= read_size;
    };
    return ata_status;
};