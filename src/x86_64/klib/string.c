/**
 * @file string.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "string.h"
#include "types.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
usize strlen(const ch* str);
s32 strcmp(const ch* str1, const ch* str2);
ch* strncpy(ch* dest, const ch* src, usize n);
usize strlen(const ch* str);
b8 isalpha(const ch c);
ch* strchr(ch* str, const ch c);
b8 scmp(const ch* s1, const ch* s2);
void sreverse(ch* str, const usize length);
ch* scat(ch* dest, const ch* src);
void* memset(void* ptr, s32 value, usize num);
void* memset16(void* ptr, u16 value, usize num);
void* memmove(void* dest, const void* src, usize count);
void* memcpy(void* dest, const void* src, usize n);
s32 memcmp(const void* ptr1, const void* ptr2, usize num);

/* INTERNAL API */
// -

usize strlen(const ch* str)
{
	const ch* s = str;

	while (*s)
		s++;
	return (usize)(s - str);
};
s32 strcmp(const ch* str1, const ch* str2)
{
	while (*str1 && (*str1 == *str2)) {
		str1++;
		str2++;
	};
	return *(const ch*)str1 - *(const ch*)str2;
};

ch* strncpy(ch* dest, const ch* src, usize n)
{
	usize i;

	for (i = 0; i < n && src[i] != '\0'; i++)
		dest[i] = src[i];

	for (; i < n; i++)
		dest[i] = '\0';
	return dest;
};

b8 isalpha(const ch c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };

ch* strchr(ch* str, const ch c)
{
	while (*str != '\0') {
		if (*str == c)
			return str;
		str++;
	}
	return 0x0;
};

void sreverse(ch* str, const usize length)
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
};

ch* scat(ch* dest, const ch* src)
{
	ch* concatenated = dest;

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

void* memset(void* ptr, s32 value, usize num)
{
	// Cast ptr to unsigned ch pointer for byte-wise operations
	ch* p = (ch*)ptr;
	// Set each byte to the specified value
	for (usize i = 0; i < num; i++)
		p[i] = (ch)value;
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
	ch* d = (ch*)dest;
	const ch* s = (const ch*)src;

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

s32 memcmp(const void* ptr1, const void* ptr2, usize num)
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