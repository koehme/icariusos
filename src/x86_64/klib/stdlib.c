/**
 * @file stdlib.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "stdlib.h"
#include "string.h"
#include "types.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
usize itoa(s64 value, const s32 base, const b8 upper, ch* buf);
usize utoa(u64 value, const s32 base, const b8 upper, ch* buf);

/* INTERNAL API */
// -

usize itoa(s64 value, const s32 base, const b8 upper, ch* buf)
{
	if (base < 2 || base > 16) {
		buf[0] = '0';
		buf[1] = '\0';
		return 0;
	};

	if (value < 0) {
		*buf++ = '-';
		u64 mag = (u64)(-(value + 1)) + 1;
		return utoa(mag, base, upper, buf);
	};
	return utoa((u64)value, base, upper, buf);
};

usize utoa(u64 value, const s32 base, const b8 upper, ch* buf)
{
	if (base < 2 || base > 16) {
		buf[0] = '0';
		buf[1] = '\0';
		return 0;
	};
	usize count = 0;
	static const ch lower_case[] = "0123456789abcdef";
	static const ch upper_case[] = "0123456789ABCDEF";
	const ch* template = upper ? upper_case : lower_case;

	ch tmp[65]; // max. 64 digits (binary) + '\0'
	usize i = 0;

	if (value == 0) {
		tmp[i++] = '0';
	} else {
		while (value > 0) {
			tmp[i++] = template[value % (u64)base];
			value /= (u64)base;
		};
	};
	// reverse into buf
	for (usize j = 0; j < i; ++j)
		buf[j] = tmp[i - j - 1];

	buf[i] = '\0';
	count = strlen(buf);
	return count;
};