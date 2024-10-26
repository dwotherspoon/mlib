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

enum mfat_buf_flag {
    MFAT_BUF_STATE_DIRTY = 1
};

/* Filesystem structure */
struct mfat_fs {
    /* Filesystem type */
    enum mfat_fs_type fs_type;
    /* Device information and functions */
    struct mfat_device *device;
	/* Volume number */
	uint8_t vol;

    /* Base of the volume */
    uint64_t base_lba;
    /* LBA of first FAT */
    uint64_t fat_start_lba;
    /* LBA of data */
    uint64_t data_start_lba;

    /* Size of FAT in sectors*/
    uint64_t fat_size;
    /* Is there a second FAT (after first) */
    uint8_t num_fats;

    /* Current buffer state */
    uint8_t buf_state;
    /* Current LBA in buffer */
    uint64_t buf_lba;
    /* Current buffer/window */
	uint8_t	buf[MFAT_SECTOR_SZ];
};

struct mfat_globals {
	struct mfat_fs *vols[MFAT_MAX_VOLUMES];
};

/* Device API */

enum mfat_device_status {
    MFAT_DEVICE_STATUS_OK = 0,
    MFAT_DEVICE_STATUS_ERROR
};

enum mfat_device_result {
    MFAT_DEVICE_RESULT_OK = 0,
    MFAT_DEVICE_RESULT_ERROR
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
    /* Drive number is not available */
    MFAT_RESULT_NOT_AVAILABLE,
    /* Device is invalid */
    MFAT_RESULT_DEVICE_INVALID,
    /* Device read or write failed */
    MFAT_RESULT_DEVICE_ERROR,
	MFAT_RESULT_TODO
};

extern char *mfat_result_lut[];
/* User API */

/* Set file pointer to offset bytes */
#define MFAT_SEEK_SET (0)
/* Add offset bytes to current file pointer position */
#define MFAT_SEEK_CUR (1)
/* Set file pointer to the size of the file add offset bytes */
#define MFAT_SEEK_END (2)

/* Mount flags? */
enum mfat_mount_flag {
    MFAT_MOUNT_FLAG_PLACEHOLDER = 0
};

/* Boot sector offsets common to all FAT types */
enum mfat_common_bs_offs {
    /* Jump instruction to boot code, two allowed forms:
     * 1. 0xeb, 0x??, 0x90 
     * 2. 0xe9, 0x??, 0x?? 
     * 
     * Forms an unconditional x86 jump to the start of the OS
     * bootstrap. */
    MFAT_COMMON_BS_OFFS_JMPBOOT = 0,
    /* Should typically be "MSWIN4.1", not required but
     * recommended for compatibility. */
    MFAT_COMMON_BS_OFFS_OEMNAME = 3, // only common to fat12, fat16, fat32
    MFAT_COMMON_BS_OFFS_BOOTSIG = 510
};

/* We don't differentiate between BPB and BS because it seems largely pointless */

enum mfat_common_bpb_offs {
    /* Count of bytes per sector, can only be: 512, 1024, 2048 or 4096
     * 512 bytes is recommended for maximum compatibility. */
    MFAT_COMMON_BPB_OFFS_BYTSPERSEC = 11,
    /* Number of sectors per allocation unit. This value
     * must be a power of 2 and greater than 0.
     * Legal values are: 1, 2, 4, 8, 16, 32, 64, 128.
     * Values that define a cluster size greater than 32K
     * will not work propertly ClusterSz = BPB_BytsPerSec * BPB_SecPerClus. */
    MFAT_COMMON_BPB_OFFS_SECPERCLUS = 13,
    /* Number of reserved sectors in the reserved region of the volume,
     * starting at the first sector of the volume. This field must not be 0.
     * For FAT12 and FAT16 should always be 1, For FAT32, typically 32. */
    MFAT_COMMON_BPB_OFFS_RSVDSECCNT = 14,
    /* Count of FAT data structures on the volume,. This field should always
     * be 2 for any FAT volume. Any value greater than or equal to 1 is valid.
     * It is highly recommended to keep this field at 2 for compatibility. */
    MFAT_COMMON_BPB_OFFS_NUMFATS = 16,
    /* For FAT12 and FAT16 volumes, this field contains the count of 32-byte
     * directory entries in the root directory. For FAT32 volumes this field
     * must be 0. For FAT12 and FAT16 volumes the value here should specify
     * a count that when multiplied by 32 is an even multiple of BPB_BytsPerSec.
     * For maximimum compatibility, FAT16 volumes should use a value of 512. */
    MFAT_COMMON_BPB_OFFS_ROOTENTCOUNT = 17,
    /* This field is the old 16-bit total count of sectors on the volume.
     * This count includes the count of all sectors in the four regions of
     * the volume. This field can be 0, when 0 then BPB_TotSec32 must be
     * non-zero. For FAT12 and FAT16, this field is used and BPB_TotSec32 is 0
     * if the total sector count fits in 16 bit (< 0x10000). */
    MFAT_COMMON_BPB_OFFS_TOTSEC16 = 19,
    /* 0xf8 is the standard value for fixed (non-removable) media. For
     * removable media 0xf0 is used.
     * Legal values are: 0xf0, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff.
     * Whatever value is here must be put in the low byte of the fat[0] entry. */
    MFAT_COMMON_BPB_OFFS_MEDIA = 21,
    /* This field is the FAT12 and FAT16 16-bit count of sectors occupied by one
     * FAT. On FAT32 volumes this field must be 0 and BPB_FATSz32 contains the FAT
     * size count. */
    MFAT_COMMON_BPB_OFFS_FATSZ16 = 22,
    /* Sectors per track for IRQ 0x13, only relevant for media with
     * geometry e.g. floppy disks. */
    MFAT_COMMON_BPB_OFFS_SECPERTRK = 24,
    /* Number of heads for IRQ 0x13, as with BPB_SecPerTrk */
    MFAT_COMMON_BPB_OFFS_NUMHEADS = 26,
    /* Count of hidden sectors before the partition that contains this
     * FAT volume. This field is generally only relevant for IRQ 0x13,
     * This field should always be 0 on non-partitioned media. */
    MFAT_COMMON_BPB_OFFS_HIDDSEC = 28,
    /* This field is the new 32-bit count of total sectors on the volume. 
     * This field can be 0, only if BPB_TotSec16 is non-zero. For FAT32
     * this field must be non-zero, but for FAT12 and FAT16 may be zero if
     * total sector count fits in 16 bit (< 0x10000). */
    MFAT_COMMON_BPB_OFFS_TOTSEC32 = 32
};

/* FAT12/16 */

enum mfat_fat12_fat16_bs_offs {
    /* The drive number of IRQ 0x13 */
    MFAT_FAT12_FAT16_BS_OFFS_DRVNUM = 36,
    /* Reserved (but used by WindowsNT),. Code that formats FAT volumes should
     * set this to 0. */
    MFAT_FAT12_FAT16_BS_OFFS_RESERVED1 = 37,
    /* Extended boot signature (0x29). This indicates that the following
     * three fields are present. */
    MFAT_FAT12_FAT16_BS_OFFS_BOOTSIG  = 38,
    /* Volume serial number, this together with BS_VolLab supports tracking
     * of removable media. These values allow FAT file system drivers to detect
     * if the wrong disk is inserted. The ID is usually generated by combining
     * the current date and time into a 32-bit value. */
    MFAT_FAT12_FAT16_BS_OFFS_VOLID = 39,
    /* Volume label, this field matches the 11-byte volume label recorded in
     * the root directory.
     * Note: FAT file system drivers should make sure that they update this field
     * when the volume label file in the root directory is changed. The default
     * value for this field should be "NO NAME    ". */
    MFAT_FAT12_FAT16_BS_OFFS_VOLLAB = 43,
    /* One of the strings "FAT12   ", "FAT16   ", "FAT     ".
     * This field should not be used to determine the type of FAT file system.
     * However, it should be set correctly for compatibility. */
    MFAT_FAT12_FAT16_BS_OFFS_FILSYSTYPE = 54
};

/* FAT32 */

enum mfat_fat32_bs_offs {
    MFAT_FAT32_BS_OFFS_DRVNUM = 64,
    MFAT_FAT32_BS_OFFS_RESERVED1 = 65,
    MFAT_FAT32_BS_OFFS_BOOTSIG = 66,
    MFAT_FAT32_BS_OFFS_VOLID = 67,
    MFAT_FAT32_BS_OFFS_VOLLAB = 71,
    /* This must always be set to the string "FAT32   ". Should not
     * be relied upon for FAT type determination. */
    MFAT_FAT32_BS_OFFS_FILSYSTYPE = 82
};

enum mfat_fat32_bpb_offs {
    MFAT_FAT32_BPB_OFFS_FATSZ32 = 36,
    MFAT_FAT32_BPB_OFFS_EXTFLAGS = 40,
    MFAT_FAT32_BPB_OFFS_FSVER = 42,
    MFAT_FAT32_BPB_OFFS_ROOTCLUS = 44,
    MFAT_FAT32_BPB_OFFS_FSINFO = 48,
    MFAT_FAT32_BPB_OFFS_BKBOOTSEC = 50,
    MFAT_FAT32_BPB_OFFS_RESERVED = 52
};

/* exFAT
 * https://learn.microsoft.com/en-us/windows/win32/fileio/exfat-specification
*/

enum mfat_exfat_bs_offs {
    MFAT_EXFAT_BS_OFFS_FILESYSTEMNAME = 3,
    MFAT_EXFAT_BS_OFFS_MUSTBEZERO = 11,
    MFAT_EXFAT_BS_OFFS_PARTITIONOFFSET = 64,
    MFAT_EXFAT_BS_OFFS_VOLUMELENGTH = 72,
    MFAT_EXFAT_BS_OFFS_FATOFFSET = 80,
    MFAT_EXFAT_BS_OFFS_FATLENGTH = 84,
    MFAT_EXFAT_BS_OFFS_CLUSTERHEAPOFFSET = 88,
    MFAT_EXFAT_BS_OFFS_CLUSTERCOUNT = 92,
    MFAT_EXFAT_BS_OFFS_FIRSTCLUSTEROFROOTDIRECTORY = 96,
    MFAT_EXFAT_BS_OFFS_VOLUMESERIALNUMBER = 100,
    MFAT_EXFAT_BS_OFFS_FILESYSTEMREVISION = 104,
    MFAT_EXFAT_BS_OFFS_VOLUMEFLAGS = 106,
    MFAT_EXFAT_BS_OFFS_BYTESPERSECTORSHIFT = 108,
    MFAT_EXFAT_BS_OFFS_SECTORSPERCLUSTERSHIFT = 109,
    MFAT_EXFAT_BS_OFFS_NUMBEROFFATS = 110,
    MFAT_EXFAT_BS_OFFS_DRIVESELECT = 111,
    MFAT_EXFAT_BS_OFFS_PERCENTINUSE = 112,
    MFAT_EXFAT_BS_OFFS_RESERVED = 113,
    MFAT_EXFAT_BS_OFFS_BOOTCODE = 120,
    MFAT_EXFAT_BS_OFFS_BOOTSIGNATURE = 510
};

#define MFAT_INVALID_LBA    (-1ULL)

/* Mount a device */
enum mfat_result mfat_mount(struct mfat_fs *fs, struct mfat_device *device, uint8_t vol, uint8_t flags);
/* Unmount a device */
enum mfat_result mfat_unmount(uint8_t vol);

#endif
