/* $Id: regionutils.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * routines to allocate, manipulate, and return
 * information about regions.
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include "draw.h"
#include "protos.h"

int regiontype = 0;

extern int regionlinkto;

/*
 * see if (x,y) lies inside the plot
 */
int inbounds(int gno, double x, double y) {
    return ((x >= g[gno].w.xg1 && x <= g[gno].w.xg2) && (y >= g[gno].w.yg1 && y <= g[gno].w.yg2));
}

int isactive_region(int regno) { return (regno == MAXREGION || regno == MAXREGION + 1 || rg[regno].active == TRUE); }

char* region_types(int it, int which) {
    static char s[128];

    strcpy(s, "UNDEFINED");
    switch (it) {
    case REGION_TOLEFT:
        strcpy(s, "REGION_TOLEFT");
        break;
    case REGION_TORIGHT:
        strcpy(s, "REGION_TORIGHT");
        break;
    case REGION_ABOVE:
        strcpy(s, "REGION_ABOVE");
        break;
    case REGION_BELOW:
        strcpy(s, "REGION_BELOW");
        break;
    case REGION_POLYI:
        if (which) {
            strcpy(s, "REGION_POLYI");
        } else {
            strcpy(s, "INSIDE POLY");
        }
        break;
    case REGION_POLYO:
        if (which) {
            strcpy(s, "REGION_POLYO");
        } else {
            strcpy(s, "OUTSIDE POLY");
        }
        break;
    }
    return s;
}

void kill_region(int r) {
    int i;

    if (rg[r].active == TRUE) {
        if (rg[r].x != NULL) {
            free(rg[r].x);
            rg[r].x = NULL;
        }
        if (rg[r].y != NULL) {
            free(rg[r].y);
            rg[r].y = NULL;
        }
    }
    rg[r].active = FALSE;
    for (i = 0; i < maxgraph; i++) {
        rg[r].linkto[i] = FALSE;
    }
}

void activate_region(int r, int type) {
    kill_region(r);
    rg[r].active = TRUE;
    rg[r].type = type;
}

void define_region(int nr, int regionlinkto, int rtype) {
    kill_region(nr);
    switch (rtype) {
    case 0:
        regiontype = REGION_POLYI;
#ifndef NONE_GUI
        do_select_region();
#endif
        break;
    case 1:
        regiontype = REGION_POLYO;
#ifndef NONE_GUI
        do_select_region();
#endif
        break;
    case 2:
        regiontype = REGION_ABOVE;
#ifndef NONE_GUI
        set_action(0);
        set_action(DEF_REGION1ST);
#endif
        break;
    case 3:
        regiontype = REGION_BELOW;
#ifndef NONE_GUI
        set_action(0);
        set_action(DEF_REGION1ST);
#endif
        break;
    case 4:
        regiontype = REGION_TOLEFT;
#ifndef NONE_GUI
        set_action(0);
        set_action(DEF_REGION1ST);
#endif
        break;
    case 5:
        regiontype = REGION_TORIGHT;
#ifndef NONE_GUI
        set_action(0);
        set_action(DEF_REGION1ST);
#endif
        break;
    }
}

/*
 * extract sets from region
 */
void extract_region(int gno, int fromset, int toset, int regno) {
    int i, j;
    double *x, *y;
    int startno, stopno;
    if (fromset == -1) {
        startno = 0;
        stopno = maxplot - 1;
    } else {
        startno = stopno = fromset;
    }
    if (regno >= MAXREGION) {
        for (j = startno; j <= stopno; j++) {
            x = getx(cg, j);
            y = gety(cg, j);
            if (isactive_set(cg, j) && (toset != j || gno != cg)) {
                for (i = 0; i < getsetlength(cg, j); i++) {
                    if (regno == MAXREGION) {
                        if (inbounds(cg, x[i], y[i])) {
                            add_point(gno, toset, x[i], y[i], 0.0, 0.0, SET_XY);
                        }
                    } else {
                        if (!inbounds(cg, x[i], y[i])) {
                            add_point(gno, toset, x[i], y[i], 0.0, 0.0, SET_XY);
                        }
                    }
                }
            }
        }
    } else {
        if (rg[regno].active == FALSE) {
            errmsg("Region not active");
            return;
        }
        if (rg[regno].linkto[cg] == FALSE) {
            errmsg("Region not linked to this graph");
            return;
        }
        for (j = startno; j <= stopno; j++) {
            x = getx(cg, j);
            y = gety(cg, j);
            if (isactive_set(cg, j) && (toset != j || gno != cg)) {
                for (i = 0; i < getsetlength(cg, j); i++) {
                    if (inregion(regno, x[i], y[i])) {
                        add_point(gno, toset, x[i], y[i], 0.0, 0.0, SET_XY);
                    }
                }
            }
        }
    }
    updatesetminmax(gno, toset);
#ifndef NONE_GUI
    update_set_status(gno, toset);
    drawgraph();
#endif
}

void delete_byindex(int gno, int setno, int* ind) {
    int i, j, cnt = 0;
    int ncols = getncols(gno, setno);
    for (i = 0; i < getsetlength(gno, setno); i++) {
        if (ind[i]) {
            cnt++;
        }
    }
    if (cnt == getsetlength(gno, setno)) {
        killset(gno, setno);
        return;
    }
    cnt = 0;
    for (i = 0; i < getsetlength(gno, setno); i++) {
        if (ind[i] == 0) {
            for (j = 0; j < ncols; j++) {
                g[gno].p[setno].ex[j][cnt] = g[gno].p[setno].ex[j][i];
            }
            cnt++;
        }
    }
    setlength(gno, setno, cnt);
}

void delete_region(int gno, int setno, int regno) {
    int i, j, k, len, *ind = NULL;
    int gstart, gstop;
    int sstart, sstop;
    double *x, *y;

    if (regno < 0 || regno > MAXREGION + 1) {
        errmsg("Invalid region");
        return;
    }
    if (regno < MAXREGION) {
        if (rg[regno].active == FALSE) {
            errmsg("Region not active");
            return;
        }
        if (gno >= 0) {
            if (rg[regno].linkto[gno] == FALSE) {
                errmsg("Region not linked to this graph");
                return;
            }
        }
    }
    if (gno == GRAPH_SELECT_CURRENT) { /* current graph */
        gstart = cg;
        gstop = cg;
    } else if (gno == GRAPH_SELECT_ALL) { /* all graphs */
        gstart = 0;
        gstop = maxgraph - 1;
    } else {
        gstart = gno; /* particular graph */
        gstop = gno;
    }

    for (k = gstart; k <= gstop; k++) {
        if (isactive_graph(k)) {
            if (setno < 0) {
                sstart = 0;
                sstop = g[k].maxplot - 1;
            } else {
                sstart = setno;
                sstop = setno;
            }
            for (j = sstart; j <= sstop; j++) {
                x = getx(k, j);
                y = gety(k, j);
                if (isactive_set(k, j)) {
                    len = getsetlength(k, j);
                    if (ind != NULL) {
                        free(ind);
                    }
                    ind = (int*)malloc(len * sizeof(int));
                    if (ind == NULL) {
                        errmsg("Error mallocing memory in delete_region, operation cancelled");
                        return;
                    }
                    for (i = 0; i < len; i++) {
                        ind[i] = 0;
                        if (regno == MAXREGION) { /* inside world */
                            if (inbounds(k, x[i], y[i])) {
                                ind[i] = 1;
                            }
                        } else if (regno == MAXREGION + 1) { /* outside world */
                            if (!inbounds(k, x[i], y[i])) {
                                ind[i] = 1;
                            }
                        } else {
                            if (inregion(regno, x[i], y[i])) { /* inside region */
                                ind[i] = 1;
                            }
                        }
                    }
                    delete_byindex(k, j, ind);
                    updatesetminmax(k, j);
#ifndef NONE_GUI
                    update_set_status(k, j);
#endif
                }
            }
        }
    }
#ifndef NONE_GUI
    drawgraph();
#endif
}

void evaluate_region(int regno, int gno, int setno, char* buf) {
    double a, b, c, d;
    double *x, *y, tmpx, tmpy;
    int errpos;
    int i, j, k;
    int gstart, gstop;
    int sstart, sstop;

    if (gno == GRAPH_SELECT_CURRENT) { /* current graph */
        gstart = cg;
        gstop = cg;
    } else if (gno == GRAPH_SELECT_ALL) { /* all graphs */
        gstart = 0;
        gstop = maxgraph - 1;
    } else {
        gstart = gno; /* particular graph */
        gstop = gno;
    }
    if (regno < MAXREGION) {
        if (rg[regno].active == FALSE) {
            errmsg("Region not active");
            return;
        }
        if (rg[regno].linkto[cg] == FALSE) {
            errmsg("Region not linked to the current graph");
            return;
        }
    }
    for (k = gstart; k <= gstop; k++) {
        if (isactive_graph(k)) {
            if (setno == SET_SELECT_ALL) { /* all sets */
                sstart = 0;
                sstop = g[k].maxplot - 1;
            } else { /* particular set */
                sstart = setno;
                sstop = setno;
            }
            for (j = sstart; j <= sstop; j++) {
                if (isactive_set(k, j)) {
                    x = getx(k, j);
                    y = gety(k, j);
                    tmpx = x[0];
                    tmpy = y[0];
                    for (i = 0; i < getsetlength(k, j); i++) {

                        if ((regno < MAXREGION && inregion(regno, x[i], y[i])) ||
                            (regno == MAXREGION && inbounds(k, x[i], y[i])) ||
                            (regno > MAXREGION && !inbounds(k, x[i], y[i]))) {

                            x[0] = x[i];
                            y[0] = y[i];
                            scanner(buf, &x[i], &y[i], 1, &a, &b, &c, &d, 1, i, j, &errpos);
                            if (errpos) {
                                x[0] = tmpx;
                                y[0] = tmpy;
                                updatesetminmax(k, j);
#ifndef NONE_GUI
                                update_set_status(k, j);
#endif
                                return;
                            }
                            if (i != 0) {
                                x[i] = x[0];
                                y[i] = y[0];
                            } else {
                                tmpx = x[0];
                                tmpy = y[0];
                            }
                            set_dirtystate();
                        }
                    }
                    x[0] = tmpx;
                    y[0] = tmpy;
                    updatesetminmax(k, j);
                } /* isactive */
#ifndef NONE_GUI
                update_set_status(k, j);
#endif
            } /* j */
        } /* isactive_graph */
    } /* k */
}

/*
 * extract sets from graph gfrom to graph gto
 * a set is in a region if any point of the set is in the region
 */
void extractsets_region(int gfrom, int gto, int rno) {
    int i, j;
    double *x, *y;
    for (j = 0; j < g[gfrom].maxplot; j++) {
        if (isactive_set(gfrom, j)) {
            x = getx(gfrom, j);
            y = gety(gfrom, j);
            for (i = 0; i < getsetlength(gfrom, j); i++) {
                if (inregion(rno, x[i], y[i])) {}
            }
        }
    }
}

/*
 * delete sets from graph gno
 * a set is in a region if any point of the set is in the region
 */
void deletesets_region(int gno, int rno) {
    int i, j;
    double *x, *y;
    for (j = 0; j < g[gno].maxplot; j++) {
        if (isactive_set(gno, j)) {
            x = getx(gno, j);
            y = gety(gno, j);
            for (i = 0; i < getsetlength(gno, j); i++) {
                if (inregion(rno, x[i], y[i])) {
                    killset(gno, j);
                    break; /* set no longer exists, so get out */
                }
            }
        }
    }
}

/*
 * report on sets in a region
 */
void reporton_region(int gno, int rno, int type, int strict) {
    char buf[256];
    int i, j, first, contained;
    double *x, *y;
    sprintf(buf, "\nRegion R%1d contains:\n", rno);
    stufftext(buf, STUFF_START);
    for (j = 0; j < g[gno].maxplot; j++) {
        if (isactive_set(gno, j)) {
            x = getx(gno, j);
            y = gety(gno, j);
            first = 1;
            contained = 0;
            for (i = 0; i < getsetlength(gno, j); i++) {
                if (inregion(rno, x[i], y[i])) {
                    contained = 1;
                    switch (type) {
                    case 0: /* report on sets */
                        if (first) {
                            first = 0;
                            sprintf(buf, "  Set S%1d\n", j);
                            stufftext(buf, STUFF_TEXT);
                        }
                        break;
                    case 1: /* points */
                        if (first) {
                            first = 0;
                            sprintf(buf, "  Set S%1d\n", j);
                            stufftext(buf, STUFF_TEXT);
                        }
                        sprintf(buf, "    %d %f %f\n", i + 1, x[i], y[i]);
                        stufftext(buf, STUFF_TEXT);
                        break;
                    }
                } else {
                    contained = 0;
                }
            }
        }
    }
    strcpy(buf, "\n");
    stufftext(buf, STUFF_STOP);
}

void load_poly_region(int r, int n, double* x, double* y) {
    int i;

    if (n > 2) {
        activate_region(r, regiontype);
        rg[r].n = n;
        rg[r].x = (double*)calloc(n, sizeof(double));
        rg[r].y = (double*)calloc(n, sizeof(double));
        for (i = 0; i < n; i++) {
            rg[r].x[i] = x[i];
            rg[r].y[i] = y[i];
        }
    }
}

void draw_region(int r) {
    int i, c, s, w;
    double vx, vy, wx, wy;

    c = setcolor(rg[r].color);
    s = setlinestyle(rg[r].lines);
    w = setlinewidth(rg[r].linew);
    switch (rg[r].type) {
    case REGION_ABOVE:
        my_move2(rg[r].x1, rg[r].y1);
        my_draw2(rg[r].x2, rg[r].y2);
        world2view(rg[r].x1, rg[r].y1, &vx, &vy);
        view2world(vx, vy + 0.05, &wx, &wy);
        draw_arrow(rg[r].x1, rg[r].y1, rg[r].x1, wy, 2, 1.0, 0);
        world2view(rg[r].x2, rg[r].y2, &vx, &vy);
        view2world(vx, vy + 0.05, &wx, &wy);
        draw_arrow(rg[r].x2, rg[r].y2, rg[r].x2, wy, 2, 1.0, 0);
        break;
    case REGION_BELOW:
        my_move2(rg[r].x1, rg[r].y1);
        my_draw2(rg[r].x2, rg[r].y2);
        world2view(rg[r].x1, rg[r].y1, &vx, &vy);
        view2world(vx, vy - 0.05, &wx, &wy);
        draw_arrow(rg[r].x1, rg[r].y1, rg[r].x1, wy, 2, 1.0, 0);
        world2view(rg[r].x2, rg[r].y2, &vx, &vy);
        view2world(vx, vy - 0.05, &wx, &wy);
        draw_arrow(rg[r].x2, rg[r].y2, rg[r].x2, wy, 2, 1.0, 0);
        break;
    case REGION_TOLEFT:
        my_move2(rg[r].x1, rg[r].y1);
        my_draw2(rg[r].x2, rg[r].y2);
        world2view(rg[r].x1, rg[r].y1, &vx, &vy);
        view2world(vx - 0.05, vy, &wx, &wy);
        draw_arrow(rg[r].x1, rg[r].y1, wx, rg[r].y1, 2, 1.0, 0);
        world2view(rg[r].x2, rg[r].y2, &vx, &vy);
        view2world(vx - 0.05, vy, &wx, &wy);
        draw_arrow(rg[r].x2, rg[r].y2, wx, rg[r].y2, 2, 1.0, 0);
        break;
    case REGION_TORIGHT:
        my_move2(rg[r].x1, rg[r].y1);
        my_draw2(rg[r].x2, rg[r].y2);
        world2view(rg[r].x1, rg[r].y1, &vx, &vy);
        view2world(vx + 0.05, vy, &wx, &wy);
        draw_arrow(rg[r].x1, rg[r].y1, wx, rg[r].y1, 2, 1.0, 0);
        world2view(rg[r].x2, rg[r].y2, &vx, &vy);
        view2world(vx + 0.05, vy, &wx, &wy);
        draw_arrow(rg[r].x2, rg[r].y2, wx, rg[r].y2, 2, 1.0, 0);
        break;
    case REGION_POLYI:
    case REGION_POLYO:
        if (rg[r].x != NULL && rg[r].n > 2) {
            my_move2(rg[r].x[0], rg[r].y[0]);
            for (i = 1; i < rg[r].n; i++) {
                my_draw2(rg[r].x[i], rg[r].y[i]);
            }
            my_draw2(rg[r].x[0], rg[r].y[0]);
        }
        break;
    }
    setcolor(c);
    setlinestyle(s);
    setlinewidth(w);
}

/*
 * routines to determine if a point lies in a polygon
 */
int intersect_to_left(double x, double y, double x1, double y1, double x2, double y2) {
    double xtmp, m, b;

    /* ignore horizontal lines */
    if (y1 == y2) {
        return 0;
    }
    /* not contained vertically */
    if (((y < y1) && (y < y2)) || ((y > y1) && (y > y2))) {
        return 0;
    }
    /* none of the above, compute the intersection */
    if ((xtmp = x2 - x1) != 0.0) {
        m = (y2 - y1) / xtmp;
        b = y1 - m * x1;
        xtmp = (y - b) / m;
    } else {
        xtmp = x1;
    }
    if (xtmp <= x) {
        /* check for intersections at a vertex */
        /* if this is the max ordinate then accept */
        if (y == y1) {
            if (y1 > y2) {
                return 1;
            } else {
                return 0;
            }
        }
        /* check for intersections at a vertex */
        if (y == y2) {
            if (y2 > y1) {
                return 1;
            } else {
                return 0;
            }
        }
        /* no vertices intersected */
        return 1;
    }
    return 0;
}

/*
 * determine if (x,y) is in the polygon xlist[], ylist[]
 */
int inbound(double x, double y, double* xlist, double* ylist, int n) {
    int i, l = 0;

    for (i = 0; i < n; i++) {
        l += intersect_to_left(x, y, xlist[i], ylist[i], xlist[(i + 1) % n], ylist[(i + 1) % n]);
    }
    return l % 2;
}

/*
 * routines to determine if a point lies to the left of an infinite line
 */
int isleft(double x, double y, double x1, double y1, double x2, double y2) {
    double xtmp, m, b;

    /* horizontal lines */
    if (y1 == y2) {
        return 0;
    }
    /* none of the above, compute the intersection */
    if ((xtmp = x2 - x1) != 0.0) {
        m = (y2 - y1) / xtmp;
        b = y1 - m * x1;
        xtmp = (y - b) / m;
    } else {
        xtmp = x1;
    }
    if (xtmp >= x) {
        return 1;
    }
    return 0;
}

/*
 * routines to determine if a point lies to the left of an infinite line
 */
int isright(double x, double y, double x1, double y1, double x2, double y2) {
    double xtmp, m, b;

    /* horizontal lines */
    if (y1 == y2) {
        return 0;
    }
    if ((xtmp = x2 - x1) != 0.0) {
        m = (y2 - y1) / xtmp;
        b = y1 - m * x1;
        xtmp = (y - b) / m;
    } else {
        xtmp = x1;
    }
    if (xtmp <= x) {
        return 1;
    }
    return 0;
}

/*
 * routines to determine if a point lies above an infinite line
 */
int isabove(double x, double y, double x1, double y1, double x2, double y2) {
    double ytmp, m, b;

    /* vertical lines */
    if (x1 == x2) {
        return 0;
    }
    if ((ytmp = y2 - y1) != 0.0) {
        m = ytmp / (x2 - x1);
        b = y1 - m * x1;
        ytmp = m * x + b;
    } else {
        ytmp = y1;
    }
    if (ytmp <= y) {
        return 1;
    }
    return 0;
}

/*
 * routines to determine if a point lies below an infinite line
 */
int isbelow(double x, double y, double x1, double y1, double x2, double y2) {
    double ytmp, m, b;

    /* vertical lines */
    if (x1 == x2) {
        return 0;
    }
    if ((ytmp = y2 - y1) != 0.0) {
        m = ytmp / (x2 - x1);
        b = y1 - m * x1;
        ytmp = m * x + b;
    } else {
        ytmp = y1;
    }
    if (ytmp >= y) {
        return 1;
    }
    return 0;
}

int inregion(int regno, double x, double y) {
    if (regno == MAXREGION) {
        return (inbounds(cg, x, y));
    }
    if (regno == MAXREGION + 1) {
        return (!inbounds(cg, x, y));
    }
    if (rg[regno].active == TRUE) {
        switch (rg[regno].type) {
        case REGION_POLYI:
            if (inbound(x, y, rg[regno].x, rg[regno].y, rg[regno].n)) {
                return 1;
            }
            break;
        case REGION_POLYO:
            if (!inbound(x, y, rg[regno].x, rg[regno].y, rg[regno].n)) {
                return 1;
            }
            break;
        case REGION_TORIGHT:
            if (isright(x, y, rg[regno].x1, rg[regno].y1, rg[regno].x2, rg[regno].y2)) {
                return 1;
            }
            break;
        case REGION_TOLEFT:
            if (isleft(x, y, rg[regno].x1, rg[regno].y1, rg[regno].x2, rg[regno].y2)) {
                return 1;
            }
            break;
        case REGION_ABOVE:
            if (isabove(x, y, rg[regno].x1, rg[regno].y1, rg[regno].x2, rg[regno].y2)) {
                return 1;
            }
            break;
        case REGION_BELOW:
            if (isbelow(x, y, rg[regno].x1, rg[regno].y1, rg[regno].x2, rg[regno].y2)) {
                return 1;
            }
            break;
        }
    }
    return 0;
}
