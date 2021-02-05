#pragma once

// Includes
#include <stdbool.h>

// INLINE macro - Forces the function to be inlined
#undef INLINE
#define INLINE __attribute__((always_inline)) __inline__

// String compare
int kstrcmp(const char * left, const char * right);
int kstrcmpi(const char * left, const char * right);

// String equality
__attribute__((always_inline)) __inline__ bool kstreq(const char * left, const char * right)
	{return kstrcmp(left, right) == 0;}
__attribute__((always_inline)) __inline__ bool kstreqi(const char * left, const char * right)
	{return kstrcmpi(left, right) == 0;}
