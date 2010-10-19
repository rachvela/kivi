#ifndef __LOGGING
#define __LOGGING

#include <stdio.h>
#include <stdarg.h>

#ifndef KIVI_DEBUG
	#define KIVI_DEBUG 0
#endif 

typedef enum log_level {
	DEBUG,
	INFO,
	WARN,
	ERROR
} log_level;

char* log_level_to_str[4] = {"DEBUG", "INFO", "WARN", "ERROR"};

void log_msg(log_level level, char* format, ...) {
	va_list argp;
	va_start(argp, format);
	printf("%s: ", log_level_to_str[level]);
	vprintf(format, argp);
	va_end(argp);
}

void vlog_msg(log_level level, char* format, va_list argp) {
	printf("%s: ", log_level_to_str[level]);
	vprintf(format, argp);
	printf("\n");
}

void log_debug(char* format, ...) {
	#ifdef KIVI_DEBUG
	if (KIVI_DEBUG) {
		va_list argp;
		va_start(argp, format);
		vlog_msg(DEBUG, format, argp);
		va_end(argp);
	}
	#endif
}

void log_info(char* format, ...) {
	va_list argp;
	va_start(argp, format);
	vlog_msg(INFO, format, argp);
	va_end(argp);
}

void log_warn(char* format, ...) {
	va_list argp;
	va_start(argp, format);
	vlog_msg(WARN, format, argp);
	va_end(argp);
}

void log_error(char* format, ...) {
	va_list argp;
	va_start(argp, format);
	vlog_msg(ERROR, format, argp);
	va_end(argp);
}

#endif