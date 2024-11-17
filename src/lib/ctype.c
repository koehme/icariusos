/**
 * @file ctype.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "ctype.h"

/* PUBLIC API */
int isalpha(int c);

int isalpha(int c)
{
	// Check for uppercase letters: 'A' (65) to 'Z' (90)
	if (c >= 'A' && c <= 'Z') {
		return 1;
	};
	// Check for lowercase letters: 'a' (97) to 'z' (122)
	if (c >= 'a' && c <= 'z') {
		return 1;
	};
	// If not an alphabetic character, return 0
	return 0;
}