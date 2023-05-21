#include <mlib/mstr.h>

int mstr_atoi(const char *str) {
  int result;
  while (mstr_isdigit(*str)) {
    result = result * 10 + (unsigned int)(*str - '0');
    str++;
  }
  return result;
}
