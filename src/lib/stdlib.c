/**
 * @file stdlib.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "math.h"
#include "stdlib.h"

/* PUBLIC API */
size_t slen(const char* str);
char* itoa(int value, char* str, int base);
void dtoa(double n, char* res, int afterpoint);
char* utoa(unsigned int num, char* str, int base);
void dtoa(double n, char* res, int afterpoint);

size_t slen(const char* str)
{
	size_t i;

	for (i = 0; str[i] != '\0'; ++i) {
	};
	return i;
};

// Reverses a string in place
void reverse_string(char* str, size_t length)
{
	size_t start = 0;
	size_t end = length - 1;

	while (start < end) {
		// Swap characters at positions start and end
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		// Move towards the middle
		start++;
		end--;
	};
	return;
};

// Converts an integer to a string based on the specified base
char* itoa(int value, char* str, int base)
{
	// Index for the resulting string
	size_t i = 0;
	// Flag for negative numbers
	bool is_negative = false;
	// Unsigned version of the value to handle overflow
	unsigned int uvalue;
	// Validate the base
	if (base < 2 || base > 36) {
		// Return an empty string for invalid bases
		str[0] = '\0';
		return str;
	};
	// Handle zero explicitly
	if (value == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	};
	// Handle negative numbers for base 10
	if (value < 0 && base == 10) {
		is_negative = true;
		// Convert to unsigned to avoid overflow with INT32_MIN
		uvalue = (unsigned int)(-(value + 1)) + 1;
	} else {
		uvalue = (unsigned int)value;
	};
	// Process individual digits
	while (uvalue != 0) {
		unsigned int rem = uvalue % base;

		if (rem > 9) {
			// Convert remainder to corresponding lowercase letter (a-f)
			str[i++] = (rem - 10) + 'a';
		} else {
			// Convert remainder to corresponding digit (0-9)
			str[i++] = rem + '0';
		};
		uvalue = uvalue / base;
	};
	// Add negative sign if necessary
	if (is_negative) {
		str[i++] = '-';
	};
	// Null-terminate the string
	str[i] = '\0';
	// Reverse the string to get the correct order
	reverse_string(str, i);
	return str;
};


char* utoa(unsigned int num, char* str, int base)
{
	size_t i = 0;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	};

	while (num != 0) {
		const uint32_t rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	};

	str[i] = '\0';
	reverse_string(str, i);
	return str;
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
	int i = slen(res);
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