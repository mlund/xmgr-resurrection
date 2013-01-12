/* config.h.  Generated from config.h.in by configure.  */
#ifndef __CONFIG_H
#define __CONFIG_H

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
#define _ALL_SOURCE 1
#endif

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
/* #undef CRAY_STACKSEG_END */

/* Define if using alloca.c.  */
/* #undef C_ALLOCA */

/* Define if you have alloca, as a function or macro.  */
#define HAVE_ALLOCA 1

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#define HAVE_ALLOCA_H 1

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define if you have <vfork.h>.  */
/* #undef HAVE_VFORK_H */

/* Define vfork as fork if vfork does not work.  */
/* #undef vfork */

/* Define if you have the drand48 function.  */
#define HAVE_DRAND48 1

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the popen function.  */
#define HAVE_POPEN 1

/* Define if you have the on_exit function.  */
/* #undef HAVE_ON_EXIT */

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the dlopen function.  */
#define HAVE_DLOPEN 1

/* Define if you have the shl_load function.  */
/* #undef HAVE_SHL_LOAD */

/* Define if your FPU arithmetics is of the DEC type.  */
/* #undef HAVE_DEC_FPU */

/* Define if your FPU arithmetics is of the little endian IEEE type.  */
#define HAVE_LIEEE_FPU 1

/* Define if your FPU arithmetics is of the big endian IEEE type.  */
/* #undef HAVE_BIEEE_FPU */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Define if you have <ieeefp.h>.  */
/* #undef HAVE_IEEEFP_H */

/* Define if you have the hypot function.  */
#define HAVE_HYPOT 1

/* Define if you have the cbrt function.  */
#define HAVE_CBRT 1

/* Define if you have the log2 function.  */
#define HAVE_LOG2 1

/* Define if you have the rint function.  */
#define HAVE_RINT 1

/* Define if you have the lgamma function.  */
#define HAVE_LGAMMA 1

/* Define if math.h defines the lgamma function.  */
#define HAVE_LGAMMA_IN_MATH_H 1

/* Define if you have the asinh function.  */
#define HAVE_ASINH 1

/* Define if you have the acosh function.  */
#define HAVE_ACOSH 1

/* Define if you have the atanh function.  */
#define HAVE_ATANH 1

/* Define if you have the erf function.  */
#define HAVE_ERF 1

/* Define if you have the erfc function.  */
#define HAVE_ERFC 1

/* Define if you have the finite function.  */
#define HAVE_FINITE 1

/* Define if you have the isfinite function.  */
/* #undef HAVE_ISFINITE */

/* Define if you have the Bessel j0 function.  */
#define HAVE_J0 1

/* Define if you have the Bessel j1 function.  */
#define HAVE_J1 1

/* Define if you have the Bessel jn function.  */
#define HAVE_JN 1

/* Define if you have the Bessel y0 function.  */
#define HAVE_Y0 1

/* Define if you have the Bessel y1 function.  */
#define HAVE_Y1 1

/* Define if you have the Bessel yn function.  */
#define HAVE_YN 1

/* Define if you have the netcdf library (-lnetcdf).  */
/* #undef HAVE_NETCDF */

/* Define if you have the mfhdf library (-lmfhdf).  */
/* #undef HAVE_MFHDF */

/* Define if you have the XDR functions.  */
#define HAVE_XDR 1

/* Define if you have a Fortran compiler.  */
/* #undef HAVE_F77 */

/* Define if the Fortran compiler adds underscore to the symbol names.  */
/* #undef NEED_F77_UNDERSCORE */

/* Define if the X Window System is missing or not being used.  */
/* #undef X_DISPLAY_MISSING */

/* Define if you have Motif (Lesstif).  */
#define HAVE_MOTIF 1

/* Define if you have the Xpm library (-lXpm).  */
#define HAVE_XPM 1

/* Define if you have the xpm.h.  */
/* #undef HAVE_XPM_H */

/* Define if you have the xpm.h header among X11 includes.  */
#define HAVE_X11_XPM_H 1

/* Define if you have the Xbae library (-lXbae).  */
/* #undef HAVE_LIBXBAE */

/* Define if you have (and want to use) libhelp  */
/* #undef WITH_LIBHELP */

/* Define if you have (and want to use) editres  */
#define WITH_EDITRES 1

#if (defined(HAVE_MOTIF) && !defined(X_DISPLAY_MISSING))
#  define MOTIF_GUI
#else
#  define NONE_GUI
#endif

/* Define if you want to compile in (a basic) support for debugging  */
/* #undef WITH_DEBUG */

#if defined(WITH_DEBUG)
#  define DEBUG
#else
#  define NDEBUG
#endif

#endif /* __CONFIG_H */
