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

typedef struct stream_t {
	size_t pos;
	ata_t* dev;
} stream_t;

void stream_init(stream_t* self, ata_t* dev);
void stream_seek(stream_t* self, const size_t pos);
int32_t stream_read(stream_t* self, uint8_t* buffer, size_t n_bytes);

#endif