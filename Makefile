CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -O0 -Isrc/include -g

TARGET_TEST = test
TARGET_MFAT = test_mfat

MLIB_OBJS = src/mlib/mstr.o src/mlib/mfat.o src/mlib/mprintf.o src/mlib/mmath.o

test: $(MLIB_OBJS)
	$(CC) $(CFLAGS) $(MLIB_OBJS) src/main.c -lm -o $(TARGET_TEST)

mfat: $(MLIB_OBJS)
	$(CC) $(CFLAGS) $(MLIB_OBJS) src/test_mfat.c -o $(TARGET_MFAT)

mkfs:
	# Make test file systems
	dd if=/dev/zero of=fat12.fs bs=1M count=32
	mkfs.vfat -F 12 -n MLIB_FAT12 fat12.fs

	dd if=/dev/zero of=fat12_4m.fs bs=1M count=4
	mkfs.vfat -F 12 -n MLIB_FAT12 fat12_4m.fs

	dd if=/dev/zero of=fat16.fs bs=1M count=32
	mkfs.vfat -F 16 -n MLIB_FAT16 fat16.fs

	dd if=/dev/zero of=fat16_16m.fs bs=1M count=16
	mkfs.vfat -F 16 -n MLIB_FAT16 fat16_16m.fs

	dd if=/dev/zero of=fat32.fs bs=1M count=32
	mkfs.vfat -F 32 -n MLIB_FAT32 fat32.fs

	dd if=/dev/zero of=fat32_64m.fs bs=1M count=64
	mkfs.vfat -F 32 -n MLIB_FAT32 fat32_64m.fs

	dd if=/dev/zero of=exfat.fs bs=1M count=32
	mkfs.exfat -L MLIB_EXFAT exfat.fs

	dd if=/dev/zero of=ntfs.fs bs=1M count=32
	mkfs.ntfs -F -L MLIB_NTFS ntfs.fs

.PHONY: clean
clean:
	rm -f $(MLIB_OBJS) $(TARGET) $(TARGET_MFAT)
