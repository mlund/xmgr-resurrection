/*							mconf.h
 *
 *	Common include file for math routines
 *
 *
 *
 * SYNOPSIS:
 *
 * #include "mconf.h"
 *
 *
 *
 * DESCRIPTION:
 *
 * This file contains definitions for error codes that are
 * passed to the common error handling routine mtherr()
 * (which see).
 *
 * The file also includes a conditional assembly definition
 * for the type of computer arithmetic (IEEE, DEC, Motorola
 * IEEE, or UNKnown).
 * 
 * For Digital Equipment PDP-11 and VAX computers, certain
 * IBM systems, and others that use numbers with a 56-bit
 * significand, the symbol DEC should be defined.  In this
 * mode, most floating point constants are given as arrays
 * of octal integers to eliminate decimal to binary conversion
 * errors that might be introduced by the compiler.
 *
 * For little-endian computers, such as IBM PC, that follow the
 * IEEE Standard for Binary Floating Point Arithmetic (ANSI/IEEE
 * Std 754-1985), the symbol IBMPC should be defined.  These
 * numbers have 53-bit significands.  In this mode, constants
 * are provided as arrays of hexadecimal 16 bit integers.
 *
 * Big-endian IEEE format is denoted MIEEE.  On some RISC
 * systems such as Sun SPARC, double precision constants
 * must be stored on 8-byte address boundaries.  Since integer
 * arrays may be aligned differently, the MIEEE configuration
 * may fail on such machines.
 *
 * To accommodate other types of computer arithmetic, all
 * constants are also provided in a normal decimal radix
 * which one can hope are correctly converted to a suitable
 * format by the available C language compiler.  To invoke
 * this mode, define the symbol UNK.
 *
 * An important difference among these modes is a predefined
 * set of machine arithmetic constants for each.  The numbers
 * MACHEP (the machine roundoff error), MAXNUM (largest number
 * represented), and several other parameters are preset by
 * the configuration symbol.  Check the file const.c to
 * ensure that these values are correct for your computer.
 *
 * Configurations NANS, INFINITIES, MINUSZERO, and DENORMAL
 * may fail on many systems.  Verify that they are supposed
 * to work on your computer.
 */

/*
Cephes Math Library Release 2.3:  June, 1995
Copyright 1984, 1987, 1989, 1995 by Stephen L. Moshier

Adjusted for use with ACE/gr by Evgeny Stambulchik, October 1997
*/

#define __ACEGR_SOURCE_

#include <config.h>
#include <cmath.h>

/* Type of computer arithmetic */
/* In ACE/gr, defined as a compiler directive - no need to define here */

/* PDP-11, Pro350, VAX:
 */
#if defined(HAVE_DEC_FPU)
#  define DEC 1
#endif

/* Intel IEEE, low order words come first:
 */
#if defined(HAVE_LIEEE_FPU)
#  define IBMPC 1
#endif

/* Motorola IEEE, high order words come first
 * (Sun 680x0 workstation):
 */
#if defined(HAVE_BIEEE_FPU)
#  define MIEEE 1
#endif

/* UNKnown arithmetic, invokes coefficients given in
 * normal decimal format.  Beware of range boundary
 * problems (MACHEP, MAXLOG, etc. in const.c) and
 * roundoff problems in pow.c:
 * (Sun SPARCstation)
 */

#if (!defined (DEC) && !defined (IBMPC) && !defined (MIEEE))
#  define UNK 1
#endif

/* Define this `volatile' if your compiler thinks
 * that floating point arithmetic obeys the associative
 * and distributive laws.  It will defeat some optimizations
 * (but probably not enough of them).
 *
 * #define VOLATILE volatile
 */

#ifndef VOLATILE
#  define VOLATILE
#endif

#ifdef PI
#  undef PI
#endif

#ifdef NAN
#  undef NAN
#endif

#ifdef INFINITY
#  undef INFINITY
#endif

/* Constant definitions for math error conditions
 */

#if defined(DOMAIN)
#  undef DOMAIN
#endif
#define DOMAIN		1	/* argument domain error */

#if defined(SING)
#  undef SING
#endif
#define SING		2	/* argument singularity */

#if defined(OVERFLOW)
#  undef OVERFLOW
#endif
#define OVERFLOW	3	/* overflow range error */

#if defined(UNDERFLOW)
#  undef UNDERFLOW
#endif
#define UNDERFLOW	4	/* underflow range error */

#if defined(TLOSS)
#  undef TLOSS
#endif
#define TLOSS		5	/* total loss of precision */

#if defined(PLOSS)
#  undef PLOSS
#endif
#define PLOSS		6	/* partial loss of precision */

#if defined(EDOM)
#  undef EDOM
#endif
#define EDOM		33

#if defined(ERANGE)
#  undef ERANGE
#endif
#define ERANGE		34

#if !defined (UNK)
  /* Define to support tiny denormal numbers, else undefine. */
#  define DENORMAL 1

  /* Define to ask for infinity support, else undefine. */
#  define INFINITIES 1

  /* Define to ask for support of numbers that are Not-a-Number,
   else undefine.  This may automatically define INFINITIES in some files. */
#  define NANS 1

  /* Define to distinguish between -0.0 and +0.0.  */
#  define MINUSZERO 1
#endif

/* Define 1 for ANSI C atan2() function
   See atan.c and clog.c. */
#define ANSIC 1

/* Get ANSI function prototypes, if you want them. */
#ifdef __STDC__
#  define ANSIPROT
#  include "protos.h"
#else
   int mtherr();
#endif

/* Variable for error reporting.  See mtherr.c.  */
extern int merror;
