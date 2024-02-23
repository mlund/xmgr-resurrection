/* $Id: compute.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * perform math between sets
 *
 */

#include <config.h>

#include <stdio.h>

#include "globals.h"
#include "protos.h"

void loadset(int gno, int selset, int toval, double startno, double stepno) {
    int i, lenset;
    double* ltmp;
    double *xtmp, *ytmp;

    if ((lenset = getsetlength(gno, selset)) <= 0) {
        char stmp[60];

        sprintf(stmp, "Length of set %d <= 0", selset);
        errmsg(stmp);
        return;
    }
    xtmp = getx(gno, selset);
    ytmp = gety(gno, selset);
    switch (toval) {
    case 1:
        ltmp = xtmp;
        break;
    case 2:
        ltmp = ytmp;
        break;
    case 3:
        ltmp = ax;
        break;
    case 4:
        ltmp = bx;
        break;
    case 5:
        ltmp = cx;
        break;
    case 6:
        ltmp = dx;
        break;
    default:
        return;
    }
    for (i = 0; i < lenset; i++) {
        *ltmp++ = startno + i * stepno;
    }
    updatesetminmax(gno, selset);
    set_dirtystate();
#ifndef NONE_GUI
    update_set_status(gno, selset);
#endif
}

/*
 * evaluate the expression in sscanstr and place the result in selset
 */
int formula(int gno, int selset, char* sscanstr) {
    char stmp[64];
    int i = 0, errpos, lenset, oldcg;
    double *xtmp, *ytmp;

    if ((lenset = getsetlength(gno, selset)) <= 0) {
        sprintf(stmp, "Length of set %d = 0", selset);
        errmsg(stmp);
        return 0;
    }
    xtmp = getx(gno, selset);
    ytmp = gety(gno, selset);

    oldcg = cg; /* kludge to get around not being able to set result graph */
    cg = gno;
    scanner(sscanstr, xtmp, ytmp, lenset, ax, bx, cx, dx, MAXARR, i, selset, &errpos);
    cg = oldcg;

    if (!errpos) {
        updatesetminmax(gno, selset);
#ifndef NONE_GUI
        update_set_status(gno, selset);
#endif
        set_dirtystate();
    }
    return (errpos);
}
