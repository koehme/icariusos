/**
 * @file string.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

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

void* memset(void* ptr, int value, size_t num)
{
	// Cast ptr to unsigned char pointer for byte-wise operations
	unsigned char* p = (unsigned char*)ptr;
	// Set each byte to the specified value
	for (size_t i = 0; i < num; i++) {
		p[i] = (unsigned char)value;
	};
	return ptr;
};

void* memset16(void* ptr, uint16_t value, size_t num)
{
	uint16_t* p = (uint16_t*)ptr;

	for (size_t i = 0; i < num; i++) {
		p[i] = value;
	};
	return ptr;
};

void* memmove(void* dest, const void* src, size_t count)
{
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;

	if (d == s || count == 0) {
		return dest;
	};

	if (d < s) {
		for (size_t i = 0; i < count; i++) {
			d[i] = s[i];
		};
	} else {
		for (size_t i = count; i != 0; i--) {
			d[i - 1] = s[i - 1];
		};
	};
	return dest;
};

void* memcpy(void* dest, const void* src, size_t n)
{
	uint8_t* byte_dest = (uint8_t*)dest;
	const uint8_t* src_ptr = (const uint8_t*)src;

	while (n) {
		*byte_dest = *src_ptr;
		byte_dest++;
		src_ptr++;
		n--;
	};
	return dest;
};

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{
	const uint8_t* p1 = ptr1;
	const uint8_t* p2 = ptr2;

	while (num > 0) {
		if (*p1 != *p2) {
			return (int32_t)(*p1) - (int32_t)(*p2);
		};
		p1++;
		p2++;
		num--;
	};
	return 0;
};