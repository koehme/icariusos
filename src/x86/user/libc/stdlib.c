#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <stdbool.h>

int atoi(const char* str)
{
	int res = 0;

	while (*str >= '0' && *str <= '9') {
		res = res * 10 + (*str - '0');
		str++;
	};
	return res;
};

void dtoa(double n, char* res, int afterpoint)
{
	// Handle negative numbers
	bool is_negative = false;

	if (n < 0) {
		is_negative = true;
		n = -n;
	};
	// Extract integer part
	const int32_t ipart = (int32_t)n;
	// Extract floating part
	double fpart = n - (double)ipart;
	// Convert integer part to string use base 10
	itoa(ipart, res, 10);
	// Get length of integer part
	int i = strlen(res);
	// Add negative sign if necessary
	if (is_negative) {
		// Shift the string to the right to make space for '-'
		for (int j = i; j >= 0; --j) {
			res[j + 1] = res[j];
		};
		res[0] = '-';
		i++;
	};
	// Check for display option after decimal point
	if (afterpoint != 0) {
		// Add decimal point
		res[i] = '.';
		i++;
		// Multiply fractional part to get desired number of digits
		for (int j = 0; j < afterpoint; j++) {
			fpart *= 10;
		};
		// Round the fractional part
		fpart += 0.5;
		// Convert fractional part to string
		itoa((int32_t)fpart, res + i, 10);
	} else {
		res[i] = '\0'; // Null-terminate the string
	};
	return;
};