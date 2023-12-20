/**
 * @file icarius.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef ICARIUS_H
#define ICARIUS_H

#include <stddef.h>

#include "vga.h"
#include "idt.h"
#include "io.h"
#include "heap.h"
#include "cursor.h"
#include "mem.h"
#include "page.h"
#include "ata.h"
#include "status.h"

void *kmalloc(const size_t size);
void *kcalloc(const size_t size);
void kfree(void *ptr);
void kprint_color(const char *str, const VGAColor color);
void kprint(const char *str);
void kprint_hex(const uint8_t value);
void kpanic(const char *str);
void ksleep(const int iterations);
void kascii_spinner(const int frames, const int delay);
void kprint_logo(void);
void kprint_motd();
void kmain(void);

#endif