#!/usr/bin/env python3

import argparse
import random
import ctypes

CHAR_SIZE = ctypes.sizeof(ctypes.c_char(0)) << 3
UCHAR_SIZE = CHAR_SIZE

SHORT_SIZE = ctypes.sizeof(ctypes.c_short(0)) << 3
USHORT_SIZE = ctypes.sizeof(ctypes.c_ushort(0)) << 3

INT_SIZE = ctypes.sizeof(ctypes.c_int(0)) << 3
UINT_SIZE = ctypes.sizeof(ctypes.c_uint(0)) << 3
WCHAR_SIZE = INT_SIZE

LONG_SIZE = ctypes.sizeof(ctypes.c_long(0)) << 3
ULONG_SIZE = ctypes.sizeof(ctypes.c_ulong(0)) << 3

LONGLONG_SIZE = ctypes.sizeof(ctypes.c_longlong(0)) << 3
ULONGLONG_SIZE = ctypes.sizeof(ctypes.c_ulonglong(0)) << 3

INTMAX_SIZE = LONGLONG_SIZE
UINTMAX_SIZE = ULONGLONG_SIZE

PTR_SIZE = ctypes.sizeof(ctypes.c_void_p) << 3
PTRDIFF_SIZE = PTR_SIZE
SIZE_SIZE = ctypes.sizeof(ctypes.c_size_t(0)) << 3

INT_U_MIN = 0
INT_U_MAX = (2 ** (INT_SIZE)) - 1
INT_S_MIN = (2 ** (INT_SIZE - 1)) * -1
INT_S_MAX = (2 ** (INT_SIZE - 1)) - 1

# Format configuration

FMT_FLAGS_CHOICES = ['#', '0', '-', ' ', '+']
FMT_FLAGS_PROB = 0.5
FMT_FLAGS_MAX =  7

FMT_WIDTH_PROB = 0.5
FMT_WIDTH_ARG_PROB = 0.25
FMT_WIDTH_RANGE = (0, 100)
FMT_WIDTH_ARG_RANGE = (-100, 100)

FMT_PRECISION_PROB = 0.5
FMT_PRECISION_ARG_PROB = 0.25
FMT_PRECISION_RANGE = (0, 100)
FMT_PRECISION_ARG_RANGE = (-100, 100)

FMT_LENGTH_CHOICES = ["h", "hh", "l", "ll", "t", "j", "z"]
FMT_LENGTH_PROB = 0.5

FMT_SPECIFIER_CHOICES = ['d', 'i', 'o', 'u', 'x', 'X', 'c', '%'] #['d', 'i', 'o', 'u', 'x', 'X', 'f', 'F', 'e', 'E', 'g', 'G', 'c', 's', 'p', '%']

# General Configuration

NUM_ARGS = (0, 50)
NUM_CHARS = (0, 500)
CHARS = (ord(' '), ord('~'))

def biased_choice(p):
    return random.random() < p

# Generates a single fmt string and arg pair
# %[flags][width][.precision][length]specifier
def gen_fmt_arg():
    result_fmt = "%"
    result_args = []
    length = None

    if biased_choice(FMT_FLAGS_PROB):
        result_fmt += "".join(random.choices(FMT_FLAGS_CHOICES, k=random.randrange(1, FMT_FLAGS_MAX)))

    # Pick any number of flag characters, multiple times allowed
    if biased_choice(FMT_WIDTH_PROB):
        if biased_choice(FMT_WIDTH_ARG_PROB):
            # Add width specifier in argument
            result_fmt += "*"
            result_args.append(random.randint(FMT_WIDTH_ARG_RANGE[0], FMT_WIDTH_ARG_RANGE[1]))
        else:
            # Width in fmt string must be positive
            result_fmt += str(random.randint(FMT_WIDTH_RANGE[0], FMT_WIDTH_RANGE[1])).removeprefix('+')

    if biased_choice(FMT_PRECISION_PROB):
        result_fmt += '.'
        if biased_choice(FMT_PRECISION_ARG_PROB):
            result_fmt += "*"
            result_args.append(random.randint(FMT_PRECISION_ARG_RANGE[0], FMT_PRECISION_ARG_RANGE[1]))
        else:
            # Precision in fmt string must be positive
            result_fmt += str(random.randint(FMT_PRECISION_RANGE[0], FMT_PRECISION_RANGE[1])).removeprefix('+')

    if biased_choice(FMT_LENGTH_PROB):
        length = random.choice(FMT_LENGTH_CHOICES)
        result_fmt += length

    specifier = random.choice(FMT_SPECIFIER_CHOICES)
    result_fmt += specifier

    if specifier in ('d', 'i'):
        # Unsigned argument
        if length == "hh":
            # signed char
            result_args.append(random.getrandbits(CHAR_SIZE))
        elif length == "h":
            # short int
            result_args.append(random.getrandbits(SHORT_SIZE))
        elif length == "l":
            # long int
            result_args.append(random.getrandbits(LONG_SIZE))
        elif length == "ll":
            # long long int
            result_args.append(random.getrandbits(LONGLONG_SIZE))
        elif length == "t":
            # ptrdiff_t
            result_args.append(random.getrandbits(PTRDIFF_SIZE))
        elif length == "j":
            # intmax_t
            result_args.append(random.getrandbits(INTMAX_SIZE))
        elif length == "z":
            # size_t
            result_args.append(random.getrandbits(SIZE_SIZE))
        else:
            # Anything else is just int
            result_args.append(random.getrandbits(INT_SIZE))
    elif specifier in ('o', 'u', 'x', 'X'):
        # Signed argument
        if length == "hh":
            # unsigned char
            result_args.append(random.getrandbits(UCHAR_SIZE))
        if length == "h":
            # unsigned short int
            result_args.append(random.getrandbits(USHORT_SIZE))
        elif length == "l":
            # unsigned long int
            result_args.append(random.getrandbits(ULONG_SIZE))
        elif length == "ll":
            # unsigned long long int
            result_args.append(random.getrandbits(ULONGLONG_SIZE))
        elif length == "t":
            # ptrdiff_t
            result_args.append(random.getrandbits(PTRDIFF_SIZE))
        elif length == "j":
            # intmax_t
            result_args.append(random.getrandbits(UINTMAX_SIZE))
        elif length == "z":
            # size_t
            result_args.append(random.getrandbits(SIZE_SIZE))
        else:
            # Anything else is just unsigned int
            result_args.append(random.getrandbits(UINT_SIZE))
    elif specifier == 'c':
        # Character argument
        result_args.append(random.randint(CHARS[0], CHARS[1]))
        pass
    elif specifier == '%':
        pass
    else:
        pass

    return result_fmt, result_args

def generate_printf():
    num_args = random.randint(NUM_ARGS[0], NUM_ARGS[1])
    allowed_chars = [chr(v) for v in range(CHARS[0], CHARS[1] + 1)]
    allowed_chars.remove('%')
    allowed_chars[ord('"')] = '\\"'
    print(allowed_chars)
    num_chars = random.randint(NUM_CHARS[0], NUM_CHARS[1])
    # Sorted list of arguments by insertion index (first item)
    arguments = sorted([(random.randint(0, num_chars), gen_fmt_arg()) for _ in range(num_args)], key=lambda i: i[0])
    chars = random.choices(allowed_chars, k=num_chars)
    # Escape quote chars
    args = []
    insert_count = 0
    for idx, (fmt, arg) in arguments:
        chars.insert(idx + insert_count, fmt)
        args.append(arg)
    return (''.join(chars), [a for sub in args for a in sub])


def main(args):
    print(gen_fmt_arg())
    print(generate_printf())

if __name__ == "__main__":
    print("Test")
    main(None)
