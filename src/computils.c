/* $Id: computils.c,v 1.2 1995/06/30 22:32:11 pturner Exp pturner $
 *
 * procedures for performing transformations from the command
 * line interpreter and the GUI.
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symdefs.h"
#include "globals.h"
#include "protos.h"

static void forwarddiff(double *x, double *y, double *resx, double *resy, int n);
static void backwarddiff(double *x, double *y, double *resx, double *resy, int n);
static void centereddiff(double *x, double *y, double *resx, double *resy, int n);
int get_points_inregion(int rno, int invr, int len, double *x, double *y, int *cnt, double **xt, double **yt);

void do_running_command(int type, int setno, int rlen)
{
    switch (type) {
    case RUN_AVG:
	type = 0;
	break;
    case RUN_MED:
	type = 1;
	break;
    case RUN_MIN:
	type = 2;
	break;
    case RUN_MAX:
	type = 3;
	break;
    case RUN_STD:
	type = 4;
	break;
    }
    do_runavg(setno, rlen, type, -1, 0);
}

void do_fourier_command(int ftype, int setno, int ltype)
{
    switch (ftype) {
    case FFT_DFT:
	do_fourier(0, setno, 0, ltype, 0, 0, 0);
	break;
    case FFT_INVDFT:
	do_fourier(0, setno, 0, ltype, 1, 0, 0);
	break;
    case FFT_FFT:
	do_fourier(1, setno, 0, ltype, 0, 0, 0);
	break;
    case FFT_INVFFT:
	do_fourier(1, setno, 0, ltype, 1, 0, 0);
	break;
    }
}

void do_histo_command(int fromset, int toset, int tograph,
		      double minb, double binw, int nbins)
{
    do_histo(fromset, toset, tograph, binw, minb, minb + nbins * binw, 0);
}

/*
 * evaluate a formula
 */
int do_compute(int setno, int loadto, int graphto, char *fstr)
{
    if (graphto < 0) {
	graphto = cg;
    }
    if (strlen(fstr) == 0) {
	errmsg("Define formula first");
	return -1;
    }
    if (isactive_set(cg, setno)) {
	/* both loadto and setno do double duty here */
	if (loadto) {
	    loadto = nextset(graphto);
	    if (loadto != -1) {
		do_copyset(cg, setno, graphto, loadto);
		setno = loadto;
	    } else {
		return -1;
	    }
	} else if (graphto != cg) {
	    loadto = setno;
	    if (isactive_set(graphto, loadto)) {
		killset(graphto, loadto);
	    }
	    do_copyset(cg, setno, graphto, loadto);
	    setno = loadto;
	}
	if (formula(graphto, setno, fstr)) {
	    if (graphto != cg || loadto != setno) {
		killset(graphto, loadto);
	    }
	    return -1;
	}
	if (!isactive_graph(graphto)) {
	    set_graph_active(graphto);
	}
	return loadto;
    }
    return -1;
}

/*
 * load a set
 */
void do_load(int setno, int toval, char *startstr, char *stepstr)
{
    int i, ier = 0, idraw = 0;
    double x, y, a, b, c, d;
    extern double result;
    double start, step;
    char *s1, *s2;

    if (strlen(startstr) == 0) {
	errmsg("Start item undefined");
	return;
    }
    s1 = (char *) malloc((strlen(startstr) + 1) * sizeof(char));
    strcpy(s1, startstr);
    scanner(s1, &x, &y, 1, &a, &b, &c, &d, 1, 0, 0, &ier);
    if (ier) {
	free(s1);
	return;
    }
    start = result;

    if (strlen(stepstr) == 0) {
	errmsg("Step item undefined");
	free(s1);
	return;
    }
    s2 = (char *) malloc((strlen(stepstr) + 1) * sizeof(char));
    strcpy(s2, stepstr);
    scanner(s2, &x, &y, 1, &a, &b, &c, &d, 1, 0, 0, &ier);
    if (ier) {
	free(s1);
	free(s2);
	return;
    }
    step = result;

    if (setno == SET_SELECT_ALL) {
	for (i = 0; i < g[cg].maxplot; i++) {
	    if (isactive_set(cg, i)) {
		loadset(cg, i, toval, start, step);
		idraw = 1;
	    }
	}
    } else if (isactive_set(cg, setno)) {
	loadset(cg, setno, toval, start, step);
	idraw = 1;
    }
    if (idraw) {
#ifdef NONE_GUI
	;
#else
	drawgraph();
#endif
    } else {
	errmsg("Set(s) not active");
    }
    free(s1);
    free(s2);
}

/*
 * evaluate a formula loading the next set
 */
void do_compute2(char *fstrx, char *fstry, char *startstr, char *stopstr, int npts, int toval)
{
    int setno, ier;
    double start, stop, step, x, y, a, b, c, d;
    extern double result;
    char comment[256];

    if (npts < 2) {
	errmsg("Number of points < 2");
	return;
    }
    /*
     * if npts is > maxarr, then increase length of scratch arrays
     */
    if (npts > maxarr) {
	if (init_scratch_arrays(npts)) {
	    return;
	}
    }
    setno = nextset(cg);
    if (setno < 0) {
	return;
    }
    activateset(cg, setno);
    setlength(cg, setno, npts);
    if (strlen(fstrx) == 0) {
	errmsg("Undefined expression for X");
	return;
    }
    if (strlen(fstry) == 0) {
	errmsg("Undefined expression for Y");
	return;
    }
    if (strlen(startstr) == 0) {
	errmsg("Start item undefined");
	return;
    }
    scanner(startstr, &x, &y, 1, &a, &b, &c, &d, 1, 0, 0, &ier);
    if (ier)
	return;
    start = result;

    if (strlen(stopstr) == 0) {
	errmsg("Stop item undefined");
	return;
    }
    scanner(stopstr, &x, &y, 1, &a, &b, &c, &d, 1, 0, 0, &ier);
    if (ier) {
	return;
    }
    stop = result;

    step = (stop - start) / (npts - 1);
    loadset(cg, setno, toval, start, step);
    strcpy(buf, "X=");
    strcat(buf, fstrx);
    strcat( strcpy( comment, buf ), ", " );
    formula(cg, setno, buf);
    strcpy(buf, "Y=");
    strcat(buf, fstry);
    formula(cg, setno, buf);
    strncat( comment, buf, 256-strlen(comment) );
    setcomment(cg, setno, comment );
#ifndef NONE_GUI
    drawgraph();
#endif
}

/*
 * forward, backward and centered differences
 */
static void forwarddiff(double *x, double *y, double *resx, double *resy, int n)
{
    int i, eflag = 0;
    double h;

    for (i = 1; i < n; i++) {
	resx[i - 1] = x[i - 1];
	h = x[i - 1] - x[i];
	if (h == 0.0) {
	    resy[i - 1] = - MAXNUM;
	    eflag = 1;
	} else {
	    resy[i - 1] = (y[i - 1] - y[i]) / h;
	}
    }
    if (eflag) {
	errmsg("Warning: infinite slope, check set status before proceeding");
    }
}

static void backwarddiff(double *x, double *y, double *resx, double *resy, int n)
{
    int i, eflag = 0;
    double h;

    for (i = 0; i < n - 1; i++) {
	resx[i] = x[i];
	h = x[i + 1] - x[i];
	if (h == 0.0) {
	    resy[i] = - MAXNUM;
	    eflag = 1;
	} else {
	    resy[i] = (y[i + 1] - y[i]) / h;
	}
    }
    if (eflag) {
	errmsg("Warning: infinite slope, check set status before proceeding");
    }
}

static void centereddiff(double *x, double *y, double *resx, double *resy, int n)
{
    int i, eflag = 0;
    double h1, h2;

    for (i = 1; i < n - 1; i++) {
	resx[i - 1] = x[i];
	h1 = x[i] - x[i - 1];
	h2 = x[i + 1] - x[i];
	if (h1 + h2 == 0.0) {
	    resy[i - 1] = - MAXNUM;
	    eflag = 1;
	} else {
	    resy[i - 1] = (y[i + 1] - y[i - 1]) / (h1 + h2);
	}
    }
    if (eflag) {
	errmsg("Warning: infinite slope, check set status before proceeding");
    }
}

static void seasonaldiff(double *x, double *y,
			 double *resx, double *resy, int n, int period)
{
    int i;

    for (i = 0; i < n - period; i++) {
	resx[i] = x[i];
	resy[i] = y[i] - y[i + period];
    }
}

/*
 * trapezoidal rule
 */
double trapint(double *x, double *y, double *resx, double *resy, int n)
{
    int i;
    double sum = 0.0;
    double h;

    for (i = 1; i < n; i++) {
	h = (x[i] - x[i - 1]);
	if (resx != NULL) {
	    resx[i - 1] = (x[i - 1] + x[i]) * 0.5;
	}
	sum = sum + h * (y[i - 1] + y[i]) * 0.5;
	if (resy != NULL) {
	    resy[i - 1] = sum;
	}
    }
    return sum;
}

/*
 * apply a digital filter
 */
void do_digfilter(int set1, int set2)
{
    int digfiltset;

    if (!(isactive_set(cg, set1) && isactive_set(cg, set2))) {
	errmsg("Set not active");
	return;
    }
    if ((getsetlength(cg, set1) < 3) || (getsetlength(cg, set2) < 3)) {
	errmsg("Set length < 3");
	return;
    }
    digfiltset = nextset(cg);
    if (digfiltset != (-1)) {
	activateset(cg, digfiltset);
	setlength(cg, digfiltset, getsetlength(cg, set1) - getsetlength(cg, set2) + 1);
	sprintf(buf, "Digital filter from set %d applied to set %d", set2, set1);
	filterser(getsetlength(cg, set1),
		  getx(cg, set1),
		  gety(cg, set1),
		  getx(cg, digfiltset),
		  gety(cg, digfiltset),
		  gety(cg, set2),
		  getsetlength(cg, set2));
	setcomment(cg, digfiltset, buf);
	log_results(buf);
	updatesetminmax(cg, digfiltset);
#ifndef NONE_GUI
	update_set_status(cg, digfiltset);
	drawgraph();
#endif
    }
}

/*
 * linear convolution
 */
void do_linearc(int set1, int set2)
{
    int linearcset, i, itmp;
    double *xtmp;

    if (!(isactive_set(cg, set1) && isactive_set(cg, set2))) {
	errmsg("Set not active");
	return;
    }
    if ((getsetlength(cg, set1) < 3) || (getsetlength(cg, set2) < 3)) {
	errmsg("Set length < 3");
	return;
    }
    linearcset = nextset(cg);
    if (linearcset != (-1)) {
	activateset(cg, linearcset);
	setlength(cg, linearcset, (itmp = getsetlength(cg, set1) + getsetlength(cg, set2) - 1));
	linearconv(gety(cg, set2), gety(cg, set1), gety(cg, linearcset), getsetlength(cg, set2), getsetlength(cg, set1));
	xtmp = getx(cg, linearcset);
	for (i = 0; i < itmp; i++) {
	    xtmp[i] = i;
	}
	sprintf(buf, "Linear convolution of set %d with set %d", set1, set2);
	setcomment(cg, linearcset, buf);
	log_results(buf);
	updatesetminmax(cg, linearcset);
#ifndef NONE_GUI
	update_set_status(cg, linearcset);
	drawgraph();
#endif
    }
}

/*
 * cross correlation
 */
void do_xcor(int set1, int set2, int maxlag)
{
    int xcorset, i, ierr;
    double *xtmp;

    if (!(isactive_set(cg, set1) && isactive_set(cg, set2))) {
	errmsg("Set not active");
	return;
    }
    if (maxlag < 0 || maxlag + 2 > getsetlength(cg, set1)) {
	errmsg("Lag incorrectly specified");
	return;
    }
    if ((getsetlength(cg, set1) < 3) || (getsetlength(cg, set2) < 3)) {
	errmsg("Set length < 3");
	return;
    }
    xcorset = nextset(cg);
    if (xcorset != (-1)) {
	activateset(cg, xcorset);
	setlength(cg, xcorset, maxlag + 1);
	if (set1 != set2) {
	    sprintf(buf, "X-correlation of set %d and %d at maximum lag %d",
                    set1, set2, maxlag);
	} else {
	    sprintf(buf, "Autocorrelation of set %d at maximum lag %d",
                    set1, maxlag);
	}
	ierr = crosscorr(gety(cg, set1), gety(cg, set2), getsetlength(cg, set1),
                         maxlag, getx(cg, xcorset), gety(cg, xcorset));
	xtmp = getx(cg, xcorset);
	for (i = 0; i <= maxlag; i++) {
	    xtmp[i] = i;
	}
	setcomment(cg, xcorset, buf);
	log_results(buf);
	updatesetminmax(cg, xcorset);
#ifndef NONE_GUI
	update_set_status(cg, xcorset);
	drawgraph();
#endif
    }
}

/*
 * splines
 */
void do_spline(int set, double start, double stop, int n, int type)
{
    int i, splineset, len;
    double delx, *x, *y, *b, *c, *d, *xtmp, *ytmp;
    double seval(int n, double u, double *x, double *y, double *b, double *c, double *d);

    if (!isactive_set(cg, set)) {
	errmsg("Set not active");
	return;
    }
    if ((len = getsetlength(cg, set)) < 3) {
	errmsg("Improper set length");
	return;
    }
    if (n <= 1) {
	errmsg("Number of steps must be > 1");
	return;
    }
    delx = (stop - start) / (n - 1);
    splineset = nextset(cg);
    if (splineset != -1) {
	activateset(cg, splineset);
	setlength(cg, splineset, n);
	x = getx(cg, set);
	y = gety(cg, set);
	b = (double *) calloc(len, sizeof(double));
	c = (double *) calloc(len, sizeof(double));
	d = (double *) calloc(len, sizeof(double));
	if (b == NULL || c == NULL || d == NULL) {
	    errmsg("Not enough memory for splines");
	    cxfree(b);
	    cxfree(c);
	    cxfree(d);
	    killset(cg, splineset);
	    return;
	}
	if (type == SPLINE_AKIMA) {
	    aspline(len, x, y, b, c, d);
	} else {
	    spline (len, x, y, b, c, d);
	}
	xtmp = getx(cg, splineset);
	ytmp = gety(cg, splineset);

	for (i = 0; i < n; i++) {
	    xtmp[i] = start + i * delx;
	    if (type == SPLINE_AKIMA) {
	    	ytmp[i] = seval(len, xtmp[i], x, y, b, c, d);
	        sprintf(buf, "Akima spline fit from set %d", set);
	    } else {
	    	ytmp[i] = seval (len, xtmp[i], x, y, b, c, d);
	        sprintf(buf, "Cubic spline fit from set %d", set);
	    }
	}
	setcomment(cg, splineset, buf);
	log_results(buf);
	updatesetminmax(cg, splineset);
#ifndef NONE_GUI
	update_set_status(cg, splineset);
#endif
	cxfree(b);
	cxfree(c);
	cxfree(d);
#ifndef NONE_GUI
	drawgraph();
#endif
    }
}


/*
 * numerical integration
 */
double do_int(int setno, int itype)
{
    int intset;
    double sum = 0;

    if (!isactive_set(cg, setno)) {
	errmsg("Set not active");
	return 0.0;
    }
    if (getsetlength(cg, setno) < 3) {
	errmsg("Set length < 3");
	return 0.0;
    }
    if (itype == 0) {
	intset = nextset(cg);
	if (intset != (-1)) {
	    activateset(cg, intset);
	    setlength(cg, intset, getsetlength(cg, setno) - 1);
	    sprintf(buf, "Cumulative sum of set %d", setno);
	    sum = trapint(getx(cg, setno), gety(cg, setno), getx(cg, intset), gety(cg, intset), getsetlength(cg, setno));
	    setcomment(cg, intset, buf);
	    log_results(buf);
	    updatesetminmax(cg, intset);
#ifndef NONE_GUI
	    update_set_status(cg, intset);
	    drawgraph();
#endif
	}
    } else {
	sum = trapint(getx(cg, setno), gety(cg, setno), NULL, NULL, getsetlength(cg, setno));
    }
    return sum;
}

/*
 * difference a set
 * itype means
 *  0 - forward
 *  1 - backward
 *  2 - centered difference
 */
void do_differ(int setno, int itype)
{
    int diffset;

    if (!isactive_set(cg, setno)) {
	errmsg("Set not active");
	return;
    }
    if (getsetlength(cg, setno) < 3) {
	errmsg("Set length < 3");
	return;
    }
    diffset = nextset(cg);
    if (diffset != (-1)) {
	activateset(cg, diffset);
	switch (itype) {
	case 0:
	    sprintf(buf, "Forward difference of set %d", setno);
	    setlength(cg, diffset, getsetlength(cg, setno) - 1);
	    forwarddiff(getx(cg, setno), gety(cg, setno), getx(cg, diffset), gety(cg, diffset), getsetlength(cg, setno));
	    break;
	case 1:
	    sprintf(buf, "Backward difference of set %d", setno);
	    setlength(cg, diffset, getsetlength(cg, setno) - 1);
	    backwarddiff(getx(cg, setno), gety(cg, setno), getx(cg, diffset), gety(cg, diffset), getsetlength(cg, setno));
	    break;
	case 2:
	    sprintf(buf, "Centered difference of set %d", setno);
	    setlength(cg, diffset, getsetlength(cg, setno) - 2);
	    centereddiff(getx(cg, setno), gety(cg, setno), getx(cg, diffset), gety(cg, diffset), getsetlength(cg, setno));
	    break;
	}
	setcomment(cg, diffset, buf);
	log_results(buf);
	updatesetminmax(cg, diffset);
#ifndef NONE_GUI
	update_set_status(cg, diffset);
	drawgraph();
#endif
    }
}

/*
 * seasonally difference a set
 */
void do_seasonal_diff(int setno, int period)
{
    int diffset;

    if (!isactive_set(cg, setno)) {
	errmsg("Set not active");
	return;
    }
    if (getsetlength(cg, setno) < 2) {
	errmsg("Set length < 2");
	return;
    }
    diffset = nextset(cg);
    if (diffset != (-1)) {
	activateset(cg, diffset);
	setlength(cg, diffset, getsetlength(cg, setno) - period);
	seasonaldiff(getx(cg, setno), gety(cg, setno),
		     getx(cg, diffset), gety(cg, diffset),
		     getsetlength(cg, setno), period);
	sprintf(buf, "Seasonal difference of set %d, period %d", setno, period);
	setcomment(cg, diffset, buf);
	log_results(buf);
	updatesetminmax(cg, diffset);
#ifndef NONE_GUI
	update_set_status(cg, diffset);
	drawgraph();
#endif
    }
}

/*
 * regression with restrictions to region rno if rno >= 0
 */
void do_regress(int setno, int ideg, int iresid, int rno, int invr)
{
    int len, fitset, i, sdeg = ideg, ifail;
    int cnt = 0;
    double *x, *y, *xt = NULL, *yt = NULL, *xr, *yr;
    char buf[256];

    if (!isactive_set(cg, setno)) {
	errmsg("Set not active");
	return;
    }
    len = getsetlength(cg, setno);
    x = getx(cg, setno);
    y = gety(cg, setno);
    if (rno == -1) {
	xt = x;
	yt = y;
    } else if (isactive_region(rno)) {
	if (!get_points_inregion(rno, invr, len, x, y, &cnt, &xt, &yt)) {
	    if (cnt == 0) {
		errmsg("No points found in region, operation cancelled");
	    } else {
		errmsg("Memory allocation failed for points in region");
	    }
	    return;
	}
	len = cnt;
    } else {
	errmsg("Selected region is not active");
	return;
    }
    /*
     * first part for polynomials, second part for linear fits to transformed
     * data
     */
    if ((len < ideg && ideg <= 10) || (len < 2 && ideg > 10)) {
	errmsg("Too few points in set, operation cancelled");
	return;
    }
    fitset = nextset(cg);
    if (fitset != -1) {
	activateset(cg, fitset);
	setlength(cg, fitset, len);
	xr = getx(cg, fitset);
	yr = gety(cg, fitset);
	for (i = 0; i < len; i++) {
	    xr[i] = xt[i];
	}
	if (ideg == 12) {	/* ln(y) = ln(A) + b * ln(x) */
	    ideg = 1;
	    for (i = 0; i < len; i++) {
		if (xt[i] <= 0.0) {
		    errmsg("One of X[i] <= 0.0");
		    return;
		}
		if (yt[i] <= 0.0) {
		    errmsg("One of Y[i] <= 0.0");
		    return;
		}
	    }
	    for (i = 0; i < len; i++) {
		xt[i] = log(xt[i]);
		yt[i] = log(yt[i]);
	    }
	} else if (ideg == 13) {
	    ideg = 1;
	    for (i = 0; i < len; i++) {
		if (yt[i] <= 0.0) {
		    errmsg("One of Y[i] <= 0.0");
		    return;
		}
	    }
	    for (i = 0; i < len; i++) {
		yt[i] = log(yt[i]);
	    }
	} else if (ideg == 14) {
	    ideg = 1;
	    for (i = 0; i < len; i++) {
		if (xt[i] <= 0.0) {
		    errmsg("One of X[i] <= 0.0");
		    return;
		}
	    }
	    for (i = 0; i < len; i++) {
		xt[i] = log(xt[i]);
	    }
	} else if (ideg == 15) {
	    ideg = 1;
	    for (i = 0; i < len; i++) {
		if (yt[i] == 0.0) {
		    errmsg("One of Y[i] = 0.0");
		    return;
		}
	    }
	    for (i = 0; i < len; i++) {
		yt[i] = 1.0 / yt[i];
	    }
	}

	ifail = fitcurve(xt, yt, len, ideg, yr);

	if (ifail) {
	    killset(cg, fitset);
	    goto bustout;
	}

	sprintf(buf, "\nRegression of set %d results to set %d\n", setno, fitset);
	stufftext(buf, STUFF_STOP);

	if (sdeg == 12) {	/* ln(y) = ln(A) + b * ln(x) */
	    sprintf(buf, "\nN.B. Statistics refer to the transformed model\n");
	    for (i = 0; i < len; i++) {
		xt[i] = xr[i] = exp(xt[i]);
		yt[i] = exp(yt[i]);
		yr[i] = exp(yr[i]);
	    }
	} else if (sdeg == 13) {
	    for (i = 0; i < len; i++) {
		yt[i] = exp(yt[i]);
		yr[i] = exp(yr[i]);
	    }
	} else if (sdeg == 14) {
	    for (i = 0; i < len; i++) {
		xt[i] = xr[i] = exp(xt[i]);
	    }
	} else if (sdeg == 15) {
	    for (i = 0; i < len; i++) {
		yt[i] = 1.0 / yt[i];
		yr[i] = 1.0 / yr[i];
	    }
	}
	switch (iresid) {
	case 1:
	    for (i = 0; i < len; i++) {
		yr[i] = yt[i] - yr[i];
	    }
	    break;
	case 2:
	    break;
	}
	sprintf(buf, "%d deg fit of set %d", ideg, setno);
	setcomment(cg, fitset, buf);
	log_results(buf);
	updatesetminmax(cg, fitset);
#ifndef NONE_GUI
	update_set_status(cg, fitset);
#endif
    }
    bustout:;
    if (rno >= 0 && cnt != 0) {	/* had a region and allocated memory there */
	free(xt);
	free(yt);
    }
}

/*
 * running averages, medians, min, max, std. deviation
 */
void do_runavg(int setno, int runlen, int runtype, int rno, int invr)
{
    int runset;
    int len, cnt = 0;
    double *x, *y, *xt = NULL, *yt = NULL, *xr, *yr;

    if (!isactive_set(cg, setno)) {
	errmsg("Set not active");
	return;
    }
    if (runlen < 2) {
	errmsg("Length of running average < 2");
	return;
    }
    len = getsetlength(cg, setno);
    x = getx(cg, setno);
    y = gety(cg, setno);
    if (rno == -1) {
	xt = x;
	yt = y;
    } else if (isactive_region(rno)) {
	if (!get_points_inregion(rno, invr, len, x, y, &cnt, &xt, &yt)) {
	    if (cnt == 0) {
		errmsg("No points found in region, operation cancelled");
	    } else {
		errmsg("Memory allocation failed for points in region");
	    }
	    return;
	}
	len = cnt;
    } else {
	errmsg("Selected region is not active");
	return;
    }
    if (runlen >= len) {
	errmsg("Length of running average > set length");
	goto bustout;
    }
    runset = nextset(cg);
    if (runset != (-1)) {
	activateset(cg, runset);
	setlength(cg, runset, len - runlen + 1);
	xr = getx(cg, runset);
	yr = gety(cg, runset);
	switch (runtype) {
	case 0:
	    runavg(xt, yt, xr, yr, len, runlen);
	    sprintf(buf, "%d-pt. avg. on set %d ", runlen, setno);
	    break;
	case 1:
	    runmedian(xt, yt, xr, yr, len, runlen);
	    sprintf(buf, "%d-pt. median on set %d ", runlen, setno);
	    break;
	case 2:
	    runminmax(xt, yt, xr, yr, len, runlen, 0);
	    sprintf(buf, "%d-pt. min on set %d ", runlen, setno);
	    break;
	case 3:
	    runminmax(xt, yt, xr, yr, len, runlen, 1);
	    sprintf(buf, "%d-pt. max on set %d ", runlen, setno);
	    break;
	case 4:
	    runstddev(xt, yt, xr, yr, len, runlen);
	    sprintf(buf, "%d-pt. std dev., set %d ", runlen, setno);
	    break;
	}
	setcomment(cg, runset, buf);
	log_results(buf);
	updatesetminmax(cg, runset);
#ifndef NONE_GUI
	update_set_status(cg, runset);
#endif
    }
  bustout:;
    if (rno >= 0 && cnt != 0) {	/* had a region and allocated memory there */
	free(xt);
	free(yt);
    }
}

/*
 * DFT by FFT or definition
 */
void do_fourier(int fftflag, int setno, int load, int loadx, int invflag, int type, int wind)
{
    int i, ilen;
    double *x, *y, *xx, *yy, delt, T;
    int i2 = 0, specset;

    if (!isactive_set(cg, setno)) {
	errmsg("Set not active");
	return;
    }
    ilen = getsetlength(cg, setno);
    if (ilen < 2) {
	errmsg("Set length < 2");
	return;
    }
    if (fftflag) {
	if ((i2 = ilog2(ilen)) <= 0) {
	    errmsg("Set length not a power of 2");
	    return;
	}
    }
    specset = nextset(cg);
    if (specset != -1) {
	activateset(cg, specset);
	setlength(cg, specset, ilen);
	xx = getx(cg, specset);
	yy = gety(cg, specset);
	x = getx(cg, setno);
	y = gety(cg, setno);
	copyx(cg, setno, specset);
	copyy(cg, setno, specset);
	if (wind != 0) {	/* apply data window if needed */
	    apply_window(xx, yy, ilen, type, wind);
	}
	if (type == 0) {	/* real data */
	    for (i = 0; i < ilen; i++) {
		xx[i] = yy[i];
		yy[i] = 0.0;
	    }
	}
	if (fftflag) {
	    fft(xx, yy, ilen, i2, !invflag);
	} else {
	    dft(xx, yy, ilen, invflag);
	}
	switch (load) {
	case 0:
	    delt = (x[ilen-1] - x[0])/(ilen -1.0);
	    T = (x[ilen - 1] - x[0]);
	    setlength(cg, specset, ilen / 2);
	    xx = getx(cg, specset);
	    yy = gety(cg, specset);
	    for (i = 0; i < ilen / 2; i++) {
		yy[i] = hypot(xx[i], yy[i]);
		switch (loadx) {
		case 0:
		    xx[i] = i;
		    break;
		case 1:
		    /* xx[i] = 2.0 * M_PI * i / ilen; */
		    xx[i] = i / T;
		    break;
		case 2:
		    if (i == 0) {
			xx[i] = T + delt;	/* the mean */
		    } else {
			/* xx[i] = (double) ilen / (double) i; */
			xx[i] = T / i;
		    }
		    break;
		}
	    }
	    break;
	case 1:
	    delt = (x[ilen-1] - x[0])/(ilen -1.0);
	    T = (x[ilen - 1] - x[0]);
	    setlength(cg, specset, ilen / 2);
	    xx = getx(cg, specset);
	    yy = gety(cg, specset);
	    for (i = 0; i < ilen / 2; i++) {
		yy[i] = -atan2(yy[i], xx[i]);
		switch (loadx) {
		case 0:
		    xx[i] = i;
		    break;
		case 1:
		    /* xx[i] = 2.0 * M_PI * i / ilen; */
		    xx[i] = i / T;
		    break;
		case 2:
		    if (i == 0) {
			xx[i] = T + delt;
		    } else {
			/* xx[i] = (double) ilen / (double) i; */
			xx[i] = T / i;
		    }
		    break;
		}
	    }
	    break;
	}
	if (fftflag) {
	    sprintf(buf, "FFT of set %d", setno);
	} else {
	    sprintf(buf, "DFT of set %d", setno);
	}
	setcomment(cg, specset, buf);
	log_results(buf);
	updatesetminmax(cg, specset);
#ifndef NONE_GUI
	update_set_status(cg, specset);
#endif
    }
}

/*
 * Apply a window to a set, result goes to a new set.
 */
void do_window(int setno, int type, int wind)
{
    int ilen;
    double *xx, *yy;
    int specset;

    if (!isactive_set(cg, setno)) {
	errmsg("Set not active");
	return;
    }
    ilen = getsetlength(cg, setno);
    if (ilen < 2) {
	errmsg("Set length < 2");
	return;
    }
    specset = nextset(cg);
    if (specset != -1) {
	char *wtype[6];
	wtype[0] = "Triangular";
	wtype[1] = "Hanning";
	wtype[2] = "Welch";
	wtype[3] = "Hamming";
	wtype[4] = "Blackman";
	wtype[5] = "Parzen";

	activateset(cg, specset);
	setlength(cg, specset, ilen);
	xx = getx(cg, specset);
	yy = gety(cg, specset);
	copyx(cg, setno, specset);
	copyy(cg, setno, specset);
	if (wind != 0) {
	    apply_window(xx, yy, ilen, type, wind);
	    sprintf(buf, "%s windowed set %d", wtype[wind - 1], setno);
	} else {		/* shouldn't happen */
	}
	setcomment(cg, specset, buf);
	log_results(buf);
	updatesetminmax(cg, specset);
#ifndef NONE_GUI
	update_set_status(cg, specset);
#endif
    }
}

void apply_window(double *xx, double *yy, int ilen, int type, int wind)
{
    int i;

    for (i = 0; i < ilen; i++) {
	switch (wind) {
	case 1:		/* triangular */
	    if (type != 0) {
		xx[i] *= 1.0 - fabs((i - 0.5 * (ilen - 1.0)) / (0.5 * (ilen - 1.0)));
	    }
	    yy[i] *= 1.0 - fabs((i - 0.5 * (ilen - 1.0)) / (0.5 * (ilen - 1.0)));

	    break;
	case 2:		/* Hanning */
	    if (type != 0) {
		xx[i] = xx[i] * (0.5 - 0.5 * cos(2.0 * M_PI * i / (ilen - 1.0)));
	    }
	    yy[i] = yy[i] * (0.5 - 0.5 * cos(2.0 * M_PI * i / (ilen - 1.0)));
	    break;
	case 3:		/* Welch (from Numerical Recipes) */
	    if (type != 0) {
		xx[i] *= 1.0 - pow((i - 0.5 * (ilen - 1.0)) / (0.5 * (ilen + 1.0)), 2.0);
	    }
	    yy[i] *= 1.0 - pow((i - 0.5 * (ilen - 1.0)) / (0.5 * (ilen + 1.0)), 2.0);
	    break;
	case 4:		/* Hamming */
	    if (type != 0) {
		xx[i] = xx[i] * (0.54 - 0.46 * cos(2.0 * M_PI * i / (ilen - 1.0)));
	    }
	    yy[i] = yy[i] * (0.54 - 0.46 * cos(2.0 * M_PI * i / (ilen - 1.0)));
	    break;
	case 5:		/* Blackman */
	    if (type != 0) {
		xx[i] = xx[i] * (0.42 - 0.5 * cos(2.0 * M_PI * i / (ilen - 1.0)) + 0.08 * cos(4.0 * M_PI * i / (ilen - 1.0)));
	    }
	    yy[i] = yy[i] * (0.42 - 0.5 * cos(2.0 * M_PI * i / (ilen - 1.0)) + 0.08 * cos(4.0 * M_PI * i / (ilen - 1.0)));
	    break;
	case 6:		/* Parzen (from Numerical Recipes) */
	    if (type != 0) {
		xx[i] *= 1.0 - fabs((i - 0.5 * (ilen - 1)) / (0.5 * (ilen + 1)));
	    }
	    yy[i] *= 1.0 - fabs((i - 0.5 * (ilen - 1)) / (0.5 * (ilen + 1)));
	    break;
	}
    }
}


/*
 * histograms
 */
void do_histo(int fromset, int toset, int tograph,
	      double binw, double xmin, double xmax, int hist_type)
{
    if (!isactive_set(cg, fromset)) {
	errmsg("Set not active");
	return;
    }
    if (getsetlength(cg, fromset) <= 0) {
	errmsg("Set length = 0");
	return;
    }
    if (binw <= 0.0) {
	errmsg("Bin width <= 0");
	return;
    }
    if (tograph == -1) {
	tograph = cg;
    }
    if (g[tograph].active == FALSE) {
	set_graph_active(tograph);
    }
    if (toset == SET_SELECT_NEXT) {
	toset = nextset(tograph);
	if (toset == -1) {
	    return;
	}
    } else if (isactive_set(tograph, toset)) {
	errmsg("Target set not empty");
	return;
    }
    histogram(fromset, toset, tograph, binw, xmin, xmax, hist_type);
#ifndef NONE_GUI
    drawgraph();
#endif
}

void histogram(int fromset, int toset, int tograph, 
	double bins, double xmin, double xmax, int hist_type)
{
    int n, i, j, nbins;
    double sum = 0.0, spread, xi, *x, *y;
    int *ind;

    n = getsetlength(cg, fromset);
    spread = xmax - xmin;
    nbins = (int) (spread / bins);
    if (nbins <= 0) {
	errmsg("No bins, no work to do");
	killset(tograph, toset);
	return;
    }
    ind = (int *) calloc(nbins, sizeof(int));
    if (ind == NULL) {
	errmsg("Not enough memory for histogram");
	killset(tograph, toset);
	return;
    }
    activateset(tograph, toset);
    setlength(tograph, toset, nbins);
    j = 0;
    y = gety(cg, fromset);
    for (i = 0; i < n; i++) {
	xi = y[i];
	if (xi >= xmin && xi <= xmax) {
	    j = (int) ((xi - xmin) / bins);
	    if (j < 0) {
		j = 0;
	    } else {
		if (j >= nbins) {
		    j = nbins - 1;
		}
	    }
	    ind[j] = ind[j] + 1;
	}
    }
    x = getx(tograph, toset);
    y = gety(tograph, toset);
    for (i = 0; i < nbins; i++) {
	x[i] = i * bins + xmin;
	sum = sum * hist_type + ind[i];	/* hist_type = 0 => regular histo */
	y[i] = sum;
    }
/*
 *     set_prop(tograph, SET, SETNUM, toset, SYMBOL, TYPE, SYM_HISTOX, 0);
 */
    g[tograph].p[toset].sym = SYM_HISTOX;
/*
 *     set_prop(tograph, SET, SETNUM, toset, LINESTYLE, 0, 0);
 */
    g[tograph].p[toset].lines = 0;
    g[tograph].p[toset].symlines = 1;
#ifndef NONE_GUI
    updatesymbols(tograph, toset);
#endif
    updatesetminmax(tograph, toset);
    sprintf(buf, "Histogram from set # %d", fromset);
    setcomment(tograph, toset, buf);
    log_results(buf);
#ifndef NONE_GUI
    update_set_status(tograph, toset);
#endif
    cxfree(ind);
#ifndef NONE_GUI
    drawgraph();
#endif
}

/*
 * sample a set, by start/step or logical expression
 */
void do_sample(int setno, int typeno, char *exprstr, int startno, int stepno)
{
    int len, npts = 0, i, resset, ier;
    double *x, *y, tmpx, tmpy;
    double a, b, c, d;
    extern double result;

    if (!isactive_set(cg, setno)) {
	errmsg("Set not active");
	return;
    }
    len = getsetlength(cg, setno);
    resset = nextset(cg);
    if (resset < 0) {
	return;
    }
    if (typeno == 0) {
	if (len <= 2) {
	    errmsg("Set has <= 2 points");
	    return;
	}
	if (startno < 1) {
	    errmsg("Start point < 1 (locations in sets are numbered starting from 1)");
	    return;
	}
	if (stepno < 1) {
	    errmsg("Step < 1");
	    return;
	}
	x = getx(cg, setno);
	y = gety(cg, setno);
	for (i = startno - 1; i < len; i += stepno) {
	    add_point(cg, resset, x[i], y[i], 0.0, 0.0, SET_XY);
	    npts++;
	}
	sprintf(buf, "Sample, %d, %d set #%d", startno, stepno, setno);
    } else {
	if (!strlen(exprstr)) {
	    errmsg("Enter logical expression first");
	    return;
	}
	x = getx(cg, setno);
	y = gety(cg, setno);
	npts = 0;
	sprintf(buf, "Sample from %d, using '%s'", setno, exprstr);
	tmpx = x[0];
	tmpy = y[0];
	for (i = 0; i < len; i++) {
	    x[0] = x[i];
	    y[0] = y[i];
	    scanner(exprstr, &x[i], &y[i], 1, &a, &b, &c, &d, 1, i, setno, &ier);
	    if (ier) {
		killset(cg, resset);
		x[0] = tmpx;
		y[0] = tmpy;
		return;
	    }
	    if ((int) result) {
		add_point(cg, resset, x[i], y[i], 0.0, 0.0, SET_XY);
		npts++;
	    }
	}
	x[0] = tmpx;
	y[0] = tmpy;
    }
    if (npts > 0) {
	updatesetminmax(cg, resset);
	setcomment(cg, resset, buf);
	log_results(buf);
#ifndef NONE_GUI
	update_set_status(cg, resset);
	drawgraph();
#endif
    }
}

#define prune_xconv(res,x,xtype)	\
    switch (deltatypeno) {		\
    case PRUNE_VIEWPORT:		\
	res = xconv(x);			\
	break;				\
    case PRUNE_WORLD:			\
	switch (xtype) {		\
	case PRUNE_LIN:			\
	    res = x;			\
	    break;			\
	case PRUNE_LOG:			\
	    res = log(x);		\
	    break;			\
	}				\
    }

#define prune_yconv(res,y,ytype)	\
    switch (deltatypeno) {		\
    case PRUNE_VIEWPORT:		\
	res = yconv(y);			\
	break;				\
    case PRUNE_WORLD:			\
	switch (ytype) {		\
	case PRUNE_LIN:			\
	    res = y;			\
	    break;			\
	case PRUNE_LOG:			\
	    res = log(y);		\
	    break;			\
	}				\
    }

/*
 * Prune data
 */
void do_prune(int setno, int typeno, int deltatypeno, char *dxstr, char *dystr, int dxtype, int dytype)
{
    int len, npts = 0, d, i, j, k, drop, resset, errpos;
    double *x, *y, *resx, *resy, xtmp, ytmp, ddx = 0.0, ddy = 0.0;
    double xj = 0.0, xjm = 0.0, xjp = 0.0, yj = 0.0, yjm = 0.0, yjp = 0.0;
    double deltax = 0.0, deltay = 0.0;
    extern double result;

    if (!isactive_set(cg, setno)) {
        errmsg("Set not active");
        return;
    }
    len = getsetlength(cg, setno);
    if (len <= 2) {
	errmsg("Set has <= 2 points");
	return;
    }
    x = getx(cg, setno);
    y = gety(cg, setno);
    switch (typeno) {
    case PRUNE_CIRCLE:
    case PRUNE_ELLIPSE:
    case PRUNE_RECTANGLE:
	scanner(dxstr, x, y, len, ax, bx, cx, dx, MAXARR, 0, setno,
                &errpos);
	if (errpos) {
	    return;
	}
	deltax = fabs(result);
	if (deltax == 0)
	    return;
	break;
    }
    switch (typeno) {
    case PRUNE_CIRCLE:
	deltay = deltax;
	break;
    case PRUNE_ELLIPSE:
    case PRUNE_RECTANGLE:
    case PRUNE_INTERPOLATION:
	scanner(dystr, x, y, len, ax, bx, cx, dx, MAXARR, 0, setno,
		&errpos);
	if (errpos) {
	    return;
	}
	deltay = fabs(result);
	if (deltay == 0)
	    return;
	break;
    }
    if (deltatypeno == PRUNE_WORLD) {
	if (dxtype == PRUNE_LOG && deltax < 1.0) {
	    deltax = 1.0 / deltax;
	}
	if (dytype == PRUNE_LOG && deltay < 1.0) {
	    deltay = 1.0 / deltay;
	}
    }
    resset = nextset(cg);
    if (resset < 0) {
        return;
    }
    add_point(cg, resset, x[0], y[0], 0.0, 0.0, SET_XY);
    npts++;
    resx = getx(cg, resset);
    resy = gety(cg, resset);
    switch (typeno) {
    case PRUNE_CIRCLE:
    case PRUNE_ELLIPSE:
	for (i = 1; i < len; i++) {
	    xtmp = x[i];
	    ytmp = y[i];
	    drop = FALSE;
	    for (j = npts - 1; j >= 0 && drop == FALSE; j--) {
		switch (deltatypeno) {
		case PRUNE_VIEWPORT:
		    ddx = (xconv(xtmp) - xconv(resx[j])) / deltax;
		    if (fabs(ddx) < 1.0) {
			ddy = (yconv(ytmp) - yconv(resy[j])) / deltay;
			if (ddx * ddx + ddy * ddy < 1.0) {
			    drop = TRUE;
			}
		    }
		    break;
		case PRUNE_WORLD:
		    switch (dxtype) {
		    case PRUNE_LIN:
			ddx = (xtmp - resx[j]) / deltax;
			break;
		    case PRUNE_LOG:
			ddx = (xtmp / resx[j]);
			if (ddx < 1.0) {
			    ddx = 1.0 / ddx;
			}
			ddx /= deltax;
			break;
		    }
		    if (fabs(ddx) < 1.0) {
			switch (dytype) {
			case PRUNE_LIN:
			    ddy = (ytmp - resy[j]) / deltay;
			    break;
			case PRUNE_LOG:
			    ddy = (ytmp / resy[j]);
			    if (ddy < 1.0) {
				ddy = 1.0 / ddy;
			    }
			    ddy /= deltay;
			    break;
			}
			if (ddx * ddx + ddy * ddy < 1.0) {
			    drop = TRUE;
			}
		    }
		    break;
		}
	    }
	    if (drop == FALSE) {
		add_point(cg, resset, xtmp, ytmp, 0.0, 0.0, SET_XY);
		npts++;
		resx = getx(cg, resset);
		resy = gety(cg, resset);
	    }
	}
	sprintf(buf, "Prune from %d, %s dx = %g dy = %g", setno, 
	    (typeno == 0) ? "Circle" : "Ellipse", deltax, deltay);
	break;
    case PRUNE_RECTANGLE:
	for (i = 1; i < len; i++) {
	    xtmp = x[i];
	    ytmp = y[i];
	    drop = FALSE;
	    for (j = npts - 1; j >= 0 && drop == FALSE; j--) {
		switch (deltatypeno) {
		case PRUNE_VIEWPORT:
		    ddx = fabs(xconv(xtmp) - xconv(resx[j]));
		    if (ddx < deltax) {
			ddy = fabs(yconv(ytmp) - yconv(resy[j]));
			if (ddy < deltay) {
			    drop = TRUE;
			}
		    }
		    break;
		case PRUNE_WORLD:
		    switch (dxtype) {
		    case PRUNE_LIN:
			ddx = fabs(xtmp - resx[j]);
			break;
		    case PRUNE_LOG:
			ddx = (xtmp / resx[j]);
			if (ddx < 1.0) {
			    ddx = 1.0 / ddx;
			}
			break;
		    }
		    if (ddx < deltax) {
			switch (dytype) {
			case PRUNE_LIN:
			    ddy = fabs(ytmp - resy[j]);
			    break;
			case PRUNE_LOG:
			    ddy = (ytmp / resy[j]);
			    if (ddy < 1.0) {
				ddy = 1.0 / ddy;
			    }
			    break;
			}
			if (ddy < deltay) {
			    drop = TRUE;
			}
		    }
		    break;
		}
	    }
	    if (drop == FALSE) {
		add_point(cg, resset, xtmp, ytmp, 0.0, 0.0, SET_XY);
		npts++;
		resx = getx(cg, resset);
		resy = gety(cg, resset);
	    }
	}
	sprintf(buf, "Prune from %d, %s dx = %g dy = %g", setno, 
	    "Rectangle", deltax, deltay);
	break;
    case PRUNE_INTERPOLATION:
	k = 0;
	prune_xconv(xjm, x[0], dxtype);
	prune_yconv(yjm, y[0], dytype);
	while (k < len - 2) {
	    d = 1;
	    i = k + d + 1;
	    drop = TRUE;
	    while (TRUE) {
		prune_xconv(xjp, x[i], dxtype);
		prune_yconv(yjp, y[i], dytype);
		for (j = k + 1; j < i && drop == TRUE; j++) {
		    prune_xconv(xj, x[j], dxtype);
		    prune_yconv(yj, y[j], dytype);
		    if (xjp == xjm) {
			ytmp = 0.5 * (yjp + yjm);
		    } else {
			ytmp = (yjp*(xj-xjm)+yjm*(xjp-xj))/(xjp-xjm);
		    }
		    switch (deltatypeno) {
		    case PRUNE_VIEWPORT:
			ddy = fabs(ytmp - yj);
			break;
		    case PRUNE_WORLD:
			switch (dytype) {
			case PRUNE_LIN:
			    ddy = fabs(ytmp - yj);
			    break;
			case PRUNE_LOG:
			    ddy = exp(fabs(ytmp - yj));
			    break;
			}
		    }
		    if (ddy > deltay) {
			drop = FALSE;
		    }
		}
		if (drop == FALSE || i == len - 1) {
		    break;
		}
		d *= 2;
		i = k + d + 1;
		if (i >= len) {
		    i = len - 1;
		}
	    }
	    if (drop == FALSE) {
		i = k + 1;
		drop = TRUE;
		while (d > 1) {
		    d /= 2;
		    i += d;
		    prune_xconv(xjp, x[i], dxtype);
		    prune_yconv(yjp, y[i], dytype);
		    drop = TRUE;
		    for (j = k + 1; j < i && drop == TRUE; j++) {
			prune_xconv(xj, x[j], dxtype);
			prune_yconv(yj, y[j], dytype);
			ytmp = (yjp*(xj-xjm)+yjm*(xjp-xj))/(xjp-xjm);
			switch (deltatypeno) {
			case PRUNE_VIEWPORT:
			    ddy = fabs(ytmp - yj);
			    break;
			case PRUNE_WORLD:
			    switch (dytype) {
			    case PRUNE_LIN:
				ddy = fabs(ytmp - yj);
				break;
			    case PRUNE_LOG:
				ddy = exp(fabs(ytmp - yj));
				break;
			    }
			}
			if (ddy > deltay) {
			    drop = FALSE;
			}
		    }
		    if (drop == FALSE) {
			i -= d;
		    }
		}
	    }
	    k = i;
	    prune_xconv(xjm, x[k], dxtype);
	    prune_yconv(yjm, y[k], dytype);
	    add_point(cg, resset, x[k], y[k], 0.0, 0.0, SET_XY);
	    npts++;
	    resx = getx(cg, resset);
	    resy = gety(cg, resset);
	}
	if (k == len - 2) {
	    add_point(cg, resset, x[len-1], y[len-1], 0.0, 0.0, SET_XY);
	    npts++;
	}
	sprintf(buf, "Prune from %d, %s dy = %g", setno, 
	    "Interpolation", deltay);
	break;
    }
    updatesetminmax(cg, resset);
    setcomment(cg, resset, buf);
    log_results(buf);
#ifndef NONE_GUI
    update_set_status(cg, resset);
    drawgraph();
#endif
}

int get_points_inregion(int rno, int invr, int len, double *x, double *y, int *cnt, double **xt, double **yt)
{
    int i, clen = 0;
    double *xtmp, *ytmp;
    *cnt = 0;
    if (isactive_region(rno)) {
	for (i = 0; i < len; i++) {
	    if (invr) {
		if (!inregion(rno, x[i], y[i])) {
		    clen++;
		}
	    } else {
		if (inregion(rno, x[i], y[i])) {
		    clen++;
		}
	    }
	}
	if (clen == 0) {
	    return 0;
	}
	xtmp = (double *) calloc(clen, sizeof(double));
	if (xtmp == NULL) {
	    return 0;
	}
	ytmp = (double *) calloc(clen, sizeof(double));
	if (ytmp == NULL) {
	    free(xtmp);
	    return 0;
	}
	clen = 0;
	for (i = 0; i < len; i++) {
	    if (invr) {
		if (!inregion(rno, x[i], y[i])) {
		    xtmp[clen] = x[i];
		    ytmp[clen] = y[i];
		    clen++;
		}
	    } else {
		if (inregion(rno, x[i], y[i])) {
		    xtmp[clen] = x[i];
		    ytmp[clen] = y[i];
		    clen++;
		}
	    }
	}
    } else {
	return 0;
    }
    *cnt = clen;
    *xt = xtmp;
    *yt = ytmp;
    return 1;
}

void do_interp( int yset, int xset, int method )
/* interpolate a set at abscissas from another set
 *  yset - set to interpolate
 *  xset - set supplying abscissas
 *  method - spline or linear interpolation
 *
 * Added by Ed Vigmond 10/2/97
 */
{
	int i, j, iset, xsetlength, ysetlength, isetlength;
	double *x1, *x2, *newx, *y, *b, *c, *d, newy;
	
    if (!isactive_set(cg, yset)) {
		errmsg("Interpolating set not active");
		return;
    }
    if (!isactive_set(cg, xset)) {
		errmsg("Sampling set not active");
		return;
    }
    /* make sure set was really killed */
    if( getsetlength( cg, iset= nextset(cg) ) )                 
                set_default_plotarr(&g[cg].p[iset]); 
    activateset( cg, iset );
    /* ensure bounds of new set */
    x1=getx( cg, yset );
    y=gety( cg, yset );
    x2=getx( cg, xset );
    newx = (double *)calloc( getsetlength( cg, xset ), sizeof( double ) );
    xsetlength = getsetlength( cg, xset );
    ysetlength = getsetlength( cg, yset );
    for( i=0, j=0; i<xsetlength; i++ )			/* get intersection of
    sets */
    	if( x2[i] >= g[cg].p[yset].xmin && x2[i] <= g[cg].p[yset].xmax )
    		newx[j++] = x2[i];
    isetlength = j;

    if( method ) {					/* spline */
		b = (double *) calloc(ysetlength, sizeof(double));
		c = (double *) calloc(ysetlength, sizeof(double));
		d = (double *) calloc(ysetlength, sizeof(double));
		if (b == NULL || c == NULL || d == NULL) {
	    	errmsg("Not enough memory for splines");
	    	cxfree(b);
	    	cxfree(c);
	    	cxfree(d);
	    	killset(cg, iset);
	    	return;
		}
		if (method == SPLINE_AKIMA){      /* Akima spline */
		    aspline(ysetlength, x1, y, b, c, d);
		} else {                          /* Plain cubic spline */
		    spline(ysetlength, x1, y, b, c, d);
		}
		for (i = 0; i < j; i++) {
		    add_point(cg, iset, newx[i], seval(ysetlength, newx[i], x1, y,
		               b, c, d), 0.0, 0.0, SET_XY);
		}
		
    }else {				          /* linear interpolation */
    	for( j=0; j<isetlength; j++ ) {
    		i=0;
    		while( (x1[i]>newx[j] || x1[i+1]<=newx[j]) && i<ysetlength )
    			i++;
    		if( i>= ysetlength)
    			newy = y[ysetlength-1];
    		else 
    			newy =
    			(newx[j]-x1[i])*(y[i+1]-y[i])/(x1[i+1]-x1[i])+y[i];
    		add_point(cg, iset, newx[j], newy, 0.0, 0.0, SET_XY);
    	}
    }
    /* activate new set and update sets */
	sprintf( buf, "Interpolated from Set %d at points from Set %d", 
										
					yset, xset );
    cxfree( newx );
	updatesetminmax(cg, iset);
	setcomment(cg, iset, buf);
#ifndef NONE_GUI
	update_set_status(cg, iset);
	drawgraph();
#endif
}

