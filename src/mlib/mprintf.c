#include <mlib/mprintf.h>
#include <mlib/mstr.h>

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




/* https://cplusplus.com/reference/cstdio/printf/ */
int mprintf_vsnprintf(char *buf, const size_t maxlen, const char *fmt, va_list args) {
    unsigned int temp, flags, width, precision;
    while (*fmt) {
        if (*fmt != '%') {
            /* Output char */
            fmt++;
        } else {
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
                    default :
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
            precision = 0;
            if (*fmt == '.') {
                fmt++;
                if (mstr_isdigit(*fmt)) {
                    /* ATOI but quicker this way */
                    while (mstr_isdigit(*fmt)) {
                        precision = precision * 10 + (unsigned int)(*fmt - '0');
                        fmt++;
                    }
                }
                else if (*fmt == '*') {
                    /* Precision from argument */
                    precision = va_arg(args, int);
                    /* TODO: What if precision is negative? Clamp? */
                    fmt++;
                }
            }
            /* Length */
            switch (*fmt) {
                case 'h':
                    break;
                case 'l':
                    break;
                case 'j':
                    break;
                case 'z':
                    break;
                case 't':
                    break;
                case 'L':
                    break;
                default:
                    break;
            }

            /* Specifier */
            switch (*fmt) {
                case 'd':
                    /* Fallthrough */
                case 'i':
                    /* Signed decimal integer */
                    break;
                case 'u':
                    /* Unsigned decimal integer */
                    break;
                case 'o':
                    /* Unsigned octal */
                    break;
                case 'x':
                    /* Unsigned hexadecimal integer (lowecase) */
                    break;
                case 'X':
                    /* Unsigned hexadecimal integer (uppercase) */
                    break;
                case 'f':
                    /* Decimal floating point (lowercase) */
                    break;
                case 'F':
                    /* Decimal floating point (uppercase) */
                    break;
                case 'e':
                    /* Scientific notation (lowercase) */
                    break;
                case 'E':
                    /* Scientific notation (uppercase) */
                    break;
                case 'g':
                    /* Shortest representation between e and f */
                    break;
                case 'G':
                    /* Shortest representation between E and F */
                    break;
                case 'a':
                    /* Hexadecimal floating point (lowercase) */
                    break;
                case 'A':
                    /* Hexadecimal floating point (uppercase) */
                    break;
                case 'c':
                    /* Character */
                    break;
                case 's':
                    /* String of characters */
                    break;
                case 'p':
                    /* Pointer address */
                    break;
                case 'n':
                    /* Nothing printed, store characters printed so far */
                    break;
                case '%':
                    /* Single % */
                    break;
            }
        }
    }
}
