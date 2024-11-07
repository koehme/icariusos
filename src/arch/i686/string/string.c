/**
 * @file string.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdarg.h>
#include <stdbool.h>

#include "status.h"
#include "string.h"
#include "vga.h"

extern VGADisplay vga_display;

bool is_alpha(const char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };

size_t slen(const char* str)
{
	size_t i;

	for (i = 0; str[i] != '\0'; ++i) {
	};
	return i;
};

char* sptr(char* str, const char c)
{
	while (*str != '\0') {
		if (*str == c) {
			return str;
		};
		str++;
	}
	return 0x0;
};

bool scmp(const char* s1, const char* s2)
{
	size_t s1_len = slen(s1);
	size_t s2_len = slen(s2);

	if (s1_len != s2_len) {
		return false;
	};
	size_t i = 0;

	while (s1[i] != '\0' && s2[i] != '\0') {
		if (s1[i] == s2[i]) {
			i++;
		} else {
			return false;
		};
	};
	return true;
};

void sreverse(char* str, const size_t length)
{
	size_t start = 0;
	size_t end = length - 1;

	while (start < end) {
		uint8_t buffer = str[start];
		str[start] = str[end];
		str[end] = buffer;
		start++;
		end--;
	};
	return;
};

// Converts an integer to a string
char* itoa(int32_t num, char* str, int32_t base)
{
	size_t i = 0;
	bool is_negative = false;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	};

	if (num < 0 && base == 10) {
		is_negative = true;
		num = -num;
	};
	// Extract digits from the number and add them to the string in reverse order
	while (num != 0) {
		// Extract in base 10 each digit for example 1024 % 10 = 4
		//                                              ^
		const int32_t rem = num % base;
		// Convert the remainder to the corresponding character and add it to the string buffer
		// If the remainder is greater than 9, use 'a' to 'f'
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		//                          ^                   ^
		//                 floats +  base 16           base 10
		// Get the next digit
		num = num / base;
	};

	if (is_negative) {
		str[i++] = '-';
	};
	str[i] = '\0';
	sreverse(str, i);
	return str;
};

char* utoa(uint32_t num, char* str, int32_t base)
{
	size_t i = 0;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	};

	while (num != 0) {
		const uint32_t rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	};

	str[i] = '\0';
	sreverse(str, i);
	return str;
};

int32_t printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	while (*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;

			switch (*fmt) {
			case 'c': {
				const char ch = va_arg(args, int32_t);
				vbe_draw_ch(&vbe_display, ch, VBE_COLOR_GREEN);
				break;
			};
			case 's': {
				const char* str = va_arg(args, const char*);
				vbe_draw_string(&vbe_display, str, VBE_COLOR_GREEN);
				break;
			};
			case 'd': {
				char buffer[1024] = {};
				const int32_t num = va_arg(args, int32_t);
				itoa(num, buffer, 10);
				vbe_draw_string(&vbe_display, buffer, VBE_COLOR_GREEN);
				break;
			};
			case 'x': {
				char buffer[1024] = {};
				const uint32_t value = va_arg(args, uint32_t);
				utoa(value, buffer, 16);
				vbe_draw_string(&vbe_display, buffer, VBE_COLOR_GREEN);
				break;
			};
			default: {
				break;
			};
			};
		} else {
			vbe_draw_ch(&vbe_display, *fmt, VBE_COLOR_GREEN);
		};
		fmt++;
	};
	va_end(args);
	return 0;
};

char* scat(char* dest, const char* src)
{
	char* concatenated = dest;

	while (*dest != '\0') {
		dest++;
	};

	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	};
	*dest = '\0';
	return concatenated;
};

// Set a memory block with a 8-bit value repeated throughout for the specified number of bytes
void* mset8(void* dest, const uint8_t value, size_t n_bytes)
{
	uint8_t* buffer = (uint8_t*)dest;

	for (; n_bytes != 0; n_bytes--) {
		*buffer = value;
		buffer++;
	};
	return dest;
};

// Set a memory block with a 16-bit value repeated throughout for the specified number of bytes
void* mset16(void* dest, const uint16_t value, size_t n_bytes)
{
	uint16_t* buffer = (uint16_t*)dest;

	for (; n_bytes != 0; n_bytes--) {
		*buffer = value;
		buffer++;
	};
	return dest;
};

// Copy a memory block from source to destination with specified number of bytes
void* mcpy(void* dest, const void* src, size_t n_bytes)
{
	uint8_t* byte_dest = (uint8_t*)dest;
	const uint8_t* src_ptr = (const uint8_t*)src;

	while (n_bytes) {
		*byte_dest = *src_ptr;
		byte_dest++;
		src_ptr++;
		n_bytes--;
	};
	return dest;
};

// Compare two memory blocks byte by byte and return the difference. If all bytes are equal, returns 0
int32_t mcmp(const void* s1, const void* s2, size_t n)
{
	const uint8_t* p1 = s1;
	const uint8_t* p2 = s2;

	while (n > 0) {
		if (*p1 != *p2) {
			return (int32_t)(*p1) - (int32_t)(*p2);
		};
		p1++;
		p2++;
		n--;
	};
	return 0;
};