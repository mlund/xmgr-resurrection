/* postscript page at scale = 0.25 */

/*
 * the following defines are tuned for our HP-LJ-III
 * and may need adjustment for other printers
 */

#ifndef __PS_H_
#define __PS_H_

/* max length of line that any PS printer accepts */
#define PS_MAXLINELEN 800

/* 8.5 x 11 letter */
/* #define PSXMIN 50
#define PSXMAX 2475
#define PSYMIN 75
#define PSYMAX 3200
#define DXPS 2400
#define DYPS 3150
#define PSWIDTHMM 204
#define PSHEIGHTMM 266 */

#define PSXMIN 75
#define PSXMAX 2475
#define PSYMIN 75
#define PSYMAX 3225
#define DXPS 2400
#define DYPS 3150
#define PSWIDTHMM 204
#define PSHEIGHTMM 266

/* 8.5 x 14 legal */
/*
#define PSXMIN 50
#define PSXMAX 2450
#define PSYMIN 50
#define PSYMAX 4100
#define DXPS 2400
#define DYPS 4050
#define PSWIDTHMM 204
#define PSHEIGHTMM 343
*/

/* A4 */
/*
#define PSXMIN 50
#define PSXMAX 2430
#define PSYMIN 50
#define PSYMAX 3457
#define DXPS 2380
#define DYPS 3450
#define PSWIDTHMM 201
#define PSHEIGHTMM 292
*/

extern double pscharsize;

extern int psxmin;
extern int psxmax;
extern int psymin;
extern int psymax;
extern int psdx;
extern int psdy;
extern int psfontsize;
extern char pscurfont[];
extern int pslwincr;
extern int pslwbegin;

#define CHARS 1.8

#define MINCOLOR 0
#define MAXCOLOR 30
#define PSMAXPAT 30
#define MINLINEWIDTH 0
#define MAXLINEWIDTH 9
#define MAXLINESTYLE 14

double xconvps_inv(int), yconvps_inv(int);
void get_bounding_box(int* x1, int* x2, int* y1, int* y2, int mode);

#endif /* __PS_H_ */
