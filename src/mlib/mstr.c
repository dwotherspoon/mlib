#include <mlib/mstr.h>

int mstr_atoi(const char *str) {
    int result;
    while (mstr_isdigit(*str)) {
        result = result * 10 + (unsigned int)(*str - '0');
        str++;
    }
    return result;
}

size_t mstr_strlen(char *str) {
    char *s = str;
    while (*s) {
        s++;
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
