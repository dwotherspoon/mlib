#include <stdio.h>
#include <string.h>

#include <mlib/mfat.h>
#include <mlib/mprintf.h>
#include <mlib/mstr.h>

#define mprintf_printf(...) mprintf_funprintf((char (*)(char))putchar,  __VA_ARGS__)



void test_mstr_strncpy() {
    char stra[10] = {0};
    char strb[10] = {0};

    mstr_strncpy(strb, stra, 1);
}

void test_mstr_printf() {
    mprintf_funprintf(putchar,
                        "INIT %s (%s) @ %iMHz\r\n", "STM32H743", "Arm Cortex-M7", 0);
}

int main(int argc, char *argv[]) {
    char temp[128];
    printf("Sizeof char is %zu\n", sizeof(char));
    printf("Sizeof short is %zu\n", sizeof(short int));
    printf("Sizeof int is %zu\n", sizeof(int));
    printf("Sizeof long is %zu\n", sizeof(long));
    printf("Sizeof long long is %zu\n", sizeof(long long));

    printf("TEST 500: %sTEST\n", mstr_itoa(500, temp, 10));
    printf("TEST 0: %sTEST\n", mstr_itoa(0, temp, 10));
    printf("TEST 1000500: %sTEST\n", mstr_itoa(1000500, temp, 10));
    printf("TEST -500: %sTEST\n", mstr_itoa(-500, temp, 10));
    printf("TEST -500 hex: %sTEST\n", mstr_itoa(-500, temp, 16));
    printf("TEST -1000500 hex: %sTEST\n", mstr_itoa(-1000500, temp, 16));

    mprintf_printf("Hello, world!\n");
    printf("Hello, world!\n");

    mprintf_printf("Hello, world! %i\n", 200);
    printf("Hello, world! %i\n", 200);

    mprintf_printf("Hello, world! %hi\n", 0x1ffaff);
    printf("Hello, world! %hi\n", 0x1ffaff);

    mprintf_printf("Hello, world! %hhi\n", 2000);
    printf("Hello, world! %hhi\n", 2000);

    uint64_t test_64_bin = 0x5555555555555555;

    mprintf_printf("Hello, world! %#lx\n", test_64_bin);
    printf("Hello, world! %#lx\n", test_64_bin);

    mprintf_printf("Hello, world! %#lo : %% : %010c END\n", test_64_bin, 'a');
    printf("Hello, world! %#lo : %% : %010c END\n", test_64_bin, 'a');

    mprintf_printf("M%i\n", 0);
    printf("%i\n", 0);

    mprintf_printf("M%i\n", 10);
    printf("%i\n", 10);

    mprintf_printf("M%.0f\n", 1.5);
    printf("%.0f\n", 1.5);

    mprintf_printf("M%f\n", 1.5);
    printf("%f\n", 1.5);
    mprintf_printf("M%.0f\n", 2.5);
    printf("%.0f\n", 2.5);
    mprintf_printf("M%f\n", 2.5);
    printf("%f\n", 2.5);

    mprintf_printf("M%f\n", 0.5);
    printf("%f\n", 0.5);

    mprintf_printf("M%f\n", 10.5);
    printf("%f\n", 10.5);

    mprintf_printf("M%f\n", -10.5);
    printf("%f\n", -10.5);

    double POS_INF = 1.0 /0.0;

    mprintf_printf("M%f\n", POS_INF);
    printf("M%f\n", POS_INF);

    mprintf_printf("M%F\n", POS_INF);
    printf("M%F\n", POS_INF);

    double NAN = 0.0/0.0;

    mprintf_printf("M%f\n", NAN);
    printf("M%f\n", NAN);

    mprintf_printf("M%F\n", NAN);
    printf("M%F\n", NAN);

    char *test_str1 = "a";

    char *test_str2 = "The Quick Brown Fox";

    mprintf_printf("M%s\n", test_str1);
    printf("%s\n", test_str1);

    mprintf_printf("M%3sE\n", test_str1);
    printf("%3sE\n", test_str1);

    mprintf_printf("M%.3sE\n", test_str1);
    printf("%.3sE\n", test_str1);

    mprintf_printf("M%03sE\n", test_str1);
    printf("%03sE\n", test_str1);

    mprintf_printf("M%-3sE\n", test_str1);
    printf("%-3sE\n", test_str1);


    mprintf_printf("M%s\n", test_str2);
    printf("%s\n", test_str2);


    char *stra = "abcde";
    char *strb = "abc";

    printf("strcmp(stra, strb) %i (%i)\n", mstr_strcmp(stra, strb), strcmp(stra, strb));
    printf("strcmp(strb, stra) %i (%i)\n", mstr_strcmp(strb, stra), strcmp(strb, stra));
    printf("strcmp(stra, strb, 3) %i (%i)\n", mstr_strncmp(stra, strb, 3), strncmp(stra, strb, 3));
    printf("strcmp(strb, stra, 3) %i (%i)\n", mstr_strncmp(strb, stra, 3), strncmp(strb, stra, 3));

    test_mstr_strncpy();

    test_mstr_printf();

    return 0;
}
