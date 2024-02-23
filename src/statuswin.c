/* $Id: statuswin.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * status popup and about
 *
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>

#include "patchlevel.h"
#include "globals.h"
#include "protos.h"
#include "motifinc.h"

#define SPAGESIZE 30

#define STATUS_NULL 0
#define STATUS_KILL 1
#define STATUS_SOFTKILL 2
#define STATUS_DEACTIVATE 3
#define STATUS_REACTIVATE 4
#define STATUS_COPY1ST 5
#define STATUS_MOVE1ST 6
#define STATUS_COPY2ND 7
#define STATUS_MOVE2ND 8
#define STATUS_PACK 9
#define STATUS_AUTOSCALE 10
#define STATUS_REVERSE 11
#define STATUS_JOIN1ST 12
#define STATUS_JOIN2ND 13

#define STATUS_GRAPH_ACTIVATE 101
#define STATUS_GRAPH_KILL 102
#define STATUS_GRAPH_COPY 103
#define STATUS_GRAPH_FOCUS 104
#define STATUS_GRAPH_AUTO 106
#define STATUS_GRAPH_HIDE 107
#define STATUS_GRAPH_SHOW 108
#define STATUS_GRAPH_COPY1ST 109
#define STATUS_GRAPH_COPY2ND 110
#define STATUS_GRAPH_MOVE1ST 111
#define STATUS_GRAPH_MOVE2ND 112
#define STATUS_GRAPH_SWAP1ST 113
#define STATUS_GRAPH_SWAP2ND 114
#define STATUS_GRAPH_TYPE 115
#define STATUS_GRAPH_COPYSETS1ST 116
#define STATUS_GRAPH_COPYSETS2ND 117

#define STATUS_REGION_DEFINE 201
#define STATUS_REGION_KILL 202
#define STATUS_REGION_EXTRACT 203
#define STATUS_REGION_EVAL 204
#define STATUS_REGION_DEL 205

static int npages;

#define getdx(gno, setn) getcol(gno, setn, 2)
#define getdy(gno, setn) getcol(gno, setn, 3)

static Widget status_frame;
static Widget status_panel;
static Widget status_sw;
static Widget* select_status_item;
static Widget status_auto_redraw_tb;
static Dimension status_minwidth = 0;
static int curpage = 0;

static Widget header_w;
static Widget* labx;
static Widget* laby;

static Widget rc4, rc5, rc6;

static char header[256];

static XFontStruct* f;
static XmFontList xmf;

static void set_status_label(Widget w, char* s);
static void page_status_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void home_status_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void end_status_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void adjust_status_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void status_item_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void set_status_action(int cd);
static void status_op(Widget w, XtPointer client_data, XtPointer call_data);
static void toggle_status_auto_redraw(Widget w, XtPointer client_data, XtPointer call_data);

static void set_status_label(Widget w, char* buf) {
    Arg al;
    XmString ls;
    ls = XmStringCreateLtoR(buf, charset);
    XtSetArg(al, XmNlabelString, ls);
    XtSetValues(w, &al, 1);
    XmStringFree(ls);
}

void update_status(int gno, int itemtype, int itemno) {
    int i;
    void update_set_status(int gno, int setno);
    void update_graph_status(int gno);
    void update_region_status(int rno);

    update_set_lists(gno);
    if (status_frame) {
        set_status_label(header_w, header);
        switch (itemtype) {
        case STATUS_SETS:
            if (itemno < 0) {
                for (i = curpage * SPAGESIZE; i < SPAGESIZE * (curpage + 1); i++) {
                    if (i < g[gno].maxplot) {
                        update_set_status(gno, i);
                    } else {
                        set_status_label(labx[i - curpage * SPAGESIZE], "Set number out of range, unavailable");
                        set_status_label(laby[i - curpage * SPAGESIZE], " ");
                    }
                }
            }
            break;
        case STATUS_GRAPHS:
            if (itemno < 0) {
                for (i = curpage * SPAGESIZE; i < SPAGESIZE * (curpage + 1); i++) {
                    if (i < maxgraph) {
                        update_graph_status(i);
                    } else {
                        set_status_label(labx[i - curpage * SPAGESIZE], "Graph number out of range, unavailable");
                        set_status_label(laby[i - curpage * SPAGESIZE], " ");
                    }
                }
            }
            break;
        case STATUS_REGIONS:
            if (itemno < 0) {
                for (i = 0; i < MAXREGION; i++) {
                    update_region_status(i);
                }
            }
            break;
        }
    }
}

void update_graph_status(int gno) {
    int i;
    int nactive = 0;
    if (gno >= curpage * SPAGESIZE && gno < (curpage + 1) * SPAGESIZE) {
        if (status_frame && cur_statusitem == STATUS_GRAPHS) {
            for (i = 0; i < g[gno].maxplot; i++) {
                if (isactive_set(gno, i)) {
                    nactive++;
                }
            }
            if (gno == cg) {
                sprintf(buf, "  %2d    %3s    %3s    %6s    %d    %d  [Current graph]", gno, on_or_off(g[gno].active),
                        yes_or_no((!g[gno].hidden)), graph_types(g[gno].type, 0), g[gno].maxplot, nactive);
            } else {
                sprintf(buf, "  %2d    %3s    %3s    %6s    %d    %d", gno, on_or_off(g[gno].active),
                        yes_or_no((!g[gno].hidden)), graph_types(g[gno].type, 0), g[gno].maxplot, nactive);
            }
            set_status_label(labx[gno - curpage * SPAGESIZE], buf);
        }
    }
}

void update_region_status(int rno) {
    if (rno >= 0 && rno < MAXREGION) {
        if (status_frame && cur_statusitem == STATUS_REGIONS) {
            sprintf(buf, "  %2d    %3s   %6s", rno, on_or_off(rg[rno].active), region_types(rg[rno].type, 0));
            set_status_label(labx[rno], buf);
        }
    }
}

void update_set_status(int gno, int setno) {
    double x1, y1, x2, y2, xbar, ybar, xsd, ysd, dxbar, dybar, dxsd, dysd;
    int ix1, ix2;
    int iy1, iy2;
    char st[15], buf1[512], buf2[512];

    if (setno >= g[gno].maxplot) {
        return;
    }
    update_set_lists(gno);
    strcpy(st, "XY");
    if (setno >= curpage * SPAGESIZE && setno < (curpage + 1) * SPAGESIZE) {
        if (status_frame && cur_statusitem == STATUS_SETS && gno == cg) {
            if (isactive_set(gno, setno)) {
                minmax(getx(gno, setno), getsetlength(gno, setno), &x1, &x2, &ix1, &ix2);
                minmax(gety(gno, setno), getsetlength(gno, setno), &y1, &y2, &iy1, &iy2);
                xbar = 0.0;
                ybar = 0.0;
                xsd = 0.0;
                ysd = 0.0;
                dxbar = 0.0;
                dybar = 0.0;
                dxsd = 0.0;
                dysd = 0.0;
                stasum(getx(gno, setno), getsetlength(gno, setno), &xbar, &xsd, 0);
                stasum(gety(gno, setno), getsetlength(gno, setno), &ybar, &ysd, 0);
                switch (dataset_type(gno, setno)) {
                case SET_XY:
                    strcpy(st, "XY");
                    break;
                case SET_XYSTRING:
                    strcpy(st, "XY Str");
                    break;
                case SET_XYZ:
                    strcpy(st, "XY Z");
                    stasum(getdx(gno, setno), getsetlength(gno, setno), &dxbar, &dxsd, 0);
                    break;
                case SET_XYDX:
                    strcpy(st, "XY DX");
                    break;
                case SET_XYDY:
                    strcpy(st, "XY DY");
                    break;
                case SET_XYDXDX:
                    strcpy(st, "XY DXDX");
                    break;
                case SET_XYDYDY:
                    strcpy(st, "XY DYDY");
                    break;
                case SET_XYDXDY:
                    strcpy(st, "XY DXDY");
                    break;
                case SET_XYZW:
                    strcpy(st, "XY ZW");
                    break;
                case SET_XYRT:
                    strcpy(st, "XY R");
                    break;
                case SET_XYBOX:
                    strcpy(st, "XY BOX");
                    break;
                case SET_XYYY:
                    strcpy(st, "XY Y1 Y2");
                    break;
                case SET_XYXX:
                    strcpy(st, "XY X1 X2");
                    break;
                case SET_XYHILO:
                    strcpy(st, "XY HILO");
                    break;
                case SET_RAWSPICE:
                    strcpy(st, "Rawspice");
                    break;
                }
                sprintf(buf1, " %2d   %7d %4s %7s | X   %11.5g %7d %11.5g %7d %11.5g %11.5g  %s", setno,
                        getsetlength(gno, setno), on_or_off(g[gno].p[setno].active), st, x1, ix1, x2, ix2, xbar, xsd,
                        getcomment(gno, setno));
                sprintf(buf2, "%26s | Y   %11.5g %7d %11.5g %7d %11.5g %11.5g", "", y1, iy1, y2, iy2, ybar, ysd);
            } else if (g[gno].p[setno].deact) {
                sprintf(buf1, " %2d    De-activated (%s)", setno, getcomment(gno, setno));
                strcpy(buf2, " ");
            } else {
                sprintf(buf1, " %2d    Undefined", setno);
                strcpy(buf2, " ");
            }
            set_status_label(labx[setno - curpage * SPAGESIZE], buf1);
            set_status_label(laby[setno - curpage * SPAGESIZE], buf2);
        }
    }
}

void clear_status(void) {
    int i;
    for (i = 0; i < SPAGESIZE; i++) {
        set_status_label(labx[i], " ");
        set_status_label(laby[i], " ");
    }
}

void update_status_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    if (status_frame) {
        update_status_auto_redraw();
    }
    update_set_lists(cg);
    status_item_proc((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
    /*
        update_status(cg, cur_statusitem, -1);
    */
}

static void page_status_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int dir = (int)client_data;
    if (dir == 1) {
        curpage = (curpage + 1) % npages;
    } else {
        curpage = curpage - 1;
        if (curpage < 0) {
            curpage = npages - 1;
        }
    }
    update_status(cg, cur_statusitem, -1);
}

static void home_status_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    curpage = 0;
    update_status(cg, cur_statusitem, -1);
}

static void end_status_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    curpage = npages - 1;
    update_status(cg, cur_statusitem, -1);
}

static void adjust_status_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    Dimension status_width, tmp_width;
    Widget sw_vsb;
    int i;

    update_status_popup(NULL, NULL, NULL);
    XtVaGetValues(header_w, XmNwidth, &status_width, NULL);
    for (i = 0; i < SPAGESIZE; i++) {
        XtVaGetValues(labx[i], XmNwidth, &tmp_width, NULL);
        if (tmp_width > status_width) {
            status_width = tmp_width;
        }
        XtVaGetValues(laby[i], XmNwidth, &tmp_width, NULL);
        if (tmp_width > status_width) {
            status_width = tmp_width;
        }
    }
    sw_vsb = XtNameToWidget(status_sw, "VertScrollBar");
    if (sw_vsb != NULL) {
        XtVaGetValues(sw_vsb, XmNwidth, &tmp_width, NULL);
        status_width += tmp_width;
        XtVaGetValues(status_sw, XmNspacing, &tmp_width, NULL);
        status_width += tmp_width;
    }
    status_width += 10;
    if (status_width < status_minwidth) {
        status_width = status_minwidth;
    }
    XtVaSetValues(status_frame, XmNwidth, status_width, NULL);
}

static void status_item_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int cd;
    update_set_lists(cg);
    if (status_frame) {
        cd = GetChoice(select_status_item);

        switch (cd) {
        case 0:
            npages = g[cg].maxplot / SPAGESIZE;
            if (npages * SPAGESIZE != g[cg].maxplot) {
                npages++;
            }
            curpage = 0;
            cur_statusitem = STATUS_SETS;
            strcpy(header, " set#     n   stat    type | X/Y         min      ");
            strcat(header, "at         max      at        mean      stddev  comment");
            XtUnmanageChild(rc5);
            XtUnmanageChild(rc6);
            XtManageChild(rc4);
            break;
        case 1:
            npages = maxgraph / SPAGESIZE;
            if (npages * SPAGESIZE != maxgraph) {
                npages++;
            }
            curpage = 0;
            cur_statusitem = STATUS_GRAPHS;
            clear_status();
            sprintf(header, " Graph # Active  Show  Type  Max sets  # Active sets");
            XtUnmanageChild(rc4);
            XtUnmanageChild(rc6);
            XtManageChild(rc5);
            break;
        case 2:
            npages = MAXREGION / SPAGESIZE;
            if (npages * SPAGESIZE != MAXREGION) {
                npages++;
            }
            curpage = 0;
            cur_statusitem = STATUS_REGIONS;
            clear_status();
            sprintf(header, " Region # Active  Type");
            XtUnmanageChild(rc4);
            XtUnmanageChild(rc5);
            /*XtManageChild(rc6);*/
            set_status_action(STATUS_NULL);
            break;
        }
        set_status_label(header_w, header);
        update_status(cg, cur_statusitem, -1);
    }
}

/*
 * write the status to the results file
 */
void update_stuff_status(void) {
    int i, j;
    double x1, y1, x2, y2, xbar, ybar, xsd, ysd;

    strcpy(buf, "\nStatus\n");
    stufftext(buf, STUFF_START);
    for (j = 0; j < maxgraph; j++) {
        if (isactive_graph(j)) {
            if (j == cg) {
                sprintf(buf, "\nStatus of sets for graph %d (current)\n", cg);
            } else {
                sprintf(buf, "\nStatus of sets for graph %d\n", j);
            }
            stufftext(buf, STUFF_TEXT);
            sprintf(buf, " set#     n   X/Y          min         max        mean      stddev  comment\n");
            stufftext(buf, STUFF_TEXT);
            for (i = 0; i < g[j].maxplot; i++) {
                if (isactive_set(j, i)) {
                    getsetminmax(j, i, &x1, &x2, &y1, &y2);
                    stasum(getx(j, i), getsetlength(j, i), &xbar, &xsd, 0);
                    stasum(gety(j, i), getsetlength(j, i), &ybar, &ysd, 0);
                    sprintf(buf, " %3d  %7d  X   %11.5g %11.5g %11.5g %11.5g  %s\n", i, getsetlength(j, i), x1, x2,
                            xbar, xsd, getcomment(j, i));
                    stufftext(buf, STUFF_TEXT);
                    sprintf(buf, "               Y   %11.5g %11.5g %11.5g %11.5g\n", y1, y2, ybar, ysd);
                    stufftext(buf, STUFF_TEXT);
                }
            }
        }
    }
    strcpy(buf, "\n");
    stufftext(buf, STUFF_STOP);
}

static int status_curset;
static int status_curgraph;
static int status_curop;
static int status_set1;
static int status_set2;
static int status_g1;
static int status_g2;

static XmString infostring;
static Widget infolab;

static Window rcwin;

static void set_status_action(int cd) {
    char buf[256];
    status_curop = cd;
    switch (cd) {
    case STATUS_KILL:
    case STATUS_SOFTKILL:
        sprintf(buf, "Click on the set index number to kill the set");
        set_window_cursor(rcwin, 3);
        break;
    case STATUS_DEACTIVATE:
        sprintf(buf, "Click on the set index number to deactivate the set");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_REACTIVATE:
        sprintf(buf, "Click on the set index number to reactivate the set");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_COPY1ST:
        sprintf(buf, "Click on the set index number to copy from");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_MOVE1ST:
        sprintf(buf, "Click on the set index number to move from");
        set_window_cursor(rcwin, 4);
        break;
    case STATUS_COPY2ND:
        sprintf(buf, "Copy set %d in graph %d to...", status_set1, status_g1);
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_MOVE2ND:
        sprintf(buf, "Move set %d in graph %d to...", status_set1, status_g1);
        set_window_cursor(rcwin, 4);
        break;
    case STATUS_REVERSE:
        sprintf(buf, "Click on the set index number to reverse");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_JOIN1ST:
        sprintf(buf, "Click on the set index number to join to");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_JOIN2ND:
        sprintf(buf, "Join set %d in graph %d to...", status_set1, status_g1);
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_AUTOSCALE:
        sprintf(buf, "Click on the set index number to autoscale");
        set_window_cursor(rcwin, 0);
        break;
        /* graphs */
    case STATUS_GRAPH_ACTIVATE:
        sprintf(buf, "Click on the graph index number to activate");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_TYPE:
        sprintf(buf, "Click on the graph index number to set the graph type");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_HIDE:
        sprintf(buf, "Click on the graph index number to hide");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_SHOW:
        sprintf(buf, "Click on the graph index number to show");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_FOCUS:
        sprintf(buf, "Click on the graph index number to set the current graph");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_KILL:
        sprintf(buf, "Click on the graph index number to kill");
        set_window_cursor(rcwin, 3);
        break;
    case STATUS_GRAPH_AUTO:
        sprintf(buf, "Click on the graph index number to autoscale");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_COPY1ST:
        sprintf(buf, "Click on the graph index number to copy from");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_COPY2ND:
        sprintf(buf, "Copy graph %d to...", status_g1);
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_MOVE1ST:
        sprintf(buf, "Click on the graph index number to move from");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_MOVE2ND:
        sprintf(buf, "Move graph %d to...", status_g1);
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_SWAP1ST:
        sprintf(buf, "Click on the graph index number to move from");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_GRAPH_SWAP2ND:
        sprintf(buf, "Swap graph %d with...", status_g1);
        set_window_cursor(rcwin, 0);
        break;
        /* regions */
    case STATUS_REGION_DEFINE:
        sprintf(buf, "Click on a region number to define");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_REGION_KILL:
        sprintf(buf, "Click on a region number to kill");
        set_window_cursor(rcwin, 3);
        break;
    case STATUS_REGION_EXTRACT:
        sprintf(buf, "Click on a region number to extract points to the next available set");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_REGION_EVAL:
        sprintf(buf, "Click on a region number to evaluate");
        set_window_cursor(rcwin, 0);
        break;
    case STATUS_REGION_DEL:
        sprintf(buf, "Click on a region number in which to kill all points");
        set_window_cursor(rcwin, 3);
        break;
    case STATUS_NULL:
        sprintf(buf, "Idle...");
        set_window_cursor(rcwin, -1);
        break;
    }
    if (infostring) {
        XmStringFree(infostring);
    }
    infostring = XmStringCreateLtoR(buf, charset);
    XtVaSetValues(infolab, XmNlabelString, infostring, NULL);
}

void select_set(Widget w, XtPointer calld, XEvent* e) {
    extern int cset; /* defined in symwin.c for now TODO move to
                      * globals.h */
    int cd = (int)calld;
    if (e->type != ButtonPress) {
        return;
    }
    switch (e->xbutton.button) {
    case Button3:
        set_status_action(STATUS_NULL);
        return;
        break;
    case Button2:
        set_status_action(STATUS_NULL);
        return;
        break;
    }

    if (cur_statusitem == STATUS_SETS) {
        if (cd + curpage * SPAGESIZE >= g[cg].maxplot) {
            set_status_action(STATUS_NULL);
            errwin("Not that many sets\n");
            return;
        }
        status_curset = cd + curpage * SPAGESIZE;
        cd = status_curset;
        if (status_curop == STATUS_NULL && double_click((XButtonEvent*)e)) {
            cset = cd;
            set_window_cursor(rcwin, 5);
            define_symbols_popup(NULL, NULL, NULL);
            set_window_cursor(rcwin, -1);
            return;
        }
        switch (status_curop) {
        case STATUS_KILL:
            do_reactivate(cg, cd);
            do_kill(cg, cd, 0);
            if (status_auto_redraw) {
                drawgraph();
            }
            set_status_action(STATUS_KILL);
            break;
        case STATUS_SOFTKILL:
            do_reactivate(cg, cd);
            do_kill(cg, cd, 1);
            if (status_auto_redraw) {
                drawgraph();
            }
            set_status_action(STATUS_SOFTKILL);
            break;
        case STATUS_DEACTIVATE:
            if (isactive_set(cg, cd)) {
                do_deactivate(cg, cd);
                if (status_auto_redraw) {
                    drawgraph();
                }
                set_status_action(STATUS_DEACTIVATE);
            } else {
                errwin("Set not active, Deactivate requires an active set");
                set_status_action(STATUS_NULL);
            }
            break;
        case STATUS_REACTIVATE:
            do_reactivate(cg, cd);
            if (status_auto_redraw) {
                drawgraph();
            }
            set_status_action(STATUS_REACTIVATE);
            break;
        case STATUS_COPY1ST:
            status_set1 = cd;
            status_g1 = cg;
            if (isactive_set(cg, cd)) {
                set_status_action(STATUS_COPY2ND);
            } else {
                errwin("Set not active, Copy requires an active set");
                set_status_action(STATUS_NULL);
            }
            break;
        case STATUS_MOVE1ST:
            status_set1 = cd;
            status_g1 = cg;
            if (isactive_set(cg, cd)) {
                set_status_action(STATUS_MOVE2ND);
            } else {
                errwin("Set not active, Move requires an active set");
                set_status_action(STATUS_NULL);
            }
            break;
        case STATUS_COPY2ND:
            status_set2 = cd;
            status_g2 = cg;
            do_copyset(status_g1, status_set1, status_g2, status_set2);
            if (status_auto_redraw) {
                drawgraph();
            }
            set_status_action(STATUS_COPY1ST);
            break;
        case STATUS_MOVE2ND:
            status_set2 = cd;
            status_g2 = cg;
            do_moveset(status_g1, status_set1, status_g2, status_set2);
            if (status_auto_redraw) {
                drawgraph();
            }
            set_status_action(STATUS_MOVE1ST);
            break;
        case STATUS_REVERSE:
            if (isactive_set(cg, cd)) {
                do_reverse_sets(cd);
                if (status_auto_redraw) {
                    drawgraph();
                }
                set_status_action(STATUS_REVERSE);
            } else {
                errwin("Set not active, Reverse operates on active sets");
                set_status_action(STATUS_NULL);
            }
            break;
        case STATUS_JOIN1ST:
            status_set1 = cd;
            status_g1 = cg;
            if (isactive_set(cg, cd)) {
                set_status_action(STATUS_JOIN2ND);
            } else {
                errwin("Set not active, Join operates on active sets");
                set_status_action(STATUS_NULL);
            }
            break;
        case STATUS_JOIN2ND:
            status_set2 = cd;
            status_g2 = cg;
            if (isactive_set(cg, cd)) {
                if ((status_g1 == status_g2) && (status_set1 == status_set2)) {
                    errwin("Can't join set to itself, use copy then join");
                    set_status_action(STATUS_NULL);
                } else {
                    do_join_sets(status_g1, status_set1, status_g2, status_set2);
                    if (status_auto_redraw) {
                        drawgraph();
                    }
                    set_status_action(STATUS_JOIN1ST);
                }
            } else {
                errwin("Set not active, Join operates on active sets");
                set_status_action(STATUS_NULL);
            }
            break;
        case STATUS_AUTOSCALE:
            if (isactive_set(cg, cd)) {
                defaultsetgraph(cg, cd);
                default_axis(cg, g[cg].auto_type, X_AXIS);
                default_axis(cg, g[cg].auto_type, ZX_AXIS);
                default_axis(cg, g[cg].auto_type, Y_AXIS);
                default_axis(cg, g[cg].auto_type, ZY_AXIS);
                update_world(cg);
                if (status_auto_redraw) {
                    drawgraph();
                }
            } else {
                errwin("Set not active, Auto operates on active sets");
                set_status_action(STATUS_NULL);
            }
            break;
        default:
            set_status_action(STATUS_NULL);
            break;
        }
    } else if (cur_statusitem == STATUS_GRAPHS) {
        status_curgraph = cd + curpage * SPAGESIZE;
        cd = status_curgraph;
        if (status_curop == STATUS_NULL && double_click((XButtonEvent*)e)) {
            if (isactive_graph(cd)) {
                switch_current_graph(cg, cd);
            } else {
                errwin("Can't make an inactive graph the current graph");
            }
            return;
        }
        switch (status_curop) {
        case STATUS_GRAPH_KILL:
            if (isactive_graph(cd)) {
                kill_graph(cd);
                update_all(cg); /* current graph */
                drawgraph();
            }
            set_status_action(STATUS_GRAPH_KILL);
            break;
        case STATUS_GRAPH_ACTIVATE:
            set_graph_active(cd);
            update_all(cg);
            drawgraph();
            set_status_action(STATUS_GRAPH_ACTIVATE);
            break;
        case STATUS_GRAPH_TYPE:
            create_gtype_frame(NULL, (XtPointer)(intptr_t)cd, (XtPointer)NULL);
            /*
                    update_all(cg);
                    drawgraph();
            */
            set_status_action(STATUS_NULL);
            break;
        case STATUS_GRAPH_FOCUS:
            switch_current_graph(cg, cd);
            update_all(cg);
            drawgraph();
            set_status_action(STATUS_NULL);
            break;
        case STATUS_GRAPH_AUTO:
            autoscale_graph(cd, -3);
            update_all(cg);
            drawgraph();
            set_status_action(STATUS_NULL);
            break;
        case STATUS_GRAPH_HIDE:
            g[cd].hidden = TRUE;
            update_all(cg);
            drawgraph();
            set_status_action(STATUS_GRAPH_HIDE);
            break;
        case STATUS_GRAPH_SHOW:
            g[cd].hidden = FALSE;
            update_all(cg);
            drawgraph();
            set_status_action(STATUS_GRAPH_SHOW);
            break;
        case STATUS_GRAPH_COPY1ST:
            status_g1 = cd;
            set_status_action(STATUS_GRAPH_COPY2ND);
            break;
        case STATUS_GRAPH_MOVE1ST:
            status_g1 = cd;
            set_status_action(STATUS_GRAPH_MOVE2ND);
            break;
        case STATUS_GRAPH_SWAP1ST:
            status_g1 = cd;
            set_status_action(STATUS_GRAPH_SWAP2ND);
            break;
        case STATUS_GRAPH_COPY2ND:
            status_g2 = cd;
            if (status_g1 == status_g2) {
                errwin("Graph from and graph to are the same");
                set_status_action(STATUS_NULL);
                break;
            } else if (isactive_graph(status_g2)) {
                if (!yesno("Graph to copy to is active, kill it?", NULL, NULL, NULL)) {
                    set_status_action(STATUS_NULL);
                    break;
                }
            }
            copy_graph(status_g1, status_g2);
            update_all(cg);
            drawgraph();
            set_status_action(STATUS_GRAPH_COPY1ST);
            break;
        case STATUS_GRAPH_MOVE2ND:
            status_g2 = cd;
            if (status_g1 == status_g2) {
                errwin("Graph from and graph to are the same");
                set_status_action(STATUS_NULL);
                break;
            } else if (isactive_graph(status_g2)) {
                if (!yesno("Graph to move to is active, kill it?", NULL, NULL, NULL)) {
                    set_status_action(STATUS_NULL);
                    break;
                }
            }
            copy_graph(status_g1, status_g2);
            kill_graph(status_g1);
            update_all(cg);
            drawgraph();
            set_status_action(STATUS_GRAPH_MOVE1ST);
            break;
        case STATUS_GRAPH_SWAP2ND:
            status_g2 = cd;
            if (status_g1 == status_g2) {
                errwin("Graph from and graph to are the same");
                set_status_action(STATUS_NULL);
                break;
            }
            swap_graph(status_g1, status_g2);
            update_all(cg);
            drawgraph();
            set_status_action(STATUS_GRAPH_SWAP1ST);
            break;
        }
    } else if (cur_statusitem == STATUS_REGIONS) {
        status_curop = STATUS_NULL;
        set_window_cursor(rcwin, -1);
        switch (status_curop) {
        case STATUS_REGION_DEFINE:
            set_status_action(STATUS_REGION_DEFINE);
            break;
        case STATUS_REGION_KILL:
            set_status_action(STATUS_REGION_KILL);
            break;
        case STATUS_REGION_EXTRACT:
            set_status_action(STATUS_REGION_EXTRACT);
            break;
        case STATUS_REGION_EVAL:
            set_status_action(STATUS_REGION_EVAL);
            break;
        case STATUS_REGION_DEL:
            set_status_action(STATUS_REGION_DEL);
            break;
        }
    }
}

static void status_op(Widget w, XtPointer client_data, XtPointer call_data) {
    int cd = (int)client_data;
    if (cd == STATUS_PACK) {
        status_curop = STATUS_NULL;
        set_window_cursor(rcwin, -1);
        set_wait_cursor();
        packsets(cg);
        unset_wait_cursor();
        set_status_action(STATUS_NULL);
    } else {
        set_status_action(cd);
    }
}

static void toggle_status_auto_redraw(Widget w, XtPointer client_data, XtPointer call_data) {
    status_auto_redraw = (int)XmToggleButtonGetState(w);
}

void update_status_auto_redraw(void) {
    if (status_auto_redraw_tb) {
        XmToggleButtonSetState(status_auto_redraw_tb, status_auto_redraw == TRUE, False);
    }
}

void define_status_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    extern Display* disp;
    int i;
    XmString header_string;
    Dimension status_width, tmp_width;
    Widget wbut, rc, rc3, fr1, fr2, sw_vsb;
    set_wait_cursor();
    if (status_frame == NULL) {
        npages = g[cg].maxplot / SPAGESIZE;
        if (npages * SPAGESIZE != g[cg].maxplot) {
            npages++;
        }
        status_frame = XmCreateDialogShell(app_shell, "Status", NULL, 0);
        handle_close(status_frame);

        f = (XFontStruct*)XLoadQueryFont(disp, "fixed");
        xmf = XmFontListCreate(f, charset);

        status_panel = XmCreateForm(status_frame, "form", NULL, 0);

        status_sw = XtVaCreateManagedWidget("sw", xmScrolledWindowWidgetClass, status_panel, XmNscrollingPolicy,
                                            XmAUTOMATIC, XmNheight, 300, NULL);
        rc3 = XmCreateRowColumn(status_sw, "rc3", NULL, 0);
        header_w = XtVaCreateManagedWidget("header", xmLabelWidgetClass, rc3, XmNalignment, XmALIGNMENT_BEGINNING,
                                           XmNfontList, xmf, XmNrecomputeSize, True, NULL);
        labx = (Widget*)malloc(maxplot * sizeof(Widget));
        laby = (Widget*)malloc(maxplot * sizeof(Widget));
        for (i = 0; i < SPAGESIZE; i++) {
            labx[i] = XtVaCreateManagedWidget("X", xmLabelWidgetClass, rc3, XmNalignment, XmALIGNMENT_BEGINNING,
                                              XmNfontList, xmf, XmNrecomputeSize, True, NULL);
            laby[i] = XtVaCreateManagedWidget("Y", xmLabelWidgetClass, rc3, XmNalignment, XmALIGNMENT_BEGINNING,
                                              XmNfontList, xmf, XmNrecomputeSize, True, NULL);
            XtAddEventHandler(labx[i], ButtonPressMask, False, (XtEventHandler)select_set, (XtPointer)(intptr_t)i);
            XtAddEventHandler(laby[i], ButtonPressMask, False, (XtEventHandler)select_set, (XtPointer)(intptr_t)i);
        }
        XtManageChild(rc3);
        XtVaSetValues(status_sw, XmNworkWindow, rc3, NULL);

        fr1 = XmCreateFrame(status_panel, "fr1", NULL, 0);
        rc = XmCreateRowColumn(fr1, "rc", NULL, 0);
        infolab = XtVaCreateManagedWidget("Idle...", xmLabelWidgetClass, rc, XmNalignment, XmALIGNMENT_BEGINNING,
                                          XmNrecomputeSize, True, NULL);
        rc4 = XmCreateRowColumn(rc, "rc", NULL, 0);
        XtVaSetValues(rc4, XmNorientation, XmHORIZONTAL, NULL);
        wbut = XtVaCreateManagedWidget("Kill", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_KILL);
        wbut = XtVaCreateManagedWidget("Soft kill", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_SOFTKILL);
        wbut = XtVaCreateManagedWidget("Deact", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_DEACTIVATE);
        wbut = XtVaCreateManagedWidget("React", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_REACTIVATE);
        wbut = XtVaCreateManagedWidget("Copy", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_COPY1ST);
        wbut = XtVaCreateManagedWidget("Move", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_MOVE1ST);
        wbut = XtVaCreateManagedWidget("Auto", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_AUTOSCALE);
        wbut = XtVaCreateManagedWidget("Reverse", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_REVERSE);
        wbut = XtVaCreateManagedWidget("Join", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_JOIN1ST);
        wbut = XtVaCreateManagedWidget("Pack", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_PACK);
        wbut = XtVaCreateManagedWidget("Cancel", xmPushButtonWidgetClass, rc4, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_NULL);
        status_auto_redraw_tb = XtVaCreateManagedWidget("Auto redraw", xmToggleButtonWidgetClass, rc4, NULL);
        XtAddCallback(status_auto_redraw_tb, XmNvalueChangedCallback, (XtCallbackProc)toggle_status_auto_redraw,
                      (XtPointer)NULL);

        XtManageChild(rc4);
        XtManageChild(rc);
        XtManageChild(fr1);

        rc5 = XmCreateRowColumn(rc, "rc", NULL, 0);
        XtVaSetValues(rc5, XmNorientation, XmHORIZONTAL, NULL);
        wbut = XtVaCreateManagedWidget("Activate", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_ACTIVATE);
        /*
            wbut = XtVaCreateManagedWidget("Type", xmPushButtonWidgetClass, rc5,
                               NULL);
            XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) status_op, (XtPointer) STATUS_GRAPH_TYPE);
        */
        wbut = XtVaCreateManagedWidget("Copy", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_COPY1ST);
        wbut = XtVaCreateManagedWidget("Move", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_MOVE1ST);
        wbut = XtVaCreateManagedWidget("Swap", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_SWAP1ST);
        wbut = XtVaCreateManagedWidget("Hide", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_HIDE);
        wbut = XtVaCreateManagedWidget("Show", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_SHOW);
        wbut = XtVaCreateManagedWidget("Auto", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_AUTO);
        wbut = XtVaCreateManagedWidget("Focus", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_FOCUS);
        wbut = XtVaCreateManagedWidget("Kill", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_GRAPH_KILL);
        wbut = XtVaCreateManagedWidget("Cancel", xmPushButtonWidgetClass, rc5, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_NULL);

        rc6 = XmCreateRowColumn(rc, "rc", NULL, 0);

        XtVaSetValues(rc6, XmNorientation, XmHORIZONTAL, NULL);
        wbut = XtVaCreateManagedWidget("Define", xmPushButtonWidgetClass, rc6, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_REGION_DEFINE);

        wbut = XtVaCreateManagedWidget("Kill", xmPushButtonWidgetClass, rc6, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_REGION_KILL);

        wbut = XtVaCreateManagedWidget("Extract", xmPushButtonWidgetClass, rc6, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_REGION_EXTRACT);

        wbut = XtVaCreateManagedWidget("Evaluate", xmPushButtonWidgetClass, rc6, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_REGION_EVAL);

        wbut = XtVaCreateManagedWidget("Delete pts", xmPushButtonWidgetClass, rc6, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_REGION_DEL);

        wbut = XtVaCreateManagedWidget("Report", xmPushButtonWidgetClass, rc6, NULL);

        wbut = XtVaCreateManagedWidget("Cancel", xmPushButtonWidgetClass, rc6, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)status_op, (XtPointer)STATUS_NULL);

        fr2 = XmCreateFrame(status_panel, "fr2", NULL, 0);
        rc = XmCreateRowColumn(fr2, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);

        wbut = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)destroy_dialog, status_frame);

        wbut = XtVaCreateManagedWidget("Update", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)update_status_popup, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("Write", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)update_stuff_status, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("Page+", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)page_status_proc, (XtPointer)1);

        wbut = XtVaCreateManagedWidget("Page-", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)page_status_proc, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("Home", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)home_status_proc, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("End", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)end_status_proc, (XtPointer)0);
        wbut = XtVaCreateManagedWidget("Adjust", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)adjust_status_proc, (XtPointer)0);

        select_status_item = CreatePanelChoice(rc, "Display: ", 4, "Sets", "Graphs", "Regions", NULL, 0);
        XtAddCallback(select_status_item[2], XmNactivateCallback, (XtCallbackProc)status_item_proc, (XtPointer)0);
        XtAddCallback(select_status_item[3], XmNactivateCallback, (XtCallbackProc)status_item_proc, (XtPointer)1);
        XtAddCallback(select_status_item[4], XmNactivateCallback, (XtCallbackProc)status_item_proc, (XtPointer)2);
        wbut = XtVaCreateManagedWidget("Help", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)HelpCB, (XtPointer) "data.html#status");

        XtManageChild(rc);
        XtManageChild(fr2);

        XtVaSetValues(status_sw, XmNtopAttachment, XmATTACH_FORM, XmNleftAttachment, XmATTACH_FORM, XmNrightAttachment,
                      XmATTACH_FORM, XmNbottomAttachment, XmATTACH_WIDGET, XmNbottomWidget, fr1, NULL);
        XtVaSetValues(fr1, XmNleftAttachment, XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, XmNbottomAttachment,
                      XmATTACH_WIDGET, XmNbottomWidget, fr2, NULL);
        XtVaSetValues(fr2, XmNleftAttachment, XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, XmNbottomAttachment,
                      XmATTACH_FORM, NULL);
        XtManageChild(status_panel);

        rcwin = XtWindow(rc3);
        update_status_popup(NULL, NULL, NULL);
        XtVaGetValues(status_sw, XmNwidth, &status_minwidth, NULL);
        XtVaGetValues(header_w, XmNlabelString, &header_string, NULL);
        status_width = XmStringWidth(xmf, header_string);
        sw_vsb = XtNameToWidget(status_sw, "VertScrollBar");
        if (sw_vsb != NULL) {
            XtVaGetValues(sw_vsb, XmNwidth, &tmp_width, NULL);
            status_width += tmp_width;
            XtVaGetValues(status_sw, XmNspacing, &tmp_width, NULL);
            status_width += tmp_width;
        }
        status_width += 25;
        if (status_width < status_minwidth) {
            status_width = status_minwidth;
        }
        XtVaSetValues(status_frame, XmNwidth, status_width, NULL);
    }
    XtRaise(status_frame);
    update_status_popup(NULL, NULL, NULL);
    unset_wait_cursor();
}

/*
 * say a few things about grtool, number of graphs, set size
 */
static Widget about_frame;
static Widget about_panel;

void create_about_grtool(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget wbut, rc;
    char buf[1024];

    set_wait_cursor();
    if (about_frame == NULL) {
        XmGetPos(app_shell, 0, &x, &y);
        about_frame = XmCreateDialogShell(app_shell, "About", NULL, 0);
        handle_close(about_frame);
        XtVaSetValues(about_frame, XmNx, x, XmNy, y, NULL);
        about_panel = XmCreateRowColumn(about_frame, "about_rc", NULL, 0);

        sprintf(buf, "%s-%d.%d.%d %s", version, MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER);
        XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        sprintf(buf, "Max number of sets per graph = %d", maxplot);
        XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        sprintf(buf, "Max scratch array length = %d", MAXARR);
        XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        sprintf(buf, "Max number of graphs = %d", maxgraph);
        XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        sprintf(buf, "Max number of lines = %d", maxlines);
        XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        sprintf(buf, "Max number of boxes = %d", maxboxes);
        XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        sprintf(buf, "Max number of strings = %d", maxstr);
        XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);
        sprintf(buf, "The home of xmgr is http://plasma-gate.weizmann.ac.il/Xmgr/");
        XtVaCreateManagedWidget(buf, xmLabelWidgetClass, about_panel, NULL);

        rc = XmCreateRowColumn(about_panel, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        wbut = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, rc, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)about_frame);
        XtManageChild(rc);

        XtManageChild(about_panel);
    }
    XtRaise(about_frame);
    unset_wait_cursor();
}
