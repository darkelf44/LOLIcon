#pragma once

enum E_Severity
{
	Trace    = -200,
	Debug    = -100,
	Notice   = 0,
	Warning  = 100,
	Error    = 200,
	Critical = 300,
};
typedef enum E_Severity Severity;

// Basic logging
void logf(Severity level, const char * msg, ...);
void logv(Severity level, const char * msg, va_list list);

// Logging for severity levels
void log_trace(const char * msg, ...);
void log_debug(const char * msg, ...);
void log_notice(const char * msg, ...);
void log_warning(const char * msg, ...);
void log_error(const char * msg, ...);
void log_critical(const char * msg, ...);