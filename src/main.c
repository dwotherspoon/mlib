#include <stdio.h>

#include <mlib/mfat.h>
#include <mlib/mprintf.h>
#include <mlib/mstr.h>

#define mprintf_printf(...) mprintf_funprintf(putchar, __VA_ARGS__)

struct fp_info {
    char *path;
    FILE *fp;
};

enum mfat_device_status fp_status(void *user) {
    return MFAT_DEVICE_STATUS_OK;
}

enum mfat_device_status fp_init(void *user) {
    return MFAT_DEVICE_STATUS_OK;
}

enum mfat_device_result fp_read(void *user, uint8_t *buf, uint64_t lba, uint32_t count) {
    return MFAT_DEVICE_RESULT_OK;
}

enum mfat_device_result fp_write(void *user, uint8_t *buf, uint64_t lba, uint32_t count) {
    return MFAT_DEVICE_RESULT_OK;
}

enum mfat_device_result fp_ioctl(void *user, uint8_t *buf, uint64_t lba, uint32_t count) {
    return MFAT_DEVICE_RESULT_OK;
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

    return 0;
}
