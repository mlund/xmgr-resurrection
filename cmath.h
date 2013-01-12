/* cmath.h - replacement for math.h or missing in libm functions */

#include <config.h>

#if defined(HAVE_LIBM)
#  include <math.h>
#endif
#if defined(HAVE_IEEEFP_H)
#  include <ieeefp.h>
#endif

#ifndef __ACEGR_SOURCE_

#ifndef MACHEP
extern double MACHEP;
#endif

#ifndef UFLOWTHRESH
extern double UFLOWTHRESH;
#endif

#ifndef MAXNUM
extern double MAXNUM;
#endif

#endif

#ifndef M_PI
#  define M_PI  3.14159265358979323846
#endif

#ifndef HAVE_HYPOT
#  define hypot(x, y) sqrt((x)*(x) + (y)*(y))
#endif

extern double round ( double x );
#ifndef HAVE_RINT
#  define rint round
#endif

#ifndef HAVE_CBRT
extern double cbrt ( double x );
#endif

/* Cygnus gnuwin32 has the log2 macro */
#ifdef log2
#  undef log2
#endif

#ifndef HAVE_LOG2
extern double log2 ( double x );
#endif

#ifndef HAVE_LGAMMA
#  define lgamma lgam
extern double lgam ( double x );
#else
#  ifndef HAVE_LGAMMA_IN_MATH_H
extern double lgamma ( double x );
extern int signgam;
#  endif
#  define lgam lgamma
#  define sgngam signgam
#endif

#ifndef HAVE_ACOSH
extern double acosh ( double x );
#endif

#ifndef HAVE_ASINH
extern double asinh ( double x );
#endif

#ifndef HAVE_ATANH
extern double atanh ( double x );
#endif

#ifndef HAVE_ERF
extern double erf ( double x );
#endif

#ifndef HAVE_ERFC
extern double erfc ( double x );
#endif

#ifndef HAVE_Y0
extern double y0 ( double x );
#endif
#ifndef HAVE_Y1
extern double y1 ( double x );
#endif
#ifndef HAVE_YN
extern double yn ( int n, double x );
#endif
#ifndef HAVE_J0
extern double j0 ( double x );
#endif
#ifndef HAVE_J1
extern double j1 ( double x );
#endif
#ifndef HAVE_JN
extern double jn ( int n, double x );
#endif

#ifndef HAVE_FINITE
#  define finite isfinite
#  ifndef HAVE_ISFINITE
extern int isfinite ( double x );
#  endif
#endif

