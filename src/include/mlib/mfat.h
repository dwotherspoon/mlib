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
    uint64_t base_sector;
    /* LBA of first FAT */
    uint64_t fat_start_sector;
    /* LBA of data */
    uint64_t data_start_sector;
    /* Root directory entry count, only for FAT12/FAT16 */
    uint16_t num_root_dir_entries;

    /* Root directory cluster (if FAT32) */
    uint32_t root_dir_cluster;
    /* Root directory sector (FAT12/FAT16) */
    uint64_t root_dir_sector;

    /* Size of FAT in sectors*/
    uint64_t fat_sz;
    /* Number of FATs (either 1 or 2). */
    uint8_t num_fats;
    /* Size of a cluster in sectors */
    uint16_t cluster_sz;

    /* Number of fat entries (clusters) */
    uint32_t num_fat_entries;

    /* Current buffer state */
    uint8_t buf_state;
    /* Current LBA in buffer */
    uint64_t buf_sector;
    /* Current buffer/window */
	uint8_t	buf[MFAT_SECTOR_SZ];
};

/* File system object */
struct mfat_object {
    /* Volume used for this object */
    uint8_t vol_id;
    /* Start cluster */
    uint32_t start_cluster;
    /* Current cluster */
    uint32_t cluster;
    /* Current sector */
    uint64_t sector;
    /* Offset within the cluster */
    uint32_t offs;
};

/* File object (with its own buffer) */
struct mfat_file {
    /* Current LBA in buffer */
    uint64_t buf_lba;
    uint8_t buf[MFAT_SECTOR_SZ];
    struct mfat_object obj;
};

/* Directory object */
struct mfat_dir {
    struct mfat_object obj;
};

#define MFAT_SFN_LEN    11
#define MFAT_LFN_LEN    255

/* Directory entry object */
struct mfat_dir_entry {
    uint8_t short_name[MFAT_SFN_LEN + 1];
    uint8_t name[MFAT_LFN_LEN + 1];
    /* Start cluster of the object */
    uint32_t cluster;
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
    MFAT_RESULT_NO_FILE,
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

enum mfat_dir_entry_offs {
    /* “Short” file name limited to 11 characters (8.3 format) */
    MFAT_DIR_ENTRY_OFFS_NAME = 0,
    /* The upper two bits of the attribute byte are reserved
     * and must always be set to 0 when a file is created.
     * These bits are not interpreted. */
    MFAT_DIR_ENTRY_OFFS_ATTR = 11,
    /* Reserved. Must be set to 0. */
    MFAT_DIR_ENTRY_OFFS_NTRES = 12,
    /* Component of the file creation time. Count of tenths of a second.
     * Valid range is: 0 <= DIR_CrtTimeTenth <= 199 */
    MFAT_DIR_ENTRY_OFFS_CRTTIMETENTH = 13,
    /* Creation time. Granularity is 2 seconds. */
    MFAT_DIR_ENTRY_OFFS_CRTTIME = 14,
    /* Creation date. */
    MFAT_DIR_ENTRY_OFFS_CRTDATE = 16,
    /* Last access date. Last access is defined as a read or write
     * operation performed on the file/directory described  by this entry.
     * This field must be updated on file modification (write operation)
     * and the date value must be equal to DIR_WrtDate. */
    MFAT_DIR_ENTRY_OFFS_LSTACCDATE = 18,
    /* High word of first data cluster number for file/directory
     * described by this entry. Only valid for volumes formatted FAT32. Must be
     * set to 0 on volumes formatted FAT12/FAT16. */
    MFAT_DIR_ENTRY_OFFS_FSTCLUSHI = 20,
    /* Last modification (write) time. Value must be equal to DIR_CrtTime
     * at file creation. */
    MFAT_DIR_ENTRY_OFFS_WRTTIME = 22,
    /* Last modification (write) date. Value must be equal to DIR_CrtDate
     * at file creation. */
    MFAT_DIR_ENTRY_OFFS_WRTDATE = 24,
    /* Low word of first data cluster number for file/directory described
     * by this entry. */
    MFAT_DIR_ENTRY_OFFS_FSTCLUSLO = 26,
    /* 32-bit quantity containing size in bytes of file/directory
     * described by this entry. */
    MFAT_DIR_ENTRY_OFFS_FILESIZE = 28
};

#define MFAT_DIR_ENTRY_SZ    (32)

/* The file cannot be modified – all modification requests
 * must fail with an appropriate error code value. */
#define MFAT_DIR_ENTRY_ATTR_READ_ONLY           0x01
/* The corresponding file or sub-directory must not be
 * listed unless a request is issued by the user/application
 * explicitly requesting inclusion of “hidden files”. */
#define MFAT_DIR_ENTRY_ATTR_HIDDEN              0x02
/* The corresponding file is tagged as a component of the
 * operating system. It must not be listed unless a request
 * is issued by the user/application explicitly requesting
 * inclusion of “system files”. */
#define MFAT_DIR_ENTRY_ATTR_SYSTEM              0x04
/* The corresponding entry contains the volume label. DIR_FstClusHI
 * and DIR_FstClusLO must always be 0 for the corresponding entry
 * (representing the volume label) since no clusters can be allocated
 * for this entry.
 * Only the root directory can contain one entry with this attribute. No
 * sub-directory must contain an entry of this type. Entries representing
 * long file names are exceptions to these rules. */
#define MFAT_DIR_ENTRY_ATTR_VOLUME_ID           0x08
/* The corresponding entry represents a directory (a child or
 * sub-directory to the containing directory).
 * DIR_FileSize for the corresponding entry must always be 0
 * (even though clusters may have been allocated for the directory). */
#define MFAT_DIR_ENTRY_ATTR_DIRECTORY           0x10
/* This attribute must be set when the file is created, renamed, or
 * modified. The presence of this attribute indicates that properties of the
 * associated file have been modified. Backup utilities can utilize this
 * information to determine the set of files that need to be backed up to
 * ensure protection in case of media and other failure conditions. */
#define MFAT_DIR_ENTRY_ATTR_ARCHIVE             0x20


enum mfat_ldir_entry_offs {
    /* The order of this entry in the sequence of long name
     * directory entries (each containing components of the
     * long file name) associated with the corresponding
     * short name directory entry.
     * 
     * The contents of this field must be masked with 0x40
     * (MFAT_LDIR_LAST_LONG_ENTRY) for the last long directory
     * name entry in the set. Therefore, each sequence of
     * long name directory entries begins with the contents
     * of this field masked with:
     * MFAT_LDIR_ENTRY_ORD_LAST_LONG_ENTRY. */
    MFAT_LDIR_ENTRY_OFFS_ORD        = 0,
    /* Contains characters 1 through 5 constituting a portion
     * of the long name. */
    MFAT_LDIR_ENTRY_OFFS_NAME1      = 1,
    /* Attributes – must be set to MFAT_LDIR_ATTR_LONG_NAME
     * defined as below:
     * See: MFAT_LDIR_ENTRY_ATTR_LONG_NAME
     * NOTE: A mask to determine whether a directory
     * entry is part of the set of a long name directory
     * entries is defined below:
     * See: MFAT_LDIR_ENTRY_ATTR_LONG_NAME_MASK */
    MFAT_LDIR_ENTRY_OFFS_ATTR       = 11,
    /* Must be set to 0 */
    MFAT_LDIR_ENTRY_OFFS_TYPE       = 12,
    /* Checksum of name in the associated short name directory
     * entry at the end of the long name directory entry set. */
    MFAT_LDIR_ENTRY_OFFS_CHKSUM     = 13,
    /* Contains characters 6 through 11 constituting a portion
     * of the long name. */
    MFAT_LDIR_ENTRY_OFFS_NAME2      = 14,
    /* Must be set to 0. */
    MFAT_LDIR_ENTRY_OFFS_FSTCLUSLO  = 26,
    /* Contains characters 12 and 13 constituting a portion
     * of the long name. */
    MFAT_LDIR_ENTRY_OFFS_NAME3      = 28
};

#define MFAT_LDIR_ENTRY_SZ      (32)

#define MFAT_LDIR_ENTRY_ORD_LAST_LONG_ENTRY     (0x40)

#define MFAT_LDIR_ENTRY_ATTR_LONG_NAME          (MFAT_DIR_ENTRY_ATTR_READ_ONLY | MFAT_DIR_ENTRY_ATTR_HIDDEN  \
                                                | MFAT_DIR_ENTRY_ATTR_SYSTEM | MFAT_DIR_ENTRY_ATTR_VOLUME_ID)

#define MFAT_LDIR_ENTRY_ATTR_LONG_NAME_MASK     (MFAT_DIR_ENTRY_ATTR_READ_ONLY | MFAT_DIR_ENTRY_ATTR_HIDDEN  \
                                                | MFAT_DIR_ENTRY_ATTR_SYSTEM | MFAT_DIR_ENTRY_ATTR_VOLUME_ID \
                                                | MFAT_DIR_ENTRY_ATTR_DIRECTORY | MFAT_DIR_ENTRY_ATTR_ARCHIVE)

#define MFAT_INVALID_SECTOR       (-1ULL)

/* Mount a device */
enum mfat_result mfat_mount(struct mfat_fs *fs, struct mfat_device *device, uint8_t vol, uint8_t flags);
/* Unmount a device */
enum mfat_result mfat_unmount(uint8_t vol);

enum mfat_result mfat_opendir(struct mfat_dir *dir, const char *path);

enum mfat_result mfat_readdir(struct mfat_dir *dir, struct mfat_dir_entry *dirent);

/* Restrictions on characters comprising the name:
 * Lower case characters are not allowed
 * Illegal values for characters in the name are as follows:
 * - Values less than 0x20 other than 0x05 for Kanji in first char
 * - 0x22, [0x2a .. 0x2c], [0x2e .. 0x2f], [0x3a .. 0x3f], [0x5b .. 0x5d], 0x7c
 * - Name may not start with 0x20 */
#define mfat_is_valid_sfn_char(C) !((C < 0x20) || (C == 0x22) || (C >= 0x2a && C <= 0x2f) || (C >= 0x2e && C <= 0x2f))

#define mfat_cluster_to_sector(FS, CLUSTER)     (((FS)->data_start_sector) + (((FS)->cluster_sz) * ((CLUSTER) - 2)))

#define mfat_is_seperator(C)	((C) == '/' || (C) == '\\')

#endif
