#ifndef _MPRINTF_H
#define _MPRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

int mprintf_vsnprintf(char *buf, const size_t maxlen, const char *fmt, va_list va);

#endif
