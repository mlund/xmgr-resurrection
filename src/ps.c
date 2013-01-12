/* $Id: ps.c,v 1.2 1995/06/30 22:32:11 pturner Exp pturner $
 *
 * driver for postscript printer
 *
 * courtesy of:
 *
 * Jim Hudgens
 * hudgens@ray.met.fsu.edu
 *
 * Further modifications by,
 * Ole Holm Nielsen
 * ohnielse@ltf.dth.dk
 *
 * ISO Latin encoding by 
 * Przemek Klosowski
 * przemek@rrdbartok.nist.gov 
 * 
 * Bounding box routines by Ed Vigmond
 * ed@gut.rose.utoronto.ca
 * 
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

typedef struct strlist Strlist;

struct strlist {
	Strlist *next;
	int ptsize;
	char *str;
	int colour;
	int angle;
	int just;
	int x, y;
	};	

static Strlist stringlist;
static Strlist *curstring;

#ifdef VMS
#  include <string.h>
#  include <unixio.h> 
#  include "vms_unix.h"
#endif

#include <time.h>      /* use time() and ctime() for %%CreationDate: */

#include "externs.h"
#include "defines.h"
#include "globals.h"
#include "protos.h"
#include "symfunc.h"
#include "patterns.h"
#include "ps.h"

/* these were found by trial and error */
#define PSTEX_XOFF 20
#define PSTEX_YOFF 20

extern char version[];
extern double charsize;
extern double devcharsize;
extern char printstr[];
extern int monomode;		/* allow 2 colors */

int ps_pathlength = 0;

void ps_header(int ps2flag);
void set_canvas_color(int xl, int yl, int xu, int yu);
static void stroke(void);
static void escape_paren(char *s);
static void putps(char *s);

void define_patterns( int );
void find_pattern( int pattern, unsigned char *pattbits );

/*
 * printer control string
 */
#ifndef PS_PRSTR
char ps_prstr[MAX_BUF_LEN] = PRINT_CMD;
#else
char ps_prstr[MAX_BUF_LEN] = PS_PRSTR;
#endif

#ifndef PS_SUFFIX
char ps_suffix[MAX_BUF_LEN] = ".ps";
#else
char ps_suffix[MAX_BUF_LEN] = PS_SUFFIX;
#endif

#ifndef EPS_SUFFIX
char eps_suffix[MAX_BUF_LEN] = ".eps";
#else
char eps_suffix[MAX_BUF_LEN] = EPS_SUFFIX;
#endif

#ifndef PSTEX_SUFFIX
char pstex_suffix[MAX_BUF_LEN] = ".pstex";
#else
char pstex_suffix[MAX_BUF_LEN] = PSTEX_SUFFIX;
#endif

#ifndef PSTEX_PACKAGE
#define PSTEX_PACKAGE	"graphics"
#endif

#ifndef PSTEX_EPSINPUT
#define PSTEX_EPSINPUT	"\\includegraphics{%s}"
#endif

#ifndef PSTEX_ROTATE
#define PSTEX_ROTATE	"\\rotatebox{90}{"
#endif

#ifndef PSTEX_RESIZE
#define PSTEX_RESIZE		"\\resizebox{%s}{%s}{"
#endif


int psxmin = PSXMIN;
int psxmax = PSXMAX;
int psymin = PSYMIN;
int psymax = PSYMAX;
int psdx = DXPS;
int psdy = DYPS;

extern int bgcolor;		/* canvas colour */

static int pscolor;
static int pslinewidth;
int pslwincr  = 2;  /* factor for setting linewidth increment */
int pslwbegin = 1;  /* offset for setting linewidth */
static int pslinestyle;

static int psdmode;
static int pspattern = 0;
static int psfont = 0;
double pscharsize = 1.7;

char *filname;
FILE *psout;

static int orientflag = PAGE_PORTRAIT;

static char tbuf[MAX_BUF_LEN];

double xconv(double x), yconv(double y);

static int prevx = 99999, prevy = 99999, prevmode;

static void stroke(void)
{
    if (ps_pathlength) {
	fprintf(psout, "s\n");
	prevx = 99999;
	prevy = 99999;
	ps_pathlength = 0;
    }
}

int pssetmode(int mode)
{
    char sysbuf[MAX_BUF_LEN];
    char *mktemp(char *);
    if (mode % 2) {
	if (!ptofile) {
#ifndef VMS
	    strcpy(tbuf, "/tmp/XMgrXXXXXX");
#else
	    sprintf(tbuf, "%s", "SYS$SCRATCH:XMgrXXXXXX.tmp");
#endif	    
	    filname = mktemp(tbuf);
	} else {
	    filname = printstr;
	}
	if ((psout = fopen(filname, "w")) == NULL) {
	    return 0;
	}
    }
    switch (mode) {

    case 3:			/*  portrait */
	orientflag = PAGE_PORTRAIT;
	pscharsize = CHARS;
	if (epsflag) {
	    int w = 0, h = 0;
	    psxmin = PSXMIN;
	    psymin = PSYMIN;
	    if (inwin) { /* if X is active, then use canvas */
#ifdef NONE_GUI
	        ; 
#else
	        get_xlib_dims(&w, &h);
#endif
	    } else { /* else use the set width */
		extern int canvasw, canvash;
		w = canvasw;
		h = canvash;
	    }
	    devwidth = w;
	    devheight = h;
	    if (devwidth == 0) {
		devwidth = PSXMAX - PSXMIN;
	    } else {
	    }
	    if (devheight == 0) {
		devheight = PSYMAX - PSYMIN;
	    } else {
	    }
	    if (((double) devheight) / devwidth >
		((double) (PSYMAX - PSYMIN)) / (PSXMAX - PSXMIN)) {
		psymax = PSYMAX;
		psxmax = (PSYMAX - PSYMIN) * devwidth / devheight + PSXMIN;
		psdy = PSYMAX - PSYMIN;
		psdx = (PSYMAX - PSYMIN) * devwidth / devheight;
	    } else {
		psxmax = PSXMAX;
		psymax = (PSXMAX - PSXMIN) * devheight / devwidth + PSYMIN;
		psdx = PSXMAX - PSXMIN;
		psdy = (PSXMAX - PSXMIN) * devheight / devwidth;
	    }

	    devwidth = psdx;
	    devheight = psdy;
	    devwidthmm = 25.4 * devwidth / 300.0;
	    devheightmm = 25.4 * devheight / 300.0;

	} else {
	    psxmin = PSXMIN;
	    psxmax = PSXMAX;
	    psymin = PSYMIN;
	    psymax = PSYMAX;
	    psdx = DXPS;
	    psdy = DYPS;
	    devwidth = DXPS;
	    devheight = DYPS;
	    devwidthmm = PSWIDTHMM;
	    devheightmm = PSHEIGHTMM;
	}
	devoffsx = 0;
	devoffsy = 0;
	break;

    case 1:			/* EPS landscape */
	orientflag = PAGE_LANDSCAPE;
	pscharsize = CHARS;
	if (epsflag) {
	    psxmin = PSYMIN;
	    psymin = PSXMIN;
	    if (devwidth == 0) {
		devwidth = PSYMAX - PSYMIN;
	    }
	    if (devheight == 0) {
		devheight = PSXMAX - PSXMIN;
	    }
	    if (((double) devheight) / devwidth >
		((double) (PSXMAX - PSXMIN)) / (PSYMAX - PSYMIN)) {
		psymax = PSXMAX;
		psxmax = (PSXMAX - PSXMIN) * devwidth / devheight + PSYMIN;
		psdy = PSXMAX - PSXMIN;
		psdx = (PSXMAX - PSXMIN) * devwidth / devheight;
	    } else {
		psxmax = PSYMAX;
		psymax = (PSYMAX - PSYMIN) * devheight / devwidth + PSXMIN;
		psdx = PSYMAX - PSYMIN;
		psdy = (PSYMAX - PSYMIN) * devheight / devwidth;
	    }
	    devwidth = psdx;
	    devheight = psdy;
	    devwidthmm = 25.4 * devwidth / 300.0;
	    devheightmm = 25.4 * devheight / 300.0;;
	} else {
	    psxmin = PSYMIN;
	    psxmax = PSYMAX;
	    psymin = PSXMIN;
	    psymax = PSXMAX;
	    psdx = DYPS;
	    psdy = DXPS;
	    devwidth = DYPS;
	    devheight = DXPS;
	    devwidthmm = PSHEIGHTMM;
	    devheightmm = PSWIDTHMM;
	}
	devoffsx = 0;
	devoffsy = 0;
	break;

    case 2:
    case 4:
    case 6:
	stroke();
	fprintf(psout, "end\n");
	fprintf(psout, "showpage\n");
	fprintf(psout, "%%%%Trailer\n");
	fclose(psout);
	if (!ptofile) {
	    sprintf(sysbuf, "%s %s", ps_prstr, filname);
	    system(sysbuf);
#ifndef VMS
	    unlink(filname);
#endif
	}
	orientflag = PAGE_PORTRAIT;
	break;
    }
    return mode;
}

void drawps(int x2, int y2, int mode)
{
    int xtmp, ytmp;

/* Eliminate garbage on output */
/*
 * It was reported to occasionally discard a point that was a
 * "moveto"... the origin of a line about to be drawn.
 * If a subsequent point in that line is valid, ps.c issues
 * a "lineto" which PostScript legitimately complains it has
 * no origin for.  The job is then discarded.
*/

/*
 *   if (x2 < 0 || y2 < 0) {
 *	return;
 *   }
*/
    xtmp = x2;
    ytmp = y2;

    if (mode) {
	if (prevmode && xtmp == prevx && ytmp == prevy) {
	    return;		/* previous mode was draw and points are the
				 * same */
	}
	fprintf(psout, "%d %d l\n", xtmp, ytmp);	/* lineto */
    } else {
	/* Avoid excessive moveto's generated by grtool */
	if (xtmp == prevx && ytmp == prevy) {
	    return;
	}
	fprintf(psout, "%d %d m\n", xtmp, ytmp);	/* moveto */
    }
    ps_pathlength++;
    prevx = xtmp;
    prevy = ytmp;

    /*
     * Printers have some maximum number of points in a path. See PostScript
     * Language Reference Manual (Red book), p. 261. Hence the fix that
     * follows
     */

    prevmode = mode;
    if (ps_pathlength > PS_MAXLINELEN) {
	stroke();
	prevmode = 0;
	fprintf(psout, "%d %d m\n", xtmp, ytmp);	/* moveto */
    }
}

int xconvps(double x)	/* world to device */
{
    return ((int) (psxmin + psdx * xconv(x)));
}

int yconvps(double y)
{
    return ((int) (psymin + psdy * yconv(y)));
}


double xconvps_inv(int x) /* ps device to viewport coord's */
{
    return  (x - psxmin)/(double)psdx;
}


double yconvps_inv(int y) /* ps device to viewport coord's */
{
    return ( y - psymin )/(double)psdy;
}


int pssetcolor(int c)
{
    extern unsigned char red[], green[], blue[];
    int itmp;

    stroke();
    if (monomode) {
	itmp = c > 0 ? 1 : 0;
    } else {
	itmp = c;
    }
    if (c != pscolor) {
	if (c >= 0) {
	    fprintf(psout, "%f %f %f setrgbcolor\n",
		    (double) red[itmp] / 255.0,
		    (double) green[itmp] / 255.0,
		    (double) blue[itmp] / 255.0);
	}
    }
    pscolor = c;
    return c;
}

int pssetlinewidth(int c)
{
    stroke();
    if (c != pslinewidth) {
	c = (c - 1) % MAXLINEWIDTH + 1;
        fprintf(psout, "%d setlinewidth\n", 
               (int) (pslwincr * (c - 1) + pslwbegin + 0.51));
    }
    pslinewidth = c;
    return c;
}

void psdrawtic(int x, int y, int dir, int updown)
{
    switch (dir) {
	case 0:
	switch (updown) {
	    case 0:
	    drawps(x, y, 0);
	    drawps(x, y + devxticl, 1);
	    break;
	case 1:
	    drawps(x, y, 0);
	    drawps(x, y - devxticl, 1);
	    break;
	}
	break;
    case 1:
	switch (updown) {
	case 0:
	    drawps(x, y, 0);
	    drawps(x + devyticl, y, 1);
	    break;
	case 1:
	    drawps(x, y, 0);
	    drawps(x - devyticl, y, 1);
	    break;
	}
	break;
    }
}

int pssetlinestyle(int style)
{
    stroke();
    if (style == pslinestyle) {
	return (pslinestyle);
    }
    switch (style) {
    case 1:			/* solid */
	fprintf(psout, "[] 0 setdash\n");
	break;
    case 2:			/* dotted */
	fprintf(psout, "[4 8] 0 setdash\n");
	break;
    case 3:			/* long dash */
	fprintf(psout, "[20 20] 0 setdash\n");
	break;
    case 4:			/* short dash */
	fprintf(psout, "[40 20] 0 setdash\n");
	break;
    case 5:			/* dot-dashed */
	fprintf(psout, "[40 20 12 20] 0 setdash\n");
	break;
    }
    return (pslinestyle = style);
}

char pscurfont[MAX_BUF_LEN] = "/Times-Roman-ISOLatin1 findfont 60 scalefont setfont";
int psfontsize = 60;

void pssetfont(int n)
{
    if (psfont == n) {
	return;
    }
    switch (n) {
    case 0:
        sprintf(pscurfont, "/Times-Roman-ISOLatin1 findfont %d scalefont setfont", psfontsize);
        break;
      case 1:
        sprintf(pscurfont, "/Times-Bold-ISOLatin1 findfont %d scalefont setfont", psfontsize);
        break;
      case 2:
        sprintf(pscurfont, "/Times-Italic-ISOLatin1 findfont %d scalefont setfont", psfontsize);
        break;
      case 3:
        sprintf(pscurfont, "/Times-BoldItalic-ISOLatin1 findfont %d scalefont setfont", psfontsize);
        break;
      case 4:
        sprintf(pscurfont, "/Helvetica-ISOLatin1 findfont %d scalefont setfont", psfontsize);
        break;
      case 5:
        sprintf(pscurfont, "/Helvetica-Bold-ISOLatin1 findfont %d scalefont setfont", psfontsize);
        break;
      case 6:
        sprintf(pscurfont, "/Helvetica-Oblique-ISOLatin1 findfont %d scalefont setfont", psfontsize);
        break;
      case 7:
        sprintf(pscurfont, "/Helvetica-BoldOblique-ISOLatin1 findfont %d scalefont setfont", psfontsize);
        break;
    case 8:
	sprintf(pscurfont, "/Symbol findfont \n%d scalefont\n setfont", psfontsize);
	break;
    case 9:
	sprintf(pscurfont, "/Symbol findfont \n%d scalefont\n setfont", psfontsize);
	break;
    case 10:
	sprintf(pscurfont, "/Symbol findfont \n%d scalefont\n setfont", psfontsize);
	break;
    }
    fprintf(psout, "%s\n", pscurfont);
    hselectfont(psfont = n);
}

void pssetfontsize(double size)
{
    int sf = psfont;

    psfontsize = (int) (size * 60);
    psfont = -1;
    pssetfont(sf);
}

static void escape_paren(char *s)
{
    char t[256];
    int i, cnt = 0;
    for (i = 0; i < strlen(s); i++) {
	if (s[i] == '(' || s[i] == ')') {
	    t[cnt++] = '\\';
	}
	t[cnt++] = s[i];
    }
    t[cnt] = 0;
    strcpy(s, t);
}

void dispstrps(int x, int y, int rot, char *s, int just, int fudge)
{
    char tmpstr[256];

    stroke();
    if (psfontsize == 0 || s == NULL || strlen(s) == 0) {
	return;
    }
    fprintf(psout, "%d %d m\n", x, y);
    fprintf(psout, "gsave\n");
    fprintf(psout, "%d %d translate\n", x, y);
    fprintf(psout, "%d rotate\n", rot);
    if (fudge) {
	fprintf(psout, "0 %d  m\n", -psfontsize / 3);
    } else {
	fprintf(psout, "0 0  m\n");
    }
    switch (just) {
    case 0:
	break;
    case 1:
	stripspecial(s, tmpstr);
	escape_paren(tmpstr);
	fprintf(psout, "(%s) RJ\n", tmpstr);
	break;
    case 2:
	stripspecial(s, tmpstr);
	escape_paren(tmpstr);
	fprintf(psout, "(%s) CS\n", tmpstr);
	break;
    }
    putps(s);
    fprintf(psout, "grestore\n");
    fprintf(psout, "newpath\n");
}

static void putps(char *s)
{
    int i, slen = strlen(s), curcnt = 0;
    int underline = 0, offset = 0;
    double saves = psfontsize / 60.0, scale = psfontsize / 60.0;
    char curstr[256], bkspchar;
    int upperset = 0;
    int symfont = 0;

    if (psfont == 9) {
	symfont = 1;
	upperset = 0x80;
    } else {
	symfont = 0;
	upperset = 0;
    }
    for (i = 0; i < slen; i++) {
	if (s[i] == '-' && isdigit(s[i + 1])) {
	    /* s[i] = 0261; */
	} else if (s[i] == '\\' && isdigit(s[i + 1])) {
	    curstr[curcnt] = 0;
	    if (curcnt >= 1) {
		fprintf(psout, "(%s) show\n", curstr);
	    }
	    curcnt = 0;
	    if (symfont) {
		symfont = 0;
		upperset = 0;
	    }
	    pssetfont(s[i + 1] - '0');
	    if (psfont == 9) {
		symfont = 1;
		upperset = 0x80;
	    }
	    i++;
	    continue;
	} else if (s[i] == '(' || s[i] == ')') {
	    curstr[curcnt++] = '\\';
	} else if (s[i] == '\\' && isoneof(s[i + 1], "cCbxsSNuU+-")) {
	    switch (s[i + 1]) {
	    case 'x':
		curstr[curcnt] = 0;
		if (curcnt >= 1) {
		    fprintf(psout, "(%s) show\n", curstr);
		}
		curcnt = 0;
		if (symfont == 0) {
		    symfont = 1;
		    upperset = 0x80;
		}
		pssetfont(10);
		i++;
		break;
	    case 's':
		curstr[curcnt] = 0;
		if (curcnt >= 1) {
		    fprintf(psout, "(%s) show\n", curstr);
		}
		curcnt = 0;
		pssetfontsize(scale = 0.6 * saves);
		offset -= psfontsize / 2;
		fprintf(psout, "0 %d rmoveto\n", -(psfontsize / 2));
		i++;
		break;
	    case 'S':
		curstr[curcnt] = 0;
		if (curcnt >= 1) {
		    fprintf(psout, "(%s) show\n", curstr);
		}
		curcnt = 0;
		pssetfontsize(scale = 0.6 * saves);
		offset += psfontsize;
		fprintf(psout, "0 %d rmoveto\n", psfontsize);
		i++;
		break;
	    case 'N':
		curstr[curcnt] = 0;
		if (curcnt >= 1) {
		    fprintf(psout, "(%s) show\n", curstr);
		}
		curcnt = 0;
		scale = saves;
		pssetfontsize(scale);
		fprintf(psout, "0 %d rmoveto\n", -offset);
		offset = 0;
/*
		fprintf(psout, "0 %d rmoveto\n", psfontsize);
*/
		i++;
		break;
	    case 'b':
        	curstr[curcnt] = '\0';
            if (curcnt >= 1) {
                fprintf(psout, "(%s) show\n", curstr);
                bkspchar = curstr[curcnt-1];
            } else
                bkspchar = 'M';
       		fprintf(psout, 
               "(%c) stringwidth pop 0 exch neg exch rmoveto\n", bkspchar );
        	curcnt = 0;
			i++;
			break;
	    case 'c':
		upperset = 0x80;
		i++;
		break;
	    case 'C':
		upperset = 0;
		i++;
		break;
	    case 'u':
		underline = 1;
		i++;
		break;
	    case 'U':
		underline = 0;
		i++;
		break;
	    case '-':
		curstr[curcnt] = 0;
		if (curcnt >= 1) {
		    fprintf(psout, "(%s) show\n", curstr);
		}
		curcnt = 0;
		scale -= 0.2;
		if (scale < 0.2) {
		    scale = 0.2;
		}
		pssetfontsize(scale);
		i++;
		break;
	    case '+':
		curstr[curcnt] = 0;
		if (curcnt >= 1) {
		    fprintf(psout, "(%s) show\n", curstr);
		}
		curcnt = 0;
		scale += 0.2;
		pssetfontsize(scale);
		i++;
		break;
	    }
	    continue;
	} else if (s[i] == '\\' && s[i + 1] == '\\') {
	    if( upperset ) {
	    	curstr[curcnt++] = s[i] + upperset;
	    } else {
	    	curstr[curcnt++] = s[i];
	    	curstr[curcnt++] = s[i];
	    }
	    i++;
	    continue;
	}
	curstr[curcnt++] = s[i] + upperset;
    }
    curstr[curcnt] = 0;
    fprintf(psout, "(%s) show\n", curstr);
}

int pssetpat(int k)
{
    stroke();
    k--;
    if (k > PSMAXPAT) {
	k = PSMAXPAT;
    } else if (k < 0) {
	k = 0;
    }
	fprintf(psout, "Pat%d setpattern\n", k );
	pscolor = -1;
    return (pspattern = k);
}

void psfill(int n, int *px, int *py)
{
    int i;

    stroke();
    drawps(px[0], py[0], 0);
    for (i = 1; i < n; i++) {
	drawps(px[i], py[i], 1);
    }
    fprintf(psout, "closepath\n");
/*
 *     fprintf(psout, "%f setgray\n", 1.0 - pspattern / (double) PSMAXPAT);
 */
    fprintf(psout, "gsave eofill grestore\n");
    stroke();
/*
 *     fprintf(psout, "0 setgray\n");
 */
}

void psfillcolor(int n, int *px, int *py)
{
    int i;

    stroke();
    drawps(px[0], py[0], 0);
    for (i = 1; i < n; i++) {
	drawps(px[i], py[i], 1);
    }
    fprintf(psout, "closepath\n");
    fprintf(psout, "gsave eofill grestore\n");
    stroke();
}

void psdrawarc(int x, int y, int r)
{
    stroke();
    fprintf(psout, "%d %d %d %d %d da\n", x, y, r, 0, 360);
}

void psfillarc(int x, int y, int r)
{
    stroke();
    fprintf(psout, "%d %d %d %d %d fa\n", x, y, r, 0, 360);
}

void psdosym(int x, int y, double s, int f, char *sym) {
  stroke();
  switch (f) {
  case 0:
    fprintf(psout,"%d %d %d d%s\n",(int)(devsymsize*s),x,y,sym);
    break;
  case 1:
    fprintf(psout,"%d %d %d f%s\n",(int)(devsymsize*s),x,y,sym);
    break;
  case 2:
    fprintf(psout,"%d %d %d o%s\n",(int)(devsymsize*s),x,y,sym);
    break;
  }
}

void psdoopensym(int x, int y, double s, int f, char *sym) {
  stroke();
  fprintf(psout,"%d %d %d d%s\n",(int)(devsymsize*s),x,y,sym);
}

void pssymcircle(int x, int y, double s, int f) {
  psdosym(x,y,s,f,"a");
}
void pssymsquare(int x, int y, double s, int f) {
  psdosym(x,y,s*0.85,f,"sq");
}
void pssymtriangle1(int x, int y, double s, int f) {
  psdosym(x,y,s,f,"t1");
}
void pssymtriangle2(int x, int y, double s, int f) {
  psdosym(x,y,s,f,"t2");
}
void pssymtriangle3(int x, int y, double s, int f) {
  psdosym(x,y,s,f,"t3");
}
void pssymtriangle4(int x, int y, double s, int f) {
  psdosym(x,y,s,f,"t4");
}
void pssymdiamond(int x, int y, double s, int f) {
  psdosym(x,y,s,f,"di");
}
void pssymplus(int x, int y, double s, int f) {
  psdoopensym(x,y,s,f,"pl");
}
void pssymx(int x, int y, double s, int f) {
  psdoopensym(x,y,0.707*s,f,"x");
}
void pssymsplat(int x, int y, double s, int f) {
  psdoopensym(x,y,s,f,"sp");
}

void psdrawellipse(int x, int y, int xm, int ym)
{
    double scalex, scaley = 1.0;
/*
    if (xm == 0 || ym == 0) {
	return;
    }
*/
    if (xm == 0) { xm = 1; }
    if (ym == 0) { ym = 1; }
    scalex = (double) xm / (double) ym;

    stroke();
    fprintf(psout, "gsave\n");
    fprintf(psout, "%f %f scale\n", scalex, scaley);
    fprintf(psout, "%d %d %d %d %d arc\n", (int) (x * 1.0 / scalex), y, ym, 0, 360);
    fprintf(psout, "stroke\n");
    fprintf(psout, "grestore\n");
}

void psfillellipse(int x, int y, int xm, int ym)
{
    double scalex, scaley = 1.0;


    if (xm == 0 || ym == 0) {
		return;
    }

    if (xm == 0) { xm = 1; }
    if (ym == 0) { ym = 1; }
    scalex = (double) xm / (double) ym;
    stroke();
    fprintf(psout, "gsave\n");
    fprintf(psout, "%f %f scale\n", (double) scalex, scaley );
    fprintf(psout, "%d %d %d %d %d arc\n", (int) (x * 1.0/scalex),y, ym,0,360);
    fprintf(psout, "gsave fill grestore\n");
    fprintf(psout, "stroke\n");
    fprintf(psout, "grestore\n");
}

void psfillellipsecolour(int x, int y, int xm, int ym)
{
    double scalex, scaley = 1.0;


    if (xm == 0 || ym == 0) {
		return;
    }

    if (xm == 0) { xm = 1; }
    if (ym == 0) { ym = 1; }
    scalex = (double) xm / (double) ym;
    stroke();
    fprintf(psout, "gsave\n");
    fprintf(psout, "%f %f scale\n", (double) scalex, scaley );
    fprintf(psout, "%d %d %d %d %d arc\n", (int) (x * 1.0/scalex),y, ym,0,360);
    fprintf(psout, "gsave fill grestore\n");
    fprintf(psout, "stroke\n");
    fprintf(psout, "grestore\n");
}

void psleavegraphics(void)
{
    pssetmode(psdmode + 1);
}

/*           postscript initialization routine  */
int psinitgraphics(int dmode)
{
    extern unsigned char red[], green[], blue[];
    int xmin, ymin, xmax, ymax;

    time_t timebuf;
    char hostnamebuf[MAXHOSTNAMELEN+1];

    ps_pathlength = 0;
    psdmode = dmode;
    if (!pssetmode(psdmode)) {
	return -1;
    }
    
    if (epsflag) 
    	get_bounding_box( &xmin, &xmax, &ymin, &ymax, dmode );
    	
    pscolor = -1;
    pslinewidth = -1;

    devconvx = xconvps;
    devconvy = yconvps;
    vector = drawps;
    devwritestr = dispstrps;
    devsetcolor = pssetcolor;
    devsetfont = pssetfont;
    devsetline = pssetlinestyle;
    devsetlinew = pssetlinewidth;
    devdrawtic = psdrawtic;
    devsetpat = pssetpat;
    devdrawarc = psdrawarc;
    devfillarc = psfillarc;
    devfill = psfill;
    devfillcolor = psfillcolor;
    devdrawellipse = psdrawellipse;
    devfillellipse = psfillellipse;
    devfillellipsecolour = psfillellipsecolour;
    devleavegraphics = psleavegraphics;
    devcharsize = pscharsize;
    devsymsize = 20;
    devxticl = 20;
    devyticl = 20;
    devarrowlength = 20;

    devsymcircle = &pssymcircle;
    devsymsquare=&pssymsquare;
    devsymdiamond=&pssymdiamond;
    devsymtriangle1=&pssymtriangle1;
    devsymtriangle2=&pssymtriangle2;
    devsymtriangle3=&pssymtriangle3;
    devsymtriangle4=&pssymtriangle4;
    devsymplus=&pssymplus;
    devsymx=&pssymx;
    devsymsplat=&pssymsplat;

    if (epsflag) {
		fprintf(psout, "%%!PS-Adobe-2.0 EPSF-1.2\n");
		if (dmode == 1) {										/* landscape */
	    	fprintf(psout, "%%%%BoundingBox: %d %d %d %d\n",
		    			(psymax+psymin-ymax)* 72 / 300,
		    			xmin * 72 / 300,
		    			(psymax+psymin-ymin)* 72 / 300,
		    			xmax * 72 / 300);
		} else {
	    	fprintf(psout, "%%%%BoundingBox: %d %d %d %d\n",
		   			xmin * 72 / 300,
		    		ymin * 72 / 300,
		    		xmax * 72 / 300,
		    		ymax * 72 / 300);
		}
    } else {
		fprintf(psout, "%%!PS-Adobe-2.0\n");
    }
    fprintf(psout, "%%%%Title: %s\n", filname);

    if ((gethostname(hostnamebuf, MAXHOSTNAMELEN) == 0) && (getlogin() != NULL)) {
        fprintf(psout, "%%%%Creator: %s@%s with %s\n", 
                getlogin(), hostnamebuf, version);
    } else {
        fprintf(psout, "%%%%Creator: %s\n", version);
    }
    time(&timebuf);
    fprintf(psout, "%%%%CreationDate: %s", ctime(&timebuf));

    fprintf(psout, "%%%%EndComments\n");

    ps_header( ps2flag );
    
    if( !epsflag )
            set_canvas_color( PSXMIN, PSYMIN, PSXMAX, PSYMAX );
    else if( dmode == 1 )
            set_canvas_color(  psymax+psymin-ymax, xmin, psymax+psymin-ymin, xmax );
    else
            set_canvas_color(  xmin, ymin, xmax, ymax );
    
    /* rotate if in landscape mode */
    if (dmode == 1) {
              fprintf(psout, "%d 0 translate\n", 2 * psymin + psdy);
              fprintf(psout, "90 rotate\n");
    }

    pssetcolor(1);
    pssetlinewidth(1);
    pssetlinestyle(0);
    psfont = -1;
    setfont(2);
    return 0;
}

/*
 * added by Ed Vigmond 17/11/96
 * check for patterns and only define them if they are used 
 */
void define_patterns( int ps2flag )
{
	int i;
	
	if( ps2flag ) {
		fprintf(psout, "/languagelevel where\n" );
		fprintf(psout, "{pop /gs_languagelevel languagelevel def}\n");
		fprintf(psout, "{/gs_languagelevel 1 def} ifelse\n");
		fprintf(psout, "gs_languagelevel 1 gt {\n");
		find_pattern( 1, pat0_bits );
		find_pattern( 2, pat1_bits );
		find_pattern( 3, pat2_bits );
		find_pattern( 4, pat3_bits );
		find_pattern( 5, pat4_bits );
		find_pattern( 6, pat5_bits );
		find_pattern( 7, pat6_bits );
		find_pattern( 8, pat7_bits );
		find_pattern( 9, pat8_bits );
		find_pattern( 10, pat9_bits );
		find_pattern( 11, pat10_bits );
		find_pattern( 12, pat11_bits );
		find_pattern( 13, pat12_bits );
		find_pattern( 14, pat13_bits );
		find_pattern( 15, pat14_bits );
		find_pattern( 16, pat15_bits );
		fprintf(psout, "}{\n");
	}
	for(i=0; i<16;i++ )
		fprintf(psout, "/Pat%d { %f setgray } def\n", i, i*16./256.);
	fprintf(psout, 	"/setpattern { } def\n" );
	if( ps2flag ) {
		fprintf(psout, "} ifelse\n" );
	}
}

/*
 *  verify pattern is used and if it is, define it
 */
void find_pattern( int pattern, unsigned char *pattbits )
{
	int k, found=1;
	
	if( found ) {
		fprintf( psout, "<</PaintType 1 /PatternType 1 /TilingType 1\n\
			/BBox [0 0 12 12] /XStep 12 /YStep 12 /PaintProc{ begin \n\
			12 12 scale 16 16 1 [16 0 0 16 0 0] \n{<" );
		for( k=0; k<32; k++ ) /* invert pattern bits and output them */
			fprintf(psout, "%.2x", ~pattbits[k] & 0xff );
		fprintf( psout, ">} image end }\n\
			>> 	matrix makepattern /Pat%d exch def \n", pattern-1 );
	}
}


/* 
 * output postscript definitions and dictionaries
 */
void ps_header( int ps2flag )
{
	extern unsigned char red[], green[], blue[];

	fprintf(psout, "80 dict begin\n");
	define_patterns( ps2flag );
	fprintf(psout, "/m {moveto} bind def\n");
	fprintf(psout, "/l {lineto} bind def\n");
	fprintf(psout, "/s {stroke} bind def\n");
	fprintf(psout, "%% Symbol fill\n");
	fprintf(psout, "/f { gsave fill grestore stroke } bind def\n");
	fprintf(psout, "%% Opaque symbol\n");
	fprintf(psout, "/o { gsave %f %f %f setrgbcolor\n",
		(double) red[0]/255.0, (double) green[0]/255.0,
		(double) blue[0]/255.0 );
	fprintf(psout, "     fill grestore stroke } bind def\n");
	fprintf(psout, "%% Circle symbol\n");
	fprintf(psout, "/a { 3 -1 roll 0 360 arc } bind def\n");
	fprintf(psout, "/da { a s } bind def\n");
	fprintf(psout, "/fa { a f } bind def\n");
	fprintf(psout, "/oa { a o } bind def\n");
	fprintf(psout, "%% Square symbol\n");
	fprintf(psout, "/sq { moveto dup dup rmoveto 2 mul\n");
	fprintf(psout, "      dup neg 0 rlineto dup neg 0 exch rlineto\n");
	fprintf(psout, "      0 rlineto closepath } bind def\n");
	fprintf(psout, "/dsq { sq s } bind def\n");
	fprintf(psout, "/fsq { sq f } bind def\n");
	fprintf(psout, "/osq { sq o } bind def\n");
	fprintf(psout, "%% Triangle symbols\n");
	fprintf(psout, "/t1 { moveto dup 0 exch rmoveto\n");
	fprintf(psout, "      dup neg dup 2 mul rlineto 2 mul 0 rlineto\n");
	fprintf(psout, "      closepath } bind def\n");
	fprintf(psout, "/dt1 { t1 s } bind def\n");
	fprintf(psout, "/ft1 { t1 f } bind def\n");
	fprintf(psout, "/ot1 { t1 o } bind def\n");
	fprintf(psout, "/t2 { moveto dup neg 0 rmoveto\n");
	fprintf(psout, "      dup dup 2 mul exch neg rlineto\n");
	fprintf(psout, "      2 mul 0 exch rlineto closepath } bind def\n");
	fprintf(psout, "/dt2 { t2 s } bind def\n");
	fprintf(psout, "/ft2 { t2 f } bind def\n");
	fprintf(psout, "/ot2 { t2 o } bind def\n");
	fprintf(psout, "/t3 { moveto dup neg 0 exch rmoveto\n");
	fprintf(psout, "      dup dup 2 mul rlineto neg 2 mul 0 rlineto\n");
	fprintf(psout, "      closepath } bind def\n");
	fprintf(psout, "/dt3 { t3 s } bind def\n");
	fprintf(psout, "/ft3 { t3 f } bind def\n");
	fprintf(psout, "/ot3 { t3 o } bind def\n");
	fprintf(psout, "/t4 { moveto dup 0 rmoveto\n");
	fprintf(psout, "      dup dup -2 mul exch rlineto\n");
	fprintf(psout, "      -2 mul 0 exch rlineto closepath } bind def\n");
	fprintf(psout, "/dt4 { t4 s } bind def\n");
	fprintf(psout, "/ft4 { t4 f } bind def\n");
	fprintf(psout, "/ot4 { t4 o } bind def\n");
	fprintf(psout, "%% Diamond symbol\n");
	fprintf(psout, "/di { moveto dup 0 exch rmoveto\n");
	fprintf(psout, "      dup neg dup rlineto dup dup neg rlineto\n");
	fprintf(psout, "      dup dup rlineto closepath } bind def\n");
	fprintf(psout, "/ddi { di s } bind def\n");
	fprintf(psout, "/fdi { di f } bind def\n");
	fprintf(psout, "/odi { di o } bind def\n");
	fprintf(psout, "%% Plus symbol\n");
	fprintf(psout, "/pl { dup 0 rmoveto dup -2 mul 0 rlineto\n");
	fprintf(psout, "      dup dup rmoveto -2 mul 0 exch rlineto\n");
	fprintf(psout, "    } bind def\n");
	fprintf(psout, "/dpl { m pl s } bind def\n");
	fprintf(psout, "%% x symbol\n");
	fprintf(psout, "/x { dup dup rmoveto dup -2 mul dup rlineto\n");
	fprintf(psout, "     2 mul dup 0 rmoveto dup neg exch rlineto\n");
	fprintf(psout, "   } bind def\n");
	fprintf(psout, "/dx { m x s } bind def\n");
	fprintf(psout, "%% Splat symbol\n");
	fprintf(psout, "/dsp { m dup pl dup 0 exch rmoveto 0.707 mul x s\n");
	fprintf(psout, "     } bind def\n");

	fprintf(psout, "/RJ {\n");
	fprintf(psout, " stringwidth neg exch neg exch\n");
	fprintf(psout, " rmoveto\n");
	fprintf(psout, "} bind def\n");

	fprintf(psout, "/CS {\n");
	fprintf(psout, " stringwidth\n");
	fprintf(psout, " 2 div neg exch 2 div neg exch\n");
	fprintf(psout, " rmoveto\n");
	fprintf(psout, "} bind def\n");

	   /*
    	* The scale converts 300 dots per inch to the standard 72 per inch
    	* 72/300 = 0.24
    	*/
	fprintf(psout, "0.24 0.24 scale\n");
	fprintf(psout, "1 setlinecap\n");
	/*
	 * CREATE ISOLatin1Encoding if not
	 * there already; actually, this differs slightly from ISO, since ISO
	 * leaves /hyphen at pos. 8#055 (decimal 45), plus it has /hyphen at
	 * 8#255 Here, we put /minus at 8#055. Phew. 
	 */
    	fprintf(psout, "\n\
	mark             \n\
	/ISOLatin1Encoding \n\
	  8#000 1 8#054 {StandardEncoding exch get} for \n\
	  /minus \n\
	  8#056 1 8#217 {StandardEncoding exch get} for \n\
	  /dotlessi \n\
	  8#301 1 8#317 {StandardEncoding exch get} for \n\
	  /space /exclamdown /cent /sterling /currency /yen /brokenbar /section \n\
	  /dieresis /copyright /ordfeminine /guillemotleft /logicalnot /hyphen \n\
	  /registered /macron /degree /plusminus /twosuperior /threesuperior /acute \n\
	  /mu /paragraph /periodcentered /cedilla /onesuperior /ordmasculine \n\
	  /guillemotright /onequarter /onehalf /threequarters /questiondown /Agrave \n\
	  /Aacute /Acircumflex /Atilde /Adieresis /Aring /AE /Ccedilla /Egrave /Eacute\n\
	  /Ecircumflex /Edieresis /Igrave /Iacute /Icircumflex /Idieresis /Eth /Ntilde\n\
	  /Ograve /Oacute /Ocircumflex /Otilde /Odieresis /multiply /Oslash /Ugrave \n\
	  /Uacute /Ucircumflex /Udieresis /Yacute /Thorn /germandbls /agrave /aacute \n\
	  /acircumflex /atilde /adieresis /aring /ae /ccedilla /egrave /eacute \n\
	  /ecircumflex /edieresis /igrave /iacute /icircumflex /idieresis /eth /ntilde\n\
	  /ograve /oacute /ocircumflex /otilde /odieresis /divide /oslash /ugrave \n\
	  /uacute /ucircumflex /udieresis /yacute /thorn /ydieresis \n\
	  /ISOLatin1Encoding where not {256 array astore def} if \n\
	 cleartomark \n");
	/* Define makeISOEncoded procedure expecting 'fontname' and returning the
	 * font on stack; the procedure installs new font 'fontname-ISOLatin1' for
	 * later use.  this can be used as either 
    	 /Times-Roman makeISOEncoded  pop ... 
    	 /Times-Roman-ISOL1 findfont 55 scalefont setfont 
	 * (when fonts are found many times) or
    	 /Times-Roman makeISOEncoded 55 scalefont setfont
	 * (for one-shot deals)
	 */
    	fprintf(psout, "\n\
	/makeISOEncoded \n\
	{ findfont /curfont exch def \n\
	  /newfont curfont maxlength dict def  \n\
	  /ISOLatin1 (-ISOLatin1) def\n\
	  /curfontname curfont /FontName get dup length string cvs def \n\
	  /newfontname curfontname length ISOLatin1 length add string \n\
    	 dup 0                  curfontname putinterval \n\
    	 dup curfontname length ISOLatin1   putinterval \n\
	  def \n\
	  curfont   \n\
	  { exch dup /FID ne  \n\
    	{ dup /Encoding eq  \n\
    	  { exch pop ISOLatin1Encoding exch }  \n\
    	  if  \n\
    	  dup /FontName eq  \n\
    	  { exch pop newfontname exch }  \n\
    	  if  \n\
    	  exch newfont 3 1 roll put  \n\
    	}  \n\
    	{ pop pop }  \n\
    	ifelse  \n\
	  }  \n\
	  forall \n\
	  newfontname newfont definefont \n\
	} def \n");

    	fprintf(psout, "\n\
	/Times-Roman makeISOEncoded pop \n\
	/Times-Bold makeISOEncoded pop \n\
	/Times-Italic makeISOEncoded pop \n\
	/Times-BoldItalic makeISOEncoded pop \n\
	/Helvetica makeISOEncoded pop \n\
	/Helvetica-Bold makeISOEncoded pop \n\
	/Helvetica-Oblique makeISOEncoded pop \n\
	/Helvetica-BoldOblique makeISOEncoded pop \n");
}


void set_canvas_color( int xl, int yl, int xu, int yu )
{
	if( bgcolor != 0 ) {
		pssetcolor( bgcolor );
 		fprintf( psout, "%d %d m\n", xl, yl );	
  		fprintf( psout, "%d %d l\n", xu, yl );	
  		fprintf( psout, "%d %d l\n", xu, yu );	
  		fprintf( psout, "%d %d l\n", xl, yu );
  		fprintf( psout, "closepath fill\n" );
  	}
}


void pstex_add2list( int x, int y, int rot, char *s, int just, int fudge   )
{
	if( s[0]=='\\' && s[1]=='L' ) {
		curstring->next = (Strlist *)malloc(sizeof(Strlist));
		curstring->next->next = NULL;
		curstring->x = x;
		curstring->y = y;
		curstring->angle = rot;
		curstring->just = just;
		curstring->str = (char *)malloc( strlen(s)-1 );
		strcpy( curstring->str, s+2 );
		curstring->colour = pscolor;
		curstring->ptsize = psfontsize;
		curstring = curstring->next;
	} else
		dispstrps( x, y, rot, s, just, fudge );
}

int xmin, ymin, xmax, ymax;

/* output strings in tex format */
int do_tex_part( int mode )
{
	char filename[512], jc;
	FILE *pstexout;
	Strlist *oldstr;
	
	strcpy( filename, printstr );
	if( mode==3 || mode==5 ){
		filename[strlen(filename)-5] = '\0';
		strcat( filename, "tex" );
	}else if( mode == 1 )
		strcat( filename, "_t" );

	if( (pstexout=fopen( filename, "w" )) == NULL ) {
		return -1;
	}
	
	/* output tex header */
	if( mode == 3 || mode == 5 ){
		fprintf( pstexout, "\\documentclass[11pt]{report}\n" );
		fprintf( pstexout, "\\pagestyle{empty}\n" );
		fprintf( pstexout, "\\setlength{\\textwidth}{6.5in}\n");
		fprintf( pstexout, "\\setlength{\\textheight}{9.25in}\n" );
		fprintf( pstexout, "\\setlength{\\leftmargin}{0.25in}\n" );
		fprintf( pstexout, "\\setlength{\\oddsidemargin}{0.2in}\n" );
		fprintf( pstexout, "\\setlength{\\evensidemargin}{0.2in}\n" );
		fprintf( pstexout, "\\setlength{\\headheight}{0in}\n" );
		fprintf( pstexout, "\\setlength{\\headsep}{0in}\n" );
		fprintf( pstexout, "\\setlength{\\footskip}{0.25in}\n" );
		fprintf( pstexout, "\\setlength{\\parindent}{3em}\n" );
		fprintf( pstexout, "\\setlength{\\topmargin}{-0.25in}\n" );
		fprintf( pstexout, "\\usepackage{%s}\n", PSTEX_PACKAGE );
		fprintf( pstexout, "\\begin{document}\n\\vspace*{\\fill}\n" );
		fprintf( pstexout, "\\begin{center}\n" );
	}
	
	fprintf( pstexout,"\\begingroup\\makeatletter\\ifx\\SetFigFont\\undefined%%\n");
	fprintf( pstexout, "\\gdef\\SetFigFont#1#2#3{%%\n");
  	fprintf( pstexout, "\\reset@font\\fontsize{#1}{#2pt}%%\n");
	fprintf( pstexout, "\\fontfamily{#3}%%\\fontseries{#4}\\fontshape{#5}%%\n");
	fprintf( pstexout, "\\selectfont}%%\n\\fi\\endgroup%%\n" );

	if( mode == 5  ) {
		fprintf( pstexout, PSTEX_RESIZE, "!", "15cm" );
		fprintf( pstexout, "\n" );
	} else if ( mode == 3  ) {
		fprintf( pstexout, PSTEX_ROTATE );
		fprintf( pstexout, PSTEX_RESIZE, "18cm", "!" );
		fprintf( pstexout, "\n" );
	}
	
	fprintf( pstexout, "\\begin{picture}(0,0)%%\n");
	fprintf( pstexout, PSTEX_EPSINPUT, printstr ); 
	fprintf( pstexout, "\n\\end{picture}%%\n");
	fprintf( pstexout, "\\setlength{\\unitlength}{0.24pt}%%\n%%\n");
	fprintf( pstexout, "\\begingroup\\makeatletter\\ifx\\SetFigFont\\undefined%%\n");
	fprintf( pstexout, "\\gdef\\SetFigFont#1#2#3{%%\n");
  	fprintf( pstexout, "\\reset@font\\fontsize{#1}{#2pt}%%\n");
	fprintf( pstexout, "\\fontfamily{#3}%%\\fontseries{#4}\\fontshape{#5}%%\n");
	fprintf( pstexout, "\\selectfont}%%\n\\fi\\endgroup%%\n" );
	fprintf( pstexout, "\\begin{picture}(%d,%d)(%d,%d)\n",
	 					xmax-xmin, ymax-ymin, xmin+PSTEX_XOFF, ymin+PSTEX_YOFF);

	curstring = &stringlist;
	while( curstring->next != NULL ) {

		if( !curstring->just )
			jc = 'l';
		else if( curstring->just==1 )
			jc = 'r';
		else
			jc = 'c';

		if( curstring->colour != 1 )		/* 1 = black */
			fprintf(pstexout,"\\special{ps: gsave %.2f %.2f %.2f setrgbcolor}\n",
			(double)red[curstring->colour]/255., 
			(double)green[curstring->colour]/255.,
			(double) blue[curstring->colour]/255. );
		
		fprintf( pstexout, "\\put(%d,%d){", curstring->x, curstring->y );		
		
		if( curstring->angle !=0 )
			fprintf( pstexout,"\\special{ps: gsave currentpoint currentpoint \
translate %d rotate neg exch neg exch translate}", -curstring->angle );

		fprintf( pstexout, "\\makebox(0,0)[%cb]{\\smash{", jc );
		fprintf( pstexout, "\\SetFigFont{%.1f}{%.1f}{%s}%s}}", 
					(double)curstring->ptsize/4., 24., "rm", curstring->str );

		if( curstring->angle !=0 )
			fprintf( pstexout, "\\special{ps:currentpoint grestore moveto}" );
			
		fprintf( pstexout, "}\n" );
		
		if( curstring->colour  != 1 )
			fprintf( pstexout, "\\special{ps: grestore}\n" );

		/* move on to the next entry */
		oldstr = curstring;
		curstring = curstring->next;
		free( oldstr->str );
		if( oldstr != &stringlist )	/*this particular structure is not dynamic*/
			free( oldstr );
	}
	
	fprintf( pstexout, "\\end{picture}\n" );
	
	if( mode == 3 ) 
		fprintf( pstexout, "}" );
	if( mode==3 || mode==5)
		fprintf( pstexout, "}\n\\end{center}\n\\vspace*{\\fill}\n\\end{document}\n" );

	fclose( pstexout );
	return 0;
}


void pstexleavegraphics(void)
{
    pssetmode(psdmode + 1);
    do_tex_part( psdmode );
}


/*           postscript and tex initialization routine  */
int pstexinitgraphics(int dmode)
{
    time_t timebuf;
    char hostnamebuf[MAXHOSTNAMELEN+1];

    ptofile=1;
    ps_pathlength = 0;
    psdmode = dmode;
    if ( !pssetmode(3) ) {
		return -1;
    }
    
    get_bounding_box( &xmin, &xmax, &ymin, &ymax, dmode );
    	
    pscolor = -1;
    pslinewidth = -1;

    devconvx = xconvps;
    devconvy = yconvps;
    vector = drawps;
    devwritestr = pstex_add2list;
    devsetcolor = pssetcolor;
    devsetfont = pssetfont;
    devsetline = pssetlinestyle;
    devsetlinew = pssetlinewidth;
    devdrawtic = psdrawtic;
    devsetpat = pssetpat;
    devdrawarc = psdrawarc;
    devfillarc = psfillarc;
    devfill = psfill;
    devfillcolor = psfillcolor;
    devdrawellipse = psdrawellipse;
    devfillellipse = psfillellipse;
    devfillellipsecolour = psfillellipsecolour;
    devleavegraphics = pstexleavegraphics;
    devcharsize = pscharsize;
    devsymsize = 20;
    devxticl = 20;
    devyticl = 20;
    devarrowlength = 20;

    curstring = &stringlist;
    stringlist.next = NULL;

    devsymcircle = &pssymcircle;
    devsymsquare=&pssymsquare;
    devsymdiamond=&pssymdiamond;
    devsymtriangle1=&pssymtriangle1;
    devsymtriangle2=&pssymtriangle2;
    devsymtriangle3=&pssymtriangle3;
    devsymtriangle4=&pssymtriangle4;
    devsymplus=&pssymplus;
    devsymx=&pssymx;
    devsymsplat=&pssymsplat;

	fprintf(psout, "%%!PS-Adobe-2.0 EPSF-1.2\n");
	if (dmode == 2 ) {							/* landscape */
	    fprintf(psout, "%%%%BoundingBox: %d %d %d %d\n",
		    		(psymax+psymin-ymax) * 72 / 300,
		    		xmin * 72 / 300,
		    		(psymax+psymin-ymin) * 72 / 300,
		    		xmax * 72 / 300);
	} else {
	    fprintf(psout, "%%%%BoundingBox: %d %d %d %d\n",
		   		xmin * 72 / 300,
		    	ymin * 72 / 300,
		    	xmax * 72 / 300,
		    	ymax * 72 / 300);
	}

    fprintf(psout, "%%%%Title: %s\n", filname);

    if ((gethostname(hostnamebuf, MAXHOSTNAMELEN) == 0) && (getlogin() != NULL)) {
        fprintf(psout, "%%%%Creator: %s@%s with %s\n", 
                getlogin(), hostnamebuf, version);
    } else {
        fprintf(psout, "%%%%Creator: %s\n", version);
    }
    time(&timebuf);
    fprintf(psout, "%%%%CreationDate: %s", ctime(&timebuf));

    fprintf(psout, "%%%%EndComments\n");
    
	ps_header( ps2flag );

	/* rotate if in landscape mode */
    if (dmode == 2) {
		set_canvas_color(  psymax+psymin-ymax, xmin, psymax+psymin-ymin, xmax );
		fprintf(psout, "%d 0 translate\n", 2 * psymin + psdy);
		fprintf(psout, "90 rotate\n");
    } else
		set_canvas_color(  xmin, ymin, xmax, ymax );
    
    pssetcolor(1);
    pssetlinewidth(1);
    pssetlinestyle(0);
    psfont = -1;
    setfont(2);
    return 0;
}


typedef struct {
	double x1, x2, y1, y2;
	} Bbox;

Bbox bounding_box;		/* current bounding box */
Bbox frame_box;			/* graph frame		*/

int	drawing_sets = FALSE;

int psinitgraphicsbb(int dmode);
void checkpoint( int x, int y );

double xconv(double), yconv(double), xconv_inv(int), yconv_inv(int);

/*
 * get_bounding_box() computes the bounding box for a postscript figure. It
 * does this by essentially creating a new  device which produces no output
 * but checks each point drawn and sees if the bounding box should be
 * adjusted. I simply took the ps.c routines which calculate the coordinates
 * of where to place things and replaced fprintf() calls with calls  to
 * checkpoint().
 */
void get_bounding_box( int *x1, int *x2, int *y1, int *y2, int mode )
{
	int i;
	
	psinitgraphicsbb( mode );
	bounding_box.x2 = bounding_box.y2 = 0;
	bounding_box.x1 = psxmax;
	bounding_box.y1 = psymax;
	for (i = 0; i < maxgraph; i++) {
	    if (isactive_graph(i) && !g[i].hidden) {
			if (checkon_ticks(i) && checkon_world(i)) {
		   	 	plotone(i);
		    	draw_annotation(i);
			}
		}
	}
	draw_annotation(-1);
	leavegraphics();
	*x1 = bounding_box.x1;
	*x2 = bounding_box.x2;
	*y1 = bounding_box.y1;
	*y2 = bounding_box.y2;
	if( debuglevel == 5 ) {
		fprintf( stderr, "Bounding Box (ps): %d %d %d %d\n", 
														*x1, *x2, *y1, *y2 );
		fprintf( stderr, "Bounding Box (view): %f %f %f %f\n",
					xconvps_inv(*x1), yconvps_inv(*y1),
					xconvps_inv(*x2), yconvps_inv(*y2)    );
	}
}

int pssetmodebb(int mode)
{
    char *mktemp(char *);
    if (mode % 2) {
    }
    switch (mode) {

    case 3:			/* portrait */
	orientflag = PAGE_PORTRAIT;
	pscharsize = CHARS;
	if ( 1 ) {
            psxmin = PSXMIN;
            psymin = PSYMIN;
            if (devwidth == 0) {
                devwidth = PSXMAX - PSXMIN;
            }
            if (devheight == 0) {
                devheight = PSYMAX - PSYMIN;
            }
            if (((double) devheight) / devwidth >
                ((double) (PSYMAX - PSYMIN)) / (PSXMAX - PSXMIN)) {
                psymax = PSYMAX;
                psxmax = (PSYMAX - PSYMIN) * devwidth / devheight + PSXMIN;
                psdy = PSYMAX - PSYMIN;
                psdx = (PSYMAX - PSYMIN) * devwidth / devheight;
            } else {
                psxmax = PSXMAX;
                psymax = (PSXMAX - PSXMIN) * devheight / devwidth + PSYMIN;
                psdx = PSXMAX - PSXMIN;
                psdy = (PSXMAX - PSXMIN) * devheight / devwidth;
            }

            devwidth = psdx;
            devheight = psdy;
            devwidthmm = 25.4 * devwidth / 300.0;
            devheightmm = 25.4 * devheight / 300.0;

	} else {
	    psxmin = PSXMIN;
	    psxmax = PSXMAX;
	    psymin = PSYMIN;
	    psymax = PSYMAX;
	    psdx = DXPS;
	    psdy = DYPS;
	    devwidth = DXPS;
	    devheight = DYPS;
	    devwidthmm = PSWIDTHMM;
	    devheightmm = PSHEIGHTMM;
	}
	devoffsx = 0;
	devoffsy = 0;
	break;

    case 1:			/* landscape */
	orientflag = PAGE_LANDSCAPE;
	pscharsize = CHARS;
	if (1) {
	    psxmin = PSYMIN;
	    psymin = PSXMIN;
	    if (devwidth == 0) {
		devwidth = PSYMAX - PSYMIN;
	    }
	    if (devheight == 0) {
		devheight = PSXMAX - PSXMIN;
	    }
	    if (((double) devheight) / devwidth >
		((double) (PSXMAX - PSXMIN)) / (PSYMAX - PSYMIN)) {
		psymax = PSXMAX;
		psxmax = (PSXMAX - PSXMIN) * devwidth / devheight + PSYMIN;
		psdy = PSXMAX - PSXMIN;
		psdx = (PSXMAX - PSXMIN) * devwidth / devheight;
	    } else {
		psxmax = PSYMAX;
		psymax = (PSYMAX - PSYMIN) * devheight / devwidth + PSXMIN;
		psdx = PSYMAX - PSYMIN;
		psdy = (PSYMAX - PSYMIN) * devheight / devwidth;
	    }
	    devwidth = psdx;
	    devheight = psdy;
	    devwidthmm = 25.4 * devwidth / 300.0;
	    devheightmm = 25.4 * devheight / 300.0;;
	} else {
	    psxmin = PSYMIN;
	    psxmax = PSYMAX;
	    psymin = PSXMIN;
	    psymax = PSXMAX;
	    psdx = DYPS;
	    psdy = DXPS;
	    devwidth = DYPS;
	    devheight = DXPS;
	    devwidthmm = PSHEIGHTMM;
	    devheightmm = PSWIDTHMM;
	}
	devoffsx = 0;
	devoffsy = 0;
	break;

    case 2:
    case 4:
	orientflag = PAGE_PORTRAIT;
	break;
    }
    return mode;
}

void drawpsbb(int x2, int y2, int mode)
{
    int xtmp, ytmp;

    if (x2 < 0 || y2 < 0) {	/* Eliminate garbage on output */
	return;
    }
    xtmp = x2;
    ytmp = y2;

    if (mode) {
	if (prevmode && xtmp == prevx && ytmp == prevy) {
	    return;		/* previous mode was draw and points are the
				 * same */
	}
 	checkpoint( xtmp, ytmp);	/* moveto */
   	} else {
	/* Avoid excessive moveto's generated by grtool */
	if (xtmp == prevx && ytmp == prevy) {
	    return;
	}
	checkpoint( xtmp, ytmp);	/* moveto */
    }
    ps_pathlength++;
    prevx = xtmp;
    prevy = ytmp;

    prevmode = mode;
    if (ps_pathlength > PS_MAXLINELEN) {
		prevmode = 0;
    }
}

int pssetcolorbb(int c)
{
	return c;
}

int pssetlinewidthbb(int c)
{
    return c;
}

void psdrawticbb(int x, int y, int dir, int updown)
{
    switch (dir) {
	case 0:
	switch (updown) {
	    case 0:
	    drawpsbb(x, y, 0);
	    drawpsbb(x, y + devxticl, 1);
	    break;
	case 1:
	    drawpsbb(x, y, 0);
	    drawpsbb(x, y - devxticl, 1);
	    break;
	}
	break;
    case 1:
	switch (updown) {
	case 0:
	    drawpsbb(x, y, 0);
	    drawpsbb(x + devyticl, y, 1);
	    break;
	case 1:
	    drawpsbb(x, y, 0);
	    drawpsbb(x - devyticl, y, 1);
	    break;
	}
	break;
    }
}

int pssetlinestylebb(int style)
{
    if (style == pslinestyle) {
	return (pslinestyle);
    }
    return (pslinestyle = style);
}

void pssetfontbb(int n)
{
    if (psfont == n) {
	return;
    }
    hselectfont(psfont = n);
}

void pssetfontsizebb(double size)
{
    int sf = psfont;

    psfontsize = (int) (size * 60);
    psfont = -1;
    pssetfontbb(sf);
}


void dispstrpsbb(int x, int y, int rot, char *s, int just, int fudge)
{
	double hgt, tx;
	double cosx, sinx;
	double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	
    if (psfontsize == 0 || s == NULL || strlen(s) == 0) {
	return;
    }
    hgt = stringextenty(charsize * pscharsize, "N")*1.2;
    tx =  stringextentx(charsize * pscharsize, s)*1.1;
    cosx = cos( rot*M_PI /180. );
    sinx = sin( rot*M_PI /180. );
    
    switch (just) {
    case 0:						/* left justified */
 		y1 = -hgt*.4;
 		y2 = hgt+20.;
    	x1 = 0;
    	x2 = tx;
		break;
	break;
    case 1:						/* right justified	*/
    	x1 = -tx;
    	x2 = 0;
 		y1 = -hgt;
 		y2 = hgt*.4;
 		break;
    case 2:						/* center justified	*/
    	x1 = -tx/2.;
    	x2 =  tx/2.;
    	y1 = -hgt;
    	y2 =  hgt/2.+40.;
		break;
    }
    if (fudge){
      y1 += psfontsize / 3;
      y2 += psfontsize / 3;
    }
   	checkpoint( (int)(x + x1*cosx - y1*sinx), (int)(y + x1*sinx + y1*cosx) );
    checkpoint( (int)(x + x2*cosx - y1*sinx), (int)(y + x2*sinx + y1*cosx) );
    checkpoint( (int)(x + x1*cosx - y2*sinx), (int)(y + x1*sinx + y2*cosx) );
    checkpoint( (int)(x + x2*cosx - y2*sinx), (int)(y + x2*sinx + y2*cosx) );
    if( debuglevel ==5 ) {
    	printf( "string %s: %f %f %f %f\n", s, xconvps_inv(x+x1*cosx - y1*sinx), 
    				xconvps_inv(x+x2*cosx - y2*sinx), 
    				yconvps_inv(y+x1*sinx + y1*cosx), 
    				yconvps_inv(y+x2*sinx + y2*cosx) );
    }
}


int pssetpatbb(int k)
{
    if (k > PSMAXPAT) {
	k = PSMAXPAT;
    } else if (k < 0) {
	k = 0;
    }
    return (pspattern = k);
}

void psfillbb(int n, int *px, int *py)
{
    int i;

    drawpsbb(px[0], py[0], 0);
    for (i = 1; i < n; i++) {
		drawpsbb(px[i], py[i], 1);
    }
}

void psfillcolorbb(int n, int *px, int *py)
{
    int i;

    drawpsbb(px[0], py[0], 0);
    for (i = 1; i < n; i++) {
		drawpsbb(px[i], py[i], 1);
    }
}

void psdrawarcbb(int x, int y, int r)
{
    checkpoint( x+r, y+r );
    checkpoint( x-r, y-r );
}

void psfillarcbb(int x, int y, int r)
{
    checkpoint( x+r, y+r );
    checkpoint( x-r, y-r );
}

void psdrawellipsebb(int x, int y, int xm, int ym)
{
    checkpoint( x+xm, y+ym );
    checkpoint( x-xm, y-ym );
}

void psfillellipsebb(int x, int y, int xm, int ym)
{
    checkpoint( x+xm, y+ym );
    checkpoint( x-xm, y-ym );
}

void psleavegraphicsbb(void)
{
    pssetmodebb(psdmode + 1);
}

/*  postscript initialization routine  */
int psinitgraphicsbb(int dmode)
{
    ps_pathlength = 0;
    psdmode = dmode;
    if (!pssetmodebb(psdmode)) {
	return -1;
    }
    devconvx = xconvps;
    devconvy = yconvps;
    vector = drawpsbb;
    devwritestr = dispstrpsbb;
    devsetcolor = pssetcolorbb;
    devsetfont = pssetfontbb;
    devsetline = pssetlinestylebb;
    devsetlinew = pssetlinewidthbb;
    devdrawtic = psdrawticbb;
    devsetpat = pssetpatbb;
    devdrawarc = psdrawarcbb;
    devfillarc = psfillarcbb;
    devfill = psfillbb;
    devfillcolor = psfillcolorbb;
    devdrawellipse = psdrawellipsebb;
    devfillellipse = psfillellipsebb;
    devfillellipsecolour = psfillellipsebb;
   	devleavegraphics = psleavegraphicsbb;
    devcharsize = pscharsize;
    devsymsize = 20;
    devxticl = 20;
    devyticl = 20;
    devarrowlength = 20;
    psfont = -1;
    setfont(2);
    return 0;
}

/*check if point is in current bounding box*/
void checkpoint( int x, int y ) 
{
	/* make sure sets are clipped to frame */
	if( drawing_sets && ( x<frame_box.x1 || x>frame_box.x2 || 
						  y<frame_box.y1 || y>frame_box.y2   ) )
		return;
	
	/* make sure a valid point is checked. If outside the page, return */
	if( x<0 || y<0 || x>psxmax || y>psymax )
		return;

	if( x<bounding_box.x1 )
		bounding_box.x1 = x;
	if( x>bounding_box.x2 )
		bounding_box.x2 = x;
	if( y<bounding_box.y1 )
		bounding_box.y1 = y;
	if( y>bounding_box.y2 )
		bounding_box.y2 = y;
}


/* 
 * get bounding box pf graph frame
 */
void set_frame_bound( int gno )
{
	world w;
	
	get_graph_world( gno, &w );
	if( w.xg1>w.xg2 )
		fswap( &w.xg1, &w.xg1 );
	if( w.yg1>w.yg2 )
		fswap( &w.yg1, &w.yg1 );
	frame_box.x1 = xconvps( w.xg1 );
	frame_box.x2 = xconvps( w.xg2 );
	frame_box.y1 = yconvps( w.yg1 );
	frame_box.y2 = yconvps( w.yg2 );
	drawing_sets = TRUE;
}


void set_drawing_finished()
{
	drawing_sets = FALSE;
}
