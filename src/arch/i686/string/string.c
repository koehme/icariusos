/**
 * @file string.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdarg.h>
#include <stdbool.h>

#include "stdlib.h"

#include "string.h"


bool is_alpha(const char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };


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

void* mmove(void* dest, const void* src, size_t n_bytes)
{
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;

	if (d == s || n_bytes == 0) {
		return dest;
	};

	if (d < s) {
		for (size_t i = 0; i < n_bytes; i++) {
			d[i] = s[i];
		};
	} else {
		for (size_t i = n_bytes; i != 0; i--) {
			d[i - 1] = s[i - 1];
		};
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
int32_t mcmp(const void* s1, const void* s2, size_t n_bytes)
{
	const uint8_t* p1 = s1;
	const uint8_t* p2 = s2;

	while (n_bytes > 0) {
		if (*p1 != *p2) {
			return (int32_t)(*p1) - (int32_t)(*p2);
		};
		p1++;
		p2++;
		n_bytes--;
	};
	return 0;
};