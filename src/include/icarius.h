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
#include "plexer.h"
#include "pparser.h"
#include "string.h"

void *kmalloc(const size_t size);
void *kcalloc(const size_t size);
void kfree(void *ptr);
void kpanic(const char *str);
void ksleep(const int n_times);
void kmain(void);

#endif