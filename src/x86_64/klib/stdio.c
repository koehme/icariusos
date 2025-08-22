/**
 * @file stdio.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "stdio.h"
#include "fb.h"
#include "stdlib.h"
#include "string.h"
#include "tty.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
usize kprintf(const char* fmt, ...);
static int _vkprintf(tty_t* tty, const char* fmt, va_list args);

/* INTERNAL API */
// -

static int _vkprintf(tty_t* tty, const char* fmt, va_list args)
{
	int count = 0;

	for (; *fmt; ++fmt) {
		if (*fmt != '%') {
			tty_putc(tty, *fmt);
			++count;
			continue;
		};
		// Parse minimal: %, [l|ll|z] [specifier]
		bool long_mod = false, longlong_mod = false, size_mod = false;
		++fmt;

		while (*fmt == 'l' || *fmt == 'z') {
			if (*fmt == 'z') {
				size_mod = true;
				++fmt;
				break;
			};
			if (long_mod) {
				longlong_mod = true;
				++fmt;
				break;
			};
			long_mod = true;
			++fmt;
		};

		switch (*fmt) {
		case 'c': {
			s32 ch = va_arg(args, s32);
			tty_putc(tty, ch);
			++count;
			break;
		};
		case 's': {
			const char* str = va_arg(args, const char*);

			if (!str)
				str = "";

			tty_puts(tty, str);
			count += strlen(str);
			break;
		};
		case 'd':
		case 'i': {
			char buf[64];
			s64 val;

			if (longlong_mod) {
				val = (s64)va_arg(args, long long); // %lld
			} else if (long_mod) {
				val = (s64)va_arg(args, long); // %ld
			} else if (size_mod) {
				val = (s64)va_arg(args, ssize_t); // %zd
			} else {
				val = (s64)va_arg(args, int); // %d
			};
			count += itoa(val, 10, false, buf);
			tty_puts(tty, buf);
			break;
		};
		case 'u': {
			char buf[64];
			s64 val;

			if (longlong_mod) {
				val = va_arg(args, unsigned long long);
			} else if (long_mod) {
				val = va_arg(args, unsigned long);
			} else if (size_mod) {
				val = (u64)va_arg(args, usize);
			} else {
				val = va_arg(args, unsigned int);
			};
			count += utoa(val, 10, false, buf);
			tty_puts(tty, buf);
			break;
		};
		case 'x':
		case 'X': {
			char buf[64];
			const bool upper = (*fmt == 'X');
			u64 val;

			if (longlong_mod) {
				val = va_arg(args, unsigned long long);
			} else if (long_mod) {
				val = va_arg(args, unsigned long);
			} else if (size_mod) {
				val = (u64)va_arg(args, usize);
			} else {
				val = va_arg(args, unsigned int);
			};
			count += itoa(val, 16, upper, buf);
			tty_puts(tty, buf);
			break;
		};
		case 'p':
		case 'P': {
			char buf[64];
			const bool upper = (*fmt == 'P');

			void* ptr = va_arg(args, void*);
			const u64 val = (u64)(uintptr_t)ptr;

			tty_puts(tty, "0x");
			count += 2;

			count += utoa(val, 16, upper, buf);
			tty_puts(tty, buf);
			break;
		};
		case '%':
			tty_putc(tty, '%');
			++count;
			break;
		default:
			tty_putc(tty, '%');
			tty_putc(tty, *fmt);
			count += 2;
			break;
		};
	};
	return count;
};

usize kprintf(const char* fmt, ...)
{
	tty_t* tty = tty_get_active();

	if (!fmt || !tty)
		return 0;

	va_list args;
	va_start(args, fmt);
	const usize n = _vkprintf(tty, fmt, args);
	va_end(args);

	return n;
};