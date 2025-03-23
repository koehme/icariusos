/**
 * @file dirent.h
 * @author Kevin Oehme
 * @copyright MIT
 */

#ifndef DIRENT_H
#define DIRENT_H

#include "types.h"

#define DT_UNKNOWN 0
#define DT_REG 1
#define DT_DIR 2

struct dirent {
	long d_ino;		 // unused at FAT16 → 0
	off_t d_off;		 // unused
	unsigned short d_reclen; // length
	unsigned char d_type;	 // DT_REG, DT_DIR
	char d_name[256];	 // filename
};

#endif