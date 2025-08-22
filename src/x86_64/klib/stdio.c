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
#include <stdarg.h>

/* EXTERNAL API */
// -

/* PUBLIC API */
usize kprintf(const ch* fmt, ...);
static usize vkprintf(const ch* fmt, va_list args);

/* INTERNAL API */
// -

static usize vkprintf(const ch* fmt, va_list args)
{
	usize count = 0;
	tty_t* tty = tty_get_active();

	if (!tty)
		return count;

	for (; *fmt; ++fmt) {
		if (*fmt != '%') {
			tty_putc(tty, *fmt);
			++count;
			continue;
		};
		// Parse minimal: %, [l|ll|z] [specifier]
		b8 long_mod = false, longlong_mod = false, size_mod = false;
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
			const ch* str = va_arg(args, const ch*);

			if (!str)
				str = "";

			tty_puts(tty, str);
			count += strlen(str);
			break;
		};
		case 'd':
		case 'i': {
			ch buf[64];
			s64 val;

			if (longlong_mod) {
				val = (s64)va_arg(args, s64ll); // %lld
			} else if (long_mod) {
				val = (s64)va_arg(args, s64); // %ld
			} else if (size_mod) {
				val = (s64)va_arg(args, ssize); // %zd
			} else {
				val = (s64)va_arg(args, s32); // %d
			};
			count += itoa(val, 10, false, buf);
			tty_puts(tty, buf);
			break;
		};
		case 'u': {
			ch buf[64];
			s64 val;

			if (longlong_mod) {
				val = va_arg(args, u64ull);
			} else if (long_mod) {
				val = va_arg(args, u64);
			} else if (size_mod) {
				val = (u64)va_arg(args, usize);
			} else {
				val = va_arg(args, s32);
			};
			count += utoa(val, 10, false, buf);
			tty_puts(tty, buf);
			break;
		};
		case 'x':
		case 'X': {
			ch buf[64];
			const b8 upper = (*fmt == 'X');
			u64 val;

			if (longlong_mod) {
				val = va_arg(args, u64ull);
			} else if (long_mod) {
				val = va_arg(args, u64);
			} else if (size_mod) {
				val = (u64)va_arg(args, usize);
			} else {
				val = va_arg(args, s32);
			};
			count += itoa(val, 16, upper, buf);
			tty_puts(tty, buf);
			break;
		};
		case 'p':
		case 'P': {
			ch buf[64];
			const b8 upper = (*fmt == 'P');

			void* ptr = va_arg(args, void*);
			const u64 val = (u64)(uptr)ptr;

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

usize kprintf(const ch* fmt, ...)
{
	if (!fmt)
		return 0;

	va_list args;
	va_start(args, fmt);
	const usize n = vkprintf(fmt, args);
	va_end(args);

	return n;
};