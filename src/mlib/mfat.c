#include <mlib/mfat.h>
#include <mlib/mstr.h>
#include <mlib/mcommon.h>


char *mfat_result_lut[] = {
    [MFAT_RESULT_OK]                = "MFAT_RESULT_OK",
	[MFAT_RESULT_INVALID_VOLUME]    = "MFAT_RESULT_INVALID_VOLUME",
    [MFAT_RESULT_NOT_AVAILABLE]     = "MFAT_RESULT_NOT_AVAILABLE",
    [MFAT_RESULT_DEVICE_INVALID]    = "MFAT_RESULT_DEVICE_INVALID",
    [MFAT_RESULT_DEVICE_ERROR]      = "MFAT_RESULT_DEVICE_ERROR",
    [MFAT_RESULT_NO_FILE]           = "MFAT_RESULT_NO_FILE",
	[MFAT_RESULT_TODO]              = "MFAT_RESULT_TODO"
};

static size_t mfat_utf16_to_utf8(uint16_t chr, uint8_t *buf) {
    /* This isn't strictly true, and doesn't cover:
     * U+010000 to U+10FFFF:
     * U' = yyyyyyyyyyxxxxxxxxxx  // U - 0x10000
     * W1 = 110110yyyyyyyyyy      // 0xD800 + yyyyyyyyyy
     * W2 = 110111xxxxxxxxxx      // 0xDC00 + xxxxxxxxxx */
    uint32_t uval = chr;

    //*buf = chr & 0xff;
    //return 1;

    /* Is this required? It seems like Linux makes LFNs with UTF-8 characters in each UTF-16 slot */
    if (uval < 0x80) {
        /* Single byte */
        *buf = uval & 0x7f;
        return 1;
    } else if (uval < 0x7ff) {
        /* Two bytes */
        /* 0b110xxxyy */
        *buf++ = 0xc0 | ((uval >> 6) & 0x1f);
        /* 0b10yyzzzz */
        *buf = 0x80 | (uval & 0x3f);
        return 2;
    } else if (uval < 0xffff) {
        /* Three bytes */
        /* 0b1110wwww */
        *buf++ = 0xe | ((uval >> 12) & 0xf);
        /* 0b10xxxxyy */
        *buf++ = 0x80 | ((uval >> 6) & 0x3f);
        /* 0b10yyzzzz */
        *buf = 0x80 | (uval & 0x3f);
        return 3;
    } else {
        /* Four bytes */
        /* 11110uvv */
        /* 10vvwwww */
        /* 10xxxxyy */
        /* 10yyzzzz */
        return 4;
    }
}

static struct mfat_globals globals = {0};

static enum mfat_result mfat_load_buffer(struct mfat_fs *fs,
                                            uint64_t sector) {
    enum mfat_device_result dev_result;
    printf("buf sector = %x, request sector = %lx\n", fs->buf_sector, sector);
    if (sector != fs->buf_sector) {
        if (fs->buf_state & MFAT_BUF_STATE_DIRTY) {
            if (fs->device->write(fs->device->user, fs->buf, fs->buf_sector, 1)
                                        != MFAT_DEVICE_RESULT_OK) {
                return MFAT_RESULT_DEVICE_ERROR;
            } else if ((fs->num_fats == 2)
                        && (fs->buf_sector >= fs->fat_start_sector)
                        && (fs->buf_sector < (fs->fat_start_sector + fs->fat_sz))) {
                // Sync to second fat
                return MFAT_RESULT_TODO;
            }
        }
        if(fs->device->read(fs->device->user, fs->buf, sector, 1)
                                        != MFAT_DEVICE_RESULT_OK) {
            fs->buf_sector = MFAT_INVALID_SECTOR;
            return MFAT_RESULT_DEVICE_ERROR;
        }
        fs->buf_sector = sector;
    }
    return MFAT_RESULT_OK;
}

/* Check what a given sector contains.
 * Sector result can be partition, or partition table
 * 0 -> EXFAT
 * 1 -> FAT32
 * 2 -> FAT12/FAT16
 * 3 -> Valid boot sig only
 * 4 -> Invalid
*/
static enum mfat_result mfat_check_filesystem(struct mfat_fs *fs, uint64_t sector,
                                            enum mfat_fs_type *type) {
    enum mfat_result res;
    uint8_t secs_per_clus, num_fats;
    uint16_t bootsig, bytes_per_sec, reserved_secs, root_ent_cnt, fat_sz16;
    uint32_t jmpboot, total_secs, root_dir_secs, data_secs;
    uint32_t clus_cnt, fat_sz, system_secs, temp;

    *type = MFAT_FS_TYPE_INVALID;
    fs->buf_state = 0;
    fs->buf_sector = MFAT_INVALID_SECTOR;
    res = mfat_load_buffer(fs, sector);
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
        /* N.B. root entry count is 0 for FAT32 */
        if ((bytes_per_sec == MFAT_SECTOR_SZ)
            && MLIB_ISPOW2(bytes_per_sec)
            && (reserved_secs > 0)
            && (num_fats == 1 || num_fats == 2)
            && (total_secs >= 128)) {

            root_dir_secs = MLIB_ALIGN_UP(root_ent_cnt * 32, bytes_per_sec) / bytes_per_sec;
            system_secs = reserved_secs + (num_fats * fat_sz) + root_dir_secs;
            data_secs = total_secs - system_secs;
            clus_cnt = data_secs / secs_per_clus;

            fs->base_sector = sector;
            fs->fat_start_sector = sector + reserved_secs;
            fs->data_start_sector = sector + system_secs;
            fs->num_fats = num_fats;
            fs->fat_sz = fat_sz;
            fs->cluster_sz = secs_per_clus;
            fs->num_fat_entries = clus_cnt + 1;

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
                fs->fs_type = MFAT_FS_TYPE_FAT32;
            }

            if (fs->fs_type == MFAT_FS_TYPE_FAT32) {
                if (root_ent_cnt) {
                    return MFAT_FS_TYPE_INVALID;
                }
                fs->root_dir_cluster = fs->buf[MFAT_FAT32_BPB_OFFS_ROOTCLUS]
                                    | (fs->buf[MFAT_FAT32_BPB_OFFS_ROOTCLUS + 1] << 8)
                                    | (fs->buf[MFAT_FAT32_BPB_OFFS_ROOTCLUS + 2] << 16)
                                    | (fs->buf[MFAT_FAT32_BPB_OFFS_ROOTCLUS + 3] << 24);
                temp = num_fats << 2;
                /* TODO: Read FSINFO */
            } else {
                if (!root_ent_cnt) {
                    return MFAT_FS_TYPE_INVALID;
                }
                fs->num_root_dir_entries = root_ent_cnt;
                fs->root_dir_cluster = 0;
                fs->root_dir_sector = fs->fat_start_sector + (fat_sz * num_fats);
                if (fs->fs_type == MFAT_FS_TYPE_FAT12) {
                    temp = (num_fats * 3) / (2 + (num_fats & 1));
                } else {
                    temp = num_fats << 1;
                }
            }

            /* Check size allocated for FATs is big enough */
            temp = MLIB_ALIGN_UP(temp, MFAT_SECTOR_SZ) / MFAT_SECTOR_SZ;

            if (fat_sz < temp) {
                return MFAT_RESULT_INVALID_VOLUME;
            }

            printf("Sectors per cluster = %u\n", secs_per_clus);
            return MFAT_DEVICE_RESULT_OK;
        }
    }
}

static void mfat_find_volume(struct mfat_fs *fs,
                    uint8_t partition) {
    enum mfat_fs_type fs_type;
    mfat_check_filesystem(fs, 63, &fs_type);
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

/* Internal Functions */

static uint32_t mfat_get_fat(struct mfat_fs *fs, uint32_t cluster) {
    uint32_t result;
    uint64_t offs;
    printf("Get next cluster for %u\n", cluster);
    if (cluster < 2 || cluster >= fs->num_fat_entries) {
        /* Error */
        return 1;
    }
    switch (fs->fs_type) {
        case MFAT_FS_TYPE_FAT12:
            /* Multiply by 1.5 */
            offs = cluster + (cluster >> 1);
            if (mfat_load_buffer(fs, fs->fat_start_sector + (offs / MFAT_SECTOR_SZ))
                                                    != MFAT_RESULT_OK) {
                return 1;
            }
            result = fs->buf[offs % MFAT_SECTOR_SZ];
            /* Due to lack of alignment, this may straddle two sectors */
            offs++;
            if (mfat_load_buffer(fs, fs->fat_start_sector + (offs / MFAT_SECTOR_SZ))
                                                    != MFAT_RESULT_OK) {
                return 1;
            }
            result |= fs->buf[offs % MFAT_SECTOR_SZ] << 8;
            return (cluster & 1) ? (result >> 4) : (result & 0xfff);
            break;
        case MFAT_FS_TYPE_FAT16:
            offs = cluster << 1;
            if (mfat_load_buffer(fs, fs->fat_start_sector + (offs / MFAT_SECTOR_SZ))
                                                    != MFAT_RESULT_OK) {
                return 1;
            }
            return fs->buf[offs % MFAT_SECTOR_SZ]
                    | (fs->buf[(offs % MFAT_SECTOR_SZ) + 1] << 8);
            break;
        case MFAT_FS_TYPE_FAT32:
            offs = cluster << 2;
            if (mfat_load_buffer(fs, fs->fat_start_sector + (offs / MFAT_SECTOR_SZ))
                                                    != MFAT_RESULT_OK) {
                return 1;
            }
            return (fs->buf[offs % MFAT_SECTOR_SZ]
                    | (fs->buf[(offs % MFAT_SECTOR_SZ) + 1] << 8)
                    | (fs->buf[(offs % MFAT_SECTOR_SZ) + 2] << 16)
                    | (fs->buf[(offs % MFAT_SECTOR_SZ) + 3] << 24)) & 0xfffffff;
            break;
        case MFAT_FS_TYPE_EXFAT:
            /* TODO */
            return 1;
            break;
    }
    return 1;
}

static enum mfat_result mfat_put_fat(struct mfat_fs *fs, uint32_t cluster, uint32_t value) {
    enum mfat_result result;
    uint64_t offs;
    if (cluster < 2 || cluster >= fs->num_fat_entries) {
        /* Error */
        return MFAT_RESULT_TODO;
    }
    switch (fs->fs_type) {
        case MFAT_FS_TYPE_FAT12:
            /* Multiply by 1.5 */
            offs = cluster + (cluster >> 1);
            result = mfat_load_buffer(fs, fs->fat_start_sector + (offs / MFAT_SECTOR_SZ));
            if (result != MFAT_RESULT_OK) {
                return result;
            }
            break;
        case MFAT_FS_TYPE_FAT16:
            offs = cluster << 1;
            result = mfat_load_buffer(fs, fs->fat_start_sector + (offs / MFAT_SECTOR_SZ));
            if (result != MFAT_RESULT_OK) {
                return result;
            }
            break;
        case MFAT_FS_TYPE_FAT32:
            offs = cluster << 2;
            result = mfat_load_buffer(fs, fs->fat_start_sector + (offs / MFAT_SECTOR_SZ));
            if (result != MFAT_RESULT_OK) {
                return result;
            }
            break;
        case MFAT_FS_TYPE_EXFAT:
            break;
    }

    return MFAT_RESULT_TODO;
}

static uint8_t mfat_ldir_checksum_sfn(uint8_t *sfn) {
    uint8_t csum, i;
    for (i = 0; i < 11; i++) {
        csum = (csum << 7) + (csum >> 1) + sfn[i];
    }
    return csum;
}

/* Load a file starting at a given cluster. */
static void mfat_file_load(struct mfat_file *file, uint32_t cluster) {
    file->obj.start_cluster = cluster;
    file->obj.cluster = cluster;
    file->obj.offs = 0;
}

/* Load a directory starting at a given cluster. 0 = ROOT */
static enum mfat_result mfat_dir_load(struct mfat_dir *dir, uint32_t cluster) {
    struct mfat_fs *fs = globals.vols[dir->obj.vol_id];
    dir->obj.start_cluster = cluster;
    dir->obj.cluster = cluster;

    /* Cluster 0 -> Map to root directory */
    if (!cluster) {
        if (fs->root_dir_cluster) {
            dir->obj.cluster = fs->root_dir_cluster;
        } else {
            dir->obj.sector = fs->root_dir_sector;
        }
    }

    if (fs->root_dir_cluster) {
        dir->obj.sector = mfat_cluster_to_sector(fs, fs->root_dir_cluster);
    }
    
    dir->obj.offs = 0;

    return MFAT_RESULT_OK;
}

static enum mfat_result mfat_object_seek(struct mfat_object *obj, size_t pos) {
    /* Is this in the current sector */
    /* Current cluster */

    /* Go back to start? */
}

/* Change to generic increment file object? */
static enum mfat_result mfat_next_dir_entry(struct mfat_dir *dir) {
    struct mfat_fs *fs = globals.vols[dir->obj.vol_id];
    uint32_t cluster;
    
    dir->obj.offs += MFAT_DIR_ENTRY_SZ;
    printf("OFFS = %u\n", dir->obj.offs);
    /* Fixed size directory? Cluster only 0 for FAT12/FAT16 */
    if (dir->obj.cluster == 0
        && ((dir->obj.offs / MFAT_DIR_ENTRY_SZ) >= fs->num_root_dir_entries)) {
            /* Reached max of fixed size root directory */
            return MFAT_RESULT_TODO;
    } else {
        /* Changed sector */
        if ((dir->obj.offs % MFAT_SECTOR_SZ) == 0) {
            printf("Change sector\n");
            /* Also changed cluster? */
            if ((dir->obj.offs % (MFAT_SECTOR_SZ * fs->cluster_sz)) == 0) {
                printf("Change cluster\n");
                cluster = mfat_get_fat(fs, dir->obj.cluster);
                printf("Next cluster = %08x\n", cluster);
                if (cluster <= 1) {
                    return MFAT_RESULT_TODO;
                }
				if (cluster == 0xffffffff) {
                    return MFAT_RESULT_DEVICE_ERROR;
                }
                if (cluster >= fs->num_fat_entries) {
                    // OR stretch
                    return MFAT_RESULT_NO_FILE;
                }
                dir->obj.cluster = cluster;
                // Free 0 or just < 2
                // Invalid cluster? (0xff...7)
                // End of file (0xfff...f)
                // Reserved? (0xff...8 to 0xff.e
                dir->obj.sector = mfat_cluster_to_sector(fs, dir->obj.cluster);
                dir->obj.offs = 0;
            }
        } 
    }
    return MFAT_RESULT_OK;
}

static uint8_t lfn_name_lut[] = {1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30};
static uint16_t lfn_buf[256] = {0}; // MOVE this to mfat_fs?

/* Could we make a generic get bytes from fs obj? */
static enum mfat_result mfat_get_dir_entry(struct mfat_dir *dir, 
                                struct mfat_dir_entry *dir_entry) {
    enum mfat_result result;
    uint8_t attr, lfn_prev_ord, lfn_prev_csum, lfn_max_ord, temp, sfn_pos;
    struct mfat_fs *fs = globals.vols[dir->obj.vol_id];
    uint16_t lfn_cur_name[13];
    uint16_t sector_offs, lfn_pos, lfn_chr;
    uint8_t *lfn_ptr, *sfn_ptr;

    lfn_prev_ord = 0;
    do {
        mfat_load_buffer(fs, dir->obj.sector);
        sector_offs = dir->obj.offs % MFAT_SECTOR_SZ;
        printf("Sector offs = %u\n" , sector_offs);
        /* DIR_NAME[0] or LDIR_ORD */
        temp = fs->buf[sector_offs];
        attr = fs->buf[sector_offs + MFAT_DIR_ENTRY_OFFS_ATTR];

        if (temp == 0xe5) {
            printf("FREE entry\n");
            /* Deleted/Free -> SKIP */
        } else if (temp == 0x0) {
            /* End of list  */
            printf("End of list\n");
            return MFAT_RESULT_NO_FILE;
        } else if ((attr & MFAT_LDIR_ENTRY_ATTR_LONG_NAME_MASK) == MFAT_LDIR_ENTRY_ATTR_LONG_NAME) {
            printf("LFN entry\n");
            if (lfn_prev_ord == 0) {
                if (!(temp & MFAT_LDIR_ENTRY_ORD_LAST_LONG_ENTRY)) {
                    return MFAT_RESULT_TODO;
                }
                lfn_prev_ord = temp & ~MFAT_LDIR_ENTRY_ORD_LAST_LONG_ENTRY;
                lfn_max_ord = lfn_prev_ord;
                lfn_prev_csum = fs->buf[sector_offs + MFAT_LDIR_ENTRY_OFFS_CHKSUM];
            } else {
                if (temp != --lfn_prev_ord) {
                    printf("BAD ORD SEQ ord = %x, expected = %x\n", temp, lfn_prev_ord);
                    return MFAT_RESULT_TODO;
                }
                if (lfn_prev_csum != fs->buf[sector_offs + MFAT_LDIR_ENTRY_OFFS_CHKSUM]) {
                    printf("BAD CSUM SEQ\n");
                    return MFAT_RESULT_TODO;
                }
            }
            /* Copy to LFN buf */
            for (lfn_pos = 0; lfn_pos < 13; lfn_pos++) {
                lfn_buf[(((temp & 0x3f) - 1) * 13) + lfn_pos] = fs->buf[sector_offs + lfn_name_lut[lfn_pos]]
                                                                | (fs->buf[sector_offs + lfn_name_lut[lfn_pos] + 1] << 8);      
            }
        } else if (attr & MFAT_DIR_ENTRY_ATTR_VOLUME_ID) {
            /* Volume label */
            printf("Volume label\n");
        } else {
            if (lfn_prev_ord) {
                if (mfat_ldir_checksum_sfn(&fs->buf[sector_offs + MFAT_DIR_ENTRY_OFFS_NAME]) != lfn_prev_csum) {
                    /* Checksum mismatch */
                    printf("LFN checksum error\n");
                    return MFAT_RESULT_TODO;
                }
                lfn_ptr = dir_entry->name;
                for (lfn_pos = 0; lfn_pos < MFAT_LFN_LEN; lfn_pos++) {
                    lfn_chr = lfn_buf[lfn_pos];
                    /* Surrogate support? */
                    lfn_ptr += mfat_utf16_to_utf8(lfn_chr, lfn_ptr);
                    if (lfn_chr == 0xffff || lfn_chr == 0x0) {
                        break;
                    }
                }
                *lfn_ptr = 0x0;
                sfn_ptr = dir_entry->short_name;
                printf("SFN = %11s, Long name = %s\n", fs->buf + sector_offs + MFAT_DIR_ENTRY_OFFS_NAME, dir_entry->name);
            } else {
                /* NO LFN set lfn = sfn */
            }
            /* TODO: Copy SFN in */
            //for (sfn_pos = 0; sfn_pos < 8; sfn_ptr++) {
            //                    
            //}
            dir_entry->cluster = fs->buf[sector_offs + MFAT_LDIR_ENTRY_OFFS_FSTCLUSLO]
                                    | (fs->buf[sector_offs + MFAT_DIR_ENTRY_OFFS_FSTCLUSLO + 1] << 8)
                                    | (fs->buf[sector_offs + MFAT_DIR_ENTRY_OFFS_FSTCLUSHI] << 16)
                                    | (fs->buf[sector_offs + MFAT_DIR_ENTRY_OFFS_FSTCLUSHI + 1] << 24);
            /* Non-LFN entry and last_lfn != 1? */
            printf("NAME = %11s\n", &fs->buf[sector_offs + MFAT_DIR_ENTRY_OFFS_NAME]);
            printf("ATTR = %02x\n", attr);
            printf("NTRES = %02x\n", fs->buf[sector_offs + MFAT_DIR_ENTRY_OFFS_NTRES]);
            printf("First cluster = %u\n", dir_entry->cluster);
            
            return MFAT_RESULT_OK;
        }
        /* Increment to next dir entry */
        result = mfat_next_dir_entry(dir);
        if (result != MFAT_RESULT_OK) {
            return result;
        }
    } while (temp);

    return MFAT_RESULT_OK;
}

enum mfat_result mfat_get_vol_id(const char *path, uint8_t *result) {
    uint8_t vol_chr = path[0];
    uint8_t vol_sep = path[1];
    if (vol_chr < MFAT_VOLUME_BASE_CHAR || vol_chr >= (MFAT_VOLUME_BASE_CHAR + MFAT_MAX_VOLUMES)) {
        return MFAT_RESULT_TODO;
    }
    if (vol_sep != ':') {
        return MFAT_RESULT_TODO;
    }
    *result = vol_chr - MFAT_VOLUME_BASE_CHAR;
    return MFAT_RESULT_OK;
}

enum mfat_result mfat_follow_path(const char *path) {
    struct mfat_dir dir;
}

/* ----------------------------------- USER INTERACE --------------------------------- */

/* User functions - Directories */
enum mfat_result mfat_opendir(struct mfat_dir *dir, const char *path) {
    /* Load root directory */
    printf("GET VOL\n");
    if (mfat_get_vol_id(path, &dir->obj.vol_id) != MFAT_RESULT_OK) {
        printf("BAD DRIVE LETTER\n");
        return MFAT_RESULT_TODO;
    }
    //mfat_follow_path(path);

    return mfat_dir_load(dir, 0);
}

int mfat_closedir() {
    return 0;
}

enum mfat_result mfat_readdir(struct mfat_dir *dir, struct mfat_dir_entry *dir_entry) {
    /* Validate dir object is real? */
    enum mfat_result res;
    if (!dir_entry) {
        /* Rewind directory object */
        res = mfat_dir_load(dir, 0);
    } else {
        res = mfat_get_dir_entry(dir, dir_entry);
        if (res != MFAT_RESULT_OK) {
            return res;
        }
        res = mfat_next_dir_entry(dir);
    }
    return res;
}

/* User functions - Files */

int mfat_open() {
    return 0;
}

int mfat_close() {
    return 0;
}

int mfat_read() {
    return 0;
}

int mfat_write() {
    return 0;
}

int mfat_seek() {
    return 0;
}

/* User functions - Other */

int mfat_stat() {
    return 0;
}

/* User functions - Filesystem */

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
    return 0;
}
