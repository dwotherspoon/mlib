#ifndef _MMATH_H
#define _MMATH_H

#define MMATH_NAN         (0.0/0.0)
#define MMATH_POS_INF     (1.0/0.0)
#define MMATH_NEG_INF     (-1.0/0.0)

#define MMATH_PI (3.14159265358979323846264338327)

double mmath_sind(double x);
float mmath_sinf(float x);
int mmath_floorf(float x);
int mmath_floord(double x);
int mmath_ceild(double x);
int mmath_ceilf(float x);


#endif
