/**
 * @file icarius.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KERNEL_H
#define KERNEL_H

#define KMAIN_DEBUG_THROTTLE 500

#include <stddef.h>

#include "multiboot2.h"
#include "vga.h"
#include "idt.h"
#include "io.h"
#include "heap.h"
#include "cursor.h"
#include "mem.h"
#include "page.h"
#include "kernel.h"
#include "keyboard.h"
#include "ata.h"
#include "status.h"
#include "plexer.h"
#include "pparser.h"
#include "string.h"
#include "timer.h"
#include "stream.h"
#include "cmos.h"
#include "superblock.h"

void *kmalloc(const size_t size);
void *kcalloc(const size_t size);
void kfree(void *ptr);
void kpanic(const char *str);
void ksleep(const uint32_t ms);
void kdelay(const uint64_t delay);
void kmain(const uint32_t magic, const uint32_t addr);

#endif