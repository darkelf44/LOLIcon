// Include file header
#include <util/kstr.h>

// Local includes
#include <util/kchar.h>

size_t kstrlen(const char * str)
{
	size_t n = 0;
	for (; str[n]; ++ n) {}
	return n;
}

// Set the value of a string buffer
void kstrcpy(char * dest, const char * src, size_t n)
{
	// Copy the string
	for (; n > 1 && *src; (-- n, ++ dest, ++ src))
		*dest = *src;
	// Terminate string
	*dest = 0;
}

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

// Convert signed integers to string
char * kint2str(char * buffer, int32_t value)
{
	if (value < 0)
	{
		// Add sign and convert
		*buffer = '-';
		return (kuint2str(buffer + 1, -value), buffer);
	}
	else
	{
		// Convert as unsigned
		return kuint2str(buffer, value);
	}
}

// Convert unsigned integers to string
char * kuint2str(char * buffer, uint32_t value)
{
	// Variables
	uint32_t limit;
	size_t n;

	// Get number of digits
	for (n = 1, limit = 10; (n < 10) && !(value < limit); ++ n)
		limit *= 10;

	// Terminate buffer
	buffer[n] = 0;
	
	// Convert digits
	while (n -- > 0)
	{
		buffer[n] = (uint32_t) value % 10 + '0';
		value /= 10;
	}

	// return buffer
	return buffer;
}

