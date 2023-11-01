#include <mlib/mprintf.h>
#include <mlib/mstr.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <float.h>

#include <stdio.h>

#define MPRINTF_ABS(VAL) (((VAL) < 0) ? -(VAL) : (VAL))

/* Flag values, used internally when parsing flags field */

/* '#' - Alternate form (0 or 0x prefix, force decimal point or keep trailing zeros)*/
#define MPRINTF_FLAG_ALTFORM    (1 << 0)
/* '0' - Zero pad flag */
#define MPRINTF_FLAG_ZEROPAD    (1 << 1)
/* '-' - Left justify flag */
#define MPRINTF_FLAG_LEFTJUST   (1 << 2)
/* ' ' - Add space if no sign */
#define MPRINTF_FLAG_SPACESIGN  (1 << 3)
/* '+' - Force sign */
#define MPRINTF_FLAG_FORCESIGN  (1 << 4)
/* Precision was specified */
#define MPRINTF_FLAG_PRECISION  (1 << 5)
/* Uppercase form */
#define MPRINTF_FLAG_UPPERCASE  (1 << 6)
/* Signed argument */
#define MPRINTF_FLAG_SIGNED     (1 << 7)
/* Character length specified */
#define MPRINTF_FLAG_CHAR       (1 << 8)
/* Short length specified */
#define MPRINTF_FLAG_SHORT      (1 << 9)
/* Long length specified */
#define MPRINTF_FLAG_LONG       (1 << 10)
/* Long long length specified */
#define MPRINTF_FLAG_LONGLONG   (1 << 11)
/* Long double length specified */
#define MPRINTF_FLAG_LONGDOUBLE (1 << 12)

/* Configuration */
#define MPRINTF_NTOA_BUFFER_LEN        (32)
#define MPRINTF_FTOA_BUFFER_LEN        (32)
#define MPRINTF_FTOA_MAX_PRECISION     (10)
#define MPRINTF_FTOA_DEFAULT_PRECISION (6)
#define MPRINTF_PUTCHAR putchar

#define MPRINTF_CONFIG_FP

char const *mprintf_lowercase_digit_lut = "0123456789abcdef";
char const *mprintf_uppercase_digit_lut = "0123456789ABCDEF";

struct mprintf_output {
    void (*func)(char c);
    char *buf;
    size_t pos;
    size_t max;
};

struct mprintf_output mprintf_console_output = {MPRINTF_PUTCHAR, NULL, 0, SIZE_MAX};

/* Put a char to the output device or string */
void mprintf_putchar(struct mprintf_output *out, char c) {
    out->pos++;

    if (out->pos >= out->max) {
        return;
    }

    if (out->func != NULL) {
        out->func(c);
    } else {
        out->buf[out->pos] = c;
    }
}

int mprintf_puts(struct mprintf_output *out, char *s) {
    return 0;
}

size_t mprintf_out_rev(struct mprintf_output *out, const char *buf, size_t len, unsigned int width, unsigned int flags) {
    const size_t start_pos = out->pos;

    // pad spaces up to given width
    if (!(flags & MPRINTF_FLAG_LEFTJUST) && !(flags & MPRINTF_FLAG_ZEROPAD)) {
        for (size_t i = len; i < width; i++) {
            mprintf_putchar(out, ' ');
        }
    }

    /* Output reversed string */
    while (len) {
        mprintf_putchar(out, buf[--len]);
    }

    /* Append pad spaces up to given width */
    if (flags & MPRINTF_FLAG_LEFTJUST) {
        while (out->pos - start_pos < width) {
            mprintf_putchar(out, ' ');
        }
    }
}

size_t mprintf_ntoa_format(struct mprintf_output *out, char *buf, size_t len, bool neg,
                            unsigned int base, unsigned int prec, unsigned int width, unsigned int flags) {
    /* Pad with leading zeros for right justification */
    if (!(flags & MPRINTF_FLAG_LEFTJUST)) {
        if (width && (flags & MPRINTF_FLAG_ZEROPAD) && (neg || (flags & (MPRINTF_FLAG_FORCESIGN | MPRINTF_FLAG_SPACESIGN)))) {
            width--;
        }
        /* First pad with zeros to precision */
        for (; (len < prec) && (len < MPRINTF_NTOA_BUFFER_LEN); len++) {
            buf[len] = '0';
        }
        /* Now pad to width if zero pad */
        for (; (flags & MPRINTF_FLAG_ZEROPAD) && (len < width) && (len < MPRINTF_NTOA_BUFFER_LEN); len++) {
            buf[len] = '0';
        }
    }

    /* Handle alternate form */
    if (flags & MPRINTF_FLAG_ALTFORM) {
        if (!(flags & MPRINTF_FLAG_PRECISION) && len && ((len == prec) || (len == width))) {
            len--;
            if (len && (base == 16)) {
                len--;
            }
        }
        if ((base == 16) && (flags & MPRINTF_FLAG_UPPERCASE) && (len < MPRINTF_NTOA_BUFFER_LEN)) {
            buf[len++] = 'X';
        }
        else if ((base == 16) && (len < MPRINTF_NTOA_BUFFER_LEN)) {
            buf[len++] = 'x';
        }
        else if ((base == 2) && (len < MPRINTF_NTOA_BUFFER_LEN)) {
            buf[len++] = 'b';
        }
        if (len < MPRINTF_NTOA_BUFFER_LEN) {
            buf[len++] = '0';
        }
    }

    /* Handle Sign */
    if (len < MPRINTF_NTOA_BUFFER_LEN) {
        if (neg) {
            buf[len++] = '-';
        }
        else if (flags & MPRINTF_FLAG_FORCESIGN) {
            buf[len++] = '+';
        }
        else if (flags & MPRINTF_FLAG_SPACESIGN) {
            buf[len++] = ' ';
        }
    }

    return mprintf_out_rev(out, buf, len, width, flags);
}

size_t mprintf_ntoa_long(struct mprintf_output *out, unsigned long value, bool neg,
                            unsigned long base, unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[MPRINTF_NTOA_BUFFER_LEN];
    size_t len = 0;

    /* No alt form for zero values */
    if (!value) {
        flags &= ~MPRINTF_FLAG_ALTFORM;
    }

    // write if precision != 0 and value is != 0
    if (!(flags & MPRINTF_FLAG_PRECISION) || value) {
        for (; value && (len < MPRINTF_NTOA_BUFFER_LEN); len++) {
            const char digit = (char)(value % base);
            buf[len] = (flags & MPRINTF_FLAG_UPPERCASE) ? mprintf_lowercase_digit_lut[digit] : mprintf_uppercase_digit_lut[digit];
            value /= base;
        }
    }

  return mprintf_ntoa_format(out, buf, len, neg, (unsigned int)base, prec, width, flags);
}

size_t mprintf_ftoa(struct mprintf_output *out, double value, unsigned int prec, unsigned int width, unsigned int flags) {
    char buf[MPRINTF_FTOA_BUFFER_LEN];
    bool neg = false;

    /* Special cases */
    if (value != value) {
        return mprintf_out_rev(out, "nan", 3, width, flags);
    }
    if (value < -DBL_MAX) {
        return mprintf_out_rev(out, "fni-", 4, width, flags);
    }
    if (value > DBL_MAX) {
        return mprintf_out_rev(out, (flags & MPRINTF_FLAG_FORCESIGN) ? "fni+" : "fni", (flags & MPRINTF_FLAG_FORCESIGN) ? 4U : 3U, width, flags);
    }

    if (!(flags & MPRINTF_FLAG_PRECISION)) {
        prec = MPRINTF_FTOA_DEFAULT_PRECISION;
    }

    if (prec < 0)
    {
        if (value < 1.0) {
            prec = 6;
        }
        else if (value < 10.0) {
            prec = 5;
        }
        else if (value < 100.0) {
            prec = 4;
        }
        else if (value < 1000.0) {
            prec = 3;
        }
        else if (value < 10000.0) {
            prec = 2;
        }
        else if (value < 100000.0) {
            prec = 1;
        } else {
            prec = 0;
        }
    }
}

/* https://cplusplus.com/reference/cstdio/printf/
*/
void mprintf_format_loop(struct mprintf_output *out, const char *fmt, va_list args) {
    unsigned int temp, flags, width, prec, base;
    while (*fmt) {
        if (*fmt != '%') {
            /* Output char */
            mprintf_putchar(out, *fmt);
            fmt++;
        } else {
            fmt++;
            /* Flags */
            flags = 0;
            do {
                switch (*fmt) {
                    case '#':
                        flags |= MPRINTF_FLAG_ALTFORM;
                        temp = 1;
                        fmt++;
                        break;
                    case '-':
                        flags |= MPRINTF_FLAG_LEFTJUST;
                        temp = 1;
                        fmt++;
                        break;
                    case '+':
                        flags |= MPRINTF_FLAG_FORCESIGN;
                        temp = 1;
                        fmt++;
                        break;
                    case ' ':
                        flags |= MPRINTF_FLAG_SPACESIGN;
                        temp = 1;
                        fmt++;
                        break;
                    case '0':
                        flags |= MPRINTF_FLAG_ZEROPAD;
                        temp = 1;
                        fmt++;
                        break;
                    default:
                        /* Not a format character, exit loop */
                        temp = 0;
                        break;
                }
            } while (temp);
            /* Width specifier */
            width = 0;
            if (mstr_isdigit(*fmt)) {
                /* ATOI but quicker this way */
                while (mstr_isdigit(*fmt)) {
                    width = width * 10 + (unsigned int)(*fmt - '0');
                    fmt++;
                }
            }
            else if (*fmt == '*') {
                /* Width from argument */
                width = va_arg(args, int);
                /* TODO: What if width is negative? */
                fmt++;
            }

            /* Precision */
            prec = 0;
            if (*fmt == '.') {
                flags |= MPRINTF_FLAG_PRECISION;
                fmt++;
                if (mstr_isdigit(*fmt)) {
                    /* ATOI but quicker this way */
                    while (mstr_isdigit(*fmt)) {
                        prec = prec * 10 + (unsigned int)(*fmt - '0');
                        fmt++;
                    }
                }
                else if (*fmt == '*') {
                    /* Precision from argument */
                    prec = va_arg(args, int);
                    /* TODO: What if prec is negative? Clamp? */
                    fmt++;
                }
            }

            /* Length */
            switch (*fmt) {
                case 'h':
                    fmt++;
                    flags |= MPRINTF_FLAG_SHORT;
                    if (*fmt == 'h') {
                        fmt++;
                        flags |= MPRINTF_FLAG_CHAR;
                    }
                    break;
                case 'l':
                    fmt++;
                    flags |= MPRINTF_FLAG_LONG;
                    if (*fmt == 'l') {
                        fmt++;
                        flags |= MPRINTF_FLAG_LONGLONG;
                    }
                    break;
                case 'j':
                    /* Maximum integer type */
                    fmt++;
                    flags |= (sizeof(intmax_t) == sizeof(long) ? MPRINTF_FLAG_LONG : MPRINTF_FLAG_LONGLONG);
                    break;
                case 'z':
                    /* Size */
                    fmt++;
                    flags |= (sizeof(size_t) == sizeof(long) ? MPRINTF_FLAG_LONG : MPRINTF_FLAG_LONGLONG);
                    break;
                case 't':
                    /* Pointer difference */
                    fmt++;
                    flags |= (sizeof(ptrdiff_t) == sizeof(long) ? MPRINTF_FLAG_LONG : MPRINTF_FLAG_LONGLONG);
                    break;
                case 'L':
                    /* Long double (80bit) */
                    fmt++;
                    flags |= (sizeof(long double) == sizeof(double) ? MPRINTF_FLAG_LONG : MPRINTF_FLAG_LONGDOUBLE);
                    break;
                default:
                    break;
            }

            /* Type Specifier */
            switch (*fmt) {
                case 'd':
                    /* Fallthrough */
                case 'i':
                    /* Signed decimal integer */

                    /* Signed integer cases. */
                    flags |= MPRINTF_FLAG_SIGNED;
                    /* Ignore zero-padding when precision is specified */
                    if (flags & MPRINTF_FLAG_PRECISION) {
                        flags &= ~MPRINTF_FLAG_ZEROPAD;
                    }

                    if (flags & MPRINTF_FLAG_LONGLONG) {
                        const long long value = va_arg(args, long long);
                    }
                    else if (flags & MPRINTF_FLAG_LONG) {
                        const long value = va_arg(args, long);
                        mprintf_ntoa_long(out, MPRINTF_ABS(value), value < 0, 10, prec, width, flags);
                    } else {
                        if (flags & MPRINTF_FLAG_CHAR) {
                            const char value = (char)va_arg(args, int);
                            mprintf_ntoa_long(out, MPRINTF_ABS(value), value < 0, 10, prec, width, flags);
                        }
                        else if (flags & MPRINTF_FLAG_SHORT) {
                            const short int value = (short int)va_arg(args, int);
                            mprintf_ntoa_long(out, MPRINTF_ABS(value), value < 0, 10, prec,  width, flags);
                        }
                        else {
                            const int value = va_arg(args, int);
                            mprintf_ntoa_long(out, MPRINTF_ABS(value), value < 0, 10, prec, width, flags);
                        }
                    }
                    fmt++;
                    break;
                case 'u':
                    /* Unsigned decimal integer */
                    /* Fallthrough */
                case 'o':
                    /* Unsigned octal */
                    /* Fallthrough */
                case 'X':
                    /* Unsigned hexadecimal integer (lowecase) */
                    /* Fallthrough */
                case 'x':
                    /* Unsigned hexadecimal integer (uppercase) */
                    /* Fallthrough */
                case 'b':
                    /* Binary integer (not standard printf) */

                    /* Unsigned integers, choose base and fix flags. */
                    if (*fmt == 'x' || *fmt == 'X') {
                        base = 16;
                    }
                    else if (*fmt == 'o') {
                        base = 8;
                    }
                    else if (*fmt == 'b') {
                        base = 2;
                    }
                    else {
                        base = 10;
                        /* No alt form for decimal numbers */
                        flags &= ~MPRINTF_FLAG_ALTFORM;
                    }

                    if (*fmt == 'X') {
                        flags |= MPRINTF_FLAG_UPPERCASE;
                    }

                    /* Force sign and space sign not allowed for unsigned numbers */
                    flags &= ~(MPRINTF_FLAG_FORCESIGN | MPRINTF_FLAG_SPACESIGN);

                    /* Ignore zero-padding when precision is specified */
                    if (flags & MPRINTF_FLAG_PRECISION) {
                        flags &= ~MPRINTF_FLAG_ZEROPAD;
                    }

                    if (flags & MPRINTF_FLAG_LONGLONG) {
                        /* mprintf_ntoa_long_long(out, va_arg(va, unsigned long long), false, base, prec, width, flags); */
                    }
                    else if (flags & MPRINTF_FLAG_LONG) {
                        mprintf_ntoa_long(out, va_arg(args, unsigned long), false, base, prec, width, flags);
                    }
                    else {
                        if (flags & MPRINTF_FLAG_CHAR) {
                            const unsigned char value = (unsigned char)va_arg(args, unsigned int);
                            mprintf_ntoa_long(out, value, false, base, prec, width, flags);
                        }
                        else if (flags & MPRINTF_FLAG_SHORT) {
                            const unsigned short int value = (unsigned short int)va_arg(args, unsigned int);
                            mprintf_ntoa_long(out, value, false, base, prec, width, flags);
                        } else {
                            mprintf_ntoa_long(out, va_arg(args, unsigned int), false, base, prec, width, flags);
                        }
                    }
                    fmt++;
                    break;
                case 'f':
                    /* Decimal floating point (lowercase) */
                    break;
                case 'F':
                    /* Decimal floating point (uppercase) */
                    flags |= MPRINTF_FLAG_UPPERCASE;
                    break;
                case 'e':
                    /* Scientific notation (lowercase) */
                    break;
                case 'E':
                    /* Scientific notation (uppercase) */
                    flags |= MPRINTF_FLAG_UPPERCASE;
                    break;
                case 'g':
                    /* Shortest representation between e and f */
                    break;
                case 'G':
                    /* Shortest representation between E and F */
                    flags |= MPRINTF_FLAG_UPPERCASE;
                    break;
                case 'a':
                    /* Hexadecimal floating point (lowercase) */
                    break;
                case 'A':
                    /* Hexadecimal floating point (uppercase) */
                    flags |= MPRINTF_FLAG_UPPERCASE;
                    break;
                case 'c':
                    /* Character */
                    temp = 1;
                    /* Right padding */
                    if (!(flags & MPRINTF_FLAG_LEFTJUST)) {
                        for (; temp < width; temp++) {
                            mprintf_putchar(out, (flags & MPRINTF_FLAG_ZEROPAD) ? '0' : ' ');
                        }
                    }

                    mprintf_putchar(out, (char)va_arg(args, int));

                    /* Left Padding */
                    if (flags & MPRINTF_FLAG_LEFTJUST) {
                        for (; temp < width; temp++) {
                            /* Seems like left padding doesn't work with zeropad */
                            mprintf_putchar(out, ' ');
                        }
                    }
                    fmt++;
                    break;
                case 's':
                    /* String of characters */
                    fmt++;
                    break;
                case 'p':
                    /* Pointer address */
                    fmt++;
                    break;
                case 'n':
                    /* Nothing printed, store characters printed so far */
                    fmt++;
                    break;
                case '%':
                    /* Single % */
                    mprintf_putchar(out, '%');
                    fmt++;
                    break;
                default:
                    /* Unmatched, output character */
                    mprintf_putchar(out, *fmt);
                    fmt++;
                    break;
            }
        }
    }
}

void mprintf_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    mprintf_format_loop(&mprintf_console_output, fmt, args);
    va_end(args);
}
