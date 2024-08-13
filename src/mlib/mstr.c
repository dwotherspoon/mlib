#include <mlib/mstr.h>
#include <stdint.h>

int mstr_atoi(const char *str) {
    int result;
    while (mstr_isdigit(*str)) {
        result = result * 10 + (unsigned int)(*str - '0');
        str++;
    }
    return result;
}

size_t mstr_strlen(const char *str) {
    return mstr_strnlen(str, SIZE_MAX);
}

size_t mstr_strnlen(const char *str, size_t max) {
    char *s = str;
    for ( ; *s && max; s++, max-- ) {
    }
    return s - str;
}

char *mstr_strrev(char *str) {
    size_t i, j;
    char temp;

    for (i = 0, j = mstr_strlen(str) - 1; i < j; i++, j--) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }

  return str;
}

char *mstr_itoa(int value, char *str, int base) {
    unsigned v, i, idx, start_idx;
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    if (value < 0 && base == 10) {
        str[0] = '-';
        idx = 1;
        v = -value;
    } else {
        idx = 0;
        v = (unsigned)value;
    }

    start_idx = idx;

    for (; v; v /= base, idx++) {
        i = v % base;
        str[idx] = (i < 10) ? ('0' + i) : ('a' + i - 10);
    }

    str[idx] = '\0';

    mstr_strrev(&str[start_idx]);

    return str;
}

int mstr_strcmp(const char *str1, const char *str2) {
    return mstr_strncmp(str1, str2, SIZE_MAX);
}

int mstr_strncmp(const char *str1, const char *str2, size_t num) {
    char c1 = '\0', c2 = '\0';
    for (; num; num--) {
        c1 = *str1++;
        c2 = *str2++;
        if (c1 == '\0' || c1 != c2) {
            return c1 - c2;
        }
    }
    return c1 - c2;
}

char *mstr_strncpy(char *dest, const char *src, size_t num) {
    size_t i;
    for (i = 0; src[i] && i < num; i++) {
        dest[i] = src[i];
    }
    for (; i < num; i++) {
        dest[i] = '\0';
    }
    return dest;
}
