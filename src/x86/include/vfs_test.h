/**
 * @file vfs_test.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef VFS_TEST_H
#define VFS_TEST_H

#include "stdio.h"
#include "string.h"
#include <stdint.h>

void test_vfs_read(const char* file);
void test_vfs_write(char* msg, uint32_t bytes);
void test_readdir(const char* path);

#endif