#include <util/kstr.h>
#include <util/kchar.h>

// Compare with case
int kstrcmp(const char * left, const char * right)
{
	// Find first difference
	for (; *left && *right && *left == *right; (++ left, ++ right)) {}
	// Return the ordering
	return (int) (unsigned char) *left - (int) (unsigned char) *right;
}

// Compare and ignore case
int kstrcmpi(const char * left, const char * right)
{
	// Find first difference
	for (; *left && *right && (klower(*left) == klower(*right)); (++ left, ++ right)) {}
	// Return the ordering
	return (int) (unsigned char) klower(*left) - (int) (unsigned char) klower(*left);
}

