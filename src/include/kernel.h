/**
 * @file kernel.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KERNEL_H
#define KERNEL_H

extern char _multiboot_start[], _multiboot_end[];
extern char _boot_start[], _boot_end[];
extern char _text_start[], _text_end[];
extern char _rodata_start[], _rodata_end[];
extern char _data_start[], _data_end[];
extern char _bss_start[], _bss_end[];
extern char _kernel_start[], _kernel_end[];

#define p2v(addr) ((void*)((addr) + 0xC0000000))
#define KERNEL_DEBUG_DELAY 0

#include <stddef.h>

#include "ata.h"
#include "cmos.h"
#include "cursor.h"
#include "gdt.h"
#include "heap.h"
#include "idt.h"
#include "io.h"
#include "kernel.h"
#include "keyboard.h"
#include "mouse.h"
#include "multiboot2.h"
#include "page.h"
#include "pci.h"
#include "pic.h"
#include "status.h"
#include "stream.h"
#include "string.h"
#include "timer.h"
#include "vbe.h"
#include "vfs.h"
#include "vga.h"

void kpanic(const char* str);
void ksleep(const uint32_t ms);
void kdelay(const uint64_t delay);
void kmain(const uint32_t magic, const uint32_t addr);

#endif