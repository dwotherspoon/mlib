#include <stdio.h>
#include <string.h>

#include <mlib/mfat.h>

struct fp_info {
    char *path;
    FILE *fp;
};

enum mfat_device_status fp_status(void *user) {
    struct fp_info *info = (struct fp_info *)user;
    printf("STATUS CALLED, user = %p\n", user);
    return MFAT_DEVICE_STATUS_OK;
}

enum mfat_device_status fp_init(void *user) {
    struct fp_info *info = (struct fp_info *)user;
    printf("INIT CALLED, user = %p\n", user);
    info->fp = fopen(info->path, "r");
    if (info->fp == NULL) {
        return MFAT_DEVICE_STATUS_ERROR;
    }
    return MFAT_DEVICE_STATUS_OK;
}

enum mfat_device_result fp_read(void *user, uint8_t *buf, uint64_t lba, uint32_t count) {
    struct fp_info *info = (struct fp_info *)user;
    size_t result;
    printf("READ CALLED, user = %p, lba = %lu\n", user, lba);
    if (info->fp == NULL) {
        return MFAT_DEVICE_RESULT_ERROR;
    }
    if (fseek(info->fp, lba * MFAT_SECTOR_SZ, SEEK_SET) != 0) {
        return MFAT_DEVICE_RESULT_ERROR;
    }
    result = fread(buf, 1, MFAT_SECTOR_SZ, info->fp);
    if (result != MFAT_SECTOR_SZ) {
        return MFAT_DEVICE_RESULT_ERROR;
    }
    return MFAT_DEVICE_RESULT_OK;
}

enum mfat_device_result fp_write(void *user, uint8_t *buf, uint64_t lba, uint32_t count) {
    return MFAT_DEVICE_STATUS_OK;
}

enum mfat_device_result fp_ioctl(void *user, uint8_t cmd, void *buf) {
    return MFAT_DEVICE_STATUS_OK;
}

struct fp_info info;

struct mfat_device dev = {
    .status = fp_status,
    .init = fp_init,
    .read = fp_read,
    .write = fp_write,
    .ioctl = fp_ioctl,
    .user = &info
};



int main(int argc, char *argv[]) {
    struct mfat_fs fs;
    enum mfat_result res;
    if (argc != 2) {
        puts("Usage: test_mfat <test.fs>");
        return -1;
    }
    printf("Using file %s\n", argv[1]);
    info.path = argv[1];
    info.fp = NULL;
    res = mfat_mount(&fs, &dev, 0, 0);
    printf("mfat_mount returned %i = %s\n", res, mfat_result_lut[res]);

    if (info.fp) {
        fclose(info.fp);
    }

    return 0;
}