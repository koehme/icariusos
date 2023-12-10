/**
 * @file icarius.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef ICARIUS_H
#define ICARIUS_H

#include <stddef.h>

void kmain(void);
void *kmalloc(const size_t size);
void kprint(const char *str);
void kpanic(const char *str);

#endif