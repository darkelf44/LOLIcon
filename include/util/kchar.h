#pragma once

// Includes
#include <stdbool.h>

// INLINE macro - Forces the function to be inlined
#undef INLINE
#define INLINE __attribute__((always_inline)) __inline__

// Locale free character functions
INLINE bool kisascii(int ch)
	{return (unsigned) ch < 0x80;}
INLINE bool kisblank(int ch)
	{return ch == ' ' || ch == '\t';}
INLINE bool kisspace(int ch)
	{return ch == ' ' || (ch >= 0x9 && ch <= 0xD);}
INLINE bool kisalpha(int ch)
	{return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');}
INLINE bool kisalnum(int ch)
	{return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');}
INLINE bool kislower(int ch)
	{return ch >= 'a' && ch <= 'z';}
INLINE bool kisupper(int ch)
	{return ch >= 'a' && ch <= 'z';}
INLINE bool kisdigit(int ch)
	{return ch >= '0' && ch <= '9';}
INLINE bool kisbinch(int ch)
	{return ch == '0' || ch == '1';}
INLINE bool kisoctch(int ch)
	{return ch >= '0' && ch <= '7';}
INLINE bool kishexch(int ch)
	{return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');}

// Locale free character conversions
INLINE bool klower(int ch)
	{return kisupper(ch) ? ch + ('a' - 'A') : ch;}
INLINE bool kupper(int ch)
	{return kislower(ch) ? ch - ('a' - 'A') : ch;}

