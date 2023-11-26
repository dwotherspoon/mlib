CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -O0 -Isrc/include

TARGET = test

MLIB_OBJS = src/mlib/mstr.o src/mlib/mfat.o src/mlib/mprintf.o src/mlib/mmath.o

all: $(MLIB_OBJS)
	$(CC) $(CFLAGS) $(MLIB_OBJS) src/main.c -o $(TARGET)

.PHONY: clean
clean:
	rm -f $(MLIB_OBJS) $(TARGET)
