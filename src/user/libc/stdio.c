#include "stdio.h"
#include "string.h"
#include "sys/types.h"
#include "syscall.h"

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