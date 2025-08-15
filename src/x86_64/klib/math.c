/**
 * @file math.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "math.h"

/* PUBLIC API */
double pow(double x, double y);

double pow(double x, double y)
{
	double result = 1.0;

	for (int i = 0; i < y; ++i) {
		result *= x;
	};
	return result;
};