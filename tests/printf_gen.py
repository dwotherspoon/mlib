#!/usr/bin/env python3
import argparse
import random

INT_SIZE = 32
INT_U_MIN = 0
INT_U_MAX = 2**(INT_SIZE) - 1
INT_S_MIN = 2 ** (INT_SIZE - 1) * -1
INT_S_MAX = 2 ** (INT_SIZE - 1) - 1

FMT_FLAGS_CHOICES = ['#', '0', '-', ' ', '+']
FMT_FLAGS_PROB = 0.5
FMT_FLAGS_MAX =  7

FMT_WIDTH_PROB = 0.5
FMT_WIDTH_ARG_PROB = 0.25

FMT_PRECISION_PROB = 0.5
FMT_PRECISION_ARG_PROB = 0.25

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
            result_args.append(random.randint(INT_S_MIN, INT_S_MAX))
        else:
            # Width in fmt stirng must be positive
            result_fmt += str(random.randint(INT_U_MIN, INT_U_MAX)).removeprefix('+')
    if biased_choice(FMT_PRECISION_PROB):
        result_fmt += '.'
        if biased_choice(FMT_PRECISION_ARG_PROB):
            result_fmt += "*"
            result_args.append(random.randint(INT_S_MIN, INT_S_MAX))
        else:
            # Precision in fmt string must be positive
            result_fmt += str(random.randint(INT_U_MIN, INT_U_MAX)).removeprefix('+')
    if biased_choice(FMT_LENGTH_PROB):
        length = random.choice(FMT_LENGTH_CHOICES)
        result_fmt += length
    
    specifier = random.choice(FMT_SPECIFIER_CHOICES)
    result_fmt += specifier

    if specifier in ('d', 'i', 'o', 'u'):
        pass

    return result_fmt, result_args

def generate_printf():
    result = []
    # Possible flag characters


def main(args):
    print(gen_fmt_arg())

if __name__ == "__main__":
    main(None)
