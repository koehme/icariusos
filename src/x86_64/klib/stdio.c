/**
 * @file stdio.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "stdio.h"
#include "stdlib.h"
#include "tty.h"

/* PUBLIC API */
int kprintf(const char* fmt, ...);

static int vkprintf(tty_t* tty, const char* fmt, va_list args)
{
	int count = 0;

	while (*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;

			if (!*fmt)
				break;

			if (*fmt == '%') {
				tty_putc(tty, '%');
				count++;
				fmt++;
				continue;
			} else {
				switch (*fmt) {
				case 'c': {
					const int ch = va_arg(args, int);
					tty_putc(tty, (char)ch);
					count++;
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
				case 'd': {
					char buf[64];
					const int num = va_arg(args, int);
					itoa(num, buf, 10);
					tty_puts(tty, buf);
					break;
				};
				case 'u': {
					char buf[64];
					unsigned int v = va_arg(args, unsigned int);
					utoa(v, buf, 10);
					tty_puts(tty, buf);
					break;
				};
				case 'x': {
					char buf[64];
					const unsigned int val = va_arg(args, unsigned int);
					utoa(val, buf, 16);
					tty_puts(tty, buf);
					break;
				};
				case 'p': {
					char buf[64];
					uintptr_t v = (uintptr_t)va_arg(args, void*);
					tty_puts(tty, "0x");
					utoa64((uint64_t)v, buf, 16);
					tty_puts(tty, buf);
					break;
				};
				case 'f': {
					char buf[64];
					double num = va_arg(args, double);
					dtoa(num, buf, 6);
					tty_puts(tty, buf);
					break;
				};
				default:
					break;
				};
			};
		} else {
			tty_putc(tty, *fmt);
			count++;
		};
		fmt++;
	};
	return 0;
};

int kprintf(const char* fmt, ...)
{
	tty_t* tty = tty_get_active();

	if (!fmt || !tty)
		return 0;

	va_list args;
	va_start(args, fmt);
	const int n = vkprintf(tty, fmt, args);
	va_end(args);

	return n;
};