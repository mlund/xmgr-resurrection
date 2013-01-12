/* $Id: plotone.c,v 1.6 1995/07/01 04:53:30 pturner Exp pturner $
 *
 * plotone.c - entry for graphics
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
/*
 * #include <setjmp.h>
 */

#include "globals.h"
#include "symdefs.h"
#include "draw.h"
#include "protos.h"

/* TODO move decl out of here */
void drawsetpoly(int gno, plotarr *p, int setno);
void drawstring(plotarr *p);

/*
 * functions for polar plots, some are to be moved out
 * of here
 */
void drawsetboxplot(int gno, int setno);
void drawpolarticks(double xc, double yc, double r, double r0, int n);
void drawpolarpoly(double *x, double *y, int n);
void drawsetxypolar(plotarr *p, int i);
void circleplot(framep f, double xc, double yc, double r);
void my_circle(double xc, double yc, double s);
void my_filledcircle(double xc, double yc, double s);
double rp2rg(double r);
double rg2rp(double r);
void rg2xy(double r, double theta, double *x, double *y);
void r2xy(double r, double theta, double *x, double *y);

int density_flag;		/* temp, until interface for density plots is
				 * completed */

static double rp = 1.0, grad;
static double xc, yc, r;

int doclear = 1;	    	/* clear the screen if true before drawing */

/*
 * set hardcopy flag and if writing to a file, check
 * to see if it exists
 */
void do_hardcopy(void)
{
    FILE *fp;
    int i;
    extern char printstr[];	/* defined in printwin.c */

    if (ptofile) {
	if (fexists(printstr)) {
	    hardcopyflag = FALSE;
	    return;
	}
	fp = fopen(printstr, "w");
	if (fp == NULL) {
	    sprintf(buf, "Can't open %s for write, hardcopy aborted", printstr);
	    errmsg(buf);
	    hardcopyflag = FALSE;
	    return;
	}
	fclose(fp);
    }
    hardcopyflag = TRUE;
    if (initgraphics(hdevice) != -1) {
	for (i = 0; i < maxgraph; i++) {
	    if (isactive_graph(i) && !g[i].hidden) {
		if (g[i].type == GRAPH_POLAR) {
		    draw_polar_graph(i);
		} else {

		    if (checkon_ticks(i) && checkon_world(i)) {
			plotone(i);
			draw_annotation(i);
		    }
		}
	    }
	}
	draw_annotation(-1);
	leavegraphics();
    } else {
	errmsg("Hardcopy failed");
    }
    hardcopyflag = FALSE;
    if (inwin) {
	doclear = 0;
	initgraphics(0);
	doclear = 1;
	defineworld(g[cg].w.xg1, g[cg].w.yg1, g[cg].w.xg2, g[cg].w.yg2, islogx(cg), islogy(cg));
	viewport(g[cg].v.xv1, g[cg].v.yv1, g[cg].v.xv2, g[cg].v.yv2);
    }
}

void draw_polar_graph(int gno)
{
    int i;
    world w;
    view v;
    framep f;
    legend leg;
    labels lab;
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);
    get_graph_labels(gno, &lab);
    get_graph_legend(gno, &leg);
    get_graph_framep(gno, &f);
    setclipping(FALSE);
    setfixedscale(v.xv1, v.yv1, v.xv2, v.yv2, &w.xg1, &w.yg1, &w.xg2, &w.yg2);
    g[gno].w = w;
    defineworld(w.xg1, w.yg1, w.xg2, w.yg2, 0, 0);
    viewport(v.xv1, v.yv1, v.xv2, v.yv2);
    xc = (w.xg1 + w.xg2) / 2.0;
    yc = (w.yg1 + w.yg2) / 2.0;
    r = (w.xg2 - w.xg1) / 2.0;
    grad = r;
    rp = g[gno].rt.xg1;
    if (f.active) {
	if (f.fillbg) {
	    setcolor(f.bgcolor);
	    my_filledcircle(xc, yc, r);
	}
	circleplot(f, xc, yc, r);
    }
    setcolor(1);
    setlinestyle(1);
    setlinewidth(1);
    drawpolarticks(xc, yc, r, r / 4.0, 4);
    for (i = 0; i < g[gno].maxplot; i++) {
	drawsetxypolar(&g[gno].p[i], i);
    }
}

/*
 * Smith chart conversion routines
 */
void rx2uv(double r, double x, double *u, double *v)
{
    *u = (r * r + x * x - 1.0) / (1.0 + 2 * r + r * r + x * x);
    *v = (2 * x) / (1.0 + 2 * r + r * r + x * x);
}

void uv2rx(double u, double v, double *r, double *x)
{
    double denom = ((1.0 - u) * (1.0 - u) + v * v);
    if (denom == 0.0) {
	*r = 0.0;
	*x = 0.0;
	return;
    }
    *r = (1.0 - (u * u + v * v)) / denom;
    *x = (2 * v) / denom;
}

void drawsmithaxes(int gno)
{
    int i, j;
    char buf[256];
    double x, r, xmin = 0.0, rmin = 0.0, u, v, dx = 0.1, dr = 0.1;
    for (j = 0; j < 100; j += 2) {
	for (i = -50; i < 50; i += 2) {
	    r = rmin + j * dr;
	    x = xmin + i * dx;
	    u = (r * r + x * x - 1.0) / (1.0 + 2 * r + r * r + x * x);
	    v = (2 * x) / (1.0 + 2 * r + r * r + x * x);
	    if (i == -50) {
		my_move2(u, v);
	    } else {
		my_draw2(u, v);
	    }
	    if (i == 0) {
		my_move2(u, v);
		sprintf(buf, "%.1f", r);
		writestr(u, v, 0, 0, buf);
	    }
	}
    }
    for (i = -50; i < 50; i += 2) {
	for (j = 0; j < 100; j += 2) {
	    r = rmin + j * dr;
	    x = xmin + i * dx;
	    u = (r * r + x * x - 1.0) / (1.0 + 2 * r + r * r + x * x);
	    v = (2 * x) / (1.0 + 2 * r + r * r + x * x);
	    if (j == 0) {
		my_move2(u, v);
	sprintf(buf, "%.1f", x);
	writestr(u, v, 0, 0, buf);
	    } else {
		my_draw2(u, v);
	    }
	}
    }
}

void draw_smith_chart(int gno)
{
    int i, j;
    world w;
    view v;
    framep f;
    legend leg;
    labels lab;
    plotarr p;
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);
    get_graph_framep(gno, &f);
    get_graph_labels(gno, &lab);
    get_graph_legend(gno, &leg);
    setclipping(TRUE);
    defineworld(w.xg1, w.yg1, w.xg2, w.yg2, 0, 0);
    viewport(v.xv1, v.yv1, v.xv2, v.yv2);
/*
    if (f.active == ON) {
	if (f.fillbg == ON) {
	    setcolor(f.bgcolor);
	    fillrectcolor(w.xg1, w.yg1, w.xg2, w.yg2);
	}
	boxplot(gno);
    }
*/
    setlinestyle(1);
    setlinewidth(1);
    drawsmithaxes(gno);
    setlinestyle(1);
    setlinewidth(1);

    if (lab.title.s[0]) {
	setlinewidth(lab.title.linew);
	setcolor(lab.title.color);
	setcharsize(lab.title.charsize);
	setfont(lab.title.font);
	drawtitle(lab.title.s, 0);
    }
    if (lab.stitle.s[0]) {
	setlinewidth(lab.stitle.linew);
	setcolor(lab.stitle.color);
	setcharsize(lab.stitle.charsize);
	setfont(lab.stitle.font);
	drawtitle(lab.stitle.s, 1);
    }
    if (g[gno].pointset) {	/* mark the reference point */
	drawpolysym(&g[gno].dsx, &g[gno].dsy, 1, SYM_CIRCLE, 0, 0, 1.0);
	drawpolysym(&g[gno].dsx, &g[gno].dsy, 1, SYM_PLUS, 0, 0, 1.0);
    }
    for (i = 0; i < g[gno].maxplot; i++) {
	get_graph_plotarr(gno, i, &p);
	if (isactive_set(gno, i)) {

	    double *x = p.ex[0], *y = p.ex[1];
	    double *xtmp, *ytmp;
	    xtmp = (double *) malloc(p.len * sizeof(double));
	    ytmp = (double *) malloc(p.len * sizeof(double));
/* TODO, check for xtmp == NULL & ytmp == NULL) */
	    for (j = 0; j < p.len; j++) {
		rx2uv(x[j], y[j], &xtmp[j], &ytmp[j]);
/*
printf("%lf %lf %lf %lf\n",x[j], y[j], xtmp[j], ytmp[j]);
        drawpolysym(&xtmp[j], &ytmp[j], 1, SYM_CIRCLE, 0, 0, 1.0);
        drawpolysym(&xtmp[j], &ytmp[j], 1, SYM_PLUS, 0, 0, 1.0);
*/
	    }
	    p.ex[0] = g[gno].p[i].ex[0] = xtmp;
	    p.ex[1] = g[gno].p[i].ex[1] = ytmp;

	    switch (dataset_type(gno, i)) {
	    case SET_XY:
		if (p.fill) {
		    drawsetfill(gno, &p);
		}
		drawsetxy(gno, &g[gno].p[i], i);
		break;
	    case SET_POLY:
		if (p.fill) {
		    drawsetfill(gno, &p);
		}
		drawsetpoly(gno, &g[gno].p[i], i);
		break;
	    case SET_XYDX:
	    case SET_XYDY:
	    case SET_XYDXDX:
	    case SET_XYDYDY:
	    case SET_XYDXDY:
		if (p.fill) {
		    drawsetfill(gno, &p);
		}
		drawseterrbar(gno, i, 0.0, 0.0);
		drawsetxy(gno, &g[gno].p[i], i);
		break;
	    case SET_XYXX:
		drawsetxy(gno, &g[gno].p[i], i);
		break;
	    case SET_XYYY:
		drawsetxy(gno, &g[gno].p[i], i);
		break;
	    case SET_XYZ:
		if (!density_flag) {
		    drawsetxy(gno, &g[gno].p[i], i);
		    drawval(&g[gno].p[i]);
		} else {
		    drawdensity(&g[gno].p[i]);
		}
		break;
	    case SET_XYZW:
		drawsetxy(gno, &g[gno].p[i], i);
		break;
	    case SET_XYRT:
		drawsetxy(gno, &g[gno].p[i], i);
		drawcirclexy(&g[gno].p[i]);
		break;
	    case SET_XYBOX:
		drawboxcolor(&g[gno].p[i]);
		break;
	    case SET_XYBOXPLOT:
		drawsetboxplot(gno, i);
		break;
	    case SET_XYHILO:
		drawsethilo(&g[gno].p[i]);
		break;
	    case SET_XYUV:
		drawsetxy(gno, &g[gno].p[i], i);
		drawflow(gno, i, g[gno].vp);
		break;
	    case SET_XYSTRING:
		drawsetxy(gno, &g[gno].p[i], i);
		if (g[gno].p[i].s != NULL) {
		    drawstring(&p);
		}
		break;
	    }
	    p.ex[0] = g[gno].p[i].ex[0] = x;
	    p.ex[1] = g[gno].p[i].ex[1] = y;
	    free(xtmp);
	    free(ytmp);
	}
    }

    setlinestyle(grdefaults.lines);
    setlinewidth(grdefaults.linew);
    if (leg.active) {
	dolegend(gno);
	setlinestyle(grdefaults.lines);
	setlinewidth(grdefaults.linew);
    }
    if (timestamp.active) {
	double xtmp, ytmp;
	setlinewidth(timestamp.linew);
	setcolor(timestamp.color);
	setcharsize(timestamp.charsize);
	setfont(timestamp.font);
	view2world(timestamp.x, timestamp.y, &xtmp, &ytmp);
	writestr(xtmp, ytmp, timestamp.rot, timestamp.just, timestamp.s);
    }
}

void plotone(int gno)
{
    int i;
    world w;
    view v;
    framep f;
    legend leg;
    labels lab;
    plotarr p;

    if (g[gno].type == GRAPH_POLAR) {
	draw_polar_graph(gno);
	return;
    }
    if (g[gno].type == GRAPH_SMITH) {
	draw_smith_chart(gno);
	return;
    }
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);
    get_graph_framep(gno, &f);
    get_graph_labels(gno, &lab);
    get_graph_legend(gno, &leg);
    setclipping(TRUE);
    defineworld(w.xg1, w.yg1, w.xg2, w.yg2, islogx(gno), islogy(gno));
    viewport(v.xv1, v.yv1, v.xv2, v.yv2);
    if (f.active) {
	if (f.fillbg) {
	    setcolor(f.bgcolor);
	    fillrectcolor(w.xg1, w.yg1, w.xg2, w.yg2);
	}
    }
    drawaxes(gno);

    if (lab.title.s[0]) {
	setlinewidth(lab.title.linew);
	setcolor(lab.title.color);
	setcharsize(lab.title.charsize);
	setfont(lab.title.font);
	drawtitle(lab.title.s, 0);
    }
    if (lab.stitle.s[0]) {
	setlinewidth(lab.stitle.linew);
	setcolor(lab.stitle.color);
	setcharsize(lab.stitle.charsize);
	setfont(lab.stitle.font);
	drawtitle(lab.stitle.s, 1);
    }
    if (g[gno].pointset) {	/* mark the reference point */
	drawpolysym(&g[gno].dsx, &g[gno].dsy, 1, SYM_CIRCLE, 0, 0, 1.0);
	drawpolysym(&g[gno].dsx, &g[gno].dsy, 1, SYM_PLUS, 0, 0, 1.0);
    }
    /*
     * draw sets
     */
    set_frame_bound( gno );	/* determine hard clipping limit for eps */
    switch (g[gno].type) {
    case GRAPH_BAR:
	{
	    int tset = 0, maxn = 0;
	    double x1, y1, x2, y2, minx, miny, maxx, maxy;
	    double tdx, fmin(double x, double y), fmax(double x, double y);

	    minx = miny = maxx = maxy = 0.0;
	    for (i = 0; i < g[gno].maxplot; i++) {
		if (isactive_set(gno, i)) {
		    tset++;
		    maxn += getsetlength(gno, i);
		    getsetminmax(gno, i, &x1, &x2, &y1, &y2);
		    if (tset == 1) {
			minx = x1;
			miny = y1;
			maxx = x2;
			maxy = y2;
		    } else {
			minx = fmin(x1, minx);
			miny = fmin(y1, miny);
			maxx = fmax(x2, maxx);
			maxy = fmax(y2, maxy);
		    }
		}
	    }
	    tdx = maxx - minx;
	    if (tset != 0 && maxn != 0) {
		double offsx, offsy;
		double bsize = g[gno].barwid * tdx / maxn;
		double cset = -tset / 2.0;

		offsx = -cset * bsize - bsize / 2.0;
		offsy = 0.0;

		for (i = 0; i < g[gno].maxplot; i++) {

		    if (isactive_set(gno, i)) {
			drawsetbar(gno, i, cset, bsize);
			switch (dataset_type(gno, i)) {
			case SET_XYDX:
			case SET_XYDY:
			case SET_XYDXDX:
			case SET_XYDYDY:
			    drawseterrbar(gno, i, offsx, offsy);
			    break;
			case SET_XYHILO:
			case SET_XYBOXPLOT:
			case SET_XYBOX:
			    break;
			case SET_XYZ:
			    break;
			}
			cset += 1.0;
			offsx = -cset * bsize - bsize / 2.0;
			offsy = 0.0;
		    }
		}
	    }
	}
	break;
    case GRAPH_STACKEDBAR:
	{
	    int tset = 0, maxn = 0;
	    double x1, y1, x2, y2, minx, miny, maxx, maxy;
	    double tdx, fmin(double x, double y), fmax(double x, double y);

	    minx = miny = maxx = maxy = 0.0;
	    for (i = 0; i < g[gno].maxplot; i++) {
		if (isactive_set(gno, i)) {
		    tset++;
		    getsetminmax(gno, i, &x1, &x2, &y1, &y2);
		    if (tset == 1) {
			maxn = getsetlength(gno, i);
			minx = x1;
			miny = y1;
			maxx = x2;
			maxy = y2;
		    } else {
			maxn = (maxn < getsetlength(gno, i)) ? getsetlength(gno, i) : maxn;
			minx = fmin(x1, minx);
			miny = fmin(y1, miny);
			maxx = fmax(x2, maxx);
			maxy = fmax(y2, maxy);
		    }
		}
	    }
	    tdx = maxx - minx;
	    if (tset != 0 && maxn != 0) {
		double bsize = g[gno].sbarwid * tdx / maxn;

		drawsetstackedbar(gno, maxn, bsize);
	    }
	}
	break;
    case GRAPH_HBAR:
	{
	    int tset = 0, maxn = 0;
	    double x1, y1, x2, y2, minx, miny, maxx, maxy;
	    double tdy, fmin(double x, double y), fmax(double x, double y);

	    minx = miny = maxx = maxy = 0.0;
	    for (i = 0; i < g[gno].maxplot; i++) {
		if (isactive_set(gno, i)) {
		    tset++;
		    maxn += getsetlength(gno, i);
		    getsetminmax(gno, i, &x1, &x2, &y1, &y2);
		    if (tset == 1) {
			minx = x1;
			miny = y1;
			maxx = x2;
			maxy = y2;
		    } else {
			minx = fmin(x1, minx);
			miny = fmin(y1, miny);
			maxx = fmax(x2, maxx);
			maxy = fmax(y2, maxy);
		    }
		}
	    }
	    tdy = maxy - miny;
	    if (tset != 0 && maxn != 0) {
		double offsx, offsy;
		double bsize = g[gno].barwid * tdy / maxn;
		double cset = -tset / 2.0;

		offsy = -cset * bsize - bsize / 2.0;
		offsx = 0.0;

		for (i = 0; i < g[gno].maxplot; i++) {

		    if (isactive_set(gno, i)) {
			drawsethbar(gno, i, cset, bsize);
			switch (dataset_type(gno, i)) {
			case SET_XYDX:
			case SET_XYDY:
			case SET_XYDXDX:
			case SET_XYDYDY:
			    drawseterrbar(gno, i, offsx, offsy);
			    break;
			case SET_XYHILO:
			case SET_XYBOXPLOT:
			case SET_XYBOX:
			    break;
			}
			cset += 1.0;
			offsy = -cset * bsize - bsize / 2.0;
			offsx = 0.0;
		    }
		}
	    }
	}
	break;
    case GRAPH_STACKEDHBAR:
	{
	    int tset = 0, maxn = 0;
	    double x1, y1, x2, y2, minx, miny, maxx, maxy;
	    double tdy, fmin(double x, double y), fmax(double x, double y);

	    minx = miny = maxx = maxy = 0.0;
	    for (i = 0; i < g[gno].maxplot; i++) {
		if (isactive_set(gno, i)) {
		    tset++;
		    getsetminmax(gno, i, &x1, &x2, &y1, &y2);
		    if (tset == 1) {
			maxn = getsetlength(gno, i);
			minx = x1;
			miny = y1;
			maxx = x2;
			maxy = y2;
		    } else {
			maxn = (maxn < getsetlength(gno, i)) ? getsetlength(gno, i) : maxn;
			minx = fmin(x1, minx);
			miny = fmin(y1, miny);
			maxx = fmax(x2, maxx);
			maxy = fmax(y2, maxy);
		    }
		}
	    }
	    tdy = maxy - miny;
	    if (tset != 0 && maxn != 0) {
		double bsize = g[gno].sbarwid * tdy / maxn;

		drawsetstackedhbar(gno, maxn, bsize);
	    }
	}
	break;
    case GRAPH_POLAR:
	break;
    case GRAPH_STACKEDLINE:
	break;
    case GRAPH_LOGX:
    case GRAPH_LOGY:
    case GRAPH_LOGXY:
    case GRAPH_XY:
	for (i = 0; i < g[gno].maxplot; i++) {
	    get_graph_plotarr(gno, i, &p);
	    if (isactive_set(gno, i)) {
		switch (dataset_type(gno, i)) {
		case SET_XY:
		    if (p.fill) {
			drawsetfill(gno, &p);
		    }
		    drawsetxy(gno, &g[gno].p[i], i);
		    break;
		case SET_POLY:
		    if (p.fill) {
			drawsetfill(gno, &p);
		    }
		    drawsetpoly(gno, &g[gno].p[i], i);
		    break;
		case SET_XYDX:
		case SET_XYDY:
		case SET_XYDXDX:
		case SET_XYDYDY:
		case SET_XYDXDY:
		    if (p.fill) {
			drawsetfill(gno, &p);
		    }
		    drawseterrbar(gno, i, 0.0, 0.0);
		    drawsetxy(gno, &g[gno].p[i], i);
		    break;
		case SET_XYXX:
		    drawsetxy(gno, &g[gno].p[i], i);
		    break;
		case SET_XYYY:
		    drawsetxy(gno, &g[gno].p[i], i);
		    break;
		case SET_XYZ:
		    if (!density_flag) {
			drawsetxy(gno, &g[gno].p[i], i);
			drawval(&g[gno].p[i]);
		    } else {
			drawdensity(&g[gno].p[i]);
		    }
		    break;
		case SET_XYZW:
		    drawsetxy(gno, &g[gno].p[i], i);
		    break;
		case SET_XYRT:
		    drawsetxy(gno, &g[gno].p[i], i);
		    drawcirclexy(&g[gno].p[i]);
		    break;
		case SET_XYBOX:
		    drawboxcolor(&g[gno].p[i]);
		    break;
		case SET_XYBOXPLOT:
		    drawsetboxplot(gno, i);
		    break;
		case SET_XYHILO:
		    drawsethilo(&g[gno].p[i]);
		    break;
		case SET_XYUV:
		    drawsetxy(gno, &g[gno].p[i], i);
		    drawflow(gno, i, g[gno].vp);
		    break;
		case SET_XYSTRING:
		    drawsetxy(gno, &g[gno].p[i], i);
		    if (g[gno].p[i].s != NULL) {
			drawstring(&p);
		    }
		    break;
		}
	    }
	}
    }
    set_drawing_finished();
    setlinestyle(grdefaults.lines);
    setlinewidth(grdefaults.linew);
/*
 * draw any defined regions for this graph
 */
    for (i = 0; i < MAXREGION; i++) {
	if (rg[i].active && rg[i].linkto[gno]) {
	    draw_region(i);
	}
    }

    setlinestyle(grdefaults.lines);
    setlinewidth(grdefaults.linew);
    if (f.active) {
	boxplot(gno);
    }
    if (timestamp.active) {
	double xtmp, ytmp;
	setlinewidth(timestamp.linew);
	setcolor(timestamp.color);
	setcharsize(timestamp.charsize);
	setfont(timestamp.font);
	view2world(timestamp.x, timestamp.y, &xtmp, &ytmp);
	writestr(xtmp, ytmp, timestamp.rot, timestamp.just, timestamp.s);
    }
    if (leg.active) {
	dolegend(gno);
	setlinestyle(grdefaults.lines);
	setlinewidth(grdefaults.linew);
    }
}

void draw_ref_point(int gno)
{
    drawpolysym(&g[gno].dsx, &g[gno].dsy, 1, SYM_CIRCLE, 0, 0, 1.0);
    drawpolysym(&g[gno].dsx, &g[gno].dsy, 1, SYM_PLUS, 0, 0, 1.0);
    drawpolysym(&g[gno].dsx, &g[gno].dsy, 1, SYM_PLUS, 0, 0, 1.0);
}

void draw_annotation(int gno)
{
    int i;

    setclipping(0);		/* shut down clipping for strings, boxes,
				 * lines, and legends */
    if (debuglevel == 5) {
	printf("Boxes\n");
    }
    for (i = 0; i < maxboxes; i++) {
	if (isactive_box(i)) {
	    draw_box(gno, i);
	}
    }
    if (debuglevel == 5) {
	printf("Ellipses\n");
    }
    for (i = 0; i < maxboxes; i++) {
	if (isactive_ellipse(i)) {
	    draw_ellipse(gno, i);
	}
    }
    if (debuglevel == 5) {
	printf("Lines\n");
    }
    for (i = 0; i < maxlines; i++) {
	if (isactive_line(i)) {
	    draw_line(gno, i);
	}
    }
    if (debuglevel == 5) {
	printf("Strings\n");
    }
    for (i = 0; i < maxstr; i++) {
	if (isactive_string(i)) {
	    if (debuglevel == 5) {
		printf("String %d\n", i);
	    }
	    draw_string(gno, i);
	}
    }
    setclipping(1);
}

/*
 * draw the legend at (legx, legy)
 * ib = 1 in loop means accumulate info for box
 * ib = -1 after loop means draw box
 */
void dolegend(int gno)
{

    int i, j = 0, sy = 0, cy = 0, ly = 0, wy = 0;
    double tmpx, tmpy;
    legend l;
    plotarr p;

    get_graph_legend(gno, &l);

    if (l.loctype == COORD_VIEW) {
		view2world(l.legx, l.legy, &tmpx, &tmpy);
    } else {
		tmpx = l.legx;
		tmpy = l.legy;
    }
    j = 0;
    setcharsize(l.charsize);
    setfont(l.font);
    if (l.box == TRUE) {	/* compute a bounding box for the legend */
		for (i = 0; i < maxplot; i++) {
	    	if (isactive_set(gno, i) && (g[gno].p[i].lstr[0])) {
				get_graph_plotarr(gno, i, &p);
				putlegend(j, 1, l.len, l.vgap, p.symsize,
			  		tmpx, tmpy, sy, ly, cy, wy,
			  		g[gno].p[i].lstr, p.symfill, -1, -1, -1);
				j++;
	    	}
		}
		if( hdevice==9 || hdevice==10 || hdevice == 11 )
			set_leg_bb( gno, hardcopyflag );
		putlegendrect(l.boxfill,
		    	  l.boxfillusing == CLRFILLED,
		    	  l.boxfillcolor,
		    	  l.boxfillpat,
		    	  l.boxlcolor,
		    	  l.boxlinew,
		    	  l.boxlines,
		    	  g[gno].l.vgap );
    }
    j = 0;
    for (i = 0; i < maxplot; i++) {
	if (isactive_set(gno, i) && (g[gno].p[i].lstr[0])) {
	    setcolor(l.color);
	    setcharsize(l.charsize);
	    setfont(l.font);
	    get_graph_plotarr(gno, i, &p);
	    sy = p.sym;
	    ly = p.lines;
	    wy = p.linew;
	    cy = p.color;
	    switch (g[gno].type) {
	    case GRAPH_XY:
		putlegend(j,	/* set position num in legend */
			  0,	/* draw */
			  l.len,/* length of legend */
			  l.vgap,	/* vertical gap */
			  p.symsize,	/* sym size */
			  tmpx,	/* loc x, y */
			  tmpy,
			  sy,	/* symbol */
			  ly,	/* line style */
			  cy,	/* color */
			  wy,	/* width */
			  g[gno].p[i].lstr,	/* legend string */
			  p.symfill,	/* sym fill */
			  p.symcolor,	/* sym color */
			  p.symlinew,	/* sym line width */
			  p.symlines);	/* sym line style */
		break;
	    case GRAPH_BAR:
	    case GRAPH_HBAR:
	    case GRAPH_STACKEDBAR:
	    case GRAPH_STACKEDHBAR:
		putbarlegend(j, 0, l.len, l.vgap, p.symsize,
			     tmpx, tmpy, sy, ly, cy, wy, g[gno].p[i].lstr,
			     p.symfill, p.fill, p.fillusing==PTNFILLED,
			     	p.fillusing==PTNFILLED?p.fillpattern:p.fillcolor );
		break;
	    default:
		putlegend(j, 0, l.len, l.vgap, p.symsize,
			  tmpx, tmpy, sy, ly, cy, wy, g[gno].p[i].lstr,
			  p.symfill, p.symcolor, p.symlinew, p.symlines);
		break;
	    }
	    j++;
	}
    }
}

/*
 * draw the graph frame
 */
void boxplot(int gno)
{
    world w, wtmp;
    framep f;
    int c, s, wi;

    get_graph_world(gno, &wtmp);
    get_graph_framep(gno, &f);

    c = setcolor(f.color);
    s = setlinestyle(f.lines);
    wi = setlinewidth(f.linew);

    if (isxreversed(gno)) {
	w.xg2 = wtmp.xg1;
	w.xg1 = wtmp.xg2;
    } else {
	w.xg1 = wtmp.xg1;
	w.xg2 = wtmp.xg2;
    }

    if (isyreversed(gno)) {
	w.yg2 = wtmp.yg1;
	w.yg1 = wtmp.yg2;
    } else {
	w.yg1 = wtmp.yg1;
	w.yg2 = wtmp.yg2;
    }

    switch (f.type) {
    case 0:
	rect(w.xg1, w.yg1, w.xg2, w.yg2);
	break;
    case 1:			/* half open */
	my_move2(w.xg1, w.yg1);
	my_draw2(w.xg2, w.yg1);
	my_move2(w.xg1, w.yg1);
	my_draw2(w.xg1, w.yg2);
	break;
    case 2:			/* break top */
	my_move2(w.xg2, w.yg2);
	my_draw2(w.xg2, w.yg1);
	my_draw2(w.xg1, w.yg1);
	my_draw2(w.xg1, w.yg2);
	break;
    case 3:			/* break bottom */
	my_move2(w.xg1, w.yg1);
	my_draw2(w.xg1, w.yg2);
	my_draw2(w.xg2, w.yg2);
	my_draw2(w.xg2, w.yg1);
	break;
    case 4:			/* break left */
	my_move2(w.xg1, w.yg2);
	my_draw2(w.xg2, w.yg2);
	my_draw2(w.xg2, w.yg1);
	my_draw2(w.xg1, w.yg1);
	break;
    case 5:			/* break right */
	my_move2(w.xg2, w.yg2);
	my_draw2(w.xg1, w.yg2);
	my_draw2(w.xg1, w.yg1);
	my_draw2(w.xg2, w.yg1);
	break;
    }
    setcolor(c);
    setlinestyle(s);
    setlinewidth(wi);
}

/*
 * draw annotative text
 */
void draw_string(int gno, int i)
{
    double xtmp1, ytmp1;
    int f, c, w;
    double s;
    plotstr pstr;

    get_graph_string(i, &pstr);
    if (debuglevel == 5) {
	printf("String %d %s\n", i, pstr.s);
    }
    if (gno != -2) {
	if (pstr.loctype == COORD_WORLD && pstr.gno != gno) {
	    return;
	}
	if (pstr.loctype == COORD_VIEW && gno != -1) {
	    return;
	}
    }
    if (strlen(pstr.s) && (pstr.charsize > 0.0) && pstr.active) {
	c = setcolor(pstr.color);
	w = setlinewidth(pstr.linew);
	s = setcharsize(pstr.charsize);
	f = setfont(pstr.font);
	if (pstr.loctype == COORD_WORLD) {
	    writestr(pstr.x, pstr.y, pstr.rot, pstr.just, pstr.s);
	} else {
	    view2world(pstr.x, pstr.y, &xtmp1, &ytmp1);
	    writestr(xtmp1, ytmp1, pstr.rot, pstr.just, pstr.s);
	}
	(void) setcolor(c);
	(void) setlinewidth(w);
	(void) setcharsize(s);
	(void) setfont(f);
    }
}

/*
 * draw annotative boxes
 */
void draw_box(int gno, int i)
{
    double xtmp1, ytmp1;
    double xtmp2, ytmp2;
    int c, l, w;
    boxtype b;

    get_graph_box(i, &b);
    if (gno != -2) {
	if (b.loctype == COORD_WORLD && b.gno != gno) {
	    return;
	}
	if (b.loctype == COORD_VIEW && gno != -1) {
	    return;
	}
    }
    if (b.active) {
		setclipping(0);

		if (b.fill == CLRFILLED) {
	    	c = setcolor(b.fillcolor);
	    	if (b.loctype == COORD_WORLD) {
				fillrectcolor(b.x1, b.y1, b.x2, b.y2);
	    	} else {
				view2world(b.x1, b.y1, &xtmp1, &ytmp1);
				view2world(b.x2, b.y2, &xtmp2, &ytmp2);
				fillrectcolor(xtmp1, ytmp1, xtmp2, ytmp2);
	    	}
	    	setcolor(c);
		} else if (b.fill == PTNFILLED) {
	    	c = setpattern(b.fillpattern);
	    	if (b.loctype == COORD_WORLD) {
				fillrectpat(b.x1, b.y1, b.x2, b.y2);
	    	} else {
				view2world(b.x1, b.y1, &xtmp1, &ytmp1);
				view2world(b.x2, b.y2, &xtmp2, &ytmp2);
				fillrectpat(xtmp1, ytmp1, xtmp2, ytmp2);
	    	}
	    	setpattern(c);
		}
		c = setcolor(b.color);
		l = setlinestyle(b.lines);
		w = setlinewidth(b.linew);
		if (b.loctype == COORD_WORLD) {
	    	rect(b.x1, b.y1, b.x2, b.y2);
		} else {
	    	view2world(b.x1, b.y1, &xtmp1, &ytmp1);
	    	view2world(b.x2, b.y2, &xtmp2, &ytmp2);
	    	rect(xtmp1, ytmp1, xtmp2, ytmp2);
		}
		setclipping(1);
		setcolor(c);
		setlinewidth(w);
		setlinestyle(l);
    }
}

/* draw annotative ellipses */
void draw_ellipse(int gno, int i)
{
    double xtmp1, ytmp1;
    double xtmp2, ytmp2;
    int c = 0, l, w;
    ellipsetype b;
    int filling;

	b = ellip[i];
	
    if (gno != -2) {
		if ((b.loctype==COORD_WORLD && b.gno!=gno) || (b.loctype==COORD_VIEW && gno!=-1)) {
			return;
		}
    }
    if (b.active) {
		setclipping(0);
         
		if(b.fill == CLRFILLED || b.fill == PTNFILLED ) {
	    	     
	    	    if(b.fill == CLRFILLED) {
	    	   		c=setcolor(b.fillcolor);
	    	    	filling = CLRFILLED;
				} else {
	    	    	c=setpattern(b.fillpattern);
	    	     	filling = PTNFILLED;
	    	     }
	    	     	 
	    	     if (b.loctype == COORD_WORLD) {
 	    	     	 my_ellipse((b.x1+b.x2)/2, (b.y1+b.y2)/2, fabs(b.x1-b.x2)/2,
 	    	     				     fabs(b.y1-b.y2)/2, filling );
	    	     } else {
		     	 	view2world(b.x1, b.y1, &xtmp1, &ytmp1);
		     	 	view2world(b.x2, b.y2, &xtmp2, &ytmp2);
 	    	     	my_ellipse((xtmp1+xtmp2)/2, (ytmp1+ytmp2)/2,fabs(xtmp1-xtmp2)/2,
 	    	     				     fabs(ytmp1-ytmp2)/2, filling );
	    	     }
		}
		
		if(b.fill == CLRFILLED) {
	    	    setcolor(c);
		} else if(b.fill == PTNFILLED) {
	    	    setpattern(c);
	        }

		c = setcolor(b.color);
		l = setlinestyle(b.lines);
		w = setlinewidth(b.linew);
		filling = UNFILLED;
		if (b.loctype == COORD_WORLD) {
 	    	my_ellipse((b.x1+b.x2)/2, (b.y1+b.y2)/2, fabs(b.x1-b.x2)/2, 
 	    					fabs(b.y1-b.y2)/2, filling );
		} else {
	    	view2world(b.x1, b.y1, &xtmp1, &ytmp1);
	    	view2world(b.x2, b.y2, &xtmp2, &ytmp2);
 	    	my_ellipse((xtmp1+xtmp2)/2, (ytmp1+ytmp2)/2, fabs(xtmp1-xtmp2)/2, 
 	    					fabs(ytmp1-ytmp2)/2, filling );
		}
		setclipping(1);
		setcolor(c);
		setlinewidth(w);
		setlinestyle(l);
	}
}

/*
 * draw annotative lines
 */
void draw_line(int gno, int i)
{
    double xtmp1, ytmp1;
    double xtmp2, ytmp2;
    int c, ll, w;
    linetype l;

    get_graph_line(i, &l);
    if (gno != -2) {
	if (l.loctype == COORD_WORLD && l.gno != gno) {
	    return;
	}
	if (l.loctype == COORD_VIEW && gno != -1) {
	    return;
	}
    }
    if (l.active) {
	setclipping(0);
	c = setcolor(l.color);
	ll = setlinestyle(l.lines);
	w = setlinewidth(l.linew);
	if (l.loctype == COORD_WORLD) {
	    draw_arrow(l.x1, l.y1, l.x2, l.y2, l.arrow, l.asize, l.atype);
	} else {
	    view2world(l.x1, l.y1, &xtmp1, &ytmp1);
	    view2world(l.x2, l.y2, &xtmp2, &ytmp2);
	    draw_arrow(xtmp1, ytmp1, xtmp2, ytmp2, l.arrow, l.asize, l.atype);
	}
	setclipping(1);
	setcolor(c);
	setlinewidth(w);
	setlinestyle(ll);
    }
}

/*
 * draw a set with a fill
 */
void drawsetfill(int gno, plotarr *p)
{
    int i, c, len;
    double *x = p->ex[0], *y = p->ex[1], *xtmp, *ytmp;

    len = p->len + 2;
    xtmp = (double *) calloc(len, sizeof(double));
    ytmp = (double *) calloc(len, sizeof(double));
    if (xtmp == NULL || ytmp == NULL) {
	errmsg("Can't malloc for fills in plotone");
	cxfree(xtmp);
	cxfree(ytmp);
	return;
    }
    for (i = 0; i < p->len; i++) {
	xtmp[i] = x[i];
	ytmp[i] = y[i];
    }
    switch (p->fill) {
    case 1:
	len = p->len;
	break;
    case 2:
	xtmp[p->len] = (p->xmax > g[gno].w.xg2) ? g[gno].w.xg2 : p->xmax;
	ytmp[p->len] = 0.0;
	xtmp[p->len + 1] = (p->xmin < g[gno].w.xg1) ? g[gno].w.xg1 : p->xmin;
	ytmp[p->len + 1] = 0.0;
	break;
    case 3:
	xtmp[p->len] = 0.0;
	ytmp[p->len] = (p->ymax > g[gno].w.yg2) ? g[gno].w.yg2 : p->ymax;
	xtmp[p->len + 1] = 0.0;
	ytmp[p->len + 1] = (p->ymin < g[gno].w.yg1) ? g[gno].w.yg1 : p->ymin;
	break;
    case 4:
	xtmp[p->len] = g[gno].w.xg1;
	ytmp[p->len] = (p->ymax > g[gno].w.yg2) ? g[gno].w.yg2 : p->ymax;
	xtmp[p->len + 1] = g[gno].w.xg1;
	ytmp[p->len + 1] = (p->ymin < g[gno].w.yg1) ? g[gno].w.yg1 : p->ymin;
	break;
    case 5:
	xtmp[p->len] = g[gno].w.xg2;
	ytmp[p->len] = (p->ymax > g[gno].w.yg2) ? g[gno].w.yg2 : p->ymax;
	xtmp[p->len + 1] = g[gno].w.xg2;
	ytmp[p->len + 1] = (p->ymin < g[gno].w.yg1) ? g[gno].w.yg1 : p->ymin;
	break;
    case 6:			/* fill to ymin */
	xtmp[p->len] = (p->xmax > g[gno].w.xg2) ? g[gno].w.xg2 : p->xmax;
	ytmp[p->len] = g[gno].w.yg1;
	xtmp[p->len + 1] = (p->xmin < g[gno].w.xg1) ? g[gno].w.xg1 : p->xmin;
	ytmp[p->len + 1] = g[gno].w.yg1;
	break;
    case 7:
	xtmp[p->len] = (p->xmax > g[gno].w.xg2) ? g[gno].w.xg2 : p->xmax;
	ytmp[p->len] = g[gno].w.yg2;
	xtmp[p->len + 1] = (p->xmin < g[gno].w.xg1) ? g[gno].w.xg1 : p->xmin;
	ytmp[p->len + 1] = g[gno].w.yg2;
	break;
    }
    if (p->fillusing == CLRFILLED) {
	c = setcolor(p->fillcolor);
	fillcolor(len, xtmp, ytmp);
	setcolor(c);
    } else if (p->fillusing == PTNFILLED) {
	c = setcolor(1);
	setpattern(p->fillpattern);
	fillpattern(len, xtmp, ytmp);
	setcolor(c);
    }
    cxfree(xtmp);
    cxfree(ytmp);
}

/*
 * draw a standard set with symbols and lines
 */
void drawsetxy(int gno, plotarr *p, int setno)
{
    int j, c, l, w, cy = p->color, sy = p->sym, ly = p->lines, wy = p->linew;
    double xbar, sd, *x = p->ex[0], *y = p->ex[1];
    char s[256];
    int save = 0, skip = p->symskip+1;

    getlineprops(&c, &l, &w);
/*
printf("%d %d %d %d %d %d\n", c, l, w, cy, ly, wy);
*/
/* draw the line */
    if (wy && ly) {
	if (cy != c) {
	    setcolor(cy);
	}
	if (wy != w) {
	    setlinewidth(wy);
	}
	if (ly != l) {
	    setlinestyle(ly);
	}
	save = 1;
	drawpoly(x, y, p->len);
    }
/* draw the symbol */
    if (sy && p->symlinew != 0 && p->symlines != 0) {
	sd = setcharsize(p->symsize);
      
	if (symcolorbug || (p->symcolor == -1)) {
  	  setplotsymcolor(gno, setno, cy);
  	  p->symcolor = cy;
	}
	
	setcolor(p->symcolor);
	setlinewidth(p->symlinew);
	setlinestyle(p->symlines);
	save = 1;
	switch (sy) {
	case SYM_SEG:		/* draw segments */
	    drawpolyseg(x, y, p->len);
	    break;
	case SYM_CHAR:		/* draw character */
	    setfont(p->font);
	    if (p->symchar > ' ') {

		s[0] = p->symchar;
		s[1] = 0;
		for (j = 0; j < p->len; j += skip) {
		    if (symok(x[j], y[j])) {
			writestr(x[j], y[j], 0, 2, s);
		    }
		}
	    }
	    break;
	case SYM_HILOX:	/* draw hilo along X */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j += skip) {
		    my_move2(x[j], y[j]);
		    my_draw2(p->ex[2][j], y[j]);
		}
	    }
	    break;
	case SYM_HILOY:	/* draw hilo along Y */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j += skip) {
		    my_move2(x[j], y[j]);
		    my_draw2(x[j], p->ex[2][j]);
		}
	    }
	    break;
	case SYM_OPENCLOSEX:	/* draw open/close along X */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j += skip) {
		    openclose(y[j], x[j], p->ex[2][j], 1.0, 0);
		}
	    }
	    break;
	case SYM_OPENCLOSEY:	/* draw open/close along Y */
	    if (p->ex[2] != NULL) {
			for (j = 0; j < p->len; j += skip) {
		    	openclose(x[j], y[j], p->ex[2][j], 1.0, 1);
			}
	    }
	    break;
	case SYM_RANGE:	/* draw bounding box */
	    rect(p->xmin, p->ymin, p->xmax, p->ymax);
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(p->xmin, xbar);
	    my_draw2(p->xmax, xbar);
	    stasum(x, p->len, &xbar, &sd, 0);
	    my_move2(xbar, p->ymin);
	    my_draw2(xbar, p->ymax);
	    break;
	case SYM_TAG_FIRST:	/* tag the first point in a set */
	    if (symok(x[0], y[0])) {
			sprintf(s, "S%1d:1", setno);
			writestr(x[0], y[0], 0, 2, s);
	    }
	    break;
	case SYM_TAG_LAST:	/* tag the last point in a set */
	    if (symok(x[p->len - 1], y[p->len - 1])) {
			sprintf(s, "S%1d:%1d", setno, p->len);
			writestr(x[p->len - 1], y[p->len - 1], 0, 2, s);
	    }
	    break;
	case SYM_TAG_CENTER:	/* tag the middle point in a set */
	    if (symok(x[p->len / 2], y[p->len / 2])) {
		sprintf(s, "S%1d:%1d", setno, p->len / 2);
		writestr(x[p->len / 2], y[p->len / 2], 0, 2, s);
	    }
	    break;
	case SYM_STRING:	/* string at plot */
	    /* drawpolystring(x, y, p->len, sy, 0); */
	    break;
	case SYM_SETNO_IND:	/* set number and index */
	case SYM_SETNO:	/* set number */
	    for (j = 0; j < p->len; j += skip) {
		if (symok(x[j], y[j])) {
		    if (sy == SYM_SETNO) {
			sprintf(s, "S%d", setno);
		    } else {
			sprintf(s, "S%1d:%1d)", setno, j + 1);
		    }
		    writestr(x[j], y[j], 0, 0, s);
		}
	    }
	    break;
	case SYM_AVGY:		/* average Y */
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(g[gno].w.xg1, xbar);
	    my_draw2(g[gno].w.xg2, xbar);
	    break;
	case SYM_AVGSTDY1:
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(g[gno].w.xg1, xbar - sd);
	    my_draw2(g[gno].w.xg2, xbar - sd);
	    my_move2(g[gno].w.xg1, xbar + sd);
	    my_draw2(g[gno].w.xg2, xbar + sd);
	    break;
	case SYM_AVGSTDY2:
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(g[gno].w.xg1, xbar - 2.0 * sd);
	    my_draw2(g[gno].w.xg2, xbar - 2.0 * sd);
	    my_move2(g[gno].w.xg1, xbar + 2.0 * sd);
	    my_draw2(g[gno].w.xg2, xbar + 2.0 * sd);
	    break;
	case SYM_AVGSTDY3:
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(g[gno].w.xg1, xbar - 3.0 * sd);
	    my_draw2(g[gno].w.xg2, xbar - 3.0 * sd);
	    my_move2(g[gno].w.xg1, xbar + 3.0 * sd);
	    my_draw2(g[gno].w.xg2, xbar + 3.0 * sd);
	    break;
	default:
	    drawpolysym(x, y, p->len, sy, p->symskip, p->symfill, p->symsize);
	    break;
	}
	setcharsize(sd);
    }
    if (save) {
	setlinewidth(w);
	setlinestyle(l);
	setcolor(c);
    }
}

/*
 * draw a SET_POLY set with symbols and lines
 */
void drawsetpoly(int gno, plotarr *p, int setno)
{
    int j, c, l, w, cy = p->color, sy = p->sym, ly = p->lines, wy = p->linew;
    int start, cnt;
    double xbar, sd, *x = p->ex[0], *y = p->ex[1];
    double *flag;
    char s[256];
    int save = 0;
    getlineprops(&c, &l, &w);
/* draw the line */
    if (p->len > 1 && wy && ly) {
	if (cy != c) {
	    setcolor(cy);
	}
	if (wy != w) {
	    setlinewidth(wy);
	}
	if (ly != l) {
	    setlinestyle(ly);
	}
	save = 1;
	start = 0;
	flag = getcol(gno, setno, 2);
	do {
	    cnt = 0;
	    for (j = start; j < p->len; j++) {
		if ((int) flag[j] == 1) {
		    cnt++;
		} else {
		    cnt++;
		    break;
		}
	    }
	    if (cnt > 1) {
		drawpoly(x + start, y + start, cnt);
	    }
	    start += cnt;
	} while (start < p->len);
    }
/* draw the symbol */
    if (sy && p->symlinew != 0 && p->symlines != 0) {
	sd = setcharsize(p->symsize);

	if (symcolorbug || (p->symcolor == -1)) {
  	  setplotsymcolor(gno, setno, cy);
  	  p->symcolor = cy;
	}

	setcolor(p->symcolor);
	setlinewidth(p->symlinew);
	setlinestyle(p->symlines);
	save = 1;
	switch (sy) {
	case SYM_SEG:		/* draw segments */
	    drawpolyseg(x, y, p->len);
	    break;
	case SYM_CHAR:		/* draw character */
	    setfont(p->font);
	    if (p->symchar > ' ') {
		int skip = p->symskip + 1;

		s[0] = p->symchar;
		s[1] = 0;
		for (j = 0; j < p->len; j += skip) {
		    if (symok(x[j], y[j])) {
			writestr(x[j], y[j], 0, 2, s);
		    }
		}
	    }
	    break;
	case SYM_HILOX:	/* draw hilo along X */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j++) {
		    my_move2(x[j], y[j]);
		    my_draw2(p->ex[2][j], y[j]);
		}
	    }
	    break;
	case SYM_HILOY:	/* draw hilo along Y */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j++) {
		    my_move2(x[j], y[j]);
		    my_draw2(x[j], p->ex[2][j]);
		}
	    }
	    break;
	case SYM_OPENCLOSEX:	/* draw open/close along X */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j++) {
		    openclose(y[j], x[j], p->ex[2][j], 1.0, 0);
		}
	    }
	    break;
	case SYM_OPENCLOSEY:	/* draw open/close along Y */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j++) {
		    openclose(x[j], y[j], p->ex[2][j], 1.0, 1);
		}
	    }
	    break;
	case SYM_RANGE:	/* draw bounding box */
	    rect(p->xmin, p->ymin, p->xmax, p->ymax);
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(p->xmin, xbar);
	    my_draw2(p->xmax, xbar);
	    stasum(x, p->len, &xbar, &sd, 0);
	    my_move2(xbar, p->ymin);
	    my_draw2(xbar, p->ymax);
	    break;
	case SYM_TAG_FIRST:	/* tag the first point in a set */
	    if (symok(x[0], y[0])) {
		sprintf(s, "S%1d:1", setno);
		writestr(x[0], y[0], 0, 2, s);
	    }
	    break;
	case SYM_TAG_LAST:	/* tag the last point in a set */
	    if (symok(x[p->len - 1], y[p->len - 1])) {
		sprintf(s, "S%1d:%1d", setno, p->len);
		writestr(x[p->len - 1], y[p->len - 1], 0, 2, s);
	    }
	    break;
	case SYM_TAG_CENTER:	/* tag the middle point in a set */
	    if (symok(x[p->len / 2], y[p->len / 2])) {
		sprintf(s, "S%1d:%1d", setno, p->len / 2);
		writestr(x[p->len / 2], y[p->len / 2], 0, 2, s);
	    }
	    break;
	case SYM_STRING:	/* string at plot */
	    /* drawpolystring(x, y, p->len, sy, 0); */
	    break;
	case SYM_SETNO_IND:	/* set number and index */
	case SYM_SETNO:	/* set number */
	    for (j = 0; j < p->len; j++) {
		if (symok(x[j], y[j])) {
		    if (sy == SYM_SETNO) {
			sprintf(s, "S%d", setno);
		    } else {
			sprintf(s, "S%1d:%1d)", setno, j + 1);
		    }
		    writestr(x[j], y[j], 0, 0, s);
		}
	    }
	    break;
	case SYM_AVGY:		/* average Y */
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(g[gno].w.xg1, xbar);
	    my_draw2(g[gno].w.xg2, xbar);
	    break;
	case SYM_AVGSTDY1:
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(g[gno].w.xg1, xbar - sd);
	    my_draw2(g[gno].w.xg2, xbar - sd);
	    my_move2(g[gno].w.xg1, xbar + sd);
	    my_draw2(g[gno].w.xg2, xbar + sd);
	    break;
	case SYM_AVGSTDY2:
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(g[gno].w.xg1, xbar - 2.0 * sd);
	    my_draw2(g[gno].w.xg2, xbar - 2.0 * sd);
	    my_move2(g[gno].w.xg1, xbar + 2.0 * sd);
	    my_draw2(g[gno].w.xg2, xbar + 2.0 * sd);
	    break;
	case SYM_AVGSTDY3:
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(g[gno].w.xg1, xbar - 3.0 * sd);
	    my_draw2(g[gno].w.xg2, xbar - 3.0 * sd);
	    my_move2(g[gno].w.xg1, xbar + 3.0 * sd);
	    my_draw2(g[gno].w.xg2, xbar + 3.0 * sd);
	    break;
	default:
	    drawpolysym(x, y, p->len, sy, p->symskip, p->symfill, p->symsize);
	    break;
	}
	setcharsize(sd);
    }
    if (save) {
	setlinewidth(w);
	setlinestyle(l);
	setcolor(c);
    }
}

/*
 * draw hi/lo-open/close
 */
void drawsethilo(plotarr *p)
{
    int i, c, w, cy = p->color, wy = p->linew;
    double *x = p->ex[0], *y = p->ex[1], *dx = p->ex[2], *dy = p->ex[3],
    *dz = p->ex[4];

    c = setcolor(cy);
    w = setlinewidth(wy);
    for (i = 0; i < p->len; i++) {
	my_move2(x[i], y[i]);
	my_draw2(x[i], dx[i]);
	openclose(x[i], dy[i], dz[i], 1.0, 1);
    }
    setlinewidth(w);
    setcolor(c);
}

/*
 * draw a value as a string at x, y
 */
void drawval(plotarr *p)
{
    int j;
    double *x = p->ex[0], *y = p->ex[1], *z = p->ex[2], loc, ss;
    char *s1, s2[256];

    s2[0] = ' ';
    s1 = &s2[1];
    ss = setcharsize(p->symsize);
    setfont(p->font);
    for (j = 0; j < p->len; j++) {
	if (symok(x[j], y[j])) {
	    loc = z[j];
	    create_ticklabel(p->format, p->prec, loc, s1);
	    if (p->sym) {
		writestr(x[j], y[j], 0, 0, s1 - 1);
	    } else {
		writestr(x[j], y[j], 0, 2, s1);
	    }
	}
    }
    setcharsize(ss);
}

/*
 * draw a string at x, y
 */
void drawstring(plotarr *p)
{
    int j;
    double *x = p->ex[0], *y = p->ex[1], ss;
    char *s1, s2[256];

    s2[0] = ' ';
    s1 = &s2[1];
    ss = setcharsize(p->symsize);
    setfont(p->font);
    for (j = 0; j < p->len; j++) {
	if (symok(x[j], y[j])) {
	    if (p->s[j] != NULL) {
		strcpy(s1, p->s[j]);
		if (p->sym) {
		    writestr(x[j], y[j], 0, 0, s1 - 1);
		} else {
		    writestr(x[j], y[j], 0, 2, s1);
		}
	    }
	}
    }
    setcharsize(ss);
}

/*
 * draw a density plot
 */
void drawdensity(plotarr *p)
{
    int j, c, w;
    double *x = p->ex[0], *y = p->ex[1], *z = p->ex[2], loc;

    c = setcolor(p->color);
    w = setlinewidth(p->linew);
    for (j = 0; j < p->len; j++) {
	if (symok(x[j], y[j])) {
	    loc = z[j];
	    if ((p->zmin == p->zmax) || (loc >= p->zmin && loc <= p->zmax)) {
		drawpolysym(&x[j], &y[j], 1, p->sym, p->symskip, p->symfill, z[j]);
	    }
	}
    }
    setcolor(c);
    setlinewidth(w);
}

/*
 * draw a rectangle with color
 */
void drawboxcolor(plotarr *p)
{
    int j, c, w, cset;
    double *x1 = p->ex[0], *y1 = p->ex[1];
    double *x2 = p->ex[2], *y2 = p->ex[3], *z = p->ex[4];

    c = setcolor(p->color);
    w = setlinewidth(p->linew);
    if (p->fill) {
	for (j = 0; j < p->len; j++) {
	    cset = (int) z[j];
	    setcolor(cset);
	    fillrectcolor(x1[j], y1[j], x2[j], y2[j]);
	}
    }
    setcolor(c);
    if (p->linew > 0 && p->lines > 0) {
	for (j = 0; j < p->len; j++) {
	    rect(x1[j], y1[j], x2[j], y2[j]);
	}
    }
    setcolor(c);
    setlinewidth(w);
}


/*
 * draw a circle centered at x, y with radius z
 */
void drawcirclexy(plotarr *p)
{
    int j;
    int cy = p->color, wy = p->linew;
    double *x = p->ex[0], *y = p->ex[1], *z = p->ex[2];

    cy = setcolor(cy);
    wy = setlinewidth(wy);
    for (j = 0; j < p->len; j++) {
	if (symok(x[j], y[j])) {
	    drawcircle(x[j], y[j], z[j], 0);
	}
    }
    setcolor(cy);
    setlinewidth(wy);
}

/*
 * draw a set in a bar chart
 */
void drawsetbar(int gno, int setno, double cset, double bsize)
{
    int i, j;
    int c = 0, l, w, p = 0;
    int cc = g[gno].p[setno].color;
    int cy = g[gno].p[setno].fillcolor;
    int py = g[gno].p[setno].fillpattern;
    int ly = g[gno].p[setno].lines, wy = g[gno].p[setno].linew;
    double *x = getx(gno, setno), *y = gety(gno, setno);
    double tmpx[4];
    double tmpy[4];

	if (g[gno].p[setno].fillusing == CLRFILLED) 
	    c = setcolor(cy);
	else if (g[gno].p[setno].fillusing == PTNFILLED) 
	    p = setpattern(py);
    l = setlinestyle(ly);
    w = setlinewidth(wy);
    if (g[gno].p[setno].fill) {
	for (i = 0; i < g[gno].p[setno].len; i++) {
	    tmpx[0] = x[i] + cset * bsize;
	    tmpy[0] = 0.0;
	    tmpx[1] = x[i] + cset * bsize;
	    tmpy[1] = y[i];
	    tmpx[2] = x[i] + (cset + 1.0) * bsize;
	    tmpy[2] = y[i];
	    tmpx[3] = x[i] + (cset + 1.0) * bsize;
	    tmpy[3] = 0.0;
	    if (tmpx[0] > g[gno].w.xg2) {
		continue;
	    }
	    if (tmpx[2] < g[gno].w.xg1) {
		continue;
	    }
	    for (j = 0; j < 4; j++) {
		if (tmpx[j] < g[gno].w.xg1)
		    tmpx[j] = g[gno].w.xg1;
		else if (tmpx[j] > g[gno].w.xg2)
		    tmpx[j] = g[gno].w.xg2;
		if (tmpy[j] < g[gno].w.yg1)
		    tmpy[j] = g[gno].w.yg1;
		else if (tmpy[j] > g[gno].w.yg2)
		    tmpy[j] = g[gno].w.yg2;
	    }
	    if (g[gno].p[setno].fillusing == PTNFILLED) {
		fillrectpat(tmpx[0], tmpy[0], tmpx[2], tmpy[2]);
	    } else if (g[gno].p[setno].fillusing == CLRFILLED) {
		fillrectcolor(tmpx[0], tmpy[0], tmpx[2], tmpy[2]);
	    }
	    if (dataset_type(gno, setno) == SET_XYZ) {
		int j;
		double *x = g[gno].p[setno].ex[0], *y = g[gno].p[setno].ex[1],
		*z = g[gno].p[setno].ex[2], loc, ss;
		char *s1, s2[256];

		s2[0] = ' ';
		s1 = &s2[1];
		ss = setcharsize(g[gno].p[setno].symsize);
		setfont(g[gno].p[setno].font);
		for (j = 0; j < g[gno].p[setno].len; j++) {
		    if (symok(x[j], y[j])) {
			loc = z[j];
			create_ticklabel(g[gno].p[setno].format, g[gno].p[setno].prec, loc, s1);
			if (g[gno].p[setno].sym) {
			    writestr(x[j] + (cset + 0.5) * bsize * 0.5, y[j], 0, 0, s1 - 1);
			} else {
			    writestr(x[j] + (cset + 0.5) * bsize * 0.5, y[j], 0, 2, s1);
			}
		    }
		}
		setcharsize(ss);
	    }
	}
    }
    if (ly && wy) {
	for (i = 0; i < g[gno].p[setno].len; i++) {
	    tmpx[0] = x[i] + cset * bsize;
	    tmpy[0] = 0.0;
	    tmpx[1] = x[i] + cset * bsize;
	    tmpy[1] = y[i];
	    tmpx[2] = x[i] + (cset + 1.0) * bsize;
	    tmpy[2] = y[i];
	    tmpx[3] = x[i] + (cset + 1.0) * bsize;
	    tmpy[3] = 0.0;
	    setcolor(cc);
	    setlinestyle(ly);
	    setlinewidth(wy);
	    my_move2(tmpx[0], tmpy[0]);
	    for (j = 0; j < 4; j++) {
		my_draw2(tmpx[(j + 1) % 4], tmpy[(j + 1) % 4]);
	    }
	}
    }
    setlinestyle(l);
    setlinewidth(w);
    setcolor(c);
    setpattern(p);
}

/*
 * draw a set in a horizontal bar chart
 */
void drawsethbar(int gno, int setno, double cset, double bsize)
{
    int i, j;
    int c, l, w, p;
    int cc = g[gno].p[setno].color;
    int cy = g[gno].p[setno].fillcolor;
    int py = g[gno].p[setno].fillpattern;
    int ly = g[gno].p[setno].lines, wy = g[gno].p[setno].linew;
    double *x = getx(gno, setno), *y = gety(gno, setno);
    double tmpx[4];
    double tmpy[4];

    c = setcolor(cy);
    p = setpattern(py);
    l = setlinestyle(ly);
    w = setlinewidth(wy);
    if (g[gno].p[setno].fill) {
	for (i = 0; i < g[gno].p[setno].len; i++) {
	    tmpy[0] = y[i] + cset * bsize;
	    tmpx[0] = 0.0;
	    tmpy[1] = y[i] + cset * bsize;
	    tmpx[1] = x[i];
	    tmpy[2] = y[i] + (cset + 1.0) * bsize;
	    tmpx[2] = x[i];
	    tmpy[3] = y[i] + (cset + 1.0) * bsize;
	    tmpx[3] = 0.0;
	    if (tmpy[0] > g[gno].w.yg2) {
		continue;
	    }
	    if (tmpy[2] < g[gno].w.yg1) {
		continue;
	    }
	    for (j = 0; j < 4; j++) {
		if (tmpy[j] < g[gno].w.yg1)
		    tmpy[j] = g[gno].w.yg1;
		else if (tmpy[j] > g[gno].w.yg2)
		    tmpy[j] = g[gno].w.yg2;
		if (tmpx[j] < g[gno].w.xg1)
		    tmpx[j] = g[gno].w.xg1;
		else if (tmpx[j] > g[gno].w.xg2)
		    tmpx[j] = g[gno].w.xg2;
	    }
	    if (g[gno].p[setno].fillusing == PTNFILLED) {
		fillrectpat(tmpx[0], tmpy[0], tmpx[2], tmpy[2]);
	    } else if (g[gno].p[setno].fillusing == CLRFILLED) {
		fillrectcolor(tmpx[0], tmpy[0], tmpx[2], tmpy[2]);
	    }
	}
    }
    if (ly && wy) {
	for (i = 0; i < g[gno].p[setno].len; i++) {
	    tmpy[0] = y[i] + cset * bsize;
	    tmpx[0] = 0.0;
	    tmpy[1] = y[i] + cset * bsize;
	    tmpx[1] = x[i];
	    tmpy[2] = y[i] + (cset + 1.0) * bsize;
	    tmpx[2] = x[i];
	    tmpy[3] = y[i] + (cset + 1.0) * bsize;
	    tmpx[3] = 0.0;
	    setcolor(cc);
	    setlinestyle(ly);
	    setlinewidth(wy);
	    my_move2(tmpx[0], tmpy[0]);
	    for (j = 0; j < 4; j++) {
		my_draw2(tmpx[(j + 1) % 4], tmpy[(j + 1) % 4]);
	    }
	}
    }
    setlinestyle(l);
    setlinewidth(w);
    setcolor(c);
    setpattern(p);
}

/*
 * draw a set in a stacked bar chart
 */
void drawsetstackedbar(int gno, int maxn, double bsize)
{
    int i, j, k;
    int c, l, w, p;
    int cc, cy, py, ly, wy;
    double *x, *y;
    double tmpx[4];
    double tmpy[4];
    double *sum = (double *) calloc(maxn, sizeof(double));

    if (sum == NULL) {
	errmsg("Can't calloc in drawsetstackedbar\n");
	return;
    }
    for (i = 0; i < g[gno].maxplot; i++) {
	if (isactive_set(gno, i)) {
	    x = getx(gno, i);
	    y = gety(gno, i);
	    cc = g[gno].p[i].color;
	    cy = g[gno].p[i].fillcolor;
	    py = g[gno].p[i].fillpattern;
	    ly = g[gno].p[i].lines;
	    wy = g[gno].p[i].linew;
	    c = setcolor(cy);
	    p = setpattern(py);
	    l = setlinestyle(ly);
	    w = setlinewidth(wy);
	    for (j = 0; j < maxn; j++) {
		if (j < getsetlength(gno, i)) {
		    if (g[gno].p[i].fill) {
			c = setcolor(cy);
			tmpx[0] = x[j] - bsize * 0.5;
			tmpy[0] = sum[j];
			tmpx[1] = x[j] - bsize * 0.5;
			tmpy[1] = sum[j] + y[j];
			tmpx[2] = x[j] + bsize * 0.5;
			tmpy[2] = sum[j] + y[j];
			tmpx[3] = x[j] + bsize * 0.5;
			tmpy[3] = sum[j];
			if (tmpx[0] > g[gno].w.xg2) {
			    continue;
			}
			if (tmpx[2] < g[gno].w.xg1) {
			    continue;
			}
			for (k = 0; k < 4; k++) {
			    if (tmpx[k] < g[gno].w.xg1)
				tmpx[k] = g[gno].w.xg1;
			    else if (tmpx[k] > g[gno].w.xg2)
				tmpx[k] = g[gno].w.xg2;
			    if (tmpy[k] < g[gno].w.yg1)
				tmpy[k] = g[gno].w.yg1;
			    else if (tmpy[k] > g[gno].w.yg2)
				tmpy[k] = g[gno].w.yg2;
			}
			if (g[gno].p[i].fillusing == PTNFILLED) {
			    fillrectpat(tmpx[0], tmpy[0], tmpx[2], tmpy[2]);
			} else if (g[gno].p[i].fillusing == CLRFILLED) {
			    fillrectcolor(tmpx[0], tmpy[0], tmpx[2], tmpy[2]);
			}
		    }
		    if (ly && wy) {
			tmpx[0] = x[j] - bsize * 0.5;
			tmpy[0] = sum[j];
			tmpx[1] = x[j] - bsize * 0.5;
			tmpy[1] = sum[j] + y[j];
			tmpx[2] = x[j] + bsize * 0.5;
			tmpy[2] = sum[j] + y[j];
			tmpx[3] = x[j] + bsize * 0.5;
			tmpy[3] = sum[j];
			setcolor(cc);
			my_move2(tmpx[0], tmpy[0]);
			for (k = 0; k < 4; k++) {
			    my_draw2(tmpx[(k + 1) % 4], tmpy[(k + 1) % 4]);
			}
		    }
		    sum[j] += y[j];
		}
	    }
	}
    }
    free(sum);
}

/*
 * draw a set in a horizontal stacked bar chart
 */
void drawsetstackedhbar(int gno, int maxn, double bsize)
{
    int i, j, k;
    int c, l, w, p;
    int cc, cy, py, ly, wy;
    double *x, *y;
    double tmpx[4];
    double tmpy[4];
    double *sum = (double *) calloc(maxn, sizeof(double));

    if (sum == NULL) {
	errmsg("Can't calloc in drawsetstackedbar\n");
	return;
    }
    for (i = 0; i < g[gno].maxplot; i++) {
	if (isactive_set(gno, i)) {
	    x = getx(gno, i);
	    y = gety(gno, i);
	    cc = g[gno].p[i].color;
	    cy = g[gno].p[i].fillcolor;
	    py = g[gno].p[i].fillpattern;
	    ly = g[gno].p[i].lines;
	    wy = g[gno].p[i].linew;
	    c = setcolor(cy);
	    p = setpattern(py);
	    l = setlinestyle(ly);
	    w = setlinewidth(wy);
	    for (j = 0; j < maxn; j++) {
		if (j < getsetlength(gno, i)) {
		    if (g[gno].p[i].fill) {
			c = setcolor(cy);
			tmpy[0] = y[j] - bsize * 0.5;
			tmpx[0] = sum[j];
			tmpy[1] = y[j] - bsize * 0.5;
			tmpx[1] = sum[j] + x[j];
			tmpy[2] = y[j] + bsize * 0.5;
			tmpx[2] = sum[j] + x[j];
			tmpy[3] = y[j] + bsize * 0.5;
			tmpx[3] = sum[j];
			if (tmpy[0] > g[gno].w.yg2) {
			    continue;
			}
			if (tmpy[2] < g[gno].w.yg1) {
			    continue;
			}
			for (k = 0; k < 4; k++) {
			    if (tmpy[k] < g[gno].w.yg1)
				tmpy[k] = g[gno].w.yg1;
			    else if (tmpy[k] > g[gno].w.yg2)
				tmpy[k] = g[gno].w.yg2;
			    if (tmpx[k] < g[gno].w.xg1)
				tmpx[k] = g[gno].w.xg1;
			    else if (tmpx[k] > g[gno].w.xg2)
				tmpx[k] = g[gno].w.xg2;
			}
			if (g[gno].p[i].fillusing == PTNFILLED) {
			    fillrectpat(tmpx[0], tmpy[0], tmpx[2], tmpy[2]);
			} else if (g[gno].p[i].fillusing == CLRFILLED) {
			    fillrectcolor(tmpx[0], tmpy[0], tmpx[2], tmpy[2]);
			}
		    }
		    if (ly && wy) {
			tmpy[0] = y[j] - bsize * 0.5;
			tmpx[0] = sum[j];
			tmpy[1] = y[j] - bsize * 0.5;
			tmpx[1] = sum[j] + x[j];
			tmpy[2] = y[j] + bsize * 0.5;
			tmpx[2] = sum[j] + x[j];
			tmpy[3] = y[j] + bsize * 0.5;
			tmpx[3] = sum[j];
			setcolor(cc);
			my_move2(tmpx[0], tmpy[0]);
			for (k = 0; k < 4; k++) {
			    my_draw2(tmpx[(k + 1) % 4], tmpy[(k + 1) % 4]);
			}
		    }
		    sum[j] += x[j];
		}
	    }
	}
    }
    free(sum);
}

/*
 * draw a set with error bars
 */
void drawseterrbar(int gno, int setno, double offsx, double offsy)
{
    int i, n = getsetlength(gno, setno);
    double *x = getx(gno, setno), *y = gety(gno, setno);
    double *dx = NULL, *dy = NULL;
    double ebarlen = g[gno].p[setno].errbarper;
    int etype = g[gno].p[setno].errbarxy;
    int c, w, l;
    int cy = g[gno].p[setno].color;
    int wy = g[gno].p[setno].linew;
    int ly = g[gno].p[setno].lines;

    c = setcolor(cy);
    l = setlinestyle(ly);
    w = setlinewidth(wy);

    switch (dataset_type(gno, setno)) {
    case SET_XYDX:
    case SET_XYDY:
	dx = getcol(gno, setno, 2);
	dy = getcol(gno, setno, 2);
	break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
	dx = getcol(gno, setno, 2);
	dy = getcol(gno, setno, 3);
	break;
    }

/*
 * draw the riser
 */
    if (g[gno].p[setno].errbar_riser) {
	setlinestyle(g[gno].p[setno].errbar_riser_lines);
	setlinewidth(g[gno].p[setno].errbar_riser_linew);
	for (i = 0; i < n; i++) {
	    switch (dataset_type(gno, setno)) {
	    case SET_XYDY:
	    case SET_XYDYDY:
		switch (etype) {
		case PLACE_BOTH:
		    my_move2(x[i] - offsx, y[i] - dy[i]);
		    my_draw2(x[i] - offsx, y[i] + dx[i]);
		    break;
		case PLACE_TOP:
		    my_move2(x[i] - offsx, y[i]);
		    my_draw2(x[i] - offsx, y[i] + dx[i]);
		    break;
		case PLACE_BOTTOM:
		    my_move2(x[i] - offsx, y[i] - dy[i]);
		    my_draw2(x[i] - offsx, y[i]);
		    break;
		}
		break;
	    case SET_XYDX:
	    case SET_XYDXDX:
		switch (etype) {
		case PLACE_BOTH:
		    my_move2(x[i] - dy[i], y[i] - offsy);
		    my_draw2(x[i] + dx[i], y[i] - offsy);
		    break;
		case PLACE_LEFT:
		    my_move2(x[i] - dy[i], y[i] - offsy);
		    my_draw2(x[i], y[i] - offsy);
		    break;
		case PLACE_RIGHT:
		    my_move2(x[i], y[i] - offsy);
		    my_draw2(x[i] + dx[i], y[i] - offsy);
		    break;
		}
		break;
	    case SET_XYDXDY:
		switch (etype) {
		case PLACE_BOTH:
		    my_move2(x[i] - dx[i], y[i] - offsy);
		    my_draw2(x[i] + dx[i], y[i] - offsy);
		    my_move2(x[i] - offsx, y[i] - dy[i]);
		    my_draw2(x[i] - offsx, y[i] + dy[i]);
		    break;
		case PLACE_LEFT:
		    my_move2(x[i] - dx[i], y[i] - offsy);
		    my_draw2(x[i], y[i] - offsy);
		    my_move2(x[i] - offsx, y[i] - dy[i]);
		    my_draw2(x[i] - offsx, y[i]);
		    break;
		case PLACE_RIGHT:
		    my_move2(x[i] + dx[i], y[i] - offsy);
		    my_draw2(x[i], y[i] - offsy);
		    my_move2(x[i] - offsx, y[i] + dy[i]);
		    my_draw2(x[i] - offsx, y[i]);
		    break;
		}
		break;
	    }
	}
    }
/*
 * draw the bar
 */
    setlinestyle(g[gno].p[setno].errbar_lines);
    setlinewidth(g[gno].p[setno].errbar_linew);
    for (i = 0; i < n; i++) {
	switch (dataset_type(gno, setno)) {
	case SET_XYDY:
	case SET_XYDYDY:
	    switch (etype) {
	    case PLACE_BOTH:
		errorbar(x[i] - offsx, y[i] - dy[i], ebarlen, 1);
		errorbar(x[i] - offsx, y[i] + dx[i], ebarlen, 1);
		break;
	    case PLACE_TOP:
		errorbar(x[i] - offsx, y[i] + dx[i], ebarlen, 1);
		break;
	    case PLACE_BOTTOM:
		errorbar(x[i] - offsx, y[i] - dy[i], ebarlen, 1);
		break;
	    }
	    break;
	case SET_XYDX:
	case SET_XYDXDX:
	    switch (etype) {
	    case PLACE_BOTH:
		errorbar(x[i] - dy[i], y[i] - offsy, ebarlen, 0);
		errorbar(x[i] + dx[i], y[i] - offsy, ebarlen, 0);
		break;
	    case PLACE_LEFT:
		errorbar(x[i] - dy[i], y[i] - offsy, ebarlen, 0);
		break;
	    case PLACE_RIGHT:
		errorbar(x[i] + dx[i], y[i] - offsy, ebarlen, 0);
		break;
	    }
	    break;
	case SET_XYDXDY:
	    switch (etype) {
	    case PLACE_BOTH:
		errorbar(x[i] + dx[i], y[i] - offsy, ebarlen, 0);
		errorbar(x[i] - dx[i], y[i] - offsy, ebarlen, 0);
		errorbar(x[i] - offsx, y[i] - dy[i], ebarlen, 1);
		errorbar(x[i] - offsx, y[i] + dy[i], ebarlen, 1);
		break;
	    case PLACE_LEFT:
		errorbar(x[i] - dx[i], y[i] - offsy, ebarlen, 0);
		errorbar(x[i] - offsx, y[i] - dy[i], ebarlen, 1);
		break;
	    case PLACE_RIGHT:
		errorbar(x[i] + dx[i], y[i] - offsy, ebarlen, 0);
		errorbar(x[i] - offsx, y[i] + dy[i], ebarlen, 1);
		break;
	    }
	    break;
	}
    }
    setlinewidth(w);
    setlinestyle(l);
    setcolor(c);
}

/*
 * draw each point in a set as a boxplot
 *
 * e[1] is the median,
 * e[2] is the lower inner value
 * e[3] is the upper inner value
 * e[4] is the lower whisker
 * e[5] is the upper whisker
 */
void drawsetboxplot(int gno, int setno)
{
    int i, n = getsetlength(gno, setno);
    double *x, *med;
    double *il, *iu;
    double *ol, *ou;
    int c, w, l;
    int cy = g[gno].p[setno].color;
    int wy = g[gno].p[setno].linew;
    int ly = g[gno].p[setno].lines;

    c = setcolor(cy);
    l = setlinestyle(ly);
    w = setlinewidth(wy);

    x = getcol(gno, setno, 0);
    med = getcol(gno, setno, 1);
    il = getcol(gno, setno, 2);
    iu = getcol(gno, setno, 3);
    ol = getcol(gno, setno, 4);
    ou = getcol(gno, setno, 5);
    for (i = 0; i < n; i++) {
	boxplotsym(x[i], med[i], il[i], iu[i], ol[i], ou[i], g[gno].p[setno].errbarper);
    }
    setlinewidth(w);
    setlinestyle(l);
    setcolor(c);
}

void drawflow(int gno, int setno, velocityp vp)
{
    int i, c, l, w, cy = vp.color, ly = vp.lines, wy = vp.linew;
    double *x = getx(gno, setno), *y = gety(gno, setno);
    double *u, *v;

    u = getcol(gno, setno, 2);
    v = getcol(gno, setno, 3);
    if (u == NULL || v == NULL) {
	return;
    }
    c = setcolor(cy);
    l = setlinestyle(ly);
    w = setlinewidth(wy);
    for (i = 0; i < getsetlength(gno, setno); i++) {
	if (symok(x[i], y[i])) {
	    velplt(x[i], y[i], u[i], v[i], vp.vscale, vp.arrowtype);
	}
    }
    setlinestyle(l);
    setlinewidth(w);
    setcolor(c);
}

/*
 * draw a standard set with symbols and lines in a polar plot
 */
void drawsetxypolar(plotarr *p, int i)
{
    int j, c, l, w, cy = p->color, sy = p->sym, ly = p->lines, wy = p->linew;
    double xbar, sd, *x = p->ex[0], *y = p->ex[1];
    char s[256];
    double *xtmp, *ytmp;
    xtmp = (double *) malloc(p->len * sizeof(double));
    ytmp = (double *) malloc(p->len * sizeof(double));
    for (i = 0; i < p->len; i++) {
	r2xy(x[i], y[i], &xtmp[i], &ytmp[i]);
    }
    x = xtmp;
    y = ytmp;

    c = setcolor(cy);
    w = setlinewidth(wy);
    if (wy && ly) {
	l = setlinestyle(ly);
	setlinewidth(wy);
	drawpoly(x, y, p->len);
	setlinestyle(l);
    }
    if (sy) {
	switch (sy) {
	case SYM_SEG:		/* draw segments */
	    drawpolyseg(x, y, p->len);
	    break;
	case SYM_CHAR:		/* draw character */
	    setfont(p->font);
	    if (p->symchar > ' ') {
		int skip = p->symskip + 1;

		s[0] = p->symchar;
		s[1] = 0;
		for (j = 0; j < p->len; j += skip) {
		    if (symok(x[j], y[j])) {
			writestr(x[j], y[j], 0, 2, s);
		    }
		}
	    }
	    break;
	case SYM_HILOX:	/* draw hilo along X */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j++) {
		    my_move2(x[j], y[j]);
		    my_draw2(p->ex[2][j], y[j]);
		}
	    }
	    break;
	case SYM_HILOY:	/* draw hilo along Y */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j++) {
		    my_move2(x[j], y[j]);
		    my_draw2(x[j], p->ex[2][j]);
		}
	    }
	    break;
	case SYM_OPENCLOSEX:	/* draw open/close along X */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j++) {
		    openclose(y[j], x[j], p->ex[2][j], 1.0, 0);
		}
	    }
	    break;
	case SYM_OPENCLOSEY:	/* draw open/close along Y */
	    if (p->ex[2] != NULL) {
		for (j = 0; j < p->len; j++) {
		    openclose(x[j], y[j], p->ex[2][j], 1.0, 1);
		}
	    }
	    break;
	case SYM_RANGE:	/* draw bounding box */
	    rect(p->xmin, p->ymin, p->xmax, p->ymax);
	    stasum(y, p->len, &xbar, &sd, 0);
	    my_move2(p->xmin, xbar);
	    my_draw2(p->xmax, xbar);
	    stasum(x, p->len, &xbar, &sd, 0);
	    my_move2(xbar, p->ymin);
	    my_draw2(xbar, p->ymax);
	    break;
	case SYM_TAG_FIRST:	/* tag the first point in a set */
	    if (symok(x[0], y[0])) {
		sprintf(s, "S%1d:1", i);
		sd = setcharsize(0.8);
		writestr(x[0], y[0], 0, 2, s);
		(void) setcharsize(sd);
	    }
	    break;
	case SYM_TAG_LAST:	/* tag the last point in a set */
	    if (symok(x[p->len - 1], y[p->len - 1])) {
		sprintf(s, "S%1d:%1d", i, p->len);
		sd = setcharsize(0.8);
		writestr(x[p->len - 1], y[p->len - 1], 0, 2, s);
		(void) setcharsize(sd);
	    }
	    break;
	case SYM_TAG_CENTER:	/* tag the middle point in a set */
	    if (symok(x[p->len / 2], y[p->len / 2])) {
		sprintf(s, "S%1d:%1d", i, p->len / 2);
		sd = setcharsize(0.8);
		writestr(x[p->len / 2], y[p->len / 2], 0, 2, s);
		(void) setcharsize(sd);
	    }
	    break;
	case SYM_STRING:	/* string at plot */
	    /* drawpolystring(x, y, p->len, sy, 0); */
	    break;
	case SYM_SETNO_IND:	/* set number and index */
	case SYM_SETNO:	/* set number */
	    for (j = 0; j < p->len; j++) {
		if (symok(x[j], y[j])) {
		    if (sy == SYM_SETNO) {
			sprintf(s, "S%d", i);
		    } else {
			sprintf(s, "S%1d:%1d)", i, j + 1);
		    }
		    writestr(x[j], y[j], 0, 0, s);
		}
	    }
	    break;
	case SYM_AVGY:		/* average Y */
	    break;
	case SYM_AVGSTDY1:
	    break;
	case SYM_AVGSTDY2:
	    break;
	case SYM_AVGSTDY3:
	    break;
	default:
	    drawpolysym(x, y, p->len, sy, p->symskip, p->symfill, p->symsize);
	    break;
	}
    }
    setlinewidth(w);
    setcolor(c);
}

/*
 * draw the graph frame
 */
void circleplot(framep f, double xc, double yc, double r)
{
    int c, s, wi;

    c = setcolor(f.color);
    s = setlinestyle(f.lines);
    wi = setlinewidth(f.linew);
    if (f.type == 0) {
	my_circle(xc, yc, r);
    } else {
    }
    setcolor(c);
    setlinestyle(s);
    setlinewidth(wi);
}

/*
 * drawpolarticks
 */
void drawpolarticks(double xc, double yc, double r, double r0, int n)
{
    int i;
    double dr = r0;
    double dt = 2.0 * M_PI / n;
    int nr = (int) (r / r0);
    for (i = 1; i < (nr + 1); i++) {
	my_circle(xc, yc, dr * i);
    }
    for (i = 0; i < n; i++) {
	my_move2(xc, yc);
	my_draw2(xc + r * cos(i * dt), yc + r * sin(i * dt));
    }
}

double rp2rg(double r)
{
    double t;
    t = r / rp;
    return t * grad;
}

double rg2rp(double r)
{
    double t;
    t = r / grad;
    return t * rp;
}

void rg2xy(double r, double theta, double *x, double *y)
{
    *x = r * cos(theta) + xc;
    *y = r * sin(theta) + yc;
}

void r2xy(double r, double theta, double *x, double *y)
{
    double r1;
    r1 = rp2rg(r);
    *x = r1 * cos(theta) + xc;
    *y = r1 * sin(theta) + yc;
}

/*
 * convert from world xy to user radius & theta
 */
void xy2r(double x, double ytmp, double *r, double *theta)
{
    double r1;
    r1 = sqrt((x - xc) * (x - xc) + (ytmp - yc) * (ytmp - yc));
    *theta = atan2(ytmp - yc, x - xc);
    *r = rg2rp(r1);
}
