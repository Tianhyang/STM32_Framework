#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

void log_init(void);
void log_info(const char *tag, const char *message);
void log_printf(const char *tag, const char *format, ...);
void log_vprintf(const char *tag, const char *format, va_list args);

#endif
