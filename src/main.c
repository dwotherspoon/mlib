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
    mprintf_printf("Hello, world!\n");
    printf("Hello, world!\n");

    mprintf_printf("Hello, world! %i\n", 200);
    printf("Hello, world! %i\n", 200);

    mprintf_printf("Hello, world! %hi\n", 0x1ffaff);
    printf("Hello, world! %hi\n", 0x1ffaff);

    mprintf_printf("Hello, world! %hhi\n", 2000);
    printf("Hello, world! %hhi\n", 2000);
    return 0;
}
