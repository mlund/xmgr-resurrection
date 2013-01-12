/* $Id: setutils.c,v 1.3 1995/04/19 03:31:49 pturner Exp pturner $
 *
 * routines to allocate, manipulate, and return
 * information about sets.
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "protos.h"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

int index_set_types[] =
{SET_XY, SET_XYDX, SET_XYDY, SET_XYDXDX, 
 SET_XYDYDY, SET_XYDXDY, SET_XYZ, 
 SET_XYHILO, SET_XYRT, SET_XYUV, 
 SET_XYBOX, SET_XYBOXPLOT, SET_XYSTRING, SET_POLY, -1};
int index_set_ncols[] =
{2, 3, 3, 4, 4, 4, 3, 5, 3, 4, 5, 2, 3, -1};

/*
 * return the string version of the set type
 */
char *set_types(int it)
{
    char *s = "XY";

    switch (it) {
    case SET_XY:
	s = "xy";
	break;
    case SET_XYDX:
	s = "xydx";
	break;
    case SET_XYDY:
	s = "xydy";
	break;
    case SET_XYDYDY:
	s = "xydydy";
	break;
    case SET_XYDXDX:
	s = "xydxdx";
	break;
    case SET_XYDXDY:
	s = "xydxdy";
	break;
    case SET_XYZ:
	s = "xyz";
	break;
    case SET_XYHILO:
	s = "xyhilo";
	break;
    case SET_XYBOX:
	s = "xybox";
	break;
    case SET_XYRT:
	s = "xyrt";
	break;
    case SET_XYUV:
	s = "xyuv";
	break;
    case SET_XYBOXPLOT:
	s = "xyboxplot";
	break;
    case SET_XYSTRING:
	s = "xystring";
	break;
    case SET_POLY:
	s = "poly";
	break;
    }
    return s;
}

/*
 * needed as initplots is called before
 * the number of planes is determined
 */
void setdefaultcolors(int gno)
{
    int i;
    for (i = 0; i < g[gno].maxplot; i++) {
 	g[gno].p[i].color = (i % ( MAXCOLORS-1)) + 1;
    }
}

/*
 * allocate arrays for a set of length len.
 * Warning! This function itself does not change the dirtystate flag.
 */
void allocxy(plotarr * p, int len)
{
    int i, ncols = 0;

    switch (p->type) {
    case SET_XYSTRING:
	if (p->s == NULL) {
	    p->s = (char **) malloc(len * sizeof(char *));
	} else {
	    p->s = (char **) realloc(p->s, len * sizeof(char *));
	}
    case SET_XY:
	ncols = 2;
	break;
    case SET_XYDX:
    case SET_XYDY:
    case SET_XYZ:
    case SET_POLY:
	ncols = 3;
	break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
    case SET_XYZW:
    case SET_XYRT:
    case SET_XYUV:
    case SET_XYYY:
    case SET_XYXX:
	ncols = 4;
	break;
    case SET_XYHILO:
    case SET_XYBOX:
	ncols = 5;
	break;
    case SET_XYBOXPLOT:
	ncols = 6;
	break;
    }
    if (ncols == 0) {
	fprintf(stderr, "Set type not found in setutils.c:allocxy()!!\n");
	return;
    }
    for (i = 0; i < ncols; i++) {
	if (p->ex[i] == NULL) {
	    if ((p->ex[i] = (double *) calloc(len, sizeof(double))) == NULL) {
		fprintf(stderr, "Insufficient memory to allocate for plots\n");
		exit(1);
	    }
	} else {
	    if ((p->ex[i] = (double *) realloc(p->ex[i], len * sizeof(double))) == NULL) {
		fprintf(stderr, "Insufficient memory to allocate for plots\n");
		exit(1);
	    }
	}
    }
    if (len != p->len) {
	set_lists_dirty(TRUE);
    }
    p->len = len;
}

int init_array(double **a, int n)
{
    if (*a != NULL) {
	*a = (double *) realloc(*a, n * sizeof(double));
    } else {
	*a = (double *) calloc(n, sizeof(double));
    }
    return *a == NULL ? 1 : 0;
}

int init_scratch_arrays(int n)
{
    if (!init_array(&ax, n)) {
	if (!init_array(&bx, n)) {
	    if (!init_array(&cx, n)) {
		if (!init_array(&dx, n)) {
		    maxarr = n;
		    return 0;
		}
		free(cx);
	    }
	    free(bx);
	}
	free(ax);
    }
    return 1;
}

/*
 * get the min/max fields of a set
 */
void getsetminmax(int gno, int setno, double *x1, double *x2, double *y1, double *y2)
{
    *x1 = g[gno].p[setno].xmin;
    *x2 = g[gno].p[setno].xmax;
    *y1 = g[gno].p[setno].ymin;
    *y2 = g[gno].p[setno].ymax;
}

/*
 * get a bounding box for the set
 * over all columns
 */
void getminmaxall(int gno, int setno)
{
    int i, n, ncols;
    double *x;

    ncols = getncols(gno, setno);
    n = getsetlength(gno, setno);
    for (i = 0; i < ncols; i++) {

	if (n == 0) {
	    g[gno].p[setno].emin[i] = 0.0;
	    g[gno].p[setno].emax[i] = 0.0;
	    g[gno].p[setno].imin[i] = 0;
	    g[gno].p[setno].imax[i] = 0;
	} else {
	    x = getcol(gno, setno, i);
	    minmax(x, n, &g[gno].p[setno].emin[i],
		   &g[gno].p[setno].emax[i],
		   &g[gno].p[setno].imin[i],
		   &g[gno].p[setno].imax[i]);
	}
    }
}

/*
 * compute the mins and maxes of a vector x
 */
void minmax(double *x, int n, double *xmin, double *xmax, int *imin, int *imax)
{
    int i;
    *xmin = x[0];
    *xmax = x[0];
    *imin = 1;
    *imax = 1;
    for (i = 1; i < n; i++) {
	if (x[i] < *xmin) {
	    *xmin = x[i];
	    *imin = i + 1;
	}
	if (x[i] > *xmax) {
	    *xmax = x[i];
	    *imax = i + 1;
	}
    }
}

/*
 * compute the mins and maxes of a vector x
 */
double vmin(double *x, int n)
{
    int i;
    double xmin;
    if (n <= 0) {
	return 0.0;
    }
    xmin = x[0];
    for (i = 1; i < n; i++) {
	if (x[i] < xmin) {
	    xmin = x[i];
	}
    }
    return xmin;
}

double vmax(double *x, int n)
{
    int i;
    double xmax;
    if (n <= 0) {
	return 0.0;
    }
    xmax = x[0];
    for (i = 1; i < n; i++) {
	if (x[i] > xmax) {
	    xmax = x[i];
	}
    }
    return xmax;
}

void getsetdxdyminmax(int gno, int setno, double *dx1, double *dx2, double *dy1, double *dy2)
{
    int itmp;

    if (getcol(gno, setno, 2) != NULL) {
	minmax(getcol(gno, setno, 2), getsetlength(gno, setno), dx1, dx2, &itmp, &itmp);
    }
    if (getcol(gno, setno, 3) != NULL) {
	minmax(getcol(gno, setno, 3), getsetlength(gno, setno), dy1, dy2, &itmp, &itmp);
    }
}

/*
 * compute the mins/maxes and update the appropriate fields of
 * set i.
 */
void updatesetminmax(int gno, int setno)
{
    double *tmp = (double *) NULL;
    double b1, b2;
    int i, n, itmp1, itmp2;

    if (isactive_set(gno, setno)) {
	n = getsetlength(gno, setno);
/* compute min/max for each column in the set */
	getminmaxall(gno, setno);
/* compute global min max (applies over all columns) */
	g[gno].p[setno].xmin = g[gno].p[setno].emin[0];
	g[gno].p[setno].xmax = g[gno].p[setno].emax[0];
	g[gno].p[setno].ymin = g[gno].p[setno].emin[1];
	g[gno].p[setno].ymax = g[gno].p[setno].emax[1];
	tmp = (double *) calloc(getsetlength(gno, setno), sizeof(double));
	if (tmp == (double *) NULL) {
	    errmsg("Error: Unable to malloc temporary in updatesetminmax()");
	    return;
	}
	switch (g[gno].p[setno].type) {
	case SET_XYSTRING:
	    break;
	case SET_XY:
	    break;
	case SET_POLY:
	    break;
	case SET_XYDX:
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[0][i] + g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].xmin = min(b1, g[gno].p[setno].xmin);
	    g[gno].p[setno].xmax = max(b2, g[gno].p[setno].xmax);
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[0][i] - g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].xmin = min(b1, g[gno].p[setno].xmin);
	    g[gno].p[setno].xmax = max(b2, g[gno].p[setno].xmax);
	    break;
	case SET_XYDY:
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[1][i] + g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].ymin = min(b1, g[gno].p[setno].ymin);
	    g[gno].p[setno].ymax = max(b2, g[gno].p[setno].ymax);
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[1][i] - g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].ymin = min(b1, g[gno].p[setno].ymin);
	    g[gno].p[setno].ymax = max(b2, g[gno].p[setno].ymax);
	    break;
	case SET_XYDXDX:
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[0][i] + g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].xmin = min(b1, g[gno].p[setno].xmin);
	    g[gno].p[setno].xmax = max(b2, g[gno].p[setno].xmax);
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[0][i] - g[gno].p[setno].ex[3][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].xmin = min(b1, g[gno].p[setno].xmin);
	    g[gno].p[setno].xmax = max(b2, g[gno].p[setno].xmax);
	    break;
	case SET_XYDYDY:
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[1][i] + g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].ymin = min(b1, g[gno].p[setno].ymin);
	    g[gno].p[setno].ymax = max(b2, g[gno].p[setno].ymax);
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[1][i] - g[gno].p[setno].ex[3][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].ymin = min(b1, g[gno].p[setno].ymin);
	    g[gno].p[setno].ymax = max(b2, g[gno].p[setno].ymax);
	    break;
	case SET_XYDXDY:
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[0][i] + g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].xmin = min(b1, g[gno].p[setno].xmin);
	    g[gno].p[setno].xmax = max(b2, g[gno].p[setno].xmax);
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[0][i] - g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].xmin = min(b1, g[gno].p[setno].xmin);
	    g[gno].p[setno].xmax = max(b2, g[gno].p[setno].xmax);
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[1][i] + g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].ymin = min(b1, g[gno].p[setno].ymin);
	    g[gno].p[setno].ymax = max(b2, g[gno].p[setno].ymax);
	    for (i = 0; i < n; i++) {
		tmp[i] = g[gno].p[setno].ex[1][i] - g[gno].p[setno].ex[2][i];
	    }
	    minmax(tmp, n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].ymin = min(b1, g[gno].p[setno].ymin);
	    g[gno].p[setno].ymax = max(b2, g[gno].p[setno].ymax);
	    break;
	case SET_XYZW:
	    break;
	case SET_XYRT:
	    break;
	case SET_XYUV:
	    break;
	case SET_XYBOX:
	    minmax(g[gno].p[setno].ex[2], n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].xmin = min(b1, g[gno].p[setno].xmin);
	    g[gno].p[setno].xmax = max(b2, g[gno].p[setno].xmax);
	    minmax(g[gno].p[setno].ex[3], n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].ymin = min(b1, g[gno].p[setno].ymin);
	    g[gno].p[setno].ymax = max(b2, g[gno].p[setno].ymax);
	    break;
	case SET_XYYY:
	    break;
	case SET_XYXX:
	    break;
	case SET_XYHILO:
	    minmax(g[gno].p[setno].ex[2], n, &b1, &b2, &itmp1, &itmp2);
	    g[gno].p[setno].ymin = min(b1, g[gno].p[setno].ymin);
	    g[gno].p[setno].ymax = max(b2, g[gno].p[setno].ymax);
	    break;
	case SET_XYBOXPLOT:
	    g[gno].p[setno].ymin = g[gno].p[setno].emin[4];
	    g[gno].p[setno].ymax = g[gno].p[setno].emax[5];
	    break;
	}
	if (tmp) {
	    free(tmp);
	}
    } else {
	g[gno].p[setno].xmin = 0.0;
	g[gno].p[setno].xmax = 0.0;
	g[gno].p[setno].ymin = 0.0;
	g[gno].p[setno].ymax = 0.0;
    }
}

void set_point(int gno, int setn, int seti, double wx, double wy)
{
    g[gno].p[setn].ex[0][seti] = wx;
    g[gno].p[setn].ex[1][seti] = wy;
    set_dirtystate();
    updatesetminmax(gno, setn);
}

void get_point(int gno, int setn, int seti, double *wx, double *wy)
{
    *wx = g[gno].p[setn].ex[0][seti];
    *wy = g[gno].p[setn].ex[1][seti];
}

void setcol(int gno, double *x, int setno, int len, int col)
{
    g[gno].p[setno].ex[col] = x;
    g[gno].p[setno].len = len;
    set_dirtystate();
}

int getncols(int gno, int setno)
{
    int i = 0;

    while (g[gno].p[setno].ex[i]) {
	i++;
    }
    return i;
}


void *geteditpoints(int gno, int setno)
{
    return g[gno].p[setno].ep;
}

void setxy(int gno, double **ex, int setno, int len, int ncols)
{
    int i;

    for (i = 0; i < ncols; i++) {
	g[gno].p[setno].ex[i] = ex[i];
    }
    g[gno].p[setno].len = len;
    set_dirtystate();
}

void setlength(int gno, int i, int length)
{
    allocxy(&g[gno].p[i], length);
    set_dirtystate();
}

void copycol2(int gfrom, int setfrom, int gto, int setto, int col)
{
    int i, n;
    double *x1, *x2;

    n = g[gfrom].p[setfrom].len;
    x1 = getcol(gfrom, setfrom, col);
    x2 = getcol(gto, setto, col);
    for (i = 0; i < n; i++) {
	x2[i] = x1[i];
    }
    set_dirtystate();
}

void copycol(int gno, int setfrom, int setto, int col)
{
    copycol2(gno, setfrom, gno, setto, col);
}

/*
 * moveset assumes both sets exist, have their length
 * properly set, and that they are both active
 */
void moveset(int gnofrom, int setfrom, int gnoto, int setto)
{
    int k;

    memcpy(&g[gnoto].p[setto], &g[gnofrom].p[setfrom], sizeof(plotarr));
    for (k = 0; k < MAX_SET_COLS; k++) {
	g[gnofrom].p[setfrom].ex[k] = NULL;
    }
    set_dirtystate();
}

/*
 * copyset assumes both sets exist, have their length
 * properly set, and that they are both active
 */
void copyset(int gnofrom, int setfrom, int gnoto, int setto)
{
    int k;
    double *savec[MAX_SET_COLS];
    int len = getsetlength(gnofrom, setfrom);

    for (k = 0; k < MAX_SET_COLS; k++) {
	savec[k] = g[gnoto].p[setto].ex[k];
    }
    memcpy(&g[gnoto].p[setto], &g[gnofrom].p[setfrom], sizeof(plotarr));
    for (k = 0; k < MAX_SET_COLS; k++) {
	g[gnoto].p[setto].ex[k] = savec[k];
	if (g[gnofrom].p[setfrom].ex[k] != NULL && g[gnoto].p[setto].ex[k] != NULL) {
	    memcpy(g[gnoto].p[setto].ex[k], g[gnofrom].p[setfrom].ex[k], len * sizeof(double));
	}
    }
    set_dirtystate();
}

/*
 * copy everything but the data
 */
void copysetprops(int gnofrom, int setfrom, int gnoto, int setto)
{
    int k;
    double *savec[MAX_SET_COLS];

    for (k = 0; k < MAX_SET_COLS; k++) {
	savec[k] = g[gnoto].p[setto].ex[k];
    }
    memcpy(&g[gnoto].p[setto], &g[gnofrom].p[setfrom], sizeof(plotarr));
    for (k = 0; k < MAX_SET_COLS; k++) {
	g[gnoto].p[setto].ex[k] = savec[k];
    }
    set_dirtystate();
}

/*
 * copy data only
 */
void copysetdata(int gnofrom, int setfrom, int gnoto, int setto)
{
    int k;
    int len = getsetlength(gnofrom, setfrom);

    for (k = 0; k < MAX_SET_COLS; k++) {
	if (g[gnofrom].p[setfrom].ex[k] != NULL && g[gnoto].p[setto].ex[k] != NULL) {
	    memcpy(g[gnoto].p[setto].ex[k], g[gnofrom].p[setfrom].ex[k], len * sizeof(double));
	}
    }
    set_dirtystate();
}

/*
 * pack all sets leaving no gaps in the set structure
 */
void packsets(int gno)
{
    int i, j, k;

    i = 0;
    for (i = 0; i < g[gno].maxplot; i++) {
	if (isactive_set(gno, i)) {
	    j = 0;
	    while (j < i) {
		if (!isactive_set(gno, j) && !g[gno].p[j].deact) {
		    memcpy(&g[gno].p[j], &g[gno].p[i], sizeof(plotarr));
		    for (k = 0; k < MAX_SET_COLS; k++) {
			g[gno].p[i].ex[k] = NULL;
		    }
		    killset(gno, i);
		    set_dirtystate();
#ifndef NONE_GUI
		    updatesymbols(gno, j);
		    updatesymbols(gno, i);
		    updatelegendstr(gno);
		    updatesetminmax(gno, j);
		    updatesetminmax(gno, i);
		    update_set_status(gno, j);
		    update_set_status(gno, i);
#endif
		}
		j++;
	    }
	}
    }
}

/*
 * action proc for menu item
 */
void do_packsets(void)
{
    packsets(cg);
}

/*
 * return the next available set in graph gno
 * ignoring deactivated sets.
 * If target is inactive and not deactivated, choose it (used for loading sets
 * from project files when sets aren't packed)
 */
int nextset(int gno)
{
    int i;

    if ( (target_set.gno == gno) &&
         (target_set.setno >= 0) &&
         (target_set.setno < g[gno].maxplot) &&
         !isactive_set(gno, target_set.setno) &&
         !g[gno].p[target_set.setno].deact ) {
	i = target_set.setno;
	target_set.gno = -1;
	target_set.setno = -1;
	return (i);
    }
    i = 0;
    for (i = 0; i < g[gno].maxplot; i++) {
	if (!isactive_set(gno, i) && !g[gno].p[i].deact) {
	    return (i);
	}
    }
    errmsg("Error - no sets available");
    return (-1);
}

/*
 * kill a set, but preserve the parameter settings
 */
void softkillset(int gno, int setno)
{
    int i;

    if (isactive_set(gno, setno)) {
	for (i = 0; i < MAX_SET_COLS; i++) {
	    if (g[gno].p[setno].ex[i] != NULL) {
		free(g[gno].p[setno].ex[i]);
	    }
	    g[gno].p[setno].ex[i] = NULL;
	}
	if (dataset_type(gno, setno) == SET_XYSTRING) {
	    for (i = 0; i < getsetlength(gno, setno); i++) {
		cxfree(g[gno].p[setno].s[i]);
	    }
	    cxfree(g[gno].p[setno].s[i]);
	    g[gno].p[setno].s = NULL;
	}
	g[gno].p[setno].active = FALSE;
	g[gno].p[setno].deact = 0;
#if defined(HAVE_LIBXBAE)
	epdtor(gno, setno);
#endif
	set_lists_dirty(TRUE);
	set_dirtystate();
    }
}

/*
 * kill a set
 */
void killset(int gno, int setno)
{
    if (isactive_set(gno, setno)) {
	softkillset(gno, setno);
	set_default_plotarr(&g[gno].p[setno]);
    }
}

/*
 * activate a set
 */
void activateset(int gno, int setno)
{
    g[gno].p[setno].active = TRUE;
    g[gno].p[setno].deact = 0;
    g[gno].p[setno].gno = gno;
    g[gno].p[setno].setno = setno;
    set_lists_dirty(TRUE);
    set_dirtystate();
}

/*
 * return 1 if there are active set(s) in the gno graph
 */
int activeset(int gno)
{
    int i;

    for (i = 0; i < g[gno].maxplot; i++) {
	if (g[gno].p[i].active == TRUE) {
	    return (1);
	}
    }
    return (0);
}

/*
 * drop points from a set
 */
void droppoints(int gno, int setno, int startno, int endno, int dist)
{
    double *x;
    int i, j, len, ncols;

    len = getsetlength(gno, setno);
    ncols = getncols(gno, setno);
    for (j = 0; j < ncols; j++) {
	x = getcol(gno, setno, j);
	for (i = endno + 1; i < len; i++) {
	    x[i - dist] = x[i];
	}
    }
    setlength(gno, setno, len - dist);
}

/*
 * join 2 sets together
 */
void joinsets(int g1, int j1, int g2, int j2)
{
    int i, j, len1, len2, ncols1, ncols2, ncols;
    double *x1, *x2;

    len1 = getsetlength(g1, j1);
    len2 = getsetlength(g2, j2);
    setlength(g2, j2, len1 + len2);
    ncols1 = getncols(g1, j1);
    ncols2 = getncols(g2, j2);
    ncols = (ncols2 < ncols1) ? ncols2 : ncols1;
    for (j = 0; j < ncols; j++) {
	x1 = getcol(g1, j1, j);
	x2 = getcol(g2, j2, j);
	for (i = len2; i < len2 + len1; i++) {
	    x2[i] = x1[i - len2];
	}
    }
}

/*
 * sort a set
 */
static double *vptr;

/*
 * for ascending and descending sorts
 */
 
int compare_points1(const void *p1, const void *p2)
{
    if (vptr[* (int * const) p1] < vptr[* (int * const) p2]) {
	return -1;
    }
    if (vptr[* (int * const) p1] > vptr[* (int * const) p2]) {
	return 1;
    }
    return 0;
}

int compare_points2(const void *p1, const void *p2)
{
    if (vptr[* (int * const) p1] > vptr[* (int * const) p2]) {
	return -1;
    }
    if (vptr[* (int * const) p1] < vptr[* (int * const) p2]) {
	return 1;
    }
    return 0;
}


void sortset(int gno, int setno, int sorton, int stype)
{
    int i, j, nc, len, *ind;
    double *dtmp, *stmp;

/*
 * get the vector to sort on
 */
    vptr = getvptr(gno, setno, sorton);
    if (vptr == NULL) {
	errmsg("NULL vector in sort, operation cancelled, check set type");
	return;
    }
    len = getsetlength(gno, setno);
    if (len <= 1) {
	errmsg("Setlength <= 1, nothing to do!");
	return;
    }
/*
 * allocate memory for permuted indices
 */
    ind = (int *) calloc(len, sizeof(int));
    if (ind == NULL) {
	errmsg("Unable to allocate memory for sort");
	return;
    }
/*
 * allocate memory for temporary array
 */
    dtmp = (double *) calloc(len, sizeof(double));
    if (dtmp == NULL) {
	free(ind);
	errmsg("Unable to allocate memory for sort");
	return;
    }
/*
 * initialize indices
 */
    for (i = 0; i < len; i++) {
	ind[i] = i;
    }

/*
 * sort
 */
    qsort(ind, len, sizeof(int),  stype ? compare_points2 : compare_points1);

/*
 * straighten things out - done one vector at a time for storage.
 */
    nc = getncols(gno, setno);
/* loop over the number of columns */
    for (j = 0; j < nc; j++) {
/* get this vector and put into the temporary vector in the right order */
	stmp = getcol(gno, setno, j);
	for (i = 0; i < len; i++) {
	    dtmp[i] = stmp[ind[i]];
	}
/* load it back to the set */
	for (i = 0; i < len; i++) {
	    stmp[i] = dtmp[i];
	}
    }
    set_dirtystate();
}

/*
 * sort a set - only does type SET_XY
 */
void sort_xy(double *tmp1, double *tmp2, int up, int sorton, int stype)
{

    int d, i, j;
    int lo = 0;
    double t1, t2;

    if (sorton == 1) {
	double *ttmp;

	ttmp = tmp1;
	tmp1 = tmp2;
	tmp2 = ttmp;
    }
    up--;

    for (d = up - lo + 1; d > 1;) {
	if (d < 5)
	    d = 1;
	else
	    d = (5 * d - 1) / 11;
	for (i = up - d; i >= lo; i--) {
	    t1 = tmp1[i];
	    t2 = tmp2[i];
	    if (!stype) {
		for (j = i + d; j <= up && (t1 > tmp1[j]); j += d) {
		    tmp1[j - d] = tmp1[j];
		    tmp2[j - d] = tmp2[j];
		}
		tmp1[j - d] = t1;
		tmp2[j - d] = t2;
	    } else {
		for (j = i + d; j <= up && (t1 < tmp1[j]); j += d) {
		    tmp1[j - d] = tmp1[j];
		    tmp2[j - d] = tmp2[j];
		}
		tmp1[j - d] = t1;
		tmp2[j - d] = t2;
	    }
	}
    }
    set_dirtystate();
}

/*
 * locate a point and the set the point is in
 */
void findpoint(int gno, double x, double y, double *xs, double *ys, int *setno, int *loc)
{
    double dx = g[gno].w.xg2 - g[gno].w.xg1, dy = g[gno].w.yg2 - g[gno].w.yg1,
    *xtmp, *ytmp, tmp, tmin = MAXNUM;
    int i, j, len;

    *setno = -1;
    for (i = 0; i < g[gno].maxplot; i++) {
	if (isactive_set(gno, i)) {
	    xtmp = getx(gno, i);
	    ytmp = gety(gno, i);
	    len = getsetlength(gno, i);
	    for (j = 0; j < len; j++) {
		if ((tmp = hypot((x - xtmp[j]) / dx, (y - ytmp[j]) / dy)) < tmin) {
		    *setno = i;
		    *loc = j + 1;
		    *xs = xtmp[j];
		    *ys = ytmp[j];
		    tmin = tmp;
		}
	    }
	}
    }
}

/*
 * locate a point in setno nearest (x, y)
 */
void findpoint_inset(int gno, int setno, double x, double y, int *loc)
{
    double dx = g[gno].w.xg2 - g[gno].w.xg1, dy = g[gno].w.yg2 - g[gno].w.yg1,
    *xtmp, *ytmp, tmp, tmin = MAXNUM;
    int j, len;

    if (isactive_set(gno, setno)) {
	xtmp = getx(gno, setno);
	ytmp = gety(gno, setno);
	len = getsetlength(gno, setno);
	for (j = 0; j < len; j++) {
	    if ((tmp = hypot((x - xtmp[j]) / dx, (y - ytmp[j]) / dy)) < tmin) {
		*loc = j + 1;
		tmin = tmp;
	    }
	}
    } else {
	*loc = -1;
    }
}

/*
 * delete the point pt in setno
 */
void del_point(int gno, int setno, int pt)
{
    int i, j, len, ncols;
    double *tmp;

    ncols = getncols(gno, setno);
    len = getsetlength(gno, setno);
    if (pt > len) {
	return;
    }
    if (pt != len) {
	for (i = pt - 1; i < len - 1; i++) {
	    for (j = 0; j < ncols; j++) {
		tmp = g[gno].p[setno].ex[j];
		tmp[i] = tmp[i + 1];
	    }
	}
    }
    if (len > 1) {
	setlength(gno, setno, len - 1);
    } else {
	softkillset(gno, setno);
    }
    updatesetminmax(gno, setno);
#ifndef NONE_GUI
    drawgraph();
#endif
}

/*
 * add a point to setno
 */
void add_point(int gno, int setno, double px, double py, double tx, double ty, int type)
{
    int len = 0;
    double *x, *y;

    if (isactive_set(gno, setno)) {
		len = getsetlength(gno, setno);
		setlength(gno, setno, len + 1);
		x = getx(gno, setno);
		y = gety(gno, setno);
		x[len] = px;
		y[len] = py;
    } else {
		g[gno].active = TRUE;
		activateset(gno, setno);
		g[gno].p[setno].type = type;
		allocxy(&g[gno].p[setno], 1);
		x = getx(gno, setno);
		y = gety(gno, setno);
		x[0] = px;
		y[0] = py;
    }
    updatesetminmax(gno, setno);
}

/*
 * add a point to setno after or before ind
 * where: 1 = after
 *		  0 = before
 */
void add_point_at(int gno, int setno, int ind, int where, double px, double py, double tx, double ty, int type)
{
    int i, len = 0, ncols;
    double *x, *y, *dx = NULL, *dy = NULL;

    len = getsetlength(gno, setno);
    if (isactive_set(gno, setno) && len > 0) {
		setlength(gno, setno, len + 1);
		ncols = getncols( gno, setno );
		x = getx(gno, setno);
		y = gety(gno, setno);
		if( ncols >=3 ) {
			dx = getcol( gno, setno, 2 );
			if( ncols >= 4 )
				dy = getcol( gno, setno, 3 );
		}
	    for (i = len - 1; i > ind; i--) {
			x[i + 1] = x[i];
			y[i + 1] = y[i];
			if( ncols >=3 ) {
				dx[i+1] = dx[i];
				if( ncols >= 4 )
					dy[i+1] = dy[i];
	    	}
	    }
		if (where) {		/* add after ind */
	    	x[ind + 1] = px;
	    	y[ind + 1] = py;
			if( ncols >=3 ) {
				dx[ind+1] = tx;
				if( ncols >= 4 )
					dy[ind+1] = ty;
	    	}
		} else {		/* add point before ind (at ind) */
			i--;
			x[i + 1] = x[i];
			y[i + 1] = y[i];
			if( ncols >=3 ) {
				dx[i+1] = dx[i];
				if( ncols >= 4 )
					dy[i+1] = dy[i];
	    	}
	    	x[ind] = px;
	    	y[ind] = py;
			if( ncols >=3 ) {
				dx[ind] = tx;
				if( ncols >= 4 )
					dy[ind] = ty;
	    	}
		}
    } else {
		g[gno].active = TRUE;
		activateset(gno, setno);
		g[gno].p[setno].type = type;
		allocxy(&g[gno].p[setno], 1);
		ncols = getncols( gno, setno );
		x = getx(gno, setno);
		y = gety(gno, setno);
		x[0] = px;
		y[0] = py;
		if( ncols >=3 ) {
			dx = getcol( gno, setno, 2 );
			dx[0] = tx;
			if( ncols >= 4 )
				dy = getcol( gno, setno, 3 );
				dy[0] = ty;
	    }
    }
    updatesetminmax(gno, setno);
}

/*
 * copy a set to another set, if the to set doesn't exist
 * get a new one, if it does, ask if it is okay to overwrite
 */
void do_copyset(int gfrom, int j1, int gto, int j2)
{
    if (!isactive_graph(gto)) {
	set_graph_active(gto);
    }
    if (!isactive_set(gfrom, j1)) {
	return;
    }
    if (j1 == j2 && gfrom == gto) {
	return;
    }
    if (isactive_set(gto, j2)) {
	killset(gto, j2);
    }
    activateset(gto, j2);
    settype(gto, j2, dataset_type(gfrom, j1));
    setlength(gto, j2, getsetlength(gfrom, j1));
    copyset(gfrom, j1, gto, j2);
    sprintf(buf, "copy of set %d", j1);
    setcomment(gto, j2, buf);
    log_results(buf);
    updatesetminmax(gto, j2);
#ifndef NONE_GUI
    update_set_status(gto, j2);
#endif
}

/*
 * move a set to another set, in possibly another graph
 */
void do_moveset(int gfrom, int j1, int gto, int j2)
{
    if (!isactive_graph(gto)) {
	set_graph_active(gto);
    }
    if (!isactive_set(gfrom, j1)) {
	return;
    }
    if (j1 == j2 && gto == gfrom) {
	return;
    }
    if (isactive_set(gto, j2)) {
	killset(gto, j2);
    }
    moveset(gfrom, j1, gto, j2);
#ifndef NONE_GUI
    updatesymbols(gto, j2);
    updatesymbols(gfrom, j1);
    updatelegendstr(gto);
#endif
    updatesetminmax(gto, j2);
#ifndef NONE_GUI
    update_set_status(gto, j2);
#endif
    killset(gfrom, j1);
#ifndef NONE_GUI
    update_set_status(gfrom, j1);
#endif
}

/*
 * swap a set with another set
 */
void do_swapset(int gfrom, int j1, int gto, int j2)
{
    plotarr p;

    if (j1 == j2 && gto == gfrom) {
	errmsg("Set from and set to are the same");
	return;
    }
    memcpy(&p, &g[gto].p[j1], sizeof(plotarr));
    memcpy(&g[gto].p[j1], &g[gfrom].p[j2], sizeof(plotarr));
    memcpy(&g[gfrom].p[j2], &p, sizeof(plotarr));
    set_lists_dirty(TRUE);
    set_dirtystate();
    updatesetminmax(gfrom, j1);
#ifndef NONE_GUI
    updatesymbols(gfrom, j1);
    updatelegendstr(gfrom);
    update_set_status(gfrom, j1);
#endif
    updatesetminmax(gto, j2);
#ifndef NONE_GUI
    updatesymbols(gto, j2);
    updatelegendstr(gto);
    update_set_status(gto, j2);
    drawgraph();
#endif
}

/*
 * activate a set and set its length
 */
void do_activateset(int gno, int setno, int len)
{
    if (isactive_set(gno, setno)) {
	sprintf(buf, "Set %d already active", setno);
	errmsg(buf);
	return;
    }
    if (len <= 0) {
	sprintf(buf, "Improper set length = %d", len);
	errmsg(buf);
	return;
    }
    activateset(gno, setno);
    setlength(gno, setno, len);
    updatesetminmax(gno, setno);
#ifndef NONE_GUI
    update_set_status(gno, setno);
#endif
}

/*
 * split a set into lpart length sets
 */
void do_splitsets(int gno, int setno, int lpart)
{
    int i, j, k, nsets, ncols, len, nleft, tmpset, psets, stype;
    char s[256];
    double *x[MAX_SET_COLS], *xtmp[MAX_SET_COLS], *xt[MAX_SET_COLS];
    plotarr p;

    if (!activeset(gno)) {
	errmsg("No active sets");
	return;
    }
    if (!isactive_set(gno, setno)) {
	sprintf(s, "Set %d not active", setno);
	errmsg(s);
	return;
    }
    if ((len = getsetlength(gno, setno)) < 3) {
	errmsg("Set length < 3");
	return;
    }
    if (lpart >= len) {
	errmsg("Split length >= set length");
	return;
    }
    if (lpart == 0) {
	errmsg("Split length = 0");
	return;
    }
    psets = len / lpart;
    nleft = len % lpart;
    if (nleft) {
	psets++;
    }
    nsets = 0;

    for (i = 0; i < g[gno].maxplot; i++) {
	if (isactive_set(gno, i)) {
	    nsets++;
	}
    }
    if (psets > (g[gno].maxplot - nsets + 1)) {
	errmsg("Not enough sets for split");
	return;
    }
    /* get number of columns in this set */
    ncols = getncols(gno, setno);

    /* copy the contents to a temporary buffer */
    for (j = 0; j < ncols; j++) {
	x[j] = getcol(gno, setno, j);
	xtmp[j] = (double *) calloc(len, sizeof(double));
	if (xtmp[j] == NULL) {
	    errmsg("Not enough memory for split");
	    for (k = 0; k < j; k++) {
		cxfree(xtmp[k]);
	    }
	    return;
	}
    }
    for (j = 0; j < ncols; j++) {
	for (i = 0; i < len; i++) {
	    xtmp[j][i] = x[j][i];
	}
    }

    /* save the set type */
    stype = dataset_type(gno, setno);
    /*
     * load the props for this set into a temporary set, set the columns to
     * NULL
     */
    p = g[gno].p[setno];
    p.len = 0;
    for (k = 0; k < MAX_SET_COLS; k++) {
	p.ex[k] = NULL;
    }

    /* return the set to the heap */
    killset(gno, setno);
    /* now load each set */

    for (i = 0; i < psets - 1; i++) {
	tmpset = nextset(gno);
	/* set the plot parameters includes the set type */
	g[gno].p[tmpset] = p;
	activateset(gno, tmpset);
	settype(gno, tmpset, stype);
	setlength(gno, tmpset, lpart);
	/* load the data into each column */
	for (k = 0; k < ncols; k++) {
	    xt[k] = getcol(gno, tmpset, k);
	    for (j = 0; j < lpart; j++) {
		xt[k][j] = xtmp[k][i * lpart + j];
	    }
	}
	sprintf(s, "partition %d of set %d", i + 1, setno);
	setcomment(gno, tmpset, s);
	log_results(buf);
	updatesetminmax(gno, tmpset);
#ifndef NONE_GUI
	update_set_status(gno, tmpset);
#endif
    }
    if (nleft == 0) {
	nleft = lpart;
    }
    tmpset = nextset(gno);
    memcpy(&g[gno].p[tmpset], &p, sizeof(plotarr));
    activateset(gno, tmpset);
    settype(gno, tmpset, stype);
    setlength(gno, tmpset, nleft);

    /* load the data into each column */
    for (k = 0; k < ncols; k++) {
	xt[k] = getcol(gno, tmpset, k);
	for (j = 0; j < nleft; j++) {
	    xt[k][j] = xtmp[k][i * lpart + j];
	}
    }

    sprintf(s, "partition %d of set %d", i + 1, setno);
    setcomment(gno, tmpset, s);
    log_results(buf);
    updatesetminmax(gno, tmpset);
#ifndef NONE_GUI
    update_set_status(gno, tmpset);
#endif
    for (k = 0; k < ncols; k++) {
	free(xtmp[k]);
    }

#ifndef NONE_GUI
    drawgraph();
#endif
}

/*
 * break a set at a point
 */
void do_breakset(int gno, int setno, int ind)
{
    int j, k, ncols, len, tmpset, stype;
    int n1, n2;
    char s[256];
    double *e1, *e2;

    if (!activeset(gno)) {
	errmsg("No active sets");
	return;
    }
    if (!isactive_set(gno, setno)) {
	sprintf(s, "Set %d not active", setno);
	errmsg(s);
	return;
    }
    if ((len = getsetlength(gno, setno)) < ind + 1) {
	errmsg("Set length less than point index");
	return;
    }
    /* get number of columns in this set */
    ncols = getncols(gno, setno);
    stype = dataset_type(gno, setno);

    n2 = len - ind;		/* upper part of new set */
    n1 = len - n2;		/* lower part of old set */
    if (n1 <= 0 || n2 <= 0) {
	errmsg("Break set length <= 0");
	return;
    }
    tmpset = nextset(gno);
    if (tmpset == -1) {
	return;
    }
    activateset(gno, tmpset);
    settype(gno, tmpset, stype);
    setlength(gno, tmpset, n2);

    /* load the data into each column */
    for (k = 0; k < ncols; k++) {
	e1 = getcol(gno, setno, k);
	e2 = getcol(gno, tmpset, k);
	for (j = ind; j < len; j++) {
	    e2[j - ind] = e1[j];
	}
    }

    setlength(gno, setno, n1);
    updatesetminmax(gno, setno);
#ifndef NONE_GUI
    update_set_status(gno, setno);
#endif

    sprintf(s, "Break S%d at point %d", setno, ind);
    setcomment(gno, tmpset, s);
    log_results(buf);
    updatesetminmax(gno, tmpset);
#ifndef NONE_GUI
    update_set_status(gno, tmpset);
    drawgraph();
#endif
}


/*
 * activate a set and set its length
 */
void do_activate(int setno, int type, int len)
{
    type = index_set_types[type];
    if (isactive_set(cg, setno)) {
	sprintf(buf, "Set %d already active", setno);
	errmsg(buf);
	return;
    }
    if (len <= 0) {
	sprintf(buf, "Improper set length = %d", len);
	errmsg(buf);
	return;
    }
    activateset(cg, setno);
    settype(cg, setno, type);
    setlength(cg, setno, len);
    updatesetminmax(cg, setno);
#ifndef NONE_GUI
    update_set_status(cg, setno);
#endif
}

/*
 * de-activate a set
 */
void do_deactivate(int gno, int setno)
{
    g[gno].p[setno].active = FALSE;
    g[gno].p[setno].deact = TRUE;
    set_lists_dirty(TRUE);
#ifndef NONE_GUI
    update_set_status(gno, setno);
#endif
}

/*
 * re-activate a set
 */
void do_reactivate(int gno, int setno)
{
    if (g[gno].p[setno].deact && (g[gno].p[setno].ex[0] != NULL)) {
	g[gno].p[setno].deact = FALSE;
	g[gno].p[setno].active = TRUE;
    }
    g[gno].p[setno].deact = FALSE;
    set_lists_dirty(TRUE);
#ifndef NONE_GUI
    update_set_status(gno, setno);
#endif
}

/*
 * change the type of a set
 */
void do_changetype(int setno, int type)
{
    type = index_set_types[type];
    settype(cg, setno, type);
    setlength(cg, setno, getsetlength(cg, setno));
    updatesetminmax(cg, setno);
#ifndef NONE_GUI
    update_set_status(cg, setno);
#endif
}

/*
 * set the length of an active set - contents are destroyed
 */
void do_setlength(int setno, int len)
{
    if (!isactive_set(cg, setno)) {
	sprintf(buf, "Set %d not active", setno);
	errmsg(buf);
	return;
    }
    if (len <= 0) {
	sprintf(buf, "Improper set length = %d", len);
	errmsg(buf);
	return;
    }
    setlength(cg, setno, len);
    updatesetminmax(cg, setno);
#ifndef NONE_GUI
    update_set_status(cg, setno);
#endif
}

/*
 * copy a set to another set, if the to set doesn't exist
 * get a new one, if it does, ask if it is okay to overwrite
 */
void do_copy(int j1, int gfrom, int j2, int gto)
{
    if (!isactive_set(gfrom, j1)) {
	sprintf(buf, "Set %d not active", j1);
	errmsg(buf);
	return;
    }
    gto--;
    if (gto == -1) {
	gto = cg;
    }
    if (!isactive_graph(gto)) {
	set_graph_active(gto);
    }
    if (j1 == j2 && gfrom == gto) {
	errmsg("Set from and set to are the same");
	return;
    }
    /* select next set */
    if (j2 == SET_SELECT_NEXT) {
	if ((j2 = nextset(gto)) != -1) {
	    activateset(gto, j2);
	    settype(gto, j2, dataset_type(gfrom, j1));
	    setlength(gto, j2, getsetlength(gfrom, j1));
	} else {
	    return;
	}
    }
    /* use user selected set */
    else {
	if (isactive_set(gto, j2)) {
	    sprintf(buf, "Set %d active, overwrite?", j2);
	    if (!yesno(buf, NULL, NULL, NULL)) {
		return;
	    }
	    killset(gto, j2);
	}
	activateset(gto, j2);
	settype(gto, j2, dataset_type(gfrom, j1));
	setlength(gto, j2, getsetlength(gfrom, j1));
    }
    copyset(gfrom, j1, gto, j2);
    sprintf(buf, "copy of set %d", j1);
    setcomment(gto, j2, buf);
    log_results(buf);
    updatesetminmax(gto, j2);
#ifndef NONE_GUI
    update_set_status(gto, j2);
    drawgraph();
#endif
}

/*
 * move a set to another set, if the to set doesn't exist
 * get a new one, if it does, ask if it is okay to overwrite
 */
void do_move(int j1, int gfrom, int j2, int gto)
{
    if (!isactive_set(gfrom, j1)) {
	sprintf(buf, "Set %d not active", j1);
	errmsg(buf);
	return;
    }
    gto--;
    if (gto == -1) {
	gto = cg;
    }
    if (!isactive_graph(gto)) {
	set_graph_active(gto);
    }
    if (j2 == SET_SELECT_NEXT) {
	if ((j2 = nextset(gto)) == -1) {
	    return;
	}
    }
    if (j1 == j2 && gto == gfrom) {
	errmsg("Set from and set to are the same");
	return;
    }
    if (isactive_set(gto, j2)) {
	sprintf(buf, "Set %d active, overwrite?", j2);
	if (!yesno(buf, NULL, NULL, NULL)) {
	    return;
	}
	killset(gto, j2);
    }
    moveset(gfrom, j1, gto, j2);
#ifndef NONE_GUI
    updatesymbols(gto, j2);
    updatesymbols(gfrom, j1);
    updatelegendstr(gto);
    updatesetminmax(gto, j2);
    update_set_status(gto, j2);
#endif
    killset(gfrom, j1);
#ifndef NONE_GUI
    update_set_status(gfrom, j1);
    drawgraph();
#endif
}

/*
 * swap a set with another set
 */
void do_swap(int j1, int gfrom, int j2, int gto)
{
    gfrom--;
    if (gfrom == -1) {
	gfrom = cg;
    }
    gto--;
    if (gto == -1) {
	gto = cg;
    }
    if (j1 == j2 && gfrom == gto) {
	errmsg("Set from and set to are the same");
	return;
    }
    do_swapset(gfrom, j1, gto, j2);
}

/*
 * drop points from an active set
 */
void do_drop_points(int setno, int startno, int endno)
{
    int dist;

    if (!isactive_set(cg, setno)) {
	sprintf(buf, "Set %d not active", setno);
	errmsg(buf);
	return;
    }
    dist = endno - startno + 1;
    if (startno < 0) {
	errmsg("Start # < 1");
	return;
    }
    if (endno >= getsetlength(cg, setno)) {
	errmsg("Ending # > set length");
	return;
    }
    if (startno > endno) {
	errmsg("Starting # > ending #");
	return;
    }
    if (dist == getsetlength(cg, setno)) {
	errmsg("# of points to drop = set length, use kill");
	return;
    }
    droppoints(cg, setno, startno, endno, dist);
    updatesetminmax(cg, setno);
#ifndef NONE_GUI
    update_set_status(cg, setno);
    drawgraph();
#endif
}

/*
 * append one set to another
 */
void do_join_sets(int gfrom, int j1, int gto, int j2)
{
    int i;

    if (j1 == -1) {
	if (!isactive_set(gfrom, j2)) {
	    activateset(gfrom, j2);
	    setlength(gfrom, j2, 0);
	}
	for (i = 0; i < g[gfrom].maxplot; i++) {
	    if (isactive_set(gfrom, i) && i != j2) {
		joinsets(gfrom, i, gfrom, j2);
		killset(gfrom, i);
#ifndef NONE_GUI
		update_set_status(gfrom, i);
#endif
	    }
	}
    } else {
	if (!isactive_set(gfrom, j1)) {
	    sprintf(buf, "Set %d not active", j1);
	    errmsg(buf);
	    return;
	}
	if (!isactive_set(gto, j2)) {
	    sprintf(buf, "Set %d not active", j2);
	    errmsg(buf);
	    return;
	}
	joinsets(gfrom, j1, gto, j2);
	killset(gfrom, j1);
#ifndef NONE_GUI
	update_set_status(gfrom, j1);
#endif
    }
    updatesetminmax(gto, j2);
#ifndef NONE_GUI
    update_set_status(gto, j2);
#endif
}

/*
 * reverse the order of a set
 */
void do_reverse_sets(int setno)
{
    int n, i, j, k, ncols;
    double *x;

    if (!isactive_set(cg, setno)) {
	sprintf(buf, "Set %d not active", setno);
	errmsg(buf);
	return;
    }
    n = getsetlength(cg, setno);
    ncols = getncols(cg, setno);
    for (k = 0; k < ncols; k++) {
	x = getcol(cg, setno, k);
	for (i = 0; i < n / 2; i++) {
	    j = (n - 1) - i;
	    fswap(&x[i], &x[j]);
	}
    }
    set_dirtystate();
#ifndef NONE_GUI
    update_set_status(cg, setno);
#endif
}

/*
 * coalesce sets
 */
void do_coalesce_sets(int setno)
{
    int i, first = 1;

    if (!activeset(cg)) {
	errmsg("No active sets");
	return;
    }
    if (isactive_set(cg, setno)) {
	sprintf(buf, "Set %d active, need an inactive set", setno);
	errmsg(buf);
	return;
    } else {
	if ((setno = nextset(cg)) != -1) {
	    activateset(cg, setno);
	} else {
	    return;
	}
	for (i = 0; i < g[cg].maxplot; i++) {
	    if (isactive_set(cg, i) && i != setno) {
		if (first) {
		    first = 0;
		    setlength(cg, setno, getsetlength(cg, i));
		    copyset(cg, i, cg, setno);
		    killset(cg, i);
#ifndef NONE_GUI
		    update_set_status(cg, i);
#endif
		} else {
		    joinsets(cg, i, cg, setno);
		    killset(cg, i);
#ifndef NONE_GUI
		    update_set_status(cg, i);
#endif
		}
	    }
	}
    }
    updatesetminmax(cg, setno);
#ifndef NONE_GUI
    update_set_status(cg, setno);
    drawgraph();
#endif
}

/*
 * kill a set
 */
void do_kill(int gno, int setno, int soft)
{
    int i;

    if (setno == g[gno].maxplot || setno == -1) {
	for (i = 0; i < g[gno].maxplot; i++) {
	    if (isactive_set(gno, i)) {
		if (soft) {
		    softkillset(gno, i);
		} else {
		    killset(gno, i);
		}
#ifndef NONE_GUI
		set_lists_dirty(TRUE);
		update_set_status(gno, i);
#endif
	    }
	}
    } else {
	if (!isactive_set(gno, setno)) {
	    sprintf(buf, "Set %d already dead", setno);
	    errmsg(buf);
	    return;
	} else {
	    if (soft) {
		softkillset(gno, setno);
	    } else {
		killset(gno, setno);
	    }
#ifndef NONE_GUI
	    set_lists_dirty(TRUE);
	    update_set_status(gno, setno);
#endif
	}
    }
}

/*
 * kill all active sets
 */
void do_flush(void)
{
    int i;

#ifndef NONE_GUI
    if (yesno("Flush all active sets, are you sure? ", NULL, NULL, NULL)) {
	set_wait_cursor();
#endif
	for (i = 0; i < g[cg].maxplot; i++) {
	    if (isactive_set(cg, i)) {
		killset(cg, i);
#ifndef NONE_GUI
		update_set_status(cg, i);
#endif
	    }
#ifndef NONE_GUI
	}
	unset_wait_cursor();
	drawgraph();
#endif
    }
}

/*
 * sort sets, only works on sets of type XY
 */
void do_sort(int setno, int sorton, int stype)
{
    int i;

    if (setno == -1) {
	for (i = 0; i < g[cg].maxplot; i++) {
	    if (isactive_set(cg, i)) {
		sort_set(i, sorton, stype);
	    }
	}
    } else {
	if (!isactive_set(cg, setno)) {
	    sprintf(buf, "Set %d not active", setno);
	    errmsg(buf);
	    return;
	} else {
	    sort_set(setno, sorton, stype);
	}
    }
#ifndef NONE_GUI
    drawgraph();
#endif
}

void sort_set(int setno, int sorton, int stype)
{
    int up;

    up = getsetlength(cg, setno);
    if (up < 2) {
	return;
    }
    sortset(cg, setno, sorton, stype);
}

void set_hotlink(int gno, int setno, int onoroff, char *fname, int src)
{
    g[gno].p[setno].hotlink = onoroff;
    if (onoroff && fname != NULL) {
	strcpy(g[gno].p[setno].hotfile, fname);
	g[gno].p[setno].hotsrc = src;
    }
    set_dirtystate();
}

int is_hotlinked(int gno, int setno)
{
    return (g[gno].p[setno].hotlink && strlen(g[gno].p[setno].hotfile) > 0);
}

char *get_hotlink_file(int gno, int setno)
{
    return g[gno].p[setno].hotfile;
}

int get_hotlink_src(int gno, int setno)
{
    return g[gno].p[setno].hotsrc;
}

void do_update_hotlink(int gno, int setno)
{
    read_set_fromfile(gno, setno, g[gno].p[setno].hotfile, g[gno].p[setno].hotsrc);
}

static int wp = 0;

void set_work_pending(int d)
{
    wp = d;
}

int work_pending(void)
{
    return wp;
}

static int dp = 0;

void set_lists_dirty(int d)
{
    dp = d;
}

int lists_dirty(void)
{
    return dp;
}

/*
 * return a pointer to the array given by v
 */
double *getvptr(int gno, int setno, int v)
{
    switch (v) {
    case DATA_X:
	return g[gno].p[setno].ex[0];
	break;
    case DATA_Y:
	return g[gno].p[setno].ex[1];
	break;
    case DATA_Y1:
	return g[gno].p[setno].ex[2];
	break;
    case DATA_Y2:
	return g[gno].p[setno].ex[3];
	break;
    case DATA_Y3:
	return g[gno].p[setno].ex[4];
	break;
    case DATA_Y4:
	return g[gno].p[setno].ex[5];
	break;
    default:
	errmsg ("Internal error in function getvptr()");
	break;
    }
    return NULL;
}

