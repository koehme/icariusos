#include "ctype.h"

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