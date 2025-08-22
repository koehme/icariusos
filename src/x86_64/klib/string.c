/**
 * @file string.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "string.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
usize strlen(const char* str);
s32 strcmp(const char* str1, const char* str2);
char* strncpy(char* dest, const char* src, usize n);
usize strlen(const char* str);
bool isalpha(const char c);
char* strchr(char* str, const char c);
bool scmp(const char* s1, const char* s2);
void sreverse(char* str, const usize length);
char* scat(char* dest, const char* src);
void* memset(void* ptr, int value, usize num);
void* memset16(void* ptr, u16 value, usize num);
void* memmove(void* dest, const void* src, usize count);
void* memcpy(void* dest, const void* src, usize n);
int memcmp(const void* ptr1, const void* ptr2, usize num);

/* INTERNAL API */
// -

usize strlen(const char* str)
{
	const char* s = str;

	while (*s)
		s++;
	return (usize)(s - str);
};
s32 strcmp(const char* str1, const char* str2)
{
	while (*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	};
	return *(const unsigned char*)str1 - *(const unsigned char*)str2;
};

char* strncpy(char* dest, const char* src, usize n)
{
	usize i;

	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];

	for (; i < n; i++)
		dest[i] = '\0';
	return dest;
};

bool isalpha(const char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };

char* strchr(char* str, const char c)
{
	while (*str != '\0') {
		if (*str == c)
			return str;
		str++;
	}
	return 0x0;
};

void sreverse(char* str, const usize length)
{
	usize start = 0;
	usize end = length - 1;

	while (start < end) {
		u8 buffer = str[start];
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

	while (*dest != '\0')
		dest++;

	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	};
	*dest = '\0';
	return concatenated;
};

void* memset(void* ptr, int value, usize num)
{
	// Cast ptr to unsigned char pointer for byte-wise operations
	unsigned char* p = (unsigned char*)ptr;
	// Set each byte to the specified value
	for (usize i = 0; i < num; i++)
		p[i] = (unsigned char)value;
	return ptr;
};

void* memset16(void* ptr, u16 value, usize num)
{
	u16* p = (u16*)ptr;

	for (usize i = 0; i < num; i++)
		p[i] = value;
	return ptr;
};

void* memmove(void* dest, const void* src, usize count)
{
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;

	if (d == s || count == 0)
		return dest;

	if (d < s) {
		for (usize i = 0; i < count; i++)
			d[i] = s[i];
	} else {
		for (usize i = count; i != 0; i--)
			d[i - 1] = s[i - 1];
	};
	return dest;
};

void* memcpy(void* dest, const void* src, usize n)
{
	u8* byte_dest = (u8*)dest;
	const u8* src_ptr = (const u8*)src;

	while (n) {
		*byte_dest = *src_ptr;
		byte_dest++;
		src_ptr++;
		n--;
	};
	return dest;
};

int memcmp(const void* ptr1, const void* ptr2, usize num)
{
	const u8* p1 = ptr1;
	const u8* p2 = ptr2;

	while (num > 0) {
		if (*p1 != *p2) {
			return (s32)(*p1) - (s32)(*p2);
		};
		p1++;
		p2++;
		num--;
	};
	return 0;
};