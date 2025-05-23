#include <string.h>

size_t strlen(const char* s)
{
	const char* p = s;

	while (*p) {
		p++;
	};
	return (size_t)(p - s);
};

void* memcpy(void* dest, const void* src, size_t n)
{
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;

	for (size_t i = 0; i < n; i++) {
		d[i] = s[i];
	};
	return dest;
};

int strcmp(const char* s1, const char* s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	};
	return *(unsigned char*)s1 - *(unsigned char*)s2;
};

void* memset(void* s, int c, size_t n)
{
	unsigned char* ptr = s;

	while (n--) {
		*ptr++ = (unsigned char)c;
	};
	return s;
};

char* strncpy(char* dest, const char* src, size_t n)
{
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++) {
		dest[i] = src[i];
	};

	for (; i < n; i++) {
		dest[i] = '\0';
	};
	return dest;
};

char* strchr(const char* str, int c)
{
	while (*str) {
		if (*str == (char)c) {
			return (char*)str;
		};
		str++;
	};

	if (c == '\0') {
		return (char*)str;
	};
	return 0;
};

char* strtok(char* str, const char* delim)
{
	static char* strtok_ptr;

	if (str != 0x0) {
		strtok_ptr = str;
	}

	if (strtok_ptr == 0x0 || *strtok_ptr == '\0') {
		return 0x0;
	};

	while (*strtok_ptr && strchr(delim, *strtok_ptr)) {
		strtok_ptr++;
	}

	if (*strtok_ptr == '\0') {
		return 0x0;
	};
	char* token_start = strtok_ptr;

	while (*strtok_ptr && !strchr(delim, *strtok_ptr)) {
		strtok_ptr++;
	};

	if (*strtok_ptr) {
		*strtok_ptr = '\0';
		strtok_ptr++;
	};
	return token_start;
};