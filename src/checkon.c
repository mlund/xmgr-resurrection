/* $Id: checkon.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * routines for sanity checks
 *
 */

#include <config.h>

#include <stdio.h>

#include "globals.h"
#include "protos.h"

int checkon_ticks(int gno) {
    /* TODO this is done in drawticks.c and the following may be unnecessary
        int i, n1, n2;
        double dx = g[gno].w.xg2 - g[gno].w.xg1;
        double dy = g[gno].w.yg2 - g[gno].w.yg1;
        for (i = 0; i < MAXAXES; i++) {
        if (g[gno].t[i].active == ON) {
            n1 = n2 = 0;
            if (g[gno].t[i].t_flag == ON) {
            if (i % 2 == 0) {
                n1 = dx / g[gno].t[i].tmajor;
                n2 = dx / g[gno].t[i].tminor;
                if (n1 > 500) {
                errmsg("Too many X-axis major ticks (> 500)");
                return 0;
                }
                if (n2 > 1000) {
                errmsg("Too many X-axis minor ticks (> 1000)");
                return 0;
                }
            } else {
                n1 = dy / g[gno].t[i].tmajor;
                n2 = dy / g[gno].t[i].tminor;
                if (n1 > 500) {
                errmsg("Too many Y-axis major ticks (> 500)");
                return 0;
                }
                if (n2 > 1000) {
                errmsg("Too many Y-axis minor ticks (> 1000)");
                return 0;
                }
            }
            }
        }
        }
    */
    return 1;
}

int checkon_world(int gno) {
    double dx = g[gno].w.xg2 - g[gno].w.xg1;
    double dy = g[gno].w.yg2 - g[gno].w.yg1;

    if (g[gno].type == GRAPH_LOGX || g[gno].type == GRAPH_LOGXY) {
        if (g[gno].w.xg1 <= 0) {
            errmsg("World X-min <= 0.0");
            return 0;
        }
        if (g[gno].w.xg2 <= 0) {
            errmsg("World X-max <= 0.0");
            return 0;
        }
    }
    if (dx <= 0.0) {
        errmsg("World DX <= 0.0");
        return 0;
    }
    if (g[gno].type == GRAPH_LOGY || g[gno].type == GRAPH_LOGXY) {
        if (g[gno].w.yg1 <= 0.0) {
            errmsg("World Y-min <= 0.0");
            return 0;
        }
        if (g[gno].w.yg2 <= 0.0) {
            errmsg("World Y-max <= 0.0");
            return 0;
        }
    }
    if (dy <= 0.0) {
        errmsg("World DY <= 0.0");
        return 0;
    }
    return 1;
}

/* possibly defunct as reversal of axes depends on xv1 > xv2 */
int checkon_viewport(int gno) {
    double dx = g[gno].v.xv2 - g[gno].v.xv1;
    double dy = g[gno].v.yv2 - g[gno].v.yv1;

    if (dx <= 0.0) {
        errmsg("Viewport DX <= 0.0");
        return 0;
    }
    if (dy <= 0.0) {
        errmsg("Viewport DY <= 0.0");
        return 0;
    }
    return 1;
}
