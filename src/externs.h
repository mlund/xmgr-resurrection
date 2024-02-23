/* $Id: externs.h,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 *      declarations for draw.c
 *
 */
#ifndef __EXTERNS_H_
#define __EXTERNS_H_

extern int devorient;                  /* device has 0,0 at upper left if true */
extern int devwidthmm;                 /* device width in mm */
extern int devwidth;                   /* device number of points in width */
extern int devheightmm;                /* device height in mm */
extern int devheight;                  /* device number of points in height */
extern int devoffsx;                   /* device offset in x (if not 0) */
extern int devoffsy;                   /* device offset in y (if not 0) */
extern int devxticl, devyticl;         /* common length for device */
extern int devarrowlength;             /* length for arrow device */
extern int devsymsize;                 /* default symbol size */
extern int devcharh, devcharw;         /* typical character height and width */
extern int (*devsetcolor)();           /* routine to set colors */
extern int (*devconvx)();              /* map world x to device */
extern int (*devconvy)();              /* map world y to device y */
extern void (*vector)();               /* device line routine */
extern void (*devwritestr)();          /* device text drawing */
extern void (*devdrawtic)();           /* draw ticks using device draw */
extern void (*devleavegraphics)();     /* device exit */
extern int (*devsetline)();            /* device set line style */
extern int (*devsetlinew)();           /* device set line width */
extern void (*devsetfont)();           /* set device font */
extern int (*devsetpat)();             /* device set fill pattern */
extern void (*devdrawarc)();           /* device arc routine */
extern void (*devfillarc)();           /* device fill arc routine */
extern void (*devdrawellipse)();       /* device ellipse routine */
extern void (*devfillellipse)();       /* device ellipse arc routine */
extern void (*devfillellipsecolour)(); /* device ellipse arc routine */
extern void (*devfill)();              /* device fill routine */
extern void (*devfillcolor)();         /* device fill color routine */

#define MAXLINELEN 800

#define NUM_COLORS 256
extern int ncolors;
extern unsigned char red[], green[], blue[];

#endif /* __EXTERNS_H_ */
