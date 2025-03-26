#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "syscall.h"

FILE file_slots[MAX_OPEN_FILES] = {0};

int puts(const char* s)
{
	if (!s) {
		return -1;
	};
	const size_t len = strlen(s);
	ssize_t written;

	if (len > 0) {
		written = write(1, s, len);

		if (written == -1 || (size_t)written != len) {
			return -1;
		};
	};
	written = write(1, "\n", 1);

	if (written == -1 || written != 1) {
		return -1;
	};
	return 0;
};

static void itoa(int num, char* str, int base)
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
	char temp[32];

	for (size_t i = 0; format[i] != '\0' && pos < size - 1; i++) {
		if (format[i] == '%' && format[i + 1] != '\0') {
			i++;

			switch (format[i]) {
			case 'd': {
				int num = va_arg(args, int);
				itoa(num, temp, 10);

				for (size_t j = 0; temp[j] != '\0' && pos < size - 1; j++) {
					buffer[pos++] = temp[j];
				};
				break;
			};
			case 'x': {
				int num = va_arg(args, int);
				itoa(num, temp, 16);

				for (size_t j = 0; temp[j] != '\0' && pos < size - 1; j++) {
					buffer[pos++] = temp[j];
				};
				break;
			};
			case 's': {
				char* str = va_arg(args, char*);

				if (!str) {
					str = "0x0";
				};
				for (size_t j = 0; str[j] != '\0' && pos < size - 1; j++) {
					buffer[pos++] = str[j];
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
		write(1, buf, len);
	};
	return len;
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

	if (fd < 0)
		return 0x0;

	for (size_t i = 0; i < MAX_OPEN_FILES; i++) {
		FILE* slot = &file_slots[i];

		if (slot->fd == 0) {
			slot->fd = fd;
			slot->mode = flags;
			slot->error = 0;
			slot->eof = 0;
			return slot;
		};
	};
	close(fd);
	return 0x0;
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
	return res;
};

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
	if (!stream || stream->fd <= 0 || !ptr) {
		return 0;
	};
	const size_t total_bytes = size * nmemb;
	const int bytes_read = read(stream->fd, ptr, total_bytes);

	if (bytes_read < 0) {
		stream->error = 1;
		return 0;
	}

	if ((size_t)bytes_read < total_bytes) {
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