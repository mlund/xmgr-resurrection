/* $Id: graphu1.c,v 1.4 1995/07/01 04:53:30 pturner Exp pturner $
 *
 * utilities for graphs
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>

#include "globals.h"
#include "draw.h"
#include "protos.h"

void flipxy(int gno);
void invertx(int gno);
void inverty(int gno);
void set_graph_active(int gno);
void update_all(int gno);


/*
 * Determine if a graph corner was double clicked
 * return 1 if clicked, 0 o.w.
 */
int focus_clicked( int cg, int x, int y )
{
	int xv1, yv1, xv2, yv2;
	
	view2device( g[cg].v.xv1, g[cg].v.yv1, &xv1, &yv1 );
	view2device( g[cg].v.xv2, g[cg].v.yv2, &xv2, &yv2 );
	if ( (fabs((float)(yv1 - y)) <= 5 || fabs((float)(yv2 - y)) <= 5) && 
	     (fabs((float)(xv1 - x)) <= 5 || fabs((float)(xv2 - x)) <= 5) ) {
		return 1;
	} else {
		return 0;
	}
}


void get_corner_clicked( int cg, int x, int y, double *xc, double *yc )
{
	double xv, yv;
	
	device2view( x, y, &xv, &yv );
	if( fabs( g[cg].v.xv1 - xv ) < fabs( g[cg].v.xv2 - xv ) )
		*xc = g[cg].v.xv1;
	else
		*xc = g[cg].v.xv2;	
	if( fabs( g[cg].v.yv1 - yv ) < fabs( g[cg].v.yv2 - yv ) )
		*yc = g[cg].v.yv1;
	else
		*yc = g[cg].v.yv2;
}


/*
 * Given the graph gno, find the graph that contains
 * (wx, wy). Used for setting the graph focus.
 */
int iscontained(int gno, double wx, double wy)
{
    int i;
    double x1, y1, x2, y2;
    double x = xconv(wx), y = yconv(wy);

    for (i = 0; i < maxgraph; i++) {
	if (g[i].active) {
	    x1 = g[i].v.xv1;
	    x2 = g[i].v.xv2;
	    y1 = g[i].v.yv1;
	    y2 = g[i].v.yv2;
	    if (isxreversed(i)) {
		fswap(&x1, &x2);
	    }
	    if (isyreversed(i)) {
		fswap(&y1, &y2);
	    }
	    if ((x1 <= x && x2 >= x) && (y1 <= y && y2 >= y)) {
		return i;
	    }
	}
    }
    return gno;
}


/*
 * Given the graph gno, find the next graph that contains
 * (wx, wy). Used for setting the graph focus.
 */
int nextcontained(int gno, double wx, double wy)
{
    int i, j;
    double x1, y1, x2, y2;
    double x = xconv(wx), y = yconv(wy);

    for (j = 0; j < maxgraph; j++) {
	i = (j + gno + 1) % maxgraph;
	if (g[i].active) {
	    x1 = g[i].v.xv1;
	    x2 = g[i].v.xv2;
	    y1 = g[i].v.yv1;
	    y2 = g[i].v.yv2;
	    if (isxreversed(i)) {
		fswap(&x1, &x2);
	    }
	    if (isyreversed(i)) {
		fswap(&y1, &y2);
	    }
	    if ((x1 <= x && x2 >= x) && (y1 <= y && y2 >= y)) {
		return i;
	    }
	}
    }
    return gno;
}

/*
 * Count the number of active sets in graph gno
 */
int nactive(int gno)
{
    int i, cnt = 0;
    for (i = 0; i < g[gno].maxplot; i++) {
	if (isactive_set(gno, i)) {
	    cnt++;
	}
    }
    return cnt;
}

int isxreversed(int gno)
{
    return (g[gno].v.xv1 > g[gno].v.xv2);
}

int isyreversed(int gno)
{
    return (g[gno].v.yv1 > g[gno].v.yv2);
}

int islogx(int gno)
{
    return (g[gno].type == GRAPH_LOGX || g[gno].type == GRAPH_LOGXY);
}

int islogy(int gno)
{
    return (g[gno].type == GRAPH_LOGY || g[gno].type == GRAPH_LOGXY);
}

char *graph_types(int it, int which)
{
    static char s[128];

    strcpy(s, "UNKNOWN");
    switch (it) {
    case GRAPH_XY:
	if (which) {
	    strcpy(s, "xy");
	} else {
	    strcpy(s, "XY");
	}
	break;
    case GRAPH_LOGX:
	if (which) {
	    strcpy(s, "logx");
	} else {
	    strcpy(s, "LOG-LINEAR");
	}
	break;
    case GRAPH_LOGY:
	if (which) {
	    strcpy(s, "logy");
	} else {
	    strcpy(s, "LINEAR-LOG");
	}
	break;
    case GRAPH_LOGXY:
	if (which) {
	    strcpy(s, "logxy");
	} else {
	    strcpy(s, "LOG-LOG");
	}
	break;
    case GRAPH_POLAR:
	strcpy(s, "polar");
	break;
    case GRAPH_BAR:
	if (which) {
	    strcpy(s, "bar");
	} else {
	    strcpy(s, "BAR");
	}
	break;
    case GRAPH_HBAR:
	if (which) {
	    strcpy(s, "hbar");
	} else {
	    strcpy(s, "HORIZONTAL GRAPH_BAR");
	}
	break;
    case GRAPH_STACKEDBAR:
	if (which) {
	    strcpy(s, "stackedbar");
	} else {
	    strcpy(s, "STACKED GRAPH_BAR");
	}
	break;
    case GRAPH_STACKEDHBAR:
	if (which) {
	    strcpy(s, "stackedhbar");
	} else {
	    strcpy(s, "STACKED HORIZONTAL GRAPH_BAR");
	}
	break;
    case GRAPH_STACKEDLINE:
	strcpy(s, "STACKED LINE");
	break;
    }
    return s;
}

int get_format_index(int f)
{
    int i = 0;

    while (f != format_types[i] && format_types[i] != FORMAT_INVALID)
	i++;
    return i;
}

char *get_format_types(int f)
{
    static char s[128];

    strcpy(s, "decimal");
    switch (f) {
    case FORMAT_DECIMAL:
	strcpy(s, "decimal");
	break;
    case FORMAT_EXPONENTIAL:
	strcpy(s, "exponential");
	break;
    case FORMAT_POWER:
	strcpy(s, "power");
	break;
    case FORMAT_GENERAL:
	strcpy(s, "general");
	break;
    case FORMAT_DDMMYY:
	strcpy(s, "ddmmyy");
	break;
    case FORMAT_MMDDYY:
	strcpy(s, "mmddyy");
	break;
    case FORMAT_MMYY:
	strcpy(s, "mmyy");
	break;
    case FORMAT_MMDD:
	strcpy(s, "mmdd");
	break;
    case FORMAT_MONTHDAY:
	strcpy(s, "monthday");
	break;
    case FORMAT_DAYMONTH:
	strcpy(s, "daymonth");
	break;
    case FORMAT_MONTHS:
	strcpy(s, "months");
	break;
    case FORMAT_MONTHSY:
	strcpy(s, "monthsy");
	break;
    case FORMAT_MONTHL:
	strcpy(s, "monthl");
	break;
    case FORMAT_DAYOFWEEKS:
	strcpy(s, "dayofweeks");
	break;
    case FORMAT_DAYOFWEEKL:
	strcpy(s, "dayofweekl");
	break;
    case FORMAT_DAYOFYEAR:
	strcpy(s, "dayofyear");
	break;
    case FORMAT_HMS:
	strcpy(s, "hms");
	break;
    case FORMAT_MMDDHMS:
	strcpy(s, "mmddhms");
	break;
    case FORMAT_MMDDYYHMS:
	strcpy(s, "mmddyyhms");
	break;
    case FORMAT_DEGREESLON:
	strcpy(s, "degreeslon");
	break;
    case FORMAT_DEGREESMMLON:
	strcpy(s, "degreesmmlon");
	break;
    case FORMAT_DEGREESMMSSLON:
	strcpy(s, "degreesmmsslon");
	break;
    case FORMAT_MMSSLON:
	strcpy(s, "mmsslon");
	break;
    case FORMAT_DEGREESLAT:
	strcpy(s, "degreeslat");
	break;
    case FORMAT_DEGREESMMLAT:
	strcpy(s, "degreesmmlat");
	break;
    case FORMAT_DEGREESMMSSLAT:
	strcpy(s, "degreesmmsslat");
	break;
    case FORMAT_MMSSLAT:
	strcpy(s, "mmsslat");
	break;
    }
    return s;
}

void kill_graph(int gno)
{
    int i, j;

    if (gno == maxgraph) {
	for (i = 0; i < maxgraph; i++) {
	    for (j = 0; j < g[i].maxplot; j++) {
		killset(i, j);
	    }
	    set_default_graph(i);
	}
    } else {
	for (i = 0; i < g[gno].maxplot; i++) {
	    killset(gno, i);
	}
	set_default_graph(gno);
    }
}

void copy_graph(int from, int to)
{
    int i, j;
    plotarr *p;
    kill_graph(to);
    p = g[to].p;
    memcpy(&g[to], &g[from], sizeof(graph));

    g[to].labs.title = copy_plotstr(g[from].labs.title);
    g[to].labs.stitle = copy_plotstr(g[from].labs.stitle);
    g[to].vp.vstr = copy_plotstr(g[from].vp.vstr);

    for (j = 0; j < 4; j++) {
	g[to].t[j].label = copy_plotstr(g[from].t[j].label);
	for (i = 0; i < MAX_TICK_LABELS; i++) {
	    if (g[from].t[j].t_speclab[i].s != NULL) {
		g[to].t[j].t_speclab[i] = copy_plotstr(g[from].t[j].t_speclab[i]);
	    }
	}
    }

    g[to].p = p;
    set_graph_active(to);	/* TODO compare maxplots */
    for (i = 0; i < g[from].maxplot; i++) {
	for (j = 0; j < MAX_SET_COLS; j++) {
	    g[to].p[i].ex[j] = NULL;
	}
	g[to].p[i].active = FALSE;
	if (isactive_set(from, i)) {
	    do_copyset(from, i, to, i);
	}
    }
}

void copy_graph_sets_only(int from, int to)
{
    int i, j;
    kill_graph(to);
    set_graph_active(to);	/* TODO compare maxplots */
    for (i = 0; i < g[from].maxplot; i++) {
	for (j = 0; j < MAX_SET_COLS; j++) {
	    g[to].p[i].ex[j] = NULL;
	}
	g[to].p[i].active = FALSE;
	if (isactive_set(from, i)) {
	    do_copyset(from, i, to, i);
	}
    }
}

void swap_graph(int from, int to)
{
    graph gtmp;
    memcpy(&gtmp, &g[from], sizeof(graph));
    memcpy(&g[from], &g[to], sizeof(graph));
    memcpy(&g[to], &gtmp, sizeof(graph));
    set_dirtystate();
}

/*
 * for flipping
 */
void do_flipxy(void)
{
    flipxy(cg);
#ifndef NONE_GUI
    drawgraph();
#endif
}

void flipxy(int gno)
{
    int i, j;
    tickmarks t;
    double *x, *y;

    for (i = 0; i < MAXAXES; i += 2) {
	memcpy(&t, &g[gno].t[i], sizeof(tickmarks));
	memcpy(&g[gno].t[i], &g[gno].t[i + 1], sizeof(tickmarks));
	memcpy(&g[gno].t[i + 1], &t, sizeof(tickmarks));
	if (g[gno].t[i].t_op == PLACE_RIGHT) {
	    g[gno].t[i].t_op = PLACE_TOP;
	} else if (g[gno].t[i].t_op == PLACE_LEFT) {
	    g[gno].t[i].t_op = PLACE_BOTTOM;
	}
	if (g[gno].t[i].tl_op == PLACE_RIGHT) {
	    g[gno].t[i].tl_op = PLACE_TOP;
	} else if (g[gno].t[i].tl_op == PLACE_LEFT) {
	    g[gno].t[i].tl_op = PLACE_BOTTOM;
	}
	if (g[gno].t[i + 1].t_op == PLACE_TOP) {
	    g[gno].t[i + 1].t_op = PLACE_RIGHT;
	} else if (g[gno].t[i + 1].t_op == PLACE_BOTTOM) {
	    g[gno].t[i + 1].t_op = PLACE_LEFT;
	}
	if (g[gno].t[i + 1].tl_op == PLACE_TOP) {
	    g[gno].t[i + 1].tl_op = PLACE_RIGHT;
	} else if (g[gno].t[i + 1].tl_op == PLACE_BOTTOM) {
	    g[gno].t[i + 1].tl_op = PLACE_LEFT;
	}
	fswap(&g[gno].t[i].label.x, &g[gno].t[i].label.y);
	fswap(&g[gno].t[i+1].label.x, &g[gno].t[i+1].label.y);
    }
    if (g[gno].type == GRAPH_LOGX) {
	g[gno].type = GRAPH_LOGY;
    } else if (g[gno].type == GRAPH_LOGY) {
	g[gno].type = GRAPH_LOGX;
    }
    fswap(&g[gno].w.xg1, &g[gno].w.yg1);
    fswap(&g[gno].w.xg2, &g[gno].w.yg2);
    fswap(&g[gno].dsx, &g[gno].dsy);
    iswap(&g[gno].fx, &g[gno].fy);
    iswap(&g[gno].px, &g[gno].py);
    for (i = 0; i < g[gno].maxplot; i++) {
	if (isactive_set(gno, i)) {
	    x = getx(gno, i);	/* TODO really need to just swap pointers */
	    y = gety(gno, i);
	    for (j = 0; j < getsetlength(gno, i); j++) {
		fswap(&x[j], &y[j]);
	    }
	    updatesetminmax(gno, i);
	}
    }
    update_all(gno);
    set_dirtystate();
}

void do_invertx(void)
{
    invertx(cg);
#ifndef NONE_GUI
    drawgraph();
#endif
}

void do_inverty(void)
{
    inverty(cg);
#ifndef NONE_GUI
    drawgraph();
#endif
}

void invertx(int gno)
{
    fswap(&g[gno].v.xv1, &g[gno].v.xv2);
    set_dirtystate();
}

void inverty(int gno)
{
    fswap(&g[gno].v.yv1, &g[gno].v.yv2);
    set_dirtystate();
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

/*
 *
 * set graph props
 *
 */

void set_graph_box(int i, boxtype * b)
{
    memcpy(&boxes[i], b, sizeof(boxtype));
}

void set_graph_line(int i, linetype * l)
{
    memcpy(&lines[i], l, sizeof(linetype));
}

void set_graph_string(int i, plotstr * s)
{
    memcpy(&pstr[i], s, sizeof(plotstr));
}

void set_graph_active(int gno)
{

    g[gno].active = TRUE;
}

void set_graph_framep(int gno, framep * f)
{
    memcpy(&g[gno].f, f, sizeof(framep));
}

void set_graph_world(int gno, world * w)
{
    memcpy(&g[gno].w, w, sizeof(world));
}

void set_graph_view(int gno, view * v)
{
    memcpy(&g[gno].v, v, sizeof(view));
}

void set_graph_viewport(int gno, double vx1, double vy1, double vx2, double vy2)
{
    if (isxreversed(gno)) {
	g[gno].v.xv2 = vx1;
	g[gno].v.xv1 = vx2;
    } else {
	g[gno].v.xv1 = vx1;
	g[gno].v.xv2 = vx2;
    }
    if (isyreversed(gno)) {
	g[gno].v.yv2 = vy1;
	g[gno].v.yv1 = vy2;
    } else {
	g[gno].v.yv1 = vy1;
	g[gno].v.yv2 = vy2;
    }
    set_dirtystate();
}

void set_graph_labels(int gno, labels * labs)
{
    memcpy(&g[gno].labs, labs, sizeof(labels));
}

void set_graph_plotarr(int gno, int i, plotarr * p)
{
    memcpy(&g[gno].p[i], p, sizeof(plotarr));
}

void set_graph_tickmarks(int gno, tickmarks * t, int a)
{
    memcpy(&g[gno].t[a], t, sizeof(tickmarks));
}

void set_graph_legend(int gno, legend * leg)
{
    memcpy(&g[gno].l, leg, sizeof(legend));
}


/*
	 the following routines determine default scaling and tickmarks
*/

void defaultgraph(int gno)
{
    double x1, x2, y1, y2;
    double xgmax, xgmin, ygmax, ygmin;
    int i, first = 1;

    if (g[gno].type == GRAPH_SMITH) {
        g[gno].w.xg1 = -1.0;
        g[gno].w.yg1 = -1.0;
        g[gno].w.xg2 = 1.0;
        g[gno].w.yg2 = 1.0;
	return;
    }
    xgmax = xgmin = ygmax = ygmin = 0.0;
    for (i = 0; i < g[gno].maxplot; i++) {
	if (isactive_set(gno, i)) {
	    getsetminmax(gno, i, &x1, &x2, &y1, &y2);
	    if (g[gno].type == GRAPH_STACKEDBAR) {
		if (first) {
		    xgmin = x1;
		    xgmax = x2;
		    ygmin = y1;
		    ygmax = y2;
		    first = 0;
		} else {
		    xgmin = (x1 < xgmin) ? x1 : xgmin;
		    xgmax = (x2 > xgmax) ? x2 : xgmax;
		    ygmin = (y1 < ygmin) ? y1 : ygmin;
		    ygmax += y2;
		}
	    } else if (g[gno].type == GRAPH_STACKEDHBAR) {
		if (first) {
		    xgmin = x1;
		    xgmax = x2;
		    ygmin = y1;
		    ygmax = y2;
		    first = 0;
		} else {
		    ygmin = (y1 < ygmin) ? y1 : ygmin;
		    ygmax = (y2 > ygmax) ? y2 : ygmax;
		    xgmin = (x1 < xgmin) ? x1 : xgmin;
		    xgmax += x2;
		}
	    } else {
		if (first) {
		    xgmin = x1;
		    xgmax = x2;
		    ygmin = y1;
		    ygmax = y2;
		    first = 0;
		} else {
		    xgmin = (x1 < xgmin) ? x1 : xgmin;
		    xgmax = (x2 > xgmax) ? x2 : xgmax;
		    ygmin = (y1 < ygmin) ? y1 : ygmin;
		    ygmax = (y2 > ygmax) ? y2 : ygmax;
		}
	    }
	}
    }
    if (xgmin != xgmax) {
	g[gno].w.xg2 = xgmax;
	g[gno].w.xg1 = xgmin;
    } else {
	if (xgmax == 0.0) {
	    xgmax = 1.0;
	}
	g[gno].w.xg1 = xgmin - 0.1 * fabs(xgmax);
	g[gno].w.xg2 = xgmin + 0.1 * fabs(xgmax);
    }
    if (ygmin != ygmax) {
	g[gno].w.yg2 = ygmax;
	g[gno].w.yg1 = ygmin;
    } else {
	if (ygmax == 0.0) {
	    ygmax = 1.0;
	}
	g[gno].w.yg1 = ygmin - 0.1 * fabs(ygmax);
	g[gno].w.yg2 = ygmin + 0.1 * fabs(ygmax);
    }
    switch (g[gno].type) {
    case GRAPH_BAR:
	g[gno].w.xg1 -= (g[gno].w.xg2 - g[gno].w.xg1) * 0.05;
	g[gno].w.xg2 += (g[gno].w.xg2 - g[gno].w.xg1) * 0.05;
	if (g[gno].w.yg1 > 0.0) {
	    g[gno].w.yg1 = 0.0;
	}
	break;
    case GRAPH_HBAR:
	g[gno].w.yg1 -= (g[gno].w.yg2 - g[gno].w.yg1) * 0.05;
	g[gno].w.yg2 += (g[gno].w.yg2 - g[gno].w.yg1) * 0.05;
	if (g[gno].w.xg1 > 0.0) {
	    g[gno].w.xg1 = 0.0;
	}
	break;
    case GRAPH_STACKEDBAR:
	g[gno].w.xg1 -= (g[gno].w.xg2 - g[gno].w.xg1) * 0.05;
	g[gno].w.xg2 += (g[gno].w.xg2 - g[gno].w.xg1) * 0.05;
	if (g[gno].w.yg1 > 0.0) {
	    g[gno].w.yg1 = 0.0;
	}
	break;
    case GRAPH_STACKEDHBAR:
	g[gno].w.yg1 -= (g[gno].w.yg2 - g[gno].w.yg1) * 0.05;
	g[gno].w.yg2 += (g[gno].w.yg2 - g[gno].w.yg1) * 0.05;
	if (g[gno].w.xg1 > 0.0) {
	    g[gno].w.xg1 = 0.0;
	}
	break;
    case GRAPH_LOGX:
	if (g[gno].w.xg1 <= 0.0) {
	    errmsg("can't set graph type to log-linear, X minimum = 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    case GRAPH_LOGY:
	if (g[gno].w.yg1 <= 0.0) {
	    errmsg("can't set graph type to linear-log, Y minimum = 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    case GRAPH_LOGXY:
	if (g[gno].w.xg1 <= 0.0) {
	    errmsg("can't set graph to log-log, X minimum <= 0.0");
	    g[gno].type = GRAPH_XY;
	} else if (g[gno].w.yg1 <= 0.0) {
	    errmsg("can't set graph type to log-log, Y minimum <= 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    case GRAPH_POLAR:
	break;
    }
}

void defaultx(int gno, int setno)
{
    int i, first = 1;
    double xgmin, xgmax, xmax, xmin, tmp;

    if (g[gno].type == GRAPH_SMITH) {
        g[gno].w.xg1 = -1.0;
        g[gno].w.xg2 = 1.0;
	return;
    }
    xgmin = xgmax = 0.0;
    if (setno < 0) {
	for (i = 0; i < g[gno].maxplot; i++) {
	    if (isactive_set(gno, i)) {
		getsetminmax(gno, i, &xmin, &xmax, &tmp, &tmp);
		if (g[gno].type == GRAPH_STACKEDBAR) {
		    if (first) {
			xgmin = xmin;
			xgmax = xmax;
			first = 0;
		    } else {
			xgmin = (xmin < xgmin) ? xmin : xgmin;
			xgmax = (xmax > xgmax) ? xmax : xgmax;
		    }
		} else if (g[gno].type == GRAPH_STACKEDHBAR) {
		    if (first) {
			xgmin = xmin;
			xgmax = xmax;
			first = 0;
		    } else {
			xgmin = (xmin < xgmin) ? xmin : xgmin;
			xgmax += xmax;
		    }
		} else {
		    if (first) {
			xgmin = xmin;
			xgmax = xmax;
			first = 0;
		    } else {
			xgmin = (xmin < xgmin) ? xmin : xgmin;
			xgmax = (xmax > xgmax) ? xmax : xgmax;
		    }
		}
	    }
	}
    } else {
	if (isactive_set(gno, setno)) {
	    getsetminmax(gno, setno, &xgmin, &xgmax, &tmp, &tmp);
	} else {
	    return;
	}
    }
    if (xgmin != xgmax) {
	g[gno].w.xg2 = xgmax;
	g[gno].w.xg1 = xgmin;
    } else {
	if (xgmax == 0.0) {
	    xgmax = 1.0;
	}
	g[gno].w.xg1 = xgmin - 0.1 * fabs(xgmax);
	g[gno].w.xg2 = xgmin + 0.1 * fabs(xgmax);
    }
    switch (g[gno].type) {
    case GRAPH_BAR:
    case GRAPH_STACKEDBAR:
	g[gno].w.xg1 -= 0.5;
	g[gno].w.xg2 += 0.5;
	if (g[gno].w.yg1 > 0.0) {
	    g[gno].w.yg1 = 0.0;
	}
	break;
    case GRAPH_HBAR:
    case GRAPH_STACKEDHBAR:
	if (g[gno].w.xg1 > 0.0) {
	    g[gno].w.xg1 = 0.0;
	}
	break;
    case GRAPH_LOGX:
	if (g[gno].w.xg1 <= 0.0) {
	    errmsg("can't set graph type to log-linear, X minimum = 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    case GRAPH_LOGY:
	if (g[gno].w.yg1 <= 0.0) {
	    errmsg("can't set graph type to linear-log, Y minimum = 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    case GRAPH_LOGXY:
	if (g[gno].w.xg1 <= 0.0) {
	    errmsg("can't set graph to log-log, X minimum <= 0.0");
	    g[gno].type = GRAPH_XY;
	} else if (g[gno].w.yg1 <= 0.0) {
	    errmsg("can't set graph type to log-log, Y minimum <= 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    }
}

void defaulty(int gno, int setno)
{
    int i, first = 1;
    double ygmax, ygmin, ymin, ymax, tmp;

    if (g[gno].type == GRAPH_SMITH) {
        g[gno].w.yg1 = -1.0;
        g[gno].w.yg2 = 1.0;
	return;
    }
    ygmin = ygmax = 0.0;
    if (setno < 0) {
	for (i = 0; i < g[gno].maxplot; i++) {
	    if (isactive_set(gno, i)) {
		getsetminmax(gno, i, &tmp, &tmp, &ymin, &ymax);
		if (g[gno].type == GRAPH_STACKEDBAR) {
		    if (first) {
			ygmin = ymin;
			ygmax = ymax;
			first = 0;
		    } else {
			ygmin = (ymin < ygmin) ? ymin : ygmin;
			ygmax += ymax;
		    }
		} else if (g[gno].type == GRAPH_STACKEDHBAR) {
		    if (first) {
			ygmin = ymin;
			ygmax = ymax;
			first = 0;
		    } else {
			ygmin = (ymin < ygmin) ? ymin : ygmin;
			ygmax = (ymax > ygmax) ? ymax : ygmax;
		    }
		} else {
		    if (first) {
			ygmin = ymin;
			ygmax = ymax;
			first = 0;
		    } else {
			ygmin = (ymin < ygmin) ? ymin : ygmin;
			ygmax = (ymax > ygmax) ? ymax : ygmax;
		    }
		}
	    }
	}
    } else {
	if (isactive_set(gno, setno)) {
	    getsetminmax(gno, setno, &tmp, &tmp, &ygmin, &ygmax);
	} else {
	    return;
	}
    }
    if (ygmin != ygmax) {
	g[gno].w.yg2 = ygmax;
	g[gno].w.yg1 = ygmin;
    } else {
	if (ygmax == 0.0) {
	    ygmax = 1.0;
	}
	g[gno].w.yg1 = ygmin - 0.1 * fabs(ygmax);
	g[gno].w.yg2 = ygmin + 0.1 * fabs(ygmax);
    }
    switch (g[gno].type) {
    case GRAPH_BAR:
    case GRAPH_STACKEDBAR:
	if (g[gno].w.yg1 > 0.0) {
	    g[gno].w.yg1 = 0.0;
	}
	break;
    case GRAPH_HBAR:
    case GRAPH_STACKEDHBAR:
	g[gno].w.yg1 -= 0.5;
	g[gno].w.yg2 += 0.5;
	if (g[gno].w.xg1 > 0.0) {
	    g[gno].w.xg1 = 0.0;
	}
	break;
    case GRAPH_LOGX:
	if (g[gno].w.xg1 <= 0.0) {
	    errmsg("can't set graph type to log-linear, X minimum = 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    case GRAPH_LOGY:
	if (g[gno].w.yg1 <= 0.0) {
	    errmsg("can't set graph type to linear-log, Y minimum = 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    case GRAPH_LOGXY:
	if (g[gno].w.xg1 <= 0.0) {
	    errmsg("can't set graph to log-log, X minimum <= 0.0");
	    g[gno].type = GRAPH_XY;
	} else if (g[gno].w.yg1 <= 0.0) {
	    errmsg("can't set graph type to log-log, Y minimum <= 0.0");
	    g[gno].type = GRAPH_XY;
	}
	break;
    }
    if (g[gno].type == GRAPH_BAR || g[gno].type == GRAPH_STACKEDBAR) {
	if (g[gno].w.yg1 > 0.0) {
	    g[gno].w.yg1 = 0.0;
	}
    }
}

#define USE_HECKBERT

#ifndef USE_HECKBERT

/*
 * Graph axis auto-scaling (Nicer minor-ticks than "Heckbert")
 *
 * Dan Hofferth, 10 Mar 97
 */

void default_ticks(int gno, int axis, double *gmin, double *gmax)
{
    tickmarks t;
    view v;
    double imax = *gmax, imin = *gmin, idiv, xdiv, dv;
    double iinc, xxp, norm, oinc, sdiv, omin, omax, odiv, sinc;

    get_graph_tickmarks(gno, &t, axis);
    get_graph_view(gno, &v);

    if (axis % 2 && (g[gno].type == GRAPH_LOGY || g[gno].type == GRAPH_LOGXY)) {
	imax = ceil(log10(imax));
	imin = floor(log10(imin));
    } else if ((axis % 2 == 0) && 
               (g[gno].type == GRAPH_LOGX || g[gno].type == GRAPH_LOGXY)) {
	imax = ceil(log10(imax));
	imin = floor(log10(imin));
    }
    
    if (axis % 2) {
	dv = fabs( v.yv2 - v.yv1 ) / 0.7;
    } else {
	dv = fabs( v.xv2 - v.xv1 ) / 0.7;
    }
    
    idiv =  0.0;
    xdiv = ceil( 10.0 * dv );
    if ( imin == imax ) {
   	imin = imin - 0.5;
   	imax = imax + 0.5;
    }
    if ( imin < 0.0 && imax > 0.0 && xdiv == 1.0 ) {
        xdiv = 2.0;
    }
    while ( -1 ) {
	iinc = ( imax - imin ) / xdiv;
	xxp = pow( 10.0, floor( log10( iinc ) ) );
   	norm = iinc / xxp;
   	if ( norm > 5.0 ) {
	    oinc = 10.0 * xxp;
	    sdiv = 5.0;
	}
	else if ( norm > 4.0 ) {
	    oinc = 5.0  * xxp;
	    sdiv = 5.0;
	} else if ( norm > 2.0 ) {
            oinc = 4.0  * xxp;
            sdiv = 4.0;
        } else if ( norm > 1.0 ) {
            oinc = 2.0  * xxp;
            sdiv = 2.0;
        } else {
            oinc = 1.0  * xxp;
            sdiv = 5.0;
        }
	omin = floor( imin / oinc ) * oinc;
   	omax = omin + xdiv * oinc;
   	if ( omin > imin || omax < imax ) {
      	    imin = (( imin < omin )? imin : omin );
      	    imax = (( imax > omax )? imax : omax );
      	    continue;
   	}
   	if ( idiv == 0.0 ) {
      	    odiv = ( omax - imax ) / oinc;
      	    odiv = ( odiv >= 0 ) ? floor( odiv ) : ceil( odiv );
      	    if ( odiv > 0 ) {
		/* imin = omin */
		xdiv = xdiv - odiv;
		continue;
	    }
	}
	break;
    }
    sinc = oinc / sdiv;
    
    if (axis % 2 && (g[gno].type == GRAPH_LOGY || g[gno].type == GRAPH_LOGXY)) {
	*gmax = pow(10.0, omax);
	*gmin = pow(10.0, omin);
	t.tmajor = (int) oinc;
	if (t.tmajor == 0.0) {
	    t.tmajor = 1.0;
	}
	if ((int) t.tmajor < 2) {
	    t.tminor = 1.0;
	} else {
	    t.tminor = 0.0;
	}
	if (fabs(omax) > 6.0 || fabs(omin) > 6.0) {
	    t.tl_format = FORMAT_POWER;
	    t.tl_prec = 0;
	} else {
	    t.tl_format = FORMAT_DECIMAL;
	    t.tl_prec = 0;
	}
    } else if ((axis % 2 == 0) && 
               (g[gno].type == GRAPH_LOGX || g[gno].type == GRAPH_LOGXY)) {
	*gmax = pow(10.0, omax);
	*gmin = pow(10.0, omin);
	t.tmajor = (int) oinc;
	if (t.tmajor == 0.0) {
	    t.tmajor = 1.0;
	}
	if (fabs(omax) > 6.0 || fabs(omin) > 6.0) {
	    t.tl_format = FORMAT_POWER;
	    t.tl_prec = 0;
	} else {
	    t.tl_format = FORMAT_DECIMAL;
	    t.tl_prec = 0;
	}
	if ((int) t.tmajor < 2) {
	    t.tminor = 1.0;
	} else {
	    t.tminor = 0.0;
	}
    } else {
	*gmax = omax;
	*gmin = omin;
	t.tmajor = oinc;
	t.tminor = sinc;
    }
    set_graph_tickmarks(gno, &t, axis);
}

#else /* do USE_HECKBERT */

/*
 * label: test program to demonstrate nice graph axis labeling
 *
 * Paul Heckbert, 2 Dec 88
 */

static double nicenum(double x, int round);

void default_ticks(int gno, int axis, double *gmin, double *gmax)
{
    tickmarks t;
    double range, d, tmpmax = *gmax, tmpmin = *gmin;

    get_graph_tickmarks(gno, &t, axis);
    if (axis % 2 && (g[gno].type == GRAPH_LOGY || g[gno].type == GRAPH_LOGXY)) {
	tmpmax = ceil(log10(tmpmax));
	tmpmin = floor(log10(tmpmin));
    } else if ((axis % 2 == 0) && (g[gno].type == GRAPH_LOGX || g[gno].type == GRAPH_LOGXY)) {
	tmpmax = ceil(log10(tmpmax));
	tmpmin = floor(log10(tmpmin));
    }
    range = nicenum(tmpmax - tmpmin, 0);
    d = nicenum(range / (t.t_num - 1), 1);
    tmpmin = floor(tmpmin / d) * d;
    tmpmax = ceil(tmpmax / d) * d;
    if (axis % 2 && (g[gno].type == GRAPH_LOGY || g[gno].type == GRAPH_LOGXY)) {
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
	    t.tl_format = FORMAT_POWER;
	    t.tl_prec = 0;
	} else {
	    t.tl_format = FORMAT_DECIMAL;
	    t.tl_prec = 0;
	}
    } else if ((axis % 2 == 0) && (g[gno].type == GRAPH_LOGX || g[gno].type == GRAPH_LOGXY)) {
	*gmax = pow(10.0, tmpmax);
	*gmin = pow(10.0, tmpmin);
	t.tmajor = (int) d;
	if (t.tmajor == 0.0) {
	    t.tmajor = 1.0;
	}
	if (fabs(tmpmax) > 6.0 || fabs(tmpmin) > 6.0) {
	    t.tl_format = FORMAT_POWER;
	    t.tl_prec = 0;
	} else {
	    t.tl_format = FORMAT_DECIMAL;
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

#endif

void defaultsetgraph(int gno, int setno)
{
    double xmax, xmin, ymax, ymin, extra_range;

    getsetminmax(gno, setno, &xmin, &xmax, &ymin, &ymax);

    if (xmin != xmax) {
        extra_range = 0.0;
    } else {
        extra_range = 0.1 * fabs(xmin);
    }
    g[gno].w.xg1 = xmin - extra_range;
    g[gno].w.xg2 = xmax + extra_range;
    
    if (ymin != ymax) {
        extra_range = 0.0;
    } else {
        extra_range = 0.1 * fabs(ymin);
    }
    g[gno].w.yg1 = ymin - extra_range;
    g[gno].w.yg2 = ymax + extra_range;
}

void default_axis(int gno, int method, int axis)
{
    int cx;
    tickmarks t;
    world w;
    double llim, ulim;
    int newprec;

    get_graph_tickmarks(gno, &t, axis);
    get_graph_world(gno, &w);
    if (axis % 2) {
	llim = w.yg1;
	ulim = w.yg2;
    } else {
	llim = w.xg1;
	ulim = w.xg2;
    }
    t.tmajor = (ulim - llim) / t.t_num;
    t.tminor = t.tmajor / 2.0;
    cx = (int) (log10(t.tmajor));

    newprec = ((cx < 0) ? -cx + 1 : t.tl_prec);
    if (t.tl_format != FORMAT_GENERAL || newprec > t.tl_prec) {
        t.tl_prec = newprec;
        if (t.tl_prec > 9) {
            t.tl_prec = 2;
            t.tl_format = FORMAT_EXPONENTIAL;
        }
    }

    set_graph_tickmarks(gno, &t, axis);
    if (method == TYPE_AUTO) {
	default_ticks(gno, axis, &llim, &ulim);
	if (axis % 2) {
	    w.yg1 = llim;
	    w.yg2 = ulim;
	} else {
	    w.xg1 = llim;
	    w.xg2 = ulim;
	}
	set_graph_world(gno, &w);
    }
}

void autoscale_graph(int gno, int axis)
{
    if (activeset(gno)) {
	switch (axis) {
	case -3:
	    defaultgraph(gno);
	    default_axis(gno, g[gno].auto_type, X_AXIS);
	    default_axis(gno, g[gno].auto_type, ZX_AXIS);
	    default_axis(gno, g[gno].auto_type, Y_AXIS);
	    default_axis(gno, g[gno].auto_type, ZY_AXIS);
	    break;
	case -2:
	    defaultx(gno, -1);
	    default_axis(gno, g[gno].auto_type, X_AXIS);
	    default_axis(gno, g[gno].auto_type, ZX_AXIS);
	    break;
	case -1:
	    defaulty(gno, -1);
	    default_axis(gno, g[gno].auto_type, Y_AXIS);
	    default_axis(gno, g[gno].auto_type, ZY_AXIS);
	    break;
	default:
	    if (axis % 2) {
		defaulty(gno, -1);
	    } else {
		defaultx(gno, -1);
	    }
	    default_axis(gno, g[gno].auto_type, axis);
	    break;
	}
	update_all(gno);
    }
}

void do_autoscale_set(int gno, int setno)
{
    if (isactive_set(gno, setno)) {
	defaultsetgraph(gno, setno);
	default_axis(gno, g[gno].auto_type, X_AXIS);
	default_axis(gno, g[gno].auto_type, ZX_AXIS);
	default_axis(gno, g[gno].auto_type, Y_AXIS);
	default_axis(gno, g[gno].auto_type, ZY_AXIS);
	update_all(gno);
#ifndef NONE_GUI
	drawgraph();
#endif
    }
}

void autoscale_set(int gno, int setno, int axis)
{
    if (isactive_set(gno, setno)) {
	switch (axis) {
	case -3:
	    defaultsetgraph(gno, setno);
	    default_axis(gno, g[gno].auto_type, X_AXIS);
	    default_axis(gno, g[gno].auto_type, ZX_AXIS);
	    default_axis(gno, g[gno].auto_type, Y_AXIS);
	    default_axis(gno, g[gno].auto_type, ZY_AXIS);
	    break;
	case -2:
	    defaultx(gno, setno);
	    default_axis(gno, g[gno].auto_type, X_AXIS);
	    default_axis(gno, g[gno].auto_type, ZX_AXIS);
	    break;
	case -1:
	    defaulty(gno, setno);
	    default_axis(gno, g[gno].auto_type, Y_AXIS);
	    default_axis(gno, g[gno].auto_type, ZY_AXIS);
	    break;
	default:
	    if (axis % 2) {
		defaulty(gno, setno);
	    } else {
		defaultx(gno, setno);
	    }
	    default_axis(gno, g[gno].auto_type, axis);
	    break;
	}
	update_all(gno);
    } else {
	errmsg("Set not active!");
    }
}

/*
 * for zooms, lz => linked to all active graphs
 *
 */
void newworld(int gno, int lz, int axes, double wx1, double wy1, double wx2, double wy2)
{
    int i, ming, maxg;
    if (lz) {
	ming = 0;
	maxg = maxgraph;
    } else {
	ming = gno;
	maxg = gno;
    }
    for (i = ming; i <= maxg; i++) {
	if (isactive_graph(i)) {
	    switch (axes) {
	    case -1:
		g[i].w.xg1 = wx1;
		g[i].w.xg2 = wx2;
		g[i].w.yg1 = wy1;
		g[i].w.yg2 = wy2;
		default_axis(i, g[i].auto_type, X_AXIS);
		default_axis(i, g[i].auto_type, ZX_AXIS);
		default_axis(i, g[i].auto_type, Y_AXIS);
		default_axis(i, g[i].auto_type, ZY_AXIS);
		break;
	    case 0:
		g[i].w.xg1 = wx1;
		g[i].w.xg2 = wx2;
		default_axis(i, g[i].auto_type, X_AXIS);
		default_axis(i, g[i].auto_type, ZX_AXIS);
		break;
	    case 1:
		g[i].w.yg1 = wy1;
		g[i].w.yg2 = wy2;
		default_axis(i, g[i].auto_type, Y_AXIS);
		default_axis(i, g[i].auto_type, ZY_AXIS);
		break;
	    }
	}
    }
}

int wipeout(int ask)
{
    if (ask && is_dirtystate()) {
        if (!yesno("Abandon unsaved project?", NULL, NULL, NULL)) {
            return 1;
        }
    }
    kill_graph(maxgraph);
    do_clear_lines();
    do_clear_boxes();
    do_clear_ellipses();
    do_clear_text();
    cg = 0;
    setdefaultcolors(cg);
    *description = 0;
#ifndef NONE_GUI
    strcpy(docname, NONAME);;
    set_title(docname);
    update_describe_popup ();
    drawgraph();
#endif
    clear_dirtystate();
    return 0;
}

void update_all(int gno)
{
    set_lists_dirty(TRUE);
#ifndef NONE_GUI
    update_set_lists(gno);
    update_world(gno);
    update_view(gno);
    update_status(gno, cur_statusitem, -1);
    update_ticks(gno);
    update_autos(gno);
    updatelegends(gno);
    updatesymbols(gno, -1);
    update_label_proc();
    update_locator_items(gno);
    update_draw();
    update_frame_items(gno);
    update_graph_items();
    update_hotlinks();
    update_prune_frame();
#endif
}
