#include <mlib/mmath.h>

double mmath_sind(double x) {
    return 0.0;
}

float mmath_sinf(float x) {
    return 0.0;
}

int mmath_floord(double x) {
    int result = (int)x;
    return x < result ? result - 1 : result;
}

int mmath_floorf(float x) {
    int result = (int)x;
    return x < result ? result - 1 : result;
}