/**
 * @file icarius.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>

#include "multiboot.h"
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

void *kmalloc(const size_t size);
void *kcalloc(const size_t size);
void kfree(void *ptr);
void kpanic(const char *str);
void ksleep(const uint32_t ms);
void kdelay(const uint64_t delay);
void kmain(uint32_t magic, multiboot_info_t *mbd);

#endif