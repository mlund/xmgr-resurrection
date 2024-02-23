/*							isfinite()
 *
 *	Floating point numeric utilities
 *
 *
 *
 * SYNOPSIS:
 *
 * int isfinite();
 * n = isfinite(x);
 *
 */

/*
Cephes Math Library Release 2.3:  March, 1995
Copyright 1984, 1995 by Stephen L. Moshier
*/

#if defined(__APPLE__) && defined(__aarch64__)
#define HAVE_ISFINITE
#endif

#include "mconf.h"

#if !defined(HAVE_FINITE) && !defined(HAVE_ISFINITE)

#ifdef UNK
#undef UNK
#if BIGENDIAN
#define MIEEE 1
#else
#define IBMPC 1
#endif
#endif

/* Return 1 if x is not infinite and is not a NaN.  */

int isfinite(x)
double x;
{
#ifdef INFINITIES
    union {
        double d;
        unsigned short s[4];
        unsigned int i[2];
    } u;

    u.d = x;

    if (sizeof(int) == 4) {
#ifdef IBMPC
        if ((u.i[1] & 0x7ff00000) != 0x7ff00000)
            return 1;
#endif
#ifdef DEC
        if ((u.s[3] & 0x7fff) != 0)
            return 1;
#endif
#ifdef MIEEE
        if ((u.i[0] & 0x7ff00000) != 0x7ff00000)
            return 1;
#endif
        return (0);
    } else {
#ifdef IBMPC
        if ((u.s[3] & 0x7ff0) != 0x7ff0)
            return 1;
#endif
#ifdef DEC
        if ((u.s[3] & 0x7fff) != 0)
            return 1;
#endif
#ifdef MIEEE
        if ((u.s[0] & 0x7ff0) != 0x7ff0)
            return 1;
#endif
        return (0);
    }
#else
    /* No INFINITY.  */
    return (1);
#endif
}

#endif /* HAVE_FINITE and HAVE_ISFINITE */
