#ifndef _MPRINTF_H
#define _MPRINTF_H

#include <stdarg.h>

void mprintf_printf(const char *fmt, ...);

#include <stddef.h>
struct mprintf_output {
    void (*func)(char c);
    char *buf;
    size_t pos;
    size_t max;
};

size_t mprintf_ftoa(struct mprintf_output *out, double value, unsigned int prec, unsigned int width, unsigned int flags);

/* By default, ignore zero padding of character (%c). */
/* #define MPRINTF_ALLOW_CHARACTER_ZEROPAD */
#define MPRINTF_FLOAT_SUPPORT
#define MPRINTF_FLOAT_DEFAULT_RECISION 6

#endif
