/**
 * @file stdio.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "stdio.h"
#include "idt.h"
#include "spinlock.h"
#include "stdlib.h"

/* PUBLIC API */
int kprintf(const char* fmt, ...);
int vkprintf(const char* fmt, va_list args);

int vkprintf(const char* fmt, va_list args)
{
	while (*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;

			if (*fmt == '%') {
				vbe_draw_ch(&vbe_display, '%', VBE_COLOR_GREEN);
			} else {
				switch (*fmt) {
				case 'c': {
					const char ch = va_arg(args, int);
					vbe_draw_ch(&vbe_display, ch, VBE_COLOR_GREEN);
					break;
				};
				case 's': {
					const char* str = va_arg(args, const char*);
					vbe_draw_string(&vbe_display, str, VBE_COLOR_GREEN);
					break;
				};
				case 'd': {
					char buffer[64];
					const int num = va_arg(args, int);
					itoa(num, buffer, 10);
					vbe_draw_string(&vbe_display, buffer, VBE_COLOR_GREEN);
					break;
				};
				case 'x': {
					char buffer[64];
					const unsigned int val = va_arg(args, unsigned int);
					utoa(val, buffer, 16);
					vbe_draw_string(&vbe_display, buffer, VBE_COLOR_GREEN);
					break;
				};
				case 'f': {
					char buffer[64] = {};
					double num = va_arg(args, double);
					dtoa(num, buffer, 6);
					vbe_draw_string(&vbe_display, buffer, VBE_COLOR_GREEN);
					break;
				};
				default:
					break;
				};
			};
		} else {
			vbe_draw_ch(&vbe_display, *fmt, VBE_COLOR_GREEN);
		};
		fmt++;
	};
	return 0;
};

int kprintf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vkprintf(fmt, args);
	va_end(args);
	return 0;
};