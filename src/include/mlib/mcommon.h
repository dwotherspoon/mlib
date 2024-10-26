#ifndef _MCOMMON_H
#define _MCOMMON_H

#define MLIB_ABS(X)             (((X) < 0) ? (-(X)) : (X))
#define MLIB_ISPOW2(X)          (((X) & ((X) - 1)) == 0)
#define MLIB_ALIGN_DOWN(A, S)   ((A) & ~((S) - 1))
#define MLIB_ALIGN_UP(A, S)     MLIB_ALIGN_DOWN((A) + (S) - 1, S)

#endif
