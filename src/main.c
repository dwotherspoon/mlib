#include <stdio.h>

#include <mlib/mfat.h>
#include <mlib/mprintf.h>

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
    printf("Sizeof char is %zu\n", sizeof(char));
    printf("Sizeof short is %zu\n", sizeof(short int));
    printf("Sizeof int is %zu\n", sizeof(int));
    printf("Sizeof long is %zu\n", sizeof(long));
    printf("Sizeof long long is %zu\n", sizeof(long long));

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


    mprintf_ftoa(NULL, 1.5, 0, 0, 0);
    mprintf_ftoa(NULL, 2.5, 0, 0, 0);

    mprintf_ftoa(NULL, 1.15, 1, 0, 0);
    mprintf_ftoa(NULL, 2.25, 1, 0, 0);

    mprintf_printf("%.0f\n", 1.5);
    mprintf_printf("%.0f\n", 2.5);
    printf("%.0f\n", 1.5);
    printf("%.0f\n", 2.5);

    return 0;
}
