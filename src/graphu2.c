/* $Id: graphu2.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * utilities for graphs - second batch, scrolling, world stack and arrange
 *
 */

#include <config.h>

#include <stdio.h>

#include "globals.h"
#include "protos.h"

void set_graph_active(int gno);
void push_world(void);
void update_world_stack(void);

void arrange_graphs(int grows, int gcols)
{
    int i, j;
    double dvx = 0.9 / gcols;
    double dvy = 0.9 / grows;
    int gtmp = 0;

    if (gcols != 1 || grows != 1) {
	for (i = 0; i < gcols; i++) {
	    for (j = 0; j < grows; j++) {
		g[gtmp].v.xv1 = 0.1 + i * dvx;
		g[gtmp].v.xv2 = 0.1 + ((i + 1) * dvx);
		g[gtmp].v.xv2 -= 0.07;
		g[gtmp].v.yv1 = 0.1 + j * dvy;
		g[gtmp].v.yv2 = 0.1 + ((j + 1) * dvy);
		g[gtmp].v.yv2 -= 0.07;
		set_graph_active(gtmp);
		update_all(cg);
		gtmp++;
	    }
	}
    }
}

/*
 * pan through world coordinates
 */

void gwindleft_proc(void)
{
    double dx;
    int i;

    if (scrolling_islinked) {
	for (i = 0; i < maxgraph; i++) {
	    if (isactive_graph(i) && !islogx(i)) {
		dx = scrollper * (g[i].w.xg2 - g[i].w.xg1);
		g[i].w.xg1 = g[i].w.xg1 - dx;
		g[i].w.xg2 = g[i].w.xg2 - dx;
	    }
	}
    } else {
	if (!islogx(cg)) {
	    dx = scrollper * (g[cg].w.xg2 - g[cg].w.xg1);
	    g[cg].w.xg1 = g[cg].w.xg1 - dx;
	    g[cg].w.xg2 = g[cg].w.xg2 - dx;
	}
    }
#ifndef NONE_GUI
    drawgraph();
#endif
}

void gwindright_proc(void)
{
    double dx;
    int i;

    if (scrolling_islinked) {
	for (i = 0; i < maxgraph; i++) {
	    if (isactive_graph(i) && !islogx(i)) {
		dx = scrollper * (g[i].w.xg2 - g[i].w.xg1);
		g[i].w.xg1 = g[i].w.xg1 + dx;
		g[i].w.xg2 = g[i].w.xg2 + dx;
	    }
	}
    } else {
	if (!islogx(cg)) {
	    dx = scrollper * (g[cg].w.xg2 - g[cg].w.xg1);
	    g[cg].w.xg1 = g[cg].w.xg1 + dx;
	    g[cg].w.xg2 = g[cg].w.xg2 + dx;
	}
    }
#ifndef NONE_GUI
    drawgraph();
#endif
}

void gwinddown_proc(void)
{
    double dy;
    int i;

    if (scrolling_islinked) {
	for (i = 0; i < maxgraph; i++) {
	    if (isactive_graph(i) && !islogy(i)) {
		dy = scrollper * (g[i].w.yg2 - g[i].w.yg1);
		g[i].w.yg1 = g[i].w.yg1 - dy;
		g[i].w.yg2 = g[i].w.yg2 - dy;
	    }
	}
    } else {
	if (!islogy(cg)) {
	    dy = scrollper * (g[cg].w.yg2 - g[cg].w.yg1);
	    g[cg].w.yg1 = g[cg].w.yg1 - dy;
	    g[cg].w.yg2 = g[cg].w.yg2 - dy;
	}
    }
#ifndef NONE_GUI
    drawgraph();
#endif
}

void gwindup_proc(void)
{
    double dy;
    int i;

    if (scrolling_islinked) {
	for (i = 0; i < maxgraph; i++) {
	    if (isactive_graph(i) && !islogy(i)) {
		dy = scrollper * (g[i].w.yg2 - g[i].w.yg1);
		g[i].w.yg1 = g[i].w.yg1 + dy;
		g[i].w.yg2 = g[i].w.yg2 + dy;
	    }
	}
    } else {
	if (!islogy(cg)) {
	    dy = scrollper * (g[cg].w.yg2 - g[cg].w.yg1);
	    g[cg].w.yg1 = g[cg].w.yg1 + dy;
	    g[cg].w.yg2 = g[cg].w.yg2 + dy;
	}
    }
#ifndef NONE_GUI
    drawgraph();
#endif
}

void gwindshrink_proc(void)
{
    double dx = shexper * (g[cg].w.xg2 - g[cg].w.xg1);
    double dy = shexper * (g[cg].w.yg2 - g[cg].w.yg1);

    if (!islogx(cg) && !islogy(cg)) {
	g[cg].w.xg1 = g[cg].w.xg1 + dx;
	g[cg].w.xg2 = g[cg].w.xg2 - dx;
	g[cg].w.yg1 = g[cg].w.yg1 + dy;
	g[cg].w.yg2 = g[cg].w.yg2 - dy;
#ifndef NONE_GUI
	drawgraph();
#endif
    } else {
	errmsg("Shrink not implemented for log plots");
    }
}

void gwindexpand_proc(void)
{
    double dx = shexper * (g[cg].w.xg2 - g[cg].w.xg1);
    double dy = shexper * (g[cg].w.yg2 - g[cg].w.yg1);

    if (!islogx(cg) && !islogy(cg)) {
	g[cg].w.xg1 = g[cg].w.xg1 - dx;
	g[cg].w.xg2 = g[cg].w.xg2 + dx;
	g[cg].w.yg1 = g[cg].w.yg1 - dy;
	g[cg].w.yg2 = g[cg].w.yg2 + dy;
#ifndef NONE_GUI
	drawgraph();
#endif
    } else {
	errmsg("Expand not implemented for log plots");
    }
}

/*
 * set scroll amount
 */
void scroll_proc(int value)
{
    scrollper = value / 100.0;
#ifndef NONE_GUI
    update_draw();
#endif
}

void scrollinout_proc(int value)
{
    shexper = value / 100.0;
#ifndef NONE_GUI
    update_draw();
#endif
}

/*
 * world stack operations
 */
void push_and_zoom(void)
{
    push_world();
#ifndef NONE_GUI
    set_action(0);
    set_action(ZOOM_1ST);
#endif
}

void cycle_world_stack(void)
{
    if (g[cg].ws_top < 1) {
	errmsg("World stack empty");
    } else {
	update_world_stack();
	g[cg].curw = (g[cg].curw + 1) % g[cg].ws_top;
 	show_world_stack( g[cg].curw );
#ifndef NONE_GUI
	drawgraph();
#endif
    }
}

void clear_world_stack(void)
{
    g[cg].ws_top = 1;
    g[cg].curw = 0;
    g[cg].ws[0].w.xg1 = g[cg].ws[0].w.xg2
                      = g[cg].ws[0].w.yg1
                      = g[cg].ws[0].w.yg2 = 0;
#ifndef NONE_GUI
    set_stack_message();
#endif
}

void show_world_stack(int n)
{
    int i;

    if (g[cg].ws_top < 1) {
	errmsg("World stack empty");
    } else {
	if (n >= g[cg].ws_top) {
	    errmsg("Selected view greater than stack depth");
	} else if (n < 0) {
	    errmsg("Selected view less than zero");
	} else {
	    g[cg].w = g[cg].ws[n].w;
	    for (i = 0; i < MAXAXES; i += 2) {
		g[cg].t[i].tmajor = g[cg].ws[n].t[i].xg1;
		g[cg].t[i].tminor = g[cg].ws[n].t[i].xg2;
		g[cg].t[i + 1].tmajor = g[cg].ws[n].t[i].yg1;
		g[cg].t[i + 1].tminor = g[cg].ws[n].t[i].yg2;
	    }
	    g[cg].curw = n;
#ifndef NONE_GUI
	    drawgraph();
#endif
	}
    }
}

/*
 * world stacks
 */

/* add a world view to the stack
 *	if there are no other views stacked, replace the view with the new view 
 *	if there are other views, simply add this one to the bottom of the stack
 */
void add_world(int gno, double x1, double x2, double y1, double y2,
	        double t1, double t2, double u1, double u2)
{
    int i;

    /* see if another entry has been stacked */
    if( g[cg].ws[0].w.xg1 == 0. &&
	g[cg].ws[0].w.xg2 == 0. &&
	g[cg].ws[0].w.yg1 == 0. &&
 	g[cg].ws[0].w.yg2 == 0. ) {	    
	    g[gno].ws_top = 0;
	}
		
    if (g[gno].ws_top < MAX_ZOOM_STACK) {
	g[gno].ws[g[gno].ws_top].w.xg1 = x1;
	g[gno].ws[g[gno].ws_top].w.xg2 = x2;
	g[gno].ws[g[gno].ws_top].w.yg1 = y1;
	g[gno].ws[g[gno].ws_top].w.yg2 = y2;
	for (i = 0; i < 3; i++) {
	    g[gno].ws[g[gno].ws_top].t[i].xg1 = t1;
	    g[gno].ws[g[gno].ws_top].t[i].xg2 = t2;
	    g[gno].ws[g[gno].ws_top].t[i].yg1 = u1;
	    g[gno].ws[g[gno].ws_top].t[i].yg2 = u2;
	}
	g[gno].ws_top++;
#ifndef NONE_GUI
		set_stack_message();
#endif
    } else {
	errmsg("World stack full");
    }
}

void push_world(void)
{
    int i;

    if (g[cg].ws_top < MAX_ZOOM_STACK) {
        update_world_stack();
        for( i=g[cg].ws_top; i>g[cg].curw; i-- ) {
               g[cg].ws[i] = g[cg].ws[i-1];
        }
	g[cg].ws_top++;
#ifndef NONE_GUI
	set_stack_message();
#endif
    } else {
	errmsg("World stack full");
    }
}


void update_world_stack()
{
    int i;

    g[cg].ws[g[cg].curw].w = g[cg].w;
    for (i = 0; i < MAXAXES; i += 2) {
    	g[cg].ws[g[cg].curw].t[i / 2].xg1 = g[cg].t[i].tmajor;
    	g[cg].ws[g[cg].curw].t[i / 2].xg2 = g[cg].t[i].tminor;
    	g[cg].ws[g[cg].curw].t[i / 2].yg1 = g[cg].t[i + 1].tmajor;
    	g[cg].ws[g[cg].curw].t[i / 2].yg2 = g[cg].t[i + 1].tminor;
    	g[cg].ws[g[cg].curw].prec[i] = g[cg].t[i].tl_prec;
    	g[cg].ws[g[cg].curw].prec[i + 1] = g[cg].t[i + 1].tl_prec;
    }
#ifndef NONE_GUI
    set_stack_message();
#endif
}

void pop_world(void)
/* modified to actually pop the current world view off the stack */
{
    int i;

    if (g[cg].ws_top <= 1) {
	errmsg("World stack empty");
    } else {
    	if( g[cg].curw != g[cg].ws_top-1 ) {
    	    for( i=g[cg].curw; i<g[cg].ws_top; i++ ) {
    			g[cg].ws[i] = g[cg].ws[i+1];
	    }
    	} else {
            g[cg].curw--;
        }
        g[cg].ws_top--;
        show_world_stack( g[cg].curw );

#ifndef NONE_GUI
        drawgraph();
#endif
    }
}

/*
 * set format string for locator
 */
static char *fchar[3] = {"lf", "le", "g"};
static char *typestr[6] = {"X, Y",
                           "DX, DY",
			   "DIST",
			   "R, Theta",
			   "VX, VY",
                           "SX, SY"};
char locator_format[256] = {"G%1d: X, Y = [%.6g, %.6g]"};

void make_format(int gno)
{
    int type, locpx, locfx, locpy, locfy;

    type = g[gno].pt_type;
    locfx = get_format_index(g[gno].fx);
    locfy = get_format_index(g[gno].fy);
    locpx = g[gno].px;
    locpy = g[gno].py;
    switch (type) {
    case 0:
	if (locfx < 3 && locfy < 3) {
	    sprintf(locator_format, "G%%1d: %s = [%%.%d%s, %%.%d%s]",
	            typestr[type], locpx, fchar[locfx], locpy, fchar[locfy]);
	} else {
	    locator_format[0] = 0;
	}
	break;
    case 2:
	locfx = locfx == FORMAT_DECIMAL ? 0 :
		locfx == FORMAT_EXPONENTIAL ? 1 :
		locfx == FORMAT_GENERAL ? 2 : 0;
	sprintf(locator_format, "G%%1d: %s = [%%.%d%s]", typestr[type], locpx,
	        fchar[locfx]);
	break;
    case 1:
    case 3:
    case 4:
	locfx = locfx == FORMAT_DECIMAL ? 0 :
		locfx == FORMAT_EXPONENTIAL ? 1 :
		locfx == FORMAT_GENERAL ? 2 : 0;
	locfy = locfy == FORMAT_DECIMAL ? 0 :
		locfy == FORMAT_EXPONENTIAL ? 1 :
		locfy == FORMAT_GENERAL ? 2 : 0;
	sprintf(locator_format, "G%%1d: %s = [%%.%d%s, %%.%d%s]", typestr[type],
	        locpx, fchar[locfx], locpy, fchar[locfy]);
	break;
    case 5:
	sprintf(locator_format, "G%%1d: %s = [%%d, %%d]", typestr[type]);
	break;
    }
}

void arrange_graphs2(int grows, int gcols, double vgap, double hgap,
		    double sx, double sy, double wx, double wy, int applyto)
{
    int i, j;
    int gtmp = 0;

    if (gcols != 1 || grows != 1) {
	for (i = 0; i < gcols; i++) {
	    for (j = 0; j < grows; j++) {
		g[gtmp].v.xv1 = sx + i * hgap + i * wx;
		g[gtmp].v.xv2 = sx + i * hgap + (i + 1) * wx;
		g[gtmp].v.yv1 = sy + j * vgap + j * wy;
		g[gtmp].v.yv2 = sy + j * vgap + (j + 1) * wy;
		if (!isactive_graph(gtmp)) {
		    set_graph_active(gtmp);
		}
		gtmp++;
	    }
	}
    }
}

void define_autos(int aon, int au, int ap, int ameth, int antx, int anty)
{
    int i, ming, maxg;

    if (au >= 0 && !isactive_set(cg, au)) {
	errmsg("Set not active");
	return;
    }
    if (ap) {
	ming = 0;
	maxg = maxgraph - 1;
    } else {
	ming = cg;
	maxg = cg;
    }
    if (ming == cg && maxg == cg) {
	if (!isactive_graph(cg)) {
	    errmsg("Current graph is not active!");
	    return;
	}
    }
    for (i = ming; i <= maxg; i++) {
	if (isactive_graph(i)) {
	    if (ameth == 0) {
		g[i].auto_type = TYPE_AUTO;
	    } else {
		g[i].auto_type = TYPE_SPEC;
	    }
	    g[i].t[0].t_num = antx + 2;
	    g[i].t[1].t_num = anty + 2;
	    if (aon > -4) {	/* -4 == just set parameters but don't
				 * autoscale */
		if (au >= 0 && isactive_set(i, au)) {
		    autoscale_set(i, au, aon);
		} else {
		    autoscale_graph(i, aon);
		}
	    }
	}
    }
    if (aon != -4) {
#ifndef NONE_GUI
	drawgraph();
#endif
    }
}

void define_arrange(int nrows, int ncols, int pack,
       double vgap, double hgap, double sx, double sy, double wx, double wy)
{
    int i, j, k, gno, applyto = 0;

    if (nrows * ncols > maxgraph) {
	errmsg("Too many rows and columns");
	return;
    }
    switch (pack) {
    case 0:
	for (j = 0; j < ncols; j++) {
	    for (i = 0; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 0; k < 2; k++) {
		    g[gno].t[k].tl_flag = TRUE;
		}
	    }
	}
	break;
    case 1:
	hgap = 0.0;
	for (j = 1; j < ncols; j++) {
	    for (i = 0; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 1; k < 6; k += 2) {
		    g[gno].t[k].tl_flag = FALSE;
		}
	    }
	}
	break;
    case 2:
	vgap = 0.0;
	for (j = 0; j < ncols; j++) {
	    for (i = 1; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 0; k < 6; k += 2) {
		    g[gno].t[k].tl_flag = FALSE;
		}
	    }
	}
	break;
    case 3:
	hgap = 0.0;
	vgap = 0.0;
	for (j = 1; j < ncols; j++) {
	    for (i = 0; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 1; k < 6; k += 2) {
		    g[gno].t[k].tl_flag = FALSE;
		}
	    }
	}
	for (j = 0; j < ncols; j++) {
	    for (i = 1; i < nrows; i++) {
		gno = i + j * nrows;
		for (k = 0; k < 6; k += 2) {
		    g[gno].t[k].tl_flag = FALSE;
		}
	    }
	}
	break;
    }
    arrange_graphs2(nrows, ncols, vgap, hgap, sx, sy, wx, wy, applyto);
#ifndef NONE_GUI
    drawgraph();
#endif
    set_dirtystate();
}
