#pragma once

// Includes
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// INLINE macro - Forces the function to be inlined
#undef INLINE
#define INLINE __attribute__((always_inline)) __inline__

// String functions
size_t kstrlen(const char * str);
void kstrcpy(char * dest, const char * src, size_t n);

// String compare
int kstrcmp(const char * left, const char * right);
int kstrcmpi(const char * left, const char * right);

// String equality
__attribute__((always_inline)) __inline__ bool kstreq(const char * left, const char * right)
	{return kstrcmp(left, right) == 0;}
__attribute__((always_inline)) __inline__ bool kstreqi(const char * left, const char * right)
	{return kstrcmpi(left, right) == 0;}

// Conversions from string
int32_t kstr2int(const char * text);
uint32_t kstr2uint(const char * text);

// Conversions to string
char * kint2str(char * buffer, int32_t value);
char * kuint2str(char * buffer, uint32_t value);
