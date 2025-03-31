#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stddef.h>

#define EOF (-1)
#define FILE_MODE_READ 0
#define FILE_MODE_WRITE 1

typedef struct {
	int fd;
	int mode; // mode: 0 = read, 1 = write
	int error;
	int eof;
} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

int puts(const char* s);
int printf(const char* restrict format, ...);
int vsnprintf(char* buffer, size_t size, const char* format, va_list args);
FILE* fopen(const char* pathname, const char* mode);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int fclose(FILE* stream);
void file_dump(FILE* f);
int feof(FILE* stream);

#endif