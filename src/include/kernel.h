/**
 * @file kernel.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "icarius.h"

#define p2v(addr) ((void*)((addr) + KERNEL_VIRTUAL_START))
#define v2p(addr) ((void*)((addr) - KERNEL_VIRTUAL_START))

#define KERNEL_DEBUG_DELAY 0

extern char _multiboot_start[], _multiboot_end[];
extern char _boot_start[], _boot_end[];
extern char _text_start[], _text_end[];
extern char _rodata_start[], _rodata_end[];
extern char _data_start[], _data_end[];
extern char _bss_start[], _bss_end[];
extern char _kernel_start[], _kernel_end[];

#include <stddef.h>

#include "ata.h"
#include "cmos.h"
#include "cursor.h"
#include "errno.h"
#include "fifo.h"
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
#include "pfa.h"
#include "pic.h"
#include "ps2.h"
#include "stream.h"
#include "string.h"
#include "task.h"
#include "timer.h"
#include "tss.h"
#include "vbe.h"
#include "vfs.h"
#include "vga.h"

/* PUBLIC API */
void panic(const char* str);
void sleep(const uint32_t ms);
void busy_wait(const uint64_t delay);
void kmain(const uint32_t magic, const uint32_t addr);

#endif