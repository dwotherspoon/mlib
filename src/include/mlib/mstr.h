#ifndef _MSTR_H
#define _MSTR_H

#include <stddef.h>

/* Common string functions required by mprintf and mfat */
static inline int mstr_isupper(unsigned char c) {
    return c >= 'A' && c <= 'Z';
}

static inline int mstr_islower(unsigned char c) {
    return c >= 'a' && c <= 'z';
}

static inline int mstr_isdigit(unsigned char c) {
    return c >= '0' && c <= '9';
}

static inline int mstr_iswhitespace(unsigned char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

static inline int mstr_issep(unsigned char c) {
    return c == '/' || c == '\\';
}

static inline unsigned char mstr_toupper(unsigned char c) {
    return mstr_islower(c) ? (c - ('a' - 'A')) : c;
}

static inline unsigned char mstr_tolower(unsigned char c) {
    return mstr_islower(c) ? (c + ('a' - 'A')) : c;
}

static inline int mstr_ishexdigit(unsigned char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static inline int mstr_hextonibble(unsigned char c) {
    if (mstr_isdigit(c)) {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    /* Error */
    return -1;
}

char *mstr_strrev(char *str);
int mstr_atoi(const char *str);
char *mstr_itoa(int value, char *str, int base);
size_t mstr_strlen(const char *str);
size_t mstr_strnlen(const char *str, size_t max);
int mstr_strncmp(const char *str1, const char *str2, size_t num);
int mstr_strcmp(const char *str1, const char *str2);
char *mstr_strncpy(char *dest, const char *src, size_t num);
char *mstr_strnchr(char *str, size_t num, const char chr);

#endif
