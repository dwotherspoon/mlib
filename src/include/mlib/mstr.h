#ifndef _MSTR_H
#define _MSTR_H

#include <stddef.h>

/* Common string functions required by mprintf and mfat */

#define mstr_isupper(C)      ((C) >= 'A' && (C) <= 'Z')
#define mstr_islower(C)      ((C) >= 'a' && (C) <= 'z')
#define mstr_isdigit(C)      ((C) >= '0' && (C) <= '9')
#define mstr_issep(C)        ((C) == '/' || (C) == '\\')
#define mstr_iswhitespace(C) ((C) == ' ' || (C) == '\t' || (C) == '\r' || (C) == '\n' || (C) == '\v' || (C) == '\f')
#define mstr_toupper(C)      (mstr_islower(C) ? ((C) - ('a' - 'A')) : (C))

char *mstr_strrev(char *str);
int mstr_atoi(const char *str);
char *mstr_itoa(int value, char *str, int base);
size_t mstr_strlen(char *str);
size_t mstr_strnlen(char *str, size_t max);

#endif
