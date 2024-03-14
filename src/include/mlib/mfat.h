#ifndef _MFAT_H
#define _MFAT_H

#include <stdint.h>
#include <mlib/mfat_config.h>

/* Check config is valid */

#if (MFAT_MAX_VOLUMES < 0) || ((MFAT_MAX_VOLUMES > 9))
	#error "MFAT_MAX_VOLUMES can not be less than 0 or greater than 9"
#endif

#if (MFAT_MIN_SECTORSZ & 0x1ff) || (MFAT_MAX_SECTORSZ & 0x1ff)
	#error "MFAT_MIN_SECTORSZ and MFAT_MAX_SECTORSZ must be a multiple of 512"
#endif

/* Struct definitions */

enum mfat_fs_type {
    MFAT_FS_TYPE_INVALID = 0,
    MFAT_FS_TYPE_FAT12,
    MFAT_FS_TYPE_FAT16,
    MFAT_FS_TYPE_FAT32,
    MFAT_FS_TYPE_EXFAT
};

/* Filesystem structure */
struct mfat_fs {
    /* Filesystem type */
    enum mfat_fs_type fs_type;
    /* Device information and functions */
    struct mfat_device *device;
	/* Volume number */
	uint8_t vol;
    /* Current buffer state */
    uint8_t buf_state;
    /* Current LBA in buffer */
    uint64_t buf_lba;
	uint8_t	buf[MFAT_MAX_SECTOR_SZ];
};

struct mfat_globals {
	struct mfat_fs *vols[MFAT_MAX_VOLUMES];
};

/* Device API */

enum mfat_device_status {
    MFAT_DEVICE_STATUS_OK = 0
};

enum mfat_device_result {
    MFAT_DEVICE_RESULT_OK = 0
};

struct mfat_device {
    enum mfat_device_status (*status)(void *user);
    enum mfat_device_status (*init)(void *user);
    enum mfat_device_result (*read)(void *user, uint8_t *buf, uint64_t lba, uint32_t count);
    enum mfat_device_result (*write)(void *user, uint8_t *buf, uint64_t lba, uint32_t count);
    enum mfat_device_result (*ioctl)(void *user, uint8_t cmd, void *buf);
    /* User data for identifying device etc. */
    void *user;
};

enum mfat_result {
	/* Successful result */
    MFAT_RESULT_OK = 0,
	/* Drive number provided was not mounted or out of range */
	MFAT_RESULT_INVALID_VOLUME,
	MFAT_RESULT_TODO
};

/* User API */

/* Set file pointer to offset bytes */
#define MFAT_SEEK_SET (0)
/* Add offset bytes to current file pointer position */
#define MFAT_SEEK_CUR (1)
/* Set file pointer to the size of the file add offset bytes */
#define MFAT_SEEK_END (2)

/* Mount flags */
#define MFAT_MOUNT_FLAG_POSTPONE (0x1)

/* Mount a device */
enum mfat_result mfat_mount(struct mfat_fs *fs, struct mfat_device *device, uint8_t vol, uint8_t flags);
/* Unmount a device */
enum mfat_result mfat_unmount(uint8_t vol);

#endif
