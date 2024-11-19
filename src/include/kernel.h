/**
 * @file kernel.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef KERNEL_H
#define KERNEL_H

#define KERNEL_VIRTUAL_START 0xC0000000
#define HEAP_START_ADDR 0xC1000000
#define HEAP_BITMAP_ADDR 0xC1400000
#define KERNEL_FRAMEBUFFER_ADDR 0xE0000000
#define MAX_KERNEL_SIZE (16 * 1024 * 1024) // 16 MiB
#define MAX_HEAP_SIZE (4 * 1024 * 1024)	   // 4 MiB
#define HEAP_ALIGNMENT 4096

#define p2v(addr) ((void*)((addr) + KERNEL_VIRTUAL_START))
#define v2p(addr) ((uintptr_t*)(addr) - KERNEL_VIRTUAL_START)

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
#include "status.h"
#include "stream.h"
#include "string.h"
#include "timer.h"
#include "vbe.h"
#include "vfs.h"
#include "vga.h"

/* PUBLIC API */
void panic(const char* str);
void sleep(const uint32_t ms);
void busy_wait(const uint64_t delay);
void kmain(const uint32_t magic, const uint32_t addr);

#endif