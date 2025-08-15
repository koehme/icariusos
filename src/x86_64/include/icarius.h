/**
 * @file icarius.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#pragma once

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))

#define PAGE_SIZE 4096