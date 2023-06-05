#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int main(int argc, char *argv[]) {
    /* Integer types */
    printf("sizeof(char) = %lu\n", sizeof(char));	
    printf("sizeof(unsigned char) = %lu\n", sizeof(unsigned char));
    printf("sizeof(short) = %lu\n", sizeof(short));
    printf("sizeof(unsigned short) = %lu\n", sizeof(unsigned short));
    printf("sizeof(int) = %lu\n", sizeof(int));
    printf("sizeof(wchar_t) = %lu\n", sizeof(wchar_t));
    printf("sizeof(unsigned int) = %lu\n", sizeof(unsigned int));
    printf("sizeof(long int) = %lu\n", sizeof(long int));
    printf("sizeof(unsigned long int) = %lu\n", sizeof(unsigned long int));
    printf("sizeof(long long int) = %lu\n", sizeof(long long int));
    printf("sizeof(unsigned long long int) = %lu\n", sizeof(unsigned long long int));
    printf("sizeof(intmax_t) = %lu\n", sizeof(intmax_t));
    printf("sizeof(uintmax_t) = %lu\n", sizeof(uintmax_t));
    printf("sizeof(size_t) = %lu\n", sizeof(size_t));
    printf("sizeof(prtdiff_t) = %lu\n", sizeof(ptrdiff_t));
    /* Float types */
    printf("sizeof(double) = %lu\n", sizeof(double));
    printf("sizeof(long double) = %lu\n", sizeof(long double));
    return 0;
}