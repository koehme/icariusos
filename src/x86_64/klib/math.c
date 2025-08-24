/**
 * @file math.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "math.h"
#include "types.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
f64 pow(const f64 x, const f64 y);

/* INTERNAL API */
// -

f64 pow(const f64 x, const f64 y)
{
	f64 result = 1.0;

	for (usize i = 0; i < y; ++i)
		result *= x;
	return result;
};