#!/usr/bin/env python3
import argparse
import random
import ctypes

CHAR_SIZE = 8 * ctypes.sizeof(ctypes.c_char(0))
UCHAR_SIZE = CHAR_SIZE

SHORT_SIZE = 8 * ctypes.sizeof(ctypes.c_short(0))
USHORT_SIZE = 8 * ctypes.sizeof(ctypes.c_ushort(0))

INT_SIZE = 8 * ctypes.sizeof(ctypes.c_int(0))
UINT_SIZE = 8 * ctypes.sizeof(ctypes.c_uint(0))
WCHAR_SIZE = INT_SIZE

LONG_SIZE = 8 * ctypes.sizeof(ctypes.c_long(0))
ULONG_SIZE = 8 * ctypes.sizeof(ctypes.c_ulong(0))

LONGLONG_SIZE = 8 * ctypes.sizeof(ctypes.c_longlong(0))
ULONGLONG_SIZE = 8 * ctypes.sizeof(ctypes.c_ulonglong(0))

INTMAX_SIZE = LONGLONG_SIZE
UINTMAX_SIZE = ULONGLONG_SIZE

PTR_SIZE = 8 * ctypes.sizeof(ctypes.c_void_p)
PTRDIFF_SIZE = PTR_SIZE
SIZE_SIZE = 8 * ctypes.sizeof(ctypes.c_size_t(0))

INT_U_MIN = 0
INT_U_MAX = (2 ** (INT_SIZE)) - 1
INT_S_MIN = (2 ** (INT_SIZE - 1)) * -1
INT_S_MAX = (2 ** (INT_SIZE - 1)) - 1

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

FMT_SPECIFIER_CHOICES = ['d', 'i', 'o', 'u', 'x', 'X', 'f', 'F', 'e', 'E', 'g', 'G', 'c', 's', 'p', '%']

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
            # Width in fmt stirng must be positive
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
            arg = random.getrandbits(CHAR_SIZE)
        elif length == "h":
            # short int
            arg = random.getrandbits(SHORT_SIZE)
        elif length == "l":
            # long int
            arg = random.getrandbits(LONG_SIZE)
        elif length == "ll":
            # long long int
            arg = random.getrandbits(LONGLONG_SIZE)
        elif length == "t":
            # ptrdiff_t
            arg = random.getrandbits(PTRDIFF_SIZE)
        elif length == "j":
            # intmax_t
            arg = random.getrandbits(INTMAX_SIZE)
        elif length == "z":
            # size_t
            arg = random.getrandbits(SIZE_SIZE)
        else:
            # Anything else is just int
            arg = random.getrandbits(INT_SIZE)
    elif specifier in ('o', 'u', 'x', 'X'):
        # Signed argument
        if length == "hh":
            # unsigned char
            arg = random.getrandbits(UCHAR_SIZE)
        if length == "h":
            # unsigned short int
            arg = random.getrandbits(USHORT_SIZE)
        elif length == "l":
            # unsigned long int
            arg = random.getrandbits(ULONG_SIZE)
        elif length == "ll":
            # unsigned long long int
            arg = random.getrandbits(ULONGLONG_SIZE)
        elif length == "t":
            # ptrdiff_t
            arg = random.getrandbits(PTRDIFF_SIZE)
        elif length == "j":
            # intmax_t
            arg = random.getrandbits(UINTMAX_SIZE)
        elif length == "z":
            # size_t
            arg = random.getrandbits(SIZE_SIZE)
        else:
            # Anything else is just unsigned int
            arg = random.getrandbits(UINT_SIZE)
    elif specifier == 'c':
        # Character argument
        pass
    elif specifier == '%':
        pass
    else:
        pass

    return result_fmt, result_args

def generate_printf():
    result = []
    # Possible flag characters


def main(args):
    print(gen_fmt_arg())

if __name__ == "__main__":
    main(None)
