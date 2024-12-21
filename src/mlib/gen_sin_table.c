
#include <stdio.h>
#include <stdint.h>
#include <gmp.h>
#include <mpfr.h>

// mpfr and gmp?

#define NUM_ENTRIES 256

double entries[NUM_ENTRIES];

int main(int argc, char *argv[]) {
    uint32_t entry;

    mpfr_t pi, inc, arg, res;
    double entry_arg, entry_val;

    mpfr_init2(pi, 128);
    mpfr_init2(inc, 128);
    mpfr_init2(arg, 128);
    mpfr_init2(res, 128);

    mpfr_const_pi(pi, MPFR_RNDN);

    mpfr_div_ui(inc, pi, 2*(NUM_ENTRIES - 1), MPFR_RNDN);

    for (entry = 0; entry < NUM_ENTRIES; entry++) {
        mpfr_mul_ui(arg, inc, entry, MPFR_RNDN);
        mpfr_sin(res, arg, MPFR_RNDN);

        entry_arg = mpfr_get_d(arg, MPFR_RNDN);
        entry_val = mpfr_get_d(res, MPFR_RNDN);

        entries[entry] = entry_val;

        printf("[%u] = sin(%.21f) = %.21f\n", entry, entry_arg, entry_val);
    }


    for (entry = 0; entry < NUM_ENTRIES; entry += 4) {
        printf("%.21f, %.21f, %.21f, %.21f,\n",
            entries[entry], entries[entry + 1], entries[entry + 2], entries[entry + 3]);
    }

    return 0;
}
