#include <mlib/mprintf.h>
#include <mlib/mstr.h>

/* https://cplusplus.com/reference/cstdio/printf/ */
static int uprintf_vsnprintf(char *buf, const size_t maxlen, const char *fmt, va_list va)
{
    while (*fmt) {
        if (*fmt != '%') {
            /* Output char */
            fmt++;
        } else {
            /* Flags */
            switch (*fmt) {
                case '-':
                    /* Left justify */
                    break;
                case '+':
                    /* Force sign for both positive and negative numbers. */
                    break;
                case ' ':
                    /* Blank space before the value */
                    break;
            }
            /* Width specifier */

            /* Precision */

            /* Length */

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
