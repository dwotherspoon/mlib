#include <mlib/mfat.h>
#include <mlib/mstr.h>
#include <mlib/mcommon.h>


char *mfat_result_lut[] = {
    [MFAT_RESULT_OK]                = "MFAT_RESAULT_OK",
	[MFAT_RESULT_INVALID_VOLUME]    = "MFAT_RESULT_INVALID_VOLUME",
    [MFAT_RESULT_NOT_AVAILABLE]     = "MFAT_RESULT_NOT_AVAILABLE",
    [MFAT_RESULT_DEVICE_INVALID]    = "MFAT_RESULT_DEVICE_INVALID",
    [MFAT_RESULT_DEVICE_ERROR]      = "MFAT_RESULT_DEVICE_ERROR",
	[MFAT_RESULT_TODO]              = "MFAT_RESULT_TODO"
};


static struct mfat_globals globals = {0};

static enum mfat_result mfat_load_buffer(struct mfat_fs *fs,
                                            uint64_t lba) {
    enum mfat_device_result dev_result;
    if (lba != fs->buf_lba) {
        if (fs->buf_state & MFAT_BUF_STATE_DIRTY) {
            if (fs->device->write(fs->device->user, fs->buf, fs->buf_lba, 1)
                                        != MFAT_DEVICE_RESULT_OK) {
                return MFAT_RESULT_DEVICE_ERROR;
            } else if ((fs->num_fats == 2)
                        && (fs->buf_lba >= fs->fat_start_lba)
                        && (fs->buf_lba < (fs->fat_start_lba + fs->fat_size))) {
                // Sync to second fat
                return MFAT_RESULT_TODO;
            }
        }
        if(fs->device->read(fs->device->user, fs->buf, lba, 1)
                                        != MFAT_DEVICE_RESULT_OK) {
            fs->buf_lba = MFAT_INVALID_LBA;
            return MFAT_RESULT_DEVICE_ERROR;
        }
        fs->buf_lba = lba;
    }
    return MFAT_RESULT_OK;
}

static enum mfat_fs_type mfat_check_fat_type(struct mfat_fs *fs, uint64_t lba) {
    uint64_t cluster_count = 0;
    /*
    A volume with count of clusters ≦4085 is FAT12.
    A volume with count of clusters ≧4086 and ≦65525 is FAT16.
    A volume with count of clusters ≧65526 is FAT32. */
    if (cluster_count < 4086) {
        return MFAT_FS_TYPE_FAT12;
    } else if(cluster_count < 65526) {
        return MFAT_FS_TYPE_FAT16;
    }
    return 0;
}

/* Check what a given sector (lba) contains.
 * Sector result can be partition, or partition table
 * 0 -> EXFAT
 * 1 -> FAT32
 * 2 -> FAT12/FAT16
 * 3 -> Valid boot sig only
 * 4 -> Invalid
*/
static enum mfat_fs_type mfat_check_filesystem(struct mfat_fs *fs, uint64_t lba,
                                            enum mfat_fs_type *type) {
    enum mfat_result res;
    uint8_t secs_per_clus, num_fats;
    uint16_t bootsig, bytes_per_sec, reserved_secs, root_ent_cnt, fat_sz16;
    uint32_t jmpboot, total_secs, root_dir_secs, data_secs;
    uint32_t clus_cnt, fat_sz, system_secs;

    *type = MFAT_FS_TYPE_INVALID;
    fs->buf_state = 0;
    fs->buf_lba = MFAT_INVALID_LBA;
    res = mfat_load_buffer(fs, lba);
    if (res != MFAT_RESULT_OK) {
        return MFAT_RESULT_DEVICE_ERROR;
    }

    bootsig = fs->buf[MFAT_COMMON_BS_OFFS_BOOTSIG]
                | (fs->buf[MFAT_COMMON_BS_OFFS_BOOTSIG + 1] << 8);

    jmpboot = fs->buf[MFAT_COMMON_BS_OFFS_JMPBOOT + 2]
                | (fs->buf[MFAT_COMMON_BS_OFFS_JMPBOOT + 1] << 8)
                | (fs->buf[MFAT_COMMON_BS_OFFS_JMPBOOT] << 16);

    if (bootsig = 0xa55 && jmpboot == 0xeb7690
        && !mstr_strncmp(fs->buf + MFAT_EXFAT_BS_OFFS_FILESYSTEMNAME,
                                                        "EXFAT   ", 8)) {
        /* It is mandatory that EXFAT FileSystemName field is correctly set. */
        return 0;
    /* Check for jmpboot in form 0xe9???? or 0xeb??90 */
    } else if (((jmpboot & 0xfd0000) == 0xe90000)
                && (!(jmpboot & 0x20000) || ((jmpboot & 0xff) == 0x90))) {
        bytes_per_sec = fs->buf[MFAT_COMMON_BPB_OFFS_BYTSPERSEC]
                            | (fs->buf[MFAT_COMMON_BPB_OFFS_BYTSPERSEC + 1] << 8);
        secs_per_clus = fs->buf[MFAT_COMMON_BPB_OFFS_SECPERCLUS];
        reserved_secs = fs->buf[MFAT_COMMON_BPB_OFFS_RSVDSECCNT]
                            | (fs->buf[MFAT_COMMON_BPB_OFFS_RSVDSECCNT + 1] << 8);
        num_fats = fs->buf[MFAT_COMMON_BPB_OFFS_NUMFATS];
        root_ent_cnt = fs->buf[MFAT_COMMON_BPB_OFFS_ROOTENTCOUNT]
                            | (fs->buf[MFAT_COMMON_BPB_OFFS_ROOTENTCOUNT + 1] << 8);
        total_secs = fs->buf[MFAT_COMMON_BPB_OFFS_TOTSEC16]
                            | (fs->buf[MFAT_COMMON_BPB_OFFS_TOTSEC16 + 1] << 8);
        if (!total_secs) {
            total_secs = fs->buf[MFAT_COMMON_BPB_OFFS_TOTSEC32]
                            | (fs->buf[MFAT_COMMON_BPB_OFFS_TOTSEC32 + 1] << 8)
                            | (fs->buf[MFAT_COMMON_BPB_OFFS_TOTSEC32 + 2] << 16)
                            | (fs->buf[MFAT_COMMON_BPB_OFFS_TOTSEC32 + 3] << 24);
            /* Error if this is not at least 0x10000? */
        }
        fat_sz16 = fs->buf[MFAT_COMMON_BPB_OFFS_FATSZ16]
                    | (fs->buf[MFAT_COMMON_BPB_OFFS_FATSZ16 + 1] << 8);
        if (!fat_sz16) {
                fat_sz = fs->buf[MFAT_FAT32_BPB_OFFS_FATSZ32]
                            | (fs->buf[MFAT_FAT32_BPB_OFFS_FATSZ32 + 1] << 8)
                            | (fs->buf[MFAT_FAT32_BPB_OFFS_FATSZ32 + 2] << 16)
                            | (fs->buf[MFAT_FAT32_BPB_OFFS_FATSZ32 + 3] << 24);
            /* Is this enough to indicate that we're probably FAT32? */
        } else {
            fat_sz = fat_sz16;
        }
        printf("has jmpboot\n"
                "\tbytes_per_sector = %u\n"
                "\tsectors_per_cluster = %u\n"
                "\treserved_sectors = %u\n"
                "\tnum_fats = %u\n"
                "\troot_entry_count = %u\n"
                "\ttotal_sectors = %u\n"
                "\tfat_sz = %u\n",
                bytes_per_sec, secs_per_clus,
                reserved_secs, num_fats, root_ent_cnt,
                total_secs, fat_sz);
        /* N.B. root entry count is 0 for FAT32 */
        if ((bytes_per_sec == MFAT_SECTOR_SZ)
            && MLIB_ISPOW2(bytes_per_sec)
            && (reserved_secs > 0)
            && (num_fats == 1 || num_fats == 2)
            && (total_secs >= 128)) {

            root_dir_secs = MLIB_ALIGN_UP(root_ent_cnt * 32, bytes_per_sec) / bytes_per_sec;
            system_secs = reserved_secs + (num_fats * fat_sz) + root_dir_secs;
            printf("system_secs = %u, reserved_secs = %u, root_dir_secs = %u\n",
                    system_secs, reserved_secs, root_dir_secs);
            data_secs = total_secs - system_secs;
            clus_cnt = data_secs / secs_per_clus;

            fs->base_lba = lba;
            fs->fat_start_lba = lba + reserved_secs;
            fs->data_start_lba = lba + system_secs;
            fs->num_fats = num_fats;
            fs->fat_size = fat_sz;

            /* N.B. mkfs.vfat is quite capable of making a FAT32 fs (e.g. exactly 32M)
             * which will fail the Microsoft cluster count check. However, we force
             * FAT32 if fat_sz16 was 0 to get around this oddity. */

            if (clus_cnt < 4085) {
                printf("FAT12\n");
                fs->fs_type = MFAT_FS_TYPE_FAT12;
            } else if (clus_cnt < 65525 && fat_sz16) {
                printf("FAT16\n");
                fs->fs_type = MFAT_FS_TYPE_FAT16;
            } else {
                printf("FAT32\n");
                /* Return error if root ent count not 0. */
                fs->fs_type = MFAT_FS_TYPE_FAT32;
            }

            printf("fat_size = %u, root_dir_sectors = %u, data_sectors = %u, cluster_count = %u\n",
                    fat_sz, root_dir_secs, data_secs, clus_cnt);

            return 1;
        }
    }
}

static void mfat_find_volume(struct mfat_fs *fs,
                    uint8_t partition) {
    enum mfat_fs_type fs_type;
    mfat_check_filesystem(fs, 0, &fs_type);
}

static enum mfat_result mfat_do_mount(uint8_t vol, uint8_t mode) {
    struct mfat_fs *fs;
    enum mfat_device_status dev_status;

    fs = globals.vols[vol];
    if (!fs) {
        return MFAT_RESULT_NOT_AVAILABLE;
    }

    if (fs->fs_type != MFAT_FS_TYPE_INVALID) {
        /* Already mounted */
        return MFAT_RESULT_TODO;
    }

    if (!fs->device) {
        return MFAT_RESULT_DEVICE_INVALID;
    }

    /* Initialise the device */
    dev_status = fs->device->init(fs->device->user);
    if (dev_status != MFAT_DEVICE_STATUS_OK) {
        return MFAT_RESULT_TODO;
    }

    /* Find FAT volume */
    mfat_find_volume(fs, 0);


    return MFAT_RESULT_TODO;
}

enum mfat_result mfat_mount(struct mfat_fs *fs,
                            struct mfat_device *device,
                            uint8_t vol, uint8_t opt) {
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

    return mfat_do_mount(vol, 0);
}



enum mfat_result mfat_unmount(uint8_t vol) {
    return MFAT_RESULT_TODO;
}
