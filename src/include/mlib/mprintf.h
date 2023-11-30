#ifndef _MPRINTF_H
#define _MPRINTF_H

#include <stdarg.h>
#include <stddef.h>

/* By default, ignore zero padding of character (%c). */
#define MPRINTF_ALLOW_CHARACTER_ZEROPAD
#define MPRINTF_FLOAT_SUPPORT
#define MPRINTF_FLOAT_DEFAULT_RECISION 6
#define MPRINTF_LONGLONG_SUPPORT

int mprintf_sprintf(char *buf, const char *fmt, ...);
int mprintf_vsprintf(char *buf, const char *fmt, va_list va);
int mprintf_snprintf(char *buf, size_t count, const char *fmt, ...);
int mprintf_vsnprintf(char *buf, size_t count, const char *fmt, va_list va);
int mprintf_funprintf(void (*out)(char c), const char *fmt, ...);

#endif
