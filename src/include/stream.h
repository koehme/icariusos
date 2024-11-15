/**
 * @file stream.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef STREAM_H
#define STREAM_H

#include <stddef.h>
#include <stdint.h>

#include "ata.h"

typedef struct Stream {
	size_t pos;
	ata_t* dev;
} Stream;

void stream_init(Stream* self, ata_t* dev);
void stream_seek(Stream* self, const size_t pos);
int32_t stream_read(Stream* self, uint8_t* buffer, size_t n_bytes);

#endif