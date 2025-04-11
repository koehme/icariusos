#include "stdio.h"
#include "errno.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"
#include "sys/unistd.h"
#include "syscall.h"
#include <stdbool.h>

static FILE file_slots[3] = {
    [0] = {.fd = STDIN_FILENO, .mode = O_RDONLY, .error = 0, .eof = 0},	 // stdin
    [1] = {.fd = STDOUT_FILENO, .mode = O_WRONLY, .error = 0, .eof = 0}, // stdout
    [2] = {.fd = STDERR_FILENO, .mode = O_WRONLY, .error = 0, .eof = 0}, // stderr
};

FILE* stdin = &file_slots[0];
FILE* stdout = &file_slots[1];
FILE* stderr = &file_slots[2];

int puts(const char* s)
{
	if (!s) {
		return -1;
	};
	const size_t len = strlen(s);
	ssize_t written;

	if (len > 0) {
		written = write(STDOUT_FILENO, s, len);

		if (written == -1 || (size_t)written != len) {
			return -1;
		};
	};
	written = write(STDOUT_FILENO, "\n", 1);

	if (written == -1 || written != 1) {
		return -1;
	};
	return 0;
};

void itoa(int num, char* str, int base)
{
	const char* digits = "0123456789abcdef";
	char buffer[32];
	int i = 0, is_negative = 0;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return;
	};

	if (num < 0 && base == 10) {
		is_negative = 1;
		num = -num;
	};

	while (num > 0) {
		buffer[i++] = digits[num % base];
		num /= base;
	};

	if (is_negative) {
		buffer[i++] = '-';
	};
	buffer[i] = '\0';
	int j;

	for (j = 0; j < i; j++) {
		str[j] = buffer[i - j - 1];
	}
	str[j] = '\0';
	return;
};

int vsnprintf(char* buffer, size_t size, const char* format, va_list args)
{
	size_t pos = 0;
	char temp[128];

	for (size_t i = 0; format[i] != '\0' && pos < size - 1; i++) {
		if (format[i] == '%') {
			if (format[i + 1] == '%') {
				buffer[pos++] = '%';
				i++;
				continue;
			};
			i++;
			bool zero_pad = false;
			bool left_align = false;
			int width = 0;

			if (format[i] == '-') {
				left_align = true;
				i++;
			};
			// Parse optional '0' flag
			if (format[i] == '0') {
				zero_pad = true;
				i++;
			};
			// Parse optional width (e.g. 2 in %2d or 08 in %08x)
			while (format[i] >= '0' && format[i] <= '9') {
				width = width * 10 + (format[i] - '0');
				i++;
			};
			switch (format[i]) {
			case 'd': {
				int num = va_arg(args, int);
				itoa(num, temp, 10);

				const size_t len = strlen(temp);

				if (!left_align && width > len) {
					char pad_char = zero_pad ? '0' : ' ';

					for (int pad = 0; pad < width - len && pos < size - 1; pad++) {
						buffer[pos++] = pad_char;
					};
				};

				for (size_t j = 0; temp[j] != '\0' && pos < size - 1; j++) {
					buffer[pos++] = temp[j];
				};
				if (left_align && width > len) {
					for (int pad = 0; pad < width - len && pos < size - 1; pad++) {
						buffer[pos++] = ' ';
					};
				};
				break;
			};
			case 'x': {
				int num = va_arg(args, int);
				itoa(num, temp, 16);

				const size_t len = strlen(temp);

				if (zero_pad && width > len) {
					for (int pad = 0; pad < width - len && pos < size - 1; pad++) {
						buffer[pos++] = '0';
					};
				};
				for (size_t j = 0; temp[j] != '\0' && pos < size - 1; j++) {
					buffer[pos++] = temp[j];
				};
				break;
			};
			case 's': {
				char* str = va_arg(args, char*);

				if (!str)
					str = "0x0";
				const size_t len = strlen(str);

				if (width > 0 && !left_align && width > len) {
					for (int pad = 0; pad < width - len && pos < size - 1; pad++) {
						buffer[pos++] = ' ';
					};
				};

				for (size_t j = 0; str[j] != '\0' && pos < size - 1; j++) {
					buffer[pos++] = str[j];
				};

				if (width > 0 && left_align && width > len) {
					for (int pad = 0; pad < width - len && pos < size - 1; pad++) {
						buffer[pos++] = ' ';
					};
				};
				break;
			};
			case 'c': {
				char c = (char)va_arg(args, int);

				if (pos < size - 1) {
					buffer[pos++] = c;
				};
				break;
			};
			case 'f': {
				double num = va_arg(args, double);
				dtoa(num, temp, 6);

				for (size_t j = 0; temp[j] != '\0' && pos < size - 1; j++) {
					buffer[pos++] = temp[j];
				};
				break;
			};
			case '%': {
				if (pos < size - 1) {
					buffer[pos++] = '%';
				};
				break;
			};
			default: {
				if (pos < size - 1) {
					buffer[pos++] = format[i];
				};
				break;
			};
			};
		} else {
			buffer[pos++] = format[i];
		};
	};
	buffer[pos] = '\0';
	return pos;
};

int printf(const char* restrict format, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, format);
	int len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	if (len > 0) {
		write(STDOUT_FILENO, buf, len);
	};
	return len;
};

void file_dump(FILE* f)
{
	if (!f) {
		printf("FILE*: 0x0 \n");
		return;
	};
	printf("----[ FILE* Dump ]----\n");
	printf("FD     : %d\n", f->fd);
	printf("Mode   : %s\n", f->mode == 0 ? "READ" : "WRITE");
	printf("Error  : %s\n", f->error ? "YES" : "NO");
	printf("EOF    : %s\n", f->eof ? "YES" : "NO");
	printf("Ptr    : 0x%x\n", (void*)f);
	printf("-----------------------\n");
	return;
};

FILE* fopen(const char* pathname, const char* mode)
{
	int flags = -1;

	if (strcmp(mode, "r") == 0) {
		flags = 0;
	} else if (strcmp(mode, "w") == 0) {
		flags = 1;
	} else {
		return 0x0;
	};
	const int fd = open(pathname, flags);

	if (fd < 0) {
		return 0x0;
	};
	FILE* file = malloc(sizeof(FILE));

	if (!file) {
		close(fd);
		errno = ENOMEM;
		return 0x0;
	};
	file->fd = fd;
	file->mode = flags;
	file->error = 0;
	file->eof = 0;
	return file;
};

int fclose(FILE* stream)
{
	if (!stream || stream->fd <= 0) {
		return -1;
	};
	const int res = close(stream->fd);
	stream->fd = 0;
	stream->mode = 0;
	stream->error = 0;
	stream->eof = 0;
	free(stream);
	return res;
};

int feof(FILE* stream)
{
	if (!stream)
		return 0;
	return stream->eof;
};

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
	if (!stream || stream->fd < 0 || !ptr) {
		return 0;
	};
	const size_t total_bytes = size * nmemb;
	const int bytes_read = read(stream->fd, ptr, total_bytes);

	if (bytes_read <= 0) {
		stream->eof = 1;
		return 0;
	};
	if ((size_t)bytes_read <= total_bytes) {
		stream->eof = 1;
	};
	return bytes_read / size;
};

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream)
{
	if (!stream || stream->fd <= 0 || !ptr) {
		return 0;
	};
	const size_t total_bytes = size * nmemb;
	const int bytes_written = write(stream->fd, ptr, total_bytes);

	if (bytes_written < 0) {
		stream->error = 1;
		return 0;
	};
	return bytes_written / size;
};