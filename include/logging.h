#pragma once

// Log severity
enum ESeverity
{
	Trace    = -200,
	Debug    = -100,
	Notice   = 0,
	Warning  = 100,
	Error    = 200,
	Critical = 300,
};
typedef enum ESeverity Severity;

// Enable/disable logging globally
extern bool enable_logging;

// Basic logging
void logf(Severity level, const char * msg, ...);
void logv(Severity level, const char * msg, va_list list);
