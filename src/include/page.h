/**
 * @file page.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

typedef enum PageFlags
{
    PAGE_WRITEABLE = 0b00000010,
} PageFlags;

typedef struct PageDirectory
{
    uint32_t *s_address_directory;
} PageDirectory;

uint32_t *page_init_directory(const uint8_t flags, const int page_size, const int entries);

#endif