/**
 * @file kernel.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KERNEL_H
#define KERNEL_H

#define KERNEL_DEBUG_DELAY 0

#include <stddef.h>

#include "ata.h"
#include "cmos.h"
#include "cursor.h"
#include "heap.h"
#include "idt.h"
#include "io.h"
#include "kernel.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiboot2.h"
#include "page.h"
#include "pci.h"
#include "status.h"
#include "stream.h"
#include "string.h"
#include "timer.h"
#include "vbe.h"
#include "vfs.h"
#include "vga.h"

void* kmalloc(const size_t size);
void* kcalloc(const size_t size);
void kfree(void* ptr);
void kpanic(const char* str);
void ksleep(const uint32_t ms);
void kdelay(const uint64_t delay);
void kmain(const uint32_t magic, const uint32_t addr);

#endif