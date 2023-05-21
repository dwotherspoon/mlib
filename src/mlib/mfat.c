#include <mlib/mfat.h>

struct mfat_globals globals = {0};

/*********************** Mount and unmount ***********************/
enum mfat_result mfat_do_mount(uint8_t vol) {
    return MFAT_RESULT_TODO;
}

enum mfat_result mfat_mount(struct mfat_fs *fs, struct mfat_device *device, uint8_t vol, uint8_t opt) {
    /* Check volume ID */
    if (vol >= MFAT_MAX_VOLUMES) {
        return MFAT_RESULT_INVALID_VOLUME;
    }

    /* Is a volume already mounted */
    if (globals.vols[vol]) {
        /* What to do? Unmount? */
        return MFAT_RESULT_TODO;
    }

    if (fs) {
        fs->vol = vol;
        fs->fs_type = MFAT_FS_TYPE_INVALID;
        fs->device = device;
        globals.vols[vol] = fs;
    }

    if (opt & MFAT_MOUNT_FLAG_POSTPONE) {
        /* Lazy mount flag, mount later. */
        return MFAT_RESULT_OK;
    } else {
        return mfat_do_mount(vol);
    }
}



enum mfat_result mfat_unmount(uint8_t vol) {
    return MFAT_RESULT_TODO;    
}
