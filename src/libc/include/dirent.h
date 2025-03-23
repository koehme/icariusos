#ifndef DIRENT_H
#define DIRENT_H

#include <sys/types.h>

#define DT_UNKNOWN 0
#define DT_REG 1
#define DT_DIR 2

struct dirent {
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[256];
};

typedef struct {
	int fd;
	struct dirent current;
	int has_entry;
} DIR;

DIR* opendir(const char* path);
struct dirent* readdir(DIR* dirp);
int closedir(DIR* dirp);

#endif