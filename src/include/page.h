/**
 * @file page.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>

extern PageDirectory *p_directory;

typedef enum PageFlags
{
    PAGE_WRITEABLE = 0b00000010,
} PageFlags;

typedef struct PageDirectory
{
    uint32_t *directory;
} PageDirectory;

#endif