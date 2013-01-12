#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "grconvert.h"

/*
graph *g;
plotstr *pstr;
boxtype *boxes;
linetype *lines;
ellipsetype *ellip;
polytype *polys;  
*/

char *graph_types(int it, int which)
{
    static char s[128];

    strcpy(s, "UNKNOWN");
    switch (it) {
    case XY:
	if (which) {
	    strcpy(s, "xy");
	} else {
	    strcpy(s, "XY");
	}
	break;
    case LOGX:
	if (which) {
	    strcpy(s, "logx");
	} else {
	    strcpy(s, "LOG-LINEAR");
	}
	break;
    case LOGY:
	if (which) {
	    strcpy(s, "logy");
	} else {
	    strcpy(s, "LINEAR-LOG");
	}
	break;
    case LOGXY:
	if (which) {
	    strcpy(s, "logxy");
	} else {
	    strcpy(s, "LOG-LOG");
	}
	break;
    case XYFIXED:
	strcpy(s, "FIXED XY");
	break;
    case POLAR:
	strcpy(s, "polar");
	break;
    case BAR:
	if (which) {
	    strcpy(s, "bar");
	} else {
	    strcpy(s, "BAR");
	}
	break;
    case HBAR:
	if (which) {
	    strcpy(s, "hbar");
	} else {
	    strcpy(s, "HORIZONTAL BAR");
	}
	break;
    case PIE:
	strcpy(s, "pie");
	break;
    case STACKEDBAR:
	if (which) {
	    strcpy(s, "stackedbar");
	} else {
	    strcpy(s, "STACKED BAR");
	}
	break;
    case STACKEDHBAR:
	if (which) {
	    strcpy(s, "stackedhbar");
	} else {
	    strcpy(s, "STACKED HORIZONTAL BAR");
	}
	break;
    case STACKEDLINE:
	strcpy(s, "STACKED LINE");
	break;
    }
    return s;
}


char *get_format_types(int f)
{
    static char s[128];

    strcpy(s, "decimal");
    switch (f) {
    case DECIMAL:
	strcpy(s, "decimal");
	break;
    case EXPONENTIAL:
	strcpy(s, "exponential");
	break;
    case POWER:
	strcpy(s, "power");
	break;
    case GENERAL:
	strcpy(s, "general");
	break;
    case DDMMYY:
	strcpy(s, "ddmmyy");
	break;
    case MMDDYY:
	strcpy(s, "mmddyy");
	break;
    case MMYY:
	strcpy(s, "mmyy");
	break;
    case MMDD:
	strcpy(s, "mmdd");
	break;
    case MONTHDAY:
	strcpy(s, "monthday");
	break;
    case DAYMONTH:
	strcpy(s, "daymonth");
	break;
    case MONTHS:
	strcpy(s, "months");
	break;
    case MONTHSY:
	strcpy(s, "monthsy");
	break;
    case MONTHL:
	strcpy(s, "monthl");
	break;
    case DAYOFWEEKS:
	strcpy(s, "dayofweeks");
	break;
    case DAYOFWEEKL:
	strcpy(s, "dayofweekl");
	break;
    case DAYOFYEAR:
	strcpy(s, "dayofyear");
	break;
    case HMS:
	strcpy(s, "hms");
	break;
    case MMDDHMS:
	strcpy(s, "mmddhms");
	break;
    case MMDDYYHMS:
	strcpy(s, "mmddyyhms");
	break;
    case DEGREESLON:
	strcpy(s, "degreeslon");
	break;
    case DEGREESMMLON:
	strcpy(s, "degreesmmlon");
	break;
    case DEGREESMMSSLON:
	strcpy(s, "degreesmmsslon");
	break;
    case MMSSLON:
	strcpy(s, "mmsslon");
	break;
    case DEGREESLAT:
	strcpy(s, "degreeslat");
	break;
    case DEGREESMMLAT:
	strcpy(s, "degreesmmlat");
	break;
    case DEGREESMMSSLAT:
	strcpy(s, "degreesmmsslat");
	break;
    case MMSSLAT:
	strcpy(s, "mmsslat");
	break;
    }
    return s;
}

void get_graph_box(int i, boxtype * b)
{
    memcpy(b, &boxes[i], sizeof(boxtype));
}

void get_graph_ellipse(int i, ellipsetype * b)
{
    memcpy(b, &ellip[i], sizeof(ellipsetype));
}

void get_graph_line(int i, linetype * l)
{
    memcpy(l, &lines[i], sizeof(linetype));
}

void get_graph_string(int i, plotstr * s)
{
    memcpy(s, &pstr[i], sizeof(plotstr));
}

void get_graph_framep(int gno, framep * f)
{
    memcpy(f, &g[gno].f, sizeof(framep));
}

void get_graph_world(int gno, world * w)
{
    memcpy(w, &g[gno].w, sizeof(world));
}

void get_graph_view(int gno, view * v)
{
    memcpy(v, &g[gno].v, sizeof(view));
}

void get_graph_labels(int gno, labels * labs)
{
    memcpy(labs, &g[gno].labs, sizeof(labels));
}

void get_graph_plotarr(int gno, int i, plotarr * p)
{
    memcpy(p, &g[gno].p[i], sizeof(plotarr));
}

void get_graph_tickmarks(int gno, tickmarks * t, int a)
{
    memcpy(t, &g[gno].t[a], sizeof(tickmarks));
}

void get_graph_legend(int gno, legend * leg)
{
    memcpy(leg, &g[gno].l, sizeof(legend));
}



void set_graph_tickmarks(int gno, tickmarks * t, int a)
{
    memcpy(&g[gno].t[a], t, sizeof(tickmarks));
}


/*
 * nicenum: find a "nice" number approximately equal to x
 * round if round=1, ceil if round=0
 */

static double nicenum(double x, int round)
{
    int exp;
    double f, y;

    exp = floor(log10(x));
    f = x / pow(10., (double) exp);	/* fraction between 1 and 10 */
    if (round)
	if (f < 1.5)
	    y = 1.;
	else if (f < 3.)
	    y = 2.;
	else if (f < 7.)
	    y = 5.;
	else
	    y = 10.;
    else if (f <= 1.)
	y = 1.;
    else if (f <= 2.)
	y = 2.;
    else if (f <= 5.)
	y = 5.;
    else
	y = 10.;
    return y * pow(10., (double) exp);
}

void default_ticks(int gno, int axis, double *gmin, double *gmax)
{
    tickmarks t;
    double range, d, tmpmax = *gmax, tmpmin = *gmin;

    get_graph_tickmarks(gno, &t, axis);
    if (axis % 2 && (g[gno].type == LOGY || g[gno].type == LOGXY)) {
	tmpmax = ceil(log10(tmpmax));
	tmpmin = floor(log10(tmpmin));
    } else if ((axis % 2 == 0) && (g[gno].type == LOGX || g[gno].type == LOGXY)) {
	tmpmax = ceil(log10(tmpmax));
	tmpmin = floor(log10(tmpmin));
    }
    range = nicenum(tmpmax - tmpmin, 0);
    d = nicenum(range / (t.t_num - 1), 1);
    tmpmin = floor(tmpmin / d) * d;
    tmpmax = ceil(tmpmax / d) * d;
    if (axis % 2 && (g[gno].type == LOGY || g[gno].type == LOGXY)) {
	*gmax = pow(10.0, tmpmax);
	*gmin = pow(10.0, tmpmin);
	t.tmajor = (int) d;
	if (t.tmajor == 0.0) {
	    t.tmajor = 1.0;
	}
	if ((int) t.tmajor < 2) {
	    t.tminor = 1.0;
	} else {
	    t.tminor = 0.0;
	}
	if (fabs(tmpmax) > 6.0 || fabs(tmpmin) > 6.0) {
	    t.tl_format = POWER;
	    t.tl_prec = 0;
	} else {
	    t.tl_format = DECIMAL;
	    t.tl_prec = 0;
	}
    } else if ((axis % 2 == 0) && (g[gno].type == LOGX || g[gno].type == LOGXY)) {
	*gmax = pow(10.0, tmpmax);
	*gmin = pow(10.0, tmpmin);
	t.tmajor = (int) d;
	if (t.tmajor == 0.0) {
	    t.tmajor = 1.0;
	}
	if (fabs(tmpmax) > 6.0 || fabs(tmpmin) > 6.0) {
	    t.tl_format = POWER;
	    t.tl_prec = 0;
	} else {
	    t.tl_format = DECIMAL;
	    t.tl_prec = 0;
	}
	if ((int) t.tmajor < 2) {
	    t.tminor = 1.0;
	} else {
	    t.tminor = 0.0;
	}
    } else {
	*gmax = tmpmax;
	*gmin = tmpmin;
	t.tmajor = d;
	t.tminor = d * 0.5;
    }
    set_graph_tickmarks(gno, &t, axis);
}

void errmsg(char *buf)
{
    printf("%s\n", buf);
}


/*
 * escape quotes
 */
char *escapequotes (char *s)
{
    char *es;
    int i, k, n, len, elen;
    
    if (s == NULL)
        return NULL;
    
    len = strlen(s);
    es = (char *) malloc ((len + 1)*sizeof(char));
    strcpy(es, s);
    n = 0;
    while ((es = strchr(es, '\"'))) {
    	es++;
    	n++;
    }
    
    elen = len + n + 1;
    es = (char *) realloc (es, elen*sizeof(char));
    
    i = k = 0;
    while (i < len) {
        if (s[i] == '\"') {
            es[k] = '\\';
            k++;
        }
        es[k] = s[i];
        i++; k++;
    }
    es[elen-1] = '\0';
    return es;
}


/*
 * return the string version of the set type
 */
char *set_types(int it)
{
    char *s = "XY";

    switch (it) {
    case XY:
	s = "xy";
	break;
    case XYDX:
	s = "xydx";
	break;
    case XYDY:
	s = "xydy";
	break;
    case XYDYDY:
	s = "xydydy";
	break;
    case XYDXDX:
	s = "xydxdx";
	break;
    case XYDXDY:
	s = "xydxdy";
	break;
    case XYZ:
	s = "xyz";
	break;
    case XYHILO:
	s = "xyhilo";
	break;
    case XYBOX:
	s = "xybox";
	break;
    case XYRT:
	s = "xyrt";
	break;
    case XYUV:
	s = "xyuv";
	break;
    case XYBOXPLOT:
	s = "xyboxplot";
	break;
    case XYSTRING:
	s = "xystring";
	break;
    case POLY:
	s = "poly";
	break;
    }
    return s;
}


int is_hotlinked(int gno, int setno)
{
    return (g[gno].p[setno].hotlink && strlen(g[gno].p[setno].hotfile) > 0);
}

/*
 * free and check for NULL pointer
 */
void cxfree(void *ptr)
{
    if (ptr != NULL) {
	free(ptr);
    }
}

