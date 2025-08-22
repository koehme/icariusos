/**
 * @file math.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "math.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
double pow(double x, double y);

/* INTERNAL API */
// -

double pow(double x, double y)
{
	double result = 1.0;

	for (int i = 0; i < y; ++i)
		result *= x;
	return result;
};