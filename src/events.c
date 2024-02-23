/* $Id: events.c,v 1.5 1995/07/01 04:53:30 pturner Exp pturner $
 *
 * event handler
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#ifndef VMS
#include <sys/param.h>
#endif
#include <time.h>
#include <string.h>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/keysym.h>

#include "globals.h"
#include "draw.h"
#include "protos.h"

#include <Xm/Xm.h>

#include "motifinc.h"

extern Widget legend_x_panel, legend_y_panel;     /* from symwin.c */
extern Widget timestamp_x_item, timestamp_y_item; /* from miscwin.c */
extern Widget canvas;
extern Widget loclab;
extern Widget arealab;
extern Widget perimlab;
extern Widget locate_item;
extern Widget locate_point_item;
extern Widget stack_depth_item;
extern Widget curw_item;
extern XmStringCharSet charset;
extern XmString string, astring, pstring;
extern XmString sdstring, cystring;
extern Colormap mycmap;
static Arg al;

extern int inpipe;

/* TODO this doesn't belong here */
int cursortype = 0;
static int cursor_oldx = -1, cursor_oldy = -1;

/* for pointer based set operations */
static int setno1, setno2, graphno1, graphno2, loc1, loc2;

/*
 * xlib objects for drawing
 */
Display* disp;
GC gc;
GC gcxor;
GC gcclr;
Window xwin;

XGCValues gc_val;
extern int win_h, win_w; /* declared in xvlib.c */

int rectflag = 0;    /* if an xor'ed rectangle is drawn with mouse */
int rubber_flag = 0; /* set rubber band line */
int mbox_flag = 0;   /* moving box attached to cursor */
int mline_flag = 0;  /* moving line attached to cursor */

int go_locateflag = TRUE; /* locator */

int add_setno; /* set to add points - set in ptswin.c */
int add_at;    /* where to begin inserting points in the set */
int move_dir;  /* restriction on point movement */

void draw_focus(int gno);
void set_action(int act);
void select_line(int x1, int y1, int x2, int y2);

/*
 * variables for the canvas event proc
 */
static int sx, sy;
static int old_x, old_y;
static int xs, ys;
static int action_flag = 0;
static int setindex = 0;
static int setnumber = 0;

/*
 * variables for the text handling routine
 */
static int strx = 0, stry = 0;
static int drawx = 0, drawy = 0;
static char tmpstr[256];
static int justflag = 0;
static double si = 0.0;
static double co = 1.0;
static char* tmpstrins; /* pointer to insertion point in tmpstr */

/*
 * for region, area and perimeter computation
 */
#define MAX_AREA_POLY 200
int narea_pts = 0;
int region_pts = 0;
extern int regiontype;
int regionlinkto = 0;

double area_polyx[MAX_AREA_POLY];
double area_polyy[MAX_AREA_POLY];
int iax[MAX_AREA_POLY];
int iay[MAX_AREA_POLY];

/* note: do_hardcopy() has been moved to plotone.c */

/*
 * draw all active graphs, when graphs are drawn, draw the focus markers
 */
void drawgraph(void) {
    int i;
    extern int drawimage_flag;

    if (inwin && (auto_redraw || force_redraw)) {
        if (cursortype) {
            cursor_oldx = cursor_oldy = -1;
        }
        set_wait_cursor();
        initgraphics(tdevice);
        if (drawimage_flag) {
            drawimage();
        }
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
        defineworld(g[cg].w.xg1, g[cg].w.yg1, g[cg].w.xg2, g[cg].w.yg2, islogx(cg), islogy(cg));
        viewport(g[cg].v.xv1, g[cg].v.yv1, g[cg].v.xv2, g[cg].v.yv2);
        draw_focus(cg);
        leavegraphics();
        force_redraw = FALSE;
        unset_wait_cursor();
    }
}

/*
 * force a redraw when auto-redraw is OFF
 */
void doforce_redraw(void) {
    double wx, wy;

    force_redraw = TRUE;
    if (tmpstr[0]) {
        device2world(strx, win_h - stry, &wx, &wy);
        define_string(tmpstr, wx, wy);
        tmpstr[0] = 0;
    }
    inwin = TRUE; /* just in case refresh was never called on start up */

    drawgraph();
}

/*
 * action callback
 */
void set_actioncb(Widget w, XtPointer client_data, XtPointer call_data) {
    int func = (int)client_data;
    set_action(0);
    set_action(func);
}

/*
 * repaint proc
 */
void refresh(Widget w, XtPointer client_data, XmDrawingAreaCallbackStruct* cbs) {
    Arg args[2];
    Dimension ww, wh;
    static int inc = 0;
    extern char batchfile[];

    disp = XtDisplay(canvas);
    xwin = XtWindow(canvas);
    XtSetArg(args[0], XmNwidth, &ww);
    XtSetArg(args[1], XmNheight, &wh);
    XtGetValues(canvas, args, 2);
    win_h = wh;
    win_w = ww;
    if (!inc) {
        inwin = TRUE;
        inc++;

        if (batchfile[0]) {
            getparms(batchfile);
        }

        if (inpipe) {
            getdata(cg, "STDIN", SOURCE_STDIN, SET_XY);
            inpipe = 0;
        }
    } else {
        int w, h;
        if (cbs->event->xexpose.count != 0)
            return;
        if (page_layout == PAGE_FREE) {
            get_default_canvas_size(&w, &h);
            set_canvas_size(w, h, 0);
            if (allow_refresh) {
                drawgraph();
            }
        }
        if (!DoesBackingStore(DefaultScreenOfDisplay(disp))) {
            if (cbs->event->type == Expose) {
                if (cbs->event->xexpose.count != 0) {
                    return;
                }
            }
            if (backingstore) {
                refresh_from_backpix();
                draw_focus(cg);
                if (rectflag) {
                    select_region(sx, sy, old_x, old_y);
                }
                if (rubber_flag) {
                    select_line(sx, sy, old_x, old_y);
                }
            }
        }
    }
}

/*
 * resize proc
 */
void resize(Widget w, XtPointer data, XEvent* event) {
#if defined(DEBUG)
    if (debuglevel == 7) {
        printf("Call to resize() event type == %d sent from %d\n", event->type, event->xany.send_event);
    }
#endif
    if (event->type == ConfigureNotify) {
        if (page_layout == PAGE_FREE) {
            Arg args[2];
            Dimension ww, wh;

            disp = XtDisplay(canvas);
            xwin = XtWindow(canvas);
            XtSetArg(args[0], XmNwidth, &ww);
            XtSetArg(args[1], XmNheight, &wh);
            XtGetValues(canvas, args, 2);
            win_h = wh;
            win_w = ww;
            get_default_canvas_size(&win_w, &win_h);
            set_canvas_size(win_w, win_h, 0);
        }

        inwin = TRUE; /* just in case refresh was never called on start up */

        if (allow_refresh) {
            drawgraph();
        }
    }
}

/*
 * for the goto point feature
 */
void setpointer(int x, int y) { XWarpPointer(disp, None, xwin, 0, None, win_w, win_h, x, y); }

/*
 * locator on main_panel
 */
void getpoints(int x, int y) {
    double wx, wy, xtmp, ytmp;
    double dsx = 0.0, dsy = 0.0;
    int newg;
    char buf[256];
    extern char locator_format[];

    device2world(x, y, &wx, &wy);
    if (g[cg].pointset) {
        dsx = g[cg].dsx;
        dsy = g[cg].dsy;
    }
    if (focus_policy == FOCUS_FOLLOWS) {
        if ((newg = iscontained(cg, wx, wy)) != cg) {
            draw_focus(cg);
            cg = newg;
            defineworld(g[cg].w.xg1, g[cg].w.yg1, g[cg].w.xg2, g[cg].w.yg2, islogx(cg), islogy(cg));
            viewport(g[cg].v.xv1, g[cg].v.yv1, g[cg].v.xv2, g[cg].v.yv2);
            draw_focus(cg);
            make_format(cg);
            device2world(x, y, &wx, &wy);
            update_all(cg);
        }
    }
    if (!go_locateflag) {
        return;
    }
    if (g[cg].type == GRAPH_POLAR) {
        double r, theta;
        xy2r(wx, wy, &r, &theta);
        sprintf(buf, "G%1d: R, T = [%f, %f]", cg, r, theta * 180.0 / M_PI);
    } else if (g[cg].type == GRAPH_SMITH) {
        double r, x;
        if (wx * wx + wy * wy <= 1.0) {
            uv2rx(wx, wy, &r, &x);
            sprintf(buf, "G%1d: R, X = [%f, %f]", cg, r, x);
        } else {
            sprintf(buf, "G%1d: Out of bounds", cg);
        }
    } else {
        switch (g[cg].pt_type) {
        case 0:
            xtmp = wx;
            ytmp = wy;
            {
                char s1[30], s2[30];
                int form = g[cg].fx;

                create_ticklabel(form, g[cg].px, wx, s1);
                form = g[cg].fy;
                create_ticklabel(form, g[cg].py, wy, s2);
                sprintf(buf, "G%1d: X, Y = [%s, %s]", cg, s1, s2);
            }
            break;
        case 1:
            xtmp = wx - dsx;
            ytmp = wy - dsy;
            sprintf(buf, locator_format, cg, xtmp, ytmp);
            break;
        case 2:
            xtmp = hypot(dsx - wx, dsy - wy);
            ytmp = 0.0;
            sprintf(buf, locator_format, cg, xtmp, ytmp);
            break;
        case 3:
            if (dsx - wx != 0.0 || dsy - wy != 0.0) {
                xtmp = hypot(dsx - wx, dsy - wy);
                ytmp = 180.0 + 180.0 / M_PI * atan2(dsy - wy, dsx - wx);
                sprintf(buf, locator_format, cg, xtmp, ytmp);
            } else {
                sprintf(buf, "ERROR: dx = dy = 0.0");
            }
            break;
        case 4:
            xtmp = xconv(wx);
            ytmp = yconv(wy);
            sprintf(buf, locator_format, cg, xtmp, ytmp);
            break;
        case 5:
            sprintf(buf, locator_format, cg, x, y);
            break;
        }
    }
    XmStringFree(string);
    string = XmStringCreateLtoR(buf, charset);
    XtSetArg(al, XmNlabelString, string);
    XtSetValues(loclab, &al, 1);
}

/*
 * for world stack
 */
void set_stack_message(void) {
    if (stack_depth_item) {
        sprintf(buf, " SD:%1d ", g[cg].ws_top);
        XmStringFree(sdstring);
        sdstring = XmStringCreateLtoR(buf, charset);
        XtSetArg(al, XmNlabelString, sdstring);
        XtSetValues(stack_depth_item, &al, 1);
        sprintf(buf, " CW:%1d ", g[cg].curw);
        XmStringFree(cystring);
        cystring = XmStringCreateLtoR(buf, charset);
        XtSetArg(al, XmNlabelString, cystring);
        XtSetValues(curw_item, &al, 1);
    }
}

/*
 * rubber band line
 */
void select_line(int x1, int y1, int x2, int y2) { XDrawLine(disp, xwin, gcxor, x1, y1, x2, y2); }

/*
 * draw a box on the display
 */
void draw_rectangle(int x1, int y1, int x2, int y2) { XDrawRectangle(disp, xwin, gc, x1, y1, x2, y2); }

/*
 * draw an xor'ed box
 */
void select_region(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx < 0) {
        iswap(&x1, &x2);
        dx = -dx;
    }
    if (dy < 0) {
        iswap(&y1, &y2);
        dy = -dy;
    }
    XDrawRectangle(disp, xwin, gcxor, x1, y1, dx, dy);
}

/*
 * draw the graph focus indicators
 */
void draw_focus(int gno) {
    int ix1, iy1, ix2, iy2;

    set_stack_message();
    if (draw_focus_flag == TRUE) {
        world2deviceabs(g[gno].w.xg1, g[gno].w.yg1, &ix1, &iy1);
        world2deviceabs(g[gno].w.xg2, g[gno].w.yg2, &ix2, &iy2);
        XFillRectangle(disp, xwin, gcxor, ix1 - 5, iy1 - 5, 10, 10);
        XFillRectangle(disp, xwin, gcxor, ix1 - 5, iy2 - 5, 10, 10);
        XFillRectangle(disp, xwin, gcxor, ix2 - 5, iy2 - 5, 10, 10);
        XFillRectangle(disp, xwin, gcxor, ix2 - 5, iy1 - 5, 10, 10);
        /* TODO
            XFillRectangle(disp, xwin, gcxor, (ix1 + ix2) / 2 - 5, iy1 - 5, 10, 10);
            XFillRectangle(disp, xwin, gcxor, (ix1 + ix2) / 2 - 5, iy2 - 5, 10, 10);
        */
    }
}

/*
 * draw a cursor for text writing
 * TODO: fix the rotation problems (cursor doesn't track)
 */
void update_text_cursor(char* s, int x, int y, char* sp)
/* s -complete text string, sp - insertion point in s */
{
    int hgt, tx, xtx, ytx, xhgt, yhgt;
    char stmp[256];

    strcpy(stmp, s);
    if ((sp - s >= 255) || (sp - s < 0)) {
        return;
    }
    stmp[sp - s] = '\0';

    hgt = stringextenty(charsize * xlibcharsize, "N") / 2;

    switch (string_just) {
    case 0:
        /* left just */
        tx = stringextentx(charsize * xlibcharsize, stmp);
        xtx = (int)tx * co;
        ytx = (int)tx * si;
        xhgt = (int)-hgt * si;
        yhgt = (int)hgt * co;
        break;
    case 1:
        /* right just */
        tx = stringextentx(charsize * xlibcharsize, s) - stringextentx(charsize * xlibcharsize, stmp);
        xtx = -(int)tx * co;
        ytx = -(int)tx * si;
        xhgt = (int)-hgt * si;
        yhgt = (int)hgt * co;
        break;
    case 2:
        /* centered */
        tx = (double)stringextentx(charsize * xlibcharsize, stmp) -
             (double)stringextentx(charsize * xlibcharsize, s) / 2.;
        xtx = (int)tx * co;
        ytx = (int)tx * si;
        xhgt = (int)-hgt * si;
        yhgt = (int)hgt * co;
        break;
    default:
        /* should never come here */
        xtx = ytx = xhgt = yhgt = 0;
        errmsg("Internal error, update_text_cursor called with wrong argument");
    }

    select_line(x + xtx + xhgt, win_h - (y + ytx + yhgt), x + xtx - xhgt, win_h - (y + ytx - yhgt));
}

void set_default_message(char* buf) {
    char *str, hbuf[MAXHOSTNAMELEN + 1];
    struct tm tm, *localtime();
    time_t time_value;
    (void)time(&time_value);
    tm = *localtime(&time_value);
    str = asctime(&tm);
    str[strlen(str) - 1] = 0;

    (void)gethostname(hbuf, MAXHOSTNAMELEN);

    sprintf(buf, "%s, %s, %s, %s", hbuf, DisplayString(disp), str, docname);
}

/*
 * set the action_flag to the desired action (actions are
 * defined in defines.h), if 0 then cleanup the results
 * from previous actions.
 */
void set_action(int act) {
    char tmpbuf[128];
    if (action_flag == STR_LOC) {
        double wx, wy;

        update_text_cursor(tmpstr, strx, stry, tmpstrins);
        setcharsize(grdefaults.charsize);
        setfont(grdefaults.font);
        setcolor(grdefaults.color);
        setlinestyle(grdefaults.lines);
        setlinewidth(grdefaults.linew);
        if (tmpstr[0]) {
            device2world(strx, win_h - stry, &wx, &wy);
            define_string(tmpstr, wx, wy);
            tmpstr[0] = 0;
        }
    }
    /*
     * indicate what's happening with a message in the left footer
     */
    switch (action_flag = act) {
    case DEL_OBJECT:
        set_cursor(3);
        echomsg("Delete object");
        break;
    case EDIT_OBJECT:
        set_cursor(0);
        echomsg("Click near a line, box, or string");
        break;
    case MOVE_OBJECT_1ST:
        set_cursor(4);
        echomsg("Pick object to move");
        break;
    case MOVE_OBJECT_2ND:
        echomsg("Place object");
        break;
    case COPY_OBJECT1ST:
        set_cursor(4);
        echomsg("Pick object to copy");
        break;
    case COPY_OBJECT2ND:
        echomsg("Place object");
        break;
    case MAKE_BOX_1ST:
        set_cursor(0);
        echomsg("First corner of box");
        break;
    case MAKE_BOX_2ND:
        echomsg("Second corner of box");
        break;
    case STR_LOC1ST:
        set_cursor(0);
        echomsg("Pick start of text line");
        break;
    case STR_LOC2ND:
        echomsg("Pick end of text line");
        break;
    case MAKE_LINE_1ST:
        set_cursor(0);
        echomsg("Pick beginning of line");
        break;
    case MAKE_LINE_2ND:
        echomsg("Pick end of line");
        break;
    case MAKE_ELLIP_1ST:
        set_cursor(0);
        echomsg("Pick beginning of bounding box for ellipse");
        break;
    case MAKE_ELLIP_2ND:
        echomsg("Pick opposite corner");
        break;
    case STR_EDIT:
        set_cursor(2);
        echomsg("Edit string");
        break;
    case STR_LOC:
        set_cursor(2);
        echomsg("Pick beginning of text");
        break;
    case FIND_POINT:
        set_cursor(1);
        echomsg("Find points");
        break;
    case TRACKER:
        set_cursor(1);
        echomsg("Tracker");
        break;
    case DEF_REGION:
        set_cursor(0);
        echomsg("Define region");
        break;
    case DEF_REGION1ST:
        set_cursor(0);
        echomsg("Pick first point for region");
        break;
    case DEF_REGION2ND:
        echomsg("Pick second point for region");
        break;
    case COMP_AREA:
        set_cursor(0);
        echomsg("Compute area");
        break;
    case COMP_PERIMETER:
        set_cursor(0);
        echomsg("Compute perimeter");
        break;
    case DISLINE1ST:
        set_cursor(0);
        echomsg("Pick start of line for distance computation");
        break;
    case DISLINE2ND:
        set_cursor(0);
        echomsg("Pick ending point");
        break;
    case SEL_POINT:
        set_cursor(0);
        echomsg("Pick reference point");
        break;
    case DEL_POINT:
        set_cursor(3);
        echomsg("Delete point");
        break;
    case MOVE_POINT1ST:
        set_cursor(4);
        echomsg("Pick point to move");
        break;
    case MOVE_POINT2ND:
        echomsg("Pick final location");
        break;
    case ADD_POINT:
        set_cursor(0);
        echomsg("Add point");
        break;
    case ADD_POINT1ST:
        set_cursor(0);
        echomsg("Pick 1st control point");
        break;
    case ADD_POINT2ND:
        set_cursor(0);
        echomsg("Pick 2nd control point");
        break;
    case ADD_POINT3RD:
        set_cursor(0);
        echomsg("Pick 3rd control point");
        break;
    case PAINT_POINTS:
        set_cursor(0);
        echomsg("Paint points - hold left mouse button down and move");
        break;
    case AUTO_NEAREST:
        set_cursor(0);
        echomsg("Autoscale on nearest set - click near a point of the set to autoscale");
        break;
    case KILL_NEAREST:
        set_cursor(3);
        echomsg("Kill nearest set - click near a point of the set to kill");
        break;
    case COPY_NEAREST1ST:
        set_cursor(0);
        echomsg("Copy nearest set - click near a point of the set to copy");
        break;
    case COPY_NEAREST2ND:
        set_cursor(0);
        sprintf(tmpbuf, "Selected S%1d in graph %d, click in the graph to place the copy", setno1, graphno1);
        echomsg(tmpbuf);
        break;
    case MOVE_NEAREST1ST:
        set_cursor(4);
        echomsg("Move nearest set - click near a point of the set to move");
        break;
    case MOVE_NEAREST2ND:
        set_cursor(4);
        sprintf(tmpbuf, "Selected S%1d in graph %d, click in the graph to move the set", setno1, graphno1);
        echomsg(tmpbuf);
        break;
    case JOIN_NEAREST1ST:
        set_cursor(0);
        echomsg("Join 2 sets - click near a point of the first set");
        break;
    case JOIN_NEAREST2ND:
        set_cursor(0);
        echomsg("Join 2 sets - click near a point of the second set");
        break;
    case DELETE_NEAREST1ST:
        set_cursor(3);
        echomsg("Delete points in a set - click near a point of the start of the range to  delete");
        break;
    case DELETE_NEAREST2ND:
        set_cursor(3);
        echomsg("Delete points in a set - click near the end of the range to delete");
        break;
    case REVERSE_NEAREST:
        set_cursor(0);
        echomsg("Reverse order of nearest set - click near a point of the set to reverse");
        break;
    case DEACTIVATE_NEAREST:
        set_cursor(0);
        echomsg("Deactivate nearest set - click near a point of the set to deactivate");
        break;
    case LEG_LOC:
        set_cursor(0);
        echomsg("Place legend");
        break;
    case ZOOM_1ST:
        set_cursor(0);
        echomsg("Pick first corner for zoom");
        break;
    case ZOOM_2ND:
        echomsg("Pick second corner for zoom");
        break;
    case ZOOMX_1ST:
        set_cursor(0);
        echomsg("Pick first point for zoom along X-axis");
        break;
    case ZOOMX_2ND:
        echomsg("Pick second point for zoom along X-axis");
        break;
    case ZOOMY_1ST:
        set_cursor(0);
        echomsg("Pick first point for zoom along Y-axis");
        break;
    case ZOOMY_2ND:
        echomsg("Pick second point for zoom along Y-axis");
        break;
    case VIEW_1ST:
        set_cursor(0);
        echomsg("Pick first corner of viewport");
        break;
    case VIEW_2ND:
        echomsg("Pick second corner of viewport");
        break;
    case PLACE_TIMESTAMP:
        set_cursor(0);
        echomsg("Click at the location for the timestamp");
        break;
    case PICK_SET:
    case PICK_EXPR:
    case PICK_HISTO:
    case PICK_FOURIER:
    case PICK_RUNAVG:
    case PICK_REG:
    case PICK_SAMPLE:
    case PICK_PRUNE:
        set_cursor(0);
        echomsg("Click near a point in the set to select");
        break;
    case PICK_BREAK:
        set_cursor(0);
        echomsg("Click near a point in a set to use as the break point");
        break;
    case 0:
        set_cursor(-1);
        set_default_message(buf);
        set_left_footer(buf);

        if (rectflag) {
            select_region(sx, sy, old_x, old_y);
            rectflag = 0;
        }
        if (rubber_flag) {
            select_line(sx, sy, old_x, old_y);
            rubber_flag = 0;
        }
        if (mbox_flag) {
            select_region(sx, sy, xs, ys);
            mbox_flag = 0;
        }
        if (mline_flag) {
            select_line(sx, sy, xs, ys);
            mline_flag = 0;
        }
        slice_first = FALSE;
        break;
    }
}

/*
 * update string drawn on the canvas
 */
void do_text_string(int op, int c)
/*
 * op = operation:    0 -> delete 1 char
                      1 -> add 1 character
                      2 -> nothing
   c = character to add
 */
{
    drawx = strx;
    drawy = stry;

    if (tmpstrins == 0)
        return; /* tmpstrins is undefined at start*/
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
    set_write_mode(0);
    dispstrxlib(drawx, drawy, string_rot, tmpstr, justflag, 0);
    switch (op) {
    case 0:
        if ((int)strlen(tmpstr) > 0) {
            memmove(tmpstrins - 1, tmpstrins, strlen(tmpstrins) + 1);
            tmpstrins -= 1;
        }
        break;
    case 1:
        memmove(tmpstrins + 1, tmpstrins, strlen(tmpstrins) + 1);
        *tmpstrins = c;
        tmpstrins += 1;
        break;
    case 2:
        break;
    }
    set_write_mode(1);
    dispstrxlib(drawx, drawy, string_rot, tmpstr, justflag, 0);
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
}

/*
 * adjust text insertion point
 */
void update_tmpstrins(KeySym key) {
    drawx = strx;
    drawy = stry;
    set_write_mode(0);
    dispstrxlib(drawx, drawy, string_rot, tmpstr, justflag, 0);
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
    switch (key) {
    case XK_Left:
        if (tmpstrins > tmpstr)
            tmpstrins -= 1;
        break;
    case XK_Right:
        if (*tmpstrins != '\0')
            tmpstrins += 1;
        break;
    default:
        break;
    }
    set_write_mode(1);
    dispstrxlib(drawx, drawy, string_rot, tmpstr, justflag, 0);
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
}

void update_tmpstrins2(int num) {
    char* stmp2;

    drawx = strx;
    drawy = stry;
    set_write_mode(0);
    dispstrxlib(drawx, drawy, string_rot, tmpstr, justflag, 0);
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
    switch (num) {
    case 1: /* ^e go to end of string */
        if (tmpstrins >= tmpstr)
            tmpstrins += strlen(tmpstrins);
        break;
    case 2: /* ^a go to start of string */
        if (tmpstrins > tmpstr)
            tmpstrins -= strlen(tmpstr) - strlen(tmpstrins);
        break;
    case 3: /* ^k delete to end of string */
        if (tmpstrins >= tmpstr)
            *tmpstrins = '\0';
        break;
    case 4: /* ^u delete to start of string */
        if (tmpstrins > tmpstr) {
            stmp2 = tmpstrins;
            stmp2 -= strlen(tmpstr) - strlen(tmpstrins);
            strcpy(tmpstr, tmpstrins);
            tmpstrins = stmp2;
        }
        break;
    }
    set_write_mode(1);
    dispstrxlib(drawx, drawy, string_rot, tmpstr, justflag, 0);
    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
}

/*
 * canvas event proc
 */
void my_proc(Widget w, XtPointer data, XEvent* event) {
    static int x, y, boxno, lineno, stringno, ellipno;
    static double wx1, wx2, wy1, wy2;
    static double wx, wy, dx, dy;
    static int ty, no, c;
    static KeySym keys;
    static XComposeStatus compose;
    int tmpcg = cg;

#if defined(DEBUG)
    if (debuglevel == 7) {
        printf("Call to my_proc() event type == %d sent from %d\n", event->type, event->xany.send_event);
    }
#endif

    /*
     * hot keys
     */
    switch (event->type) {
    case KeyPress:
        buf[0] = 0;
        XLookupString((XKeyEvent*)event, buf, 1, &keys, &compose);
        switch (c = buf[0]) {
        case 1: /* ^A */
            if (action_flag == STR_LOC) {
                update_tmpstrins2(2);
            } else if (activeset(cg)) {
                defaultgraph(cg);
                default_axis(cg, g[cg].auto_type, X_AXIS);
                default_axis(cg, g[cg].auto_type, ZX_AXIS);
                default_axis(cg, g[cg].auto_type, Y_AXIS);
                default_axis(cg, g[cg].auto_type, ZY_AXIS);
                update_world(cg);
                drawgraph();
            } else {
                errwin("No active sets!");
            }
            break;
        case 2: /* ^B */
            set_action(0);
            set_action(MAKE_BOX_1ST);
            break;
        case 3: /* ^C */
            break;
        case 4: /* ^D */
            set_action(0);
            set_action(DEL_OBJECT);
            break;
        case 5: /* ^E */
            if (action_flag == STR_LOC) {
                update_tmpstrins2(1);
            }
            break;
        case 6: /* ^F */
            break;
        case 7: /* ^G */
            create_world_frame(NULL, NULL, NULL);
            break;
            /* stay off 8 (^H) - needed by text routines */
        case 11: /* ^K */
            if (action_flag == STR_LOC) {
                update_tmpstrins2(3);
            }
            break;
        case 12: /* ^L */
            legend_loc_proc(NULL, NULL, NULL);
            break;
        case 14: /* ^N */
            set_action(0);
            set_action(MOVE_OBJECT_1ST);
            break;
        case 16: /* ^P */
            set_action(0);
            set_action(MAKE_LINE_1ST);
            break;
        case 18: /* ^R */
            break;
        case 19: /* ^S */
            break;
        case 20: /* ^T */
            break;
        case 21: /* ^U */
            if (action_flag == STR_LOC) {
                update_tmpstrins2(4);
            }
            break;
        case 22: /* ^V */
            set_action(0);
            set_action(VIEW_1ST);
            break;
        case 23: /* ^W */
            set_action(0);
            set_action(STR_LOC);
            break;
        case 24: /* ^X */
            bailout();
            break;
        case 26: /* ^Z */
            set_action(0);
            set_action(ZOOM_1ST);
            break;
        case 8:
        case 127:
            if (action_flag == STR_LOC) {
                do_text_string(0, 0);
            }
            break;
        case '\r':
        case '\n':
            if (action_flag == STR_LOC) {
                int itmp;

                update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
                if (tmpstr[0]) {
                    device2world(drawx, win_h - drawy, &wx, &wy);
                    define_string(tmpstr, wx, wy);
                }
                itmp = (int)(1.25 * stringextenty(charsize * xlibcharsize, "Ny"));
                strx = strx + si * itmp;
                stry = stry - co * itmp;
                tmpstr[0] = 0;
                tmpstrins = tmpstr;
                update_text_cursor(tmpstr, strx, stry, tmpstrins);
            }
            break;
        default:
            if (action_flag == STR_LOC) {
                if (c == 0)
                    update_tmpstrins(keys);
                else if (c >= 32 && c < 128) {
                    do_text_string(1, c);
                }
            }
            break;
        }
        break;
    case ButtonPress:
        switch (event->xbutton.button) {
        case Button3:
            getpoints(x, y);
            if (action_flag == COMP_AREA || action_flag == COMP_PERIMETER) {
                if (narea_pts >= 3) {
                    int i;

                    for (i = 0; i < narea_pts; i++) {
                        XDrawLine(disp, xwin, gc, iax[i], iay[i], iax[(i + 1) % narea_pts], iay[(i + 1) % narea_pts]);
                    }
                }
            }
            if (action_flag == DEF_REGION) {
                if (region_pts >= 3) {
                    int i;

                    for (i = 0; i < region_pts; i++) {
                        XDrawLine(disp, xwin, gc, iax[i], iay[i], iax[(i + 1) % region_pts], iay[(i + 1) % region_pts]);
                    }
                    load_poly_region(nr, region_pts, area_polyx, area_polyy);
                    if (regionlinkto) {
                        int i;

                        for (i = 0; i < maxgraph; i++) {
                            rg[nr].linkto[i] = TRUE;
                        }
                    } else {
                        int i;

                        for (i = 0; i < maxgraph; i++) {
                            rg[nr].linkto[i] = FALSE;
                        }
                        rg[nr].linkto[cg] = TRUE;
                    }
                }
            }
            narea_pts = 0;
            region_pts = 0;
            set_action(0);
            break;
        case Button1:
            if (action_flag == 0) {
                if (focus_policy == FOCUS_CLICK) {
                    int newg;

                    device2world(x, y, &wx, &wy);
                    if (!focus_clicked(cg, x, y) && (newg = nextcontained(cg, wx, wy)) != cg &&
                        !double_click((XButtonEvent*)event)) {
                        draw_focus(cg);
                        cg = newg;
                        defineworld(g[cg].w.xg1, g[cg].w.yg1, g[cg].w.xg2, g[cg].w.yg2, islogx(cg), islogy(cg));
                        viewport(g[cg].v.xv1, g[cg].v.yv1, g[cg].v.xv2, g[cg].v.yv2);
                        draw_focus(cg);
                        make_format(cg);
                        device2world(x, y, &wx, &wy);
                        update_all(cg);
                    }
                }
            }
            c = go_locateflag;
            go_locateflag = TRUE;
            getpoints(x, y);
            go_locateflag = c;
            {
                if (!action_flag && allow_dc && double_click((XButtonEvent*)event)) {
                    if (tmpcg == cg) { /* don't allow a change of focus */
                        int setno, loc;
                        extern int cset;

                        device2world(x, y, &wx, &wy);
                        if (focus_clicked(cg, x, y)) {
                            double xm, ym;

                            get_corner_clicked(cg, x, y, &xm, &ym);
                            if (xm == g[cg].v.xv1) {
                                xm = g[cg].v.xv2;
                            } else {
                                xm = g[cg].v.xv1;
                            }
                            if (ym == g[cg].v.yv1) {
                                ym = g[cg].v.yv2;
                            } else {
                                ym = g[cg].v.yv1;
                            }
                            view2device(xm, ym, &sx, &sy);
                            set_action(VIEW_2ND);
                            rectflag = 1;
                            select_region(sx, sy, x, y);
                            break;
                        } else if (symok(wx, wy)) {
                            findpoint(cg, wx, wy, &wx, &wy, &setno, &loc);
                            if (setno != -1) {
                                cset = setno;
                                set_wait_cursor();
                                define_symbols_popup(NULL, NULL, NULL);
                                unset_wait_cursor();
                            } else {
                                set_wait_cursor();
                                create_file_popup(NULL, NULL, NULL);
                                unset_wait_cursor();
                                /* annoying error message
                                                    errwin("No set!");
                                */
                            }
                        } else {
                            if (wx < g[cg].w.xg1) {
                                curaxis = 1;
                                create_ticks_frame(NULL, NULL, NULL);
                            } else if (wy < g[cg].w.yg1) {
                                curaxis = 0;
                                create_ticks_frame(NULL, NULL, NULL);
                            } else if (wy > g[cg].w.yg2) {
                                create_label_frame(NULL, NULL, NULL);
                            } else if (wx > g[cg].w.xg2) {
                                define_legend_popup(NULL, NULL, NULL);
                            }
                        }
                    }
                }
            }
            switch (action_flag) {
            case DEL_OBJECT: /* delete a box or a line */
                set_action(0);
                device2world(x, y, &wx, &wy);
                find_item(cg, wx, wy, &ty, &no);
                if (ty >= 0) {
                    switch (ty) {
                    case OBJECT_BOX:
                        set_write_mode(0);
                        draw_box(-2, no);
                        set_write_mode(1);
                        kill_box(no);
                        break;
                    case OBJECT_ELLIPSE:
                        set_write_mode(0);
                        draw_ellipse(-2, no);
                        set_write_mode(1);
                        kill_ellipse(no);
                        break;
                    case OBJECT_LINE:
                        set_write_mode(0);
                        draw_line(-2, no);
                        set_write_mode(1);
                        kill_line(no);
                        break;
                    case OBJECT_STRING:
                        set_write_mode(0);
                        draw_string(-2, no);
                        set_write_mode(1);
                        kill_string(no);
                        break;
                    }
                    set_action(DEL_OBJECT);
                }
                break;
                /*
                 * select an object to move
                 */
            case MOVE_OBJECT_1ST:
                set_action(MOVE_OBJECT_2ND);
                device2world(x, y, &wx, &wy);
                find_item(cg, wx, wy, &ty, &no);
                if (ty < 0) {
                    set_action(MOVE_OBJECT_1ST);
                } else {
                    switch (ty) {
                    case OBJECT_BOX:
                        if (boxes[no].loctype == COORD_VIEW) {
                            sx = (int)(win_w * boxes[no].x1);
                            sy = (int)(win_h - win_h * boxes[no].y1);
                            xs = (int)(win_w * boxes[no].x2);
                            ys = (int)(win_h - win_h * boxes[no].y2);
                        } else {
                            world2deviceabs(boxes[no].x1, boxes[no].y1, &sx, &sy);
                            world2deviceabs(boxes[no].x2, boxes[no].y2, &xs, &ys);
                        }
                        select_region(sx, sy, xs, ys);
                        mbox_flag = 1;
                        break;
                    case OBJECT_ELLIPSE:
                        if (ellip[no].loctype == COORD_VIEW) {
                            sx = (int)(win_w * ellip[no].x1);
                            sy = (int)(win_h - win_h * ellip[no].y1);
                            xs = (int)(win_w * ellip[no].x2);
                            ys = (int)(win_h - win_h * ellip[no].y2);
                        } else {
                            world2deviceabs(ellip[no].x1, ellip[no].y1, &sx, &sy);
                            world2deviceabs(ellip[no].x2, ellip[no].y2, &xs, &ys);
                        }
                        select_region(sx, sy, xs, ys);
                        mbox_flag = 1;
                        break;
                    case OBJECT_LINE:
                        if (lines[no].loctype == COORD_VIEW) {
                            sx = (int)(win_w * lines[no].x1);
                            sy = (int)(win_h - win_h * lines[no].y1);
                            xs = (int)(win_w * lines[no].x2);
                            ys = (int)(win_h - win_h * lines[no].y2);
                        } else {
                            world2deviceabs(lines[no].x1, lines[no].y1, &sx, &sy);
                            world2deviceabs(lines[no].x2, lines[no].y2, &xs, &ys);
                        }
                        select_line(sx, sy, xs, ys);
                        mline_flag = 1;
                        break;
                    case OBJECT_STRING:
                        xs = stringextentx(charsize * xlibcharsize, pstr[no].s);
                        ys = stringextenty(charsize * xlibcharsize, pstr[no].s);
                        if (pstr[no].loctype == COORD_VIEW) {
                            sx = (int)(win_w * pstr[no].x);
                            sy = (int)(win_h - win_h * pstr[no].y);
                        } else {
                            world2device(pstr[no].x, pstr[no].y, &sx, &sy);
                        }
                        xs = sx + xs;
                        ys = sy + ys;
                        mbox_flag = 1;
                        select_region(sx, sy, xs, ys);
                        break;
                    }
                }

                break;
                /*
                 * box has been selected and new position found
                 */
            case MOVE_OBJECT_2ND:
                dx = sx - x;
                dy = sy - y;

                set_action(0);
                sx = x;
                sy = y;
                xs = xs - dx;
                ys = ys - dy;
                device2world(sx, sy, &wx1, &wy1);
                device2world(xs, ys, &wx2, &wy2);
                switch (ty) {
                case OBJECT_BOX:
                    set_write_mode(0);
                    draw_box(-2, no);
                    if (boxes[no].loctype == COORD_VIEW) {
                        wx1 = xconv(wx1);
                        wy1 = yconv(wy1);
                        wx2 = xconv(wx2);
                        wy2 = yconv(wy2);
                    } else {
                        /*
                         * 			boxes[no].gno = cg;
                         */
                    }
                    boxes[no].x1 = wx1;
                    boxes[no].x2 = wx2;
                    boxes[no].y1 = wy1;
                    boxes[no].y2 = wy2;
                    set_write_mode(1);
                    draw_box(-2, no);
                    break;
                case OBJECT_ELLIPSE:
                    set_write_mode(0);
                    draw_ellipse(-2, no);
                    if (ellip[no].loctype == COORD_VIEW) {
                        wx1 = xconv(wx1);
                        wy1 = yconv(wy1);
                        wx2 = xconv(wx2);
                        wy2 = yconv(wy2);
                    } else {
                        /*
                         * 			ellip[no].gno = cg;
                         */
                    }
                    ellip[no].x1 = wx1;
                    ellip[no].x2 = wx2;
                    ellip[no].y1 = wy1;
                    ellip[no].y2 = wy2;
                    set_write_mode(1);
                    draw_ellipse(-2, no);
                    break;
                case OBJECT_LINE:
                    set_write_mode(0);
                    draw_line(-2, no);
                    if (lines[no].loctype == COORD_VIEW) {
                        wx1 = xconv(wx1);
                        wy1 = yconv(wy1);
                        wx2 = xconv(wx2);
                        wy2 = yconv(wy2);
                    } else {
                        /*
                         * 			lines[no].gno = cg;
                         */
                    }
                    lines[no].x1 = wx1;
                    lines[no].x2 = wx2;
                    lines[no].y1 = wy1;
                    lines[no].y2 = wy2;
                    set_write_mode(1);
                    draw_line(-2, no);
                    break;
                case OBJECT_STRING:
                    set_write_mode(0);
                    draw_string(-2, no);
                    wy = (double)stringextenty(charsize * xlibcharsize, pstr[no].s) / (double)win_h;
                    if (pstr[no].loctype == COORD_VIEW) {
                        wx1 = xconv(wx1);
                        wy1 = yconv(wy1) - wy / 2.;
                    } else {
                        world2view(wx1, wy1, &wx2, &wy2);
                        wy2 -= wy / 2.;
                        view2world(wx2, wy2, &wx1, &wy1);
                    }
                    pstr[no].x = wx1;
                    pstr[no].y = wy1;
                    set_write_mode(1);
                    draw_string(-2, no);
                    break;
                }
                set_action(MOVE_OBJECT_1ST);
                break;
                /*
                 * select a box, ellipse or a line to copy
                 */
            case COPY_OBJECT1ST:
                set_action(COPY_OBJECT2ND);
                device2world(x, y, &wx, &wy);
                find_item(cg, wx, wy, &ty, &no);
                if (ty < 0) {
                    /*
                     * 		    set_action(0);
                     */
                    set_action(COPY_OBJECT1ST);
                } else {
                    switch (ty) {
                    case OBJECT_BOX:
                        if (boxes[no].loctype == COORD_VIEW) {
                            sx = (int)(win_w * boxes[no].x1);
                            sy = (int)(win_h - win_h * boxes[no].y1);
                            xs = (int)(win_w * boxes[no].x2);
                            ys = (int)(win_h - win_h * boxes[no].y2);
                        } else {
                            world2deviceabs(boxes[no].x1, boxes[no].y1, &sx, &sy);
                            world2deviceabs(boxes[no].x2, boxes[no].y2, &xs, &ys);
                        }
                        select_region(sx, sy, xs, ys);
                        mbox_flag = 1;
                        break;
                    case OBJECT_ELLIPSE:
                        if (ellip[no].loctype == COORD_VIEW) {
                            sx = (int)(win_w * ellip[no].x1);
                            sy = (int)(win_h - win_h * ellip[no].y1);
                            xs = (int)(win_w * ellip[no].x2);
                            ys = (int)(win_h - win_h * ellip[no].y2);
                        } else {
                            world2deviceabs(ellip[no].x1, ellip[no].y1, &sx, &sy);
                            world2deviceabs(ellip[no].x2, ellip[no].y2, &xs, &ys);
                        }
                        select_region(sx, sy, xs, ys);
                        mbox_flag = 1;
                        break;
                    case OBJECT_LINE:
                        if (lines[no].loctype == COORD_VIEW) {
                            sx = (int)(win_w * lines[no].x1);
                            sy = (int)(win_h - win_h * lines[no].y1);
                            xs = (int)(win_w * lines[no].x2);
                            ys = (int)(win_h - win_h * lines[no].y2);
                        } else {
                            world2deviceabs(lines[no].x1, lines[no].y1, &sx, &sy);
                            world2deviceabs(lines[no].x2, lines[no].y2, &xs, &ys);
                        }
                        select_line(sx, sy, xs, ys);
                        mline_flag = 1;
                        break;
                    case OBJECT_STRING:
                        xs = stringextentx(charsize * xlibcharsize, pstr[no].s);
                        ys = stringextenty(charsize * xlibcharsize, pstr[no].s);
                        if (pstr[no].loctype == COORD_VIEW) {
                            sx = (int)(win_w * pstr[no].x);
                            sy = (int)(win_h - win_h * pstr[no].y);
                        } else {
                            world2device(pstr[no].x, pstr[no].y, &sx, &sy);
                        }
                        xs = sx + xs;
                        ys = sy + ys;
                        mbox_flag = 1;
                        select_region(sx, sy, xs, ys);
                        break;
                    }
                }

                break;
                /*
                 * box has been selected and new position found
                 */
            case COPY_OBJECT2ND:
                dx = sx - x;
                dy = sy - y;

                set_action(0);
                sx = x;
                sy = y;
                xs = xs - dx;
                ys = ys - dy;
                device2world(sx, sy, &wx1, &wy1);
                device2world(xs, ys, &wx2, &wy2);
                switch (ty) {
                case OBJECT_BOX:
                    if ((boxno = next_box()) >= 0) {
                        copy_object(ty, no, boxno);
                        if (boxes[no].loctype == COORD_VIEW) {
                            wx1 = xconv(wx1);
                            wy1 = yconv(wy1);
                            wx2 = xconv(wx2);
                            wy2 = yconv(wy2);
                        } else {
                            boxes[boxno].gno = cg;
                        }
                        boxes[boxno].x1 = wx1;
                        boxes[boxno].x2 = wx2;
                        boxes[boxno].y1 = wy1;
                        boxes[boxno].y2 = wy2;
                        draw_box(-2, boxno);
                    }
                    break;
                case OBJECT_ELLIPSE:
                    if ((ellipno = next_ellipse()) >= 0) {
                        copy_object(ty, no, ellipno);
                        if (ellip[no].loctype == COORD_VIEW) {
                            wx1 = xconv(wx1);
                            wy1 = yconv(wy1);
                            wx2 = xconv(wx2);
                            wy2 = yconv(wy2);
                        } else {
                            ellip[ellipno].gno = cg;
                        }
                        ellip[ellipno].x1 = wx1;
                        ellip[ellipno].x2 = wx2;
                        ellip[ellipno].y1 = wy1;
                        ellip[ellipno].y2 = wy2;
                        draw_ellipse(-2, ellipno);
                    }
                    break;
                case OBJECT_LINE:
                    if ((lineno = next_line()) >= 0) {
                        copy_object(ty, no, lineno);
                        if (lines[no].loctype == COORD_VIEW) {
                            wx1 = xconv(wx1);
                            wy1 = yconv(wy1);
                            wx2 = xconv(wx2);
                            wy2 = yconv(wy2);
                        } else {
                            lines[lineno].gno = cg;
                        }
                        lines[lineno].x1 = wx1;
                        lines[lineno].x2 = wx2;
                        lines[lineno].y1 = wy1;
                        lines[lineno].y2 = wy2;
                        draw_line(-2, lineno);
                    }
                    break;
                case OBJECT_STRING:
                    if ((stringno = next_string()) >= 0) {
                        copy_object(ty, no, stringno);
                        if (pstr[no].loctype == COORD_VIEW) {
                            wx1 = xconv(wx1);
                            wy1 = yconv(wy1);
                        } else {
                            pstr[stringno].gno = cg;
                        }
                        pstr[stringno].x = wx1;
                        pstr[stringno].y = wy1;
                        draw_string(-2, stringno);
                    }
                    break;
                }
                set_action(COPY_OBJECT1ST);
                break;
                /*
                 * select a box or a line to move
                 */
            case EDIT_OBJECT:
                set_action(EDIT_OBJECT);
                device2world(x, y, &wx, &wy);
                find_item(cg, wx, wy, &ty, &no);
                if (ty < 0) {
                    /*
                     * 		    set_action(0);
                     */
                } else {
                    switch (ty) {
                    case OBJECT_BOX:
                        box_edit_popup(no);
                        break;
                    case OBJECT_ELLIPSE:
                        ellipse_edit_popup(no);
                        break;
                    case OBJECT_LINE:
                        line_edit_popup(no);
                        break;
                    case OBJECT_STRING:
                        string_edit_popup(no);
                        break;
                    }
                }

                break;
                /*
                 * make a new box, select first corner
                 */
            case MAKE_BOX_1ST:
                set_action(MAKE_BOX_2ND);
                rectflag = 1;
                sx = x;
                sy = y;
                select_region(sx, sy, x, y);
                break;
                /*
                 * make a new box, select opposite corner
                 */
            case MAKE_BOX_2ND:
                set_action(0);
                if ((boxno = next_box()) >= 0) {
                    device2world(sx, sy, &wx1, &wy1);
                    device2world(x, y, &wx2, &wy2);
                    if (box_loctype == COORD_VIEW) {
                        wx1 = xconv(wx1);
                        wy1 = yconv(wy1);
                        wx2 = xconv(wx2);
                        wy2 = yconv(wy2);
                    } else {
                        boxes[boxno].gno = cg;
                    }
                    boxes[boxno].loctype = box_loctype;
                    boxes[boxno].x1 = wx1;
                    boxes[boxno].x2 = wx2;
                    boxes[boxno].y1 = wy1;
                    boxes[boxno].y2 = wy2;
                    boxes[boxno].color = box_color;
                    boxes[boxno].linew = box_linew;
                    boxes[boxno].lines = box_lines;
                    boxes[boxno].fill = box_fill;
                    boxes[boxno].fillcolor = box_fillcolor;
                    boxes[boxno].fillpattern = box_fillpat;
                    draw_box(-2, boxno);
                    set_action(MAKE_BOX_1ST);
                }
                break;
                /*
                 * make an ellipse, select first corner
                 */
            case MAKE_ELLIP_1ST:
                set_action(MAKE_ELLIP_2ND);
                rectflag = 1;
                sx = x;
                sy = y;
                select_region(sx, sy, x, y);
                break;
                /*
                 * make a new ellipse, select opposite corner
                 */
            case MAKE_ELLIP_2ND:
                set_action(0);
                if ((ellipno = next_ellipse()) >= 0) {
                    device2world(sx, sy, &wx1, &wy1);
                    device2world(x, y, &wx2, &wy2);
                    ellip[ellipno] = defellip;
                    if (ellip[ellipno].loctype == COORD_VIEW) {
                        wx1 = xconv(wx1);
                        wy1 = yconv(wy1);
                        wx2 = xconv(wx2);
                        wy2 = yconv(wy2);
                    } else {
                        ellip[ellipno].gno = cg;
                    }
                    ellip[ellipno].x1 = wx1;
                    ellip[ellipno].x2 = wx2;
                    ellip[ellipno].y1 = wy1;
                    ellip[ellipno].y2 = wy2;
                    draw_ellipse(-2, ellipno);
                    /* 		    if (wy1 > wy2) {
                                    fswap(&wy1, &wy2);
                                }
                                if (sx > x) { iswap(&sx, &x); }
                                if (sy < y) { iswap(&sy, &y); }
                                XDrawArc(disp, xwin, gc, sx, y, x - sx, sy - y, 0, 360 * 64);
                     */
                    set_action(MAKE_ELLIP_1ST);
                }
                break;
                /*
                 * locate angled string
                 */
            case STR_LOC1ST:
                set_action(STR_LOC2ND);
                rubber_flag = 1;
                sx = x;
                sy = y;
                select_line(sx, sy, x, y);
                break;
            case STR_LOC2ND:
                device2world(sx, sy, &wx1, &wy1);
                device2world(x, y, &wx2, &wy2);
                wx1 = xconv(wx1);
                wy1 = yconv(wy1);
                wx2 = xconv(wx2);
                wy2 = yconv(wy2);
                string_rot = (int)((atan2((wy2 - wy1) * win_h, (wx2 - wx1) * win_w) * 180.0 / M_PI) + 360.0) % 360;
                updatestrings();
                set_action(0);
                set_action(STR_LOC);
                break;
                /*
                 * make a new line, select start point
                 */
            case MAKE_LINE_1ST:
                sx = x;
                sy = y;
                set_action(MAKE_LINE_2ND);
                rubber_flag = 1;
                select_line(sx, sy, x, y);
                break;
                /*
                 * make a new line, select end point
                 */
            case MAKE_LINE_2ND:
                set_action(0);
                if ((lineno = next_line()) >= 0) {
                    device2world(sx, sy, &wx1, &wy1);
                    device2world(x, y, &wx2, &wy2);
                    if (line_loctype == COORD_VIEW) {
                        wx1 = xconv(wx1);
                        wy1 = yconv(wy1);
                        wx2 = xconv(wx2);
                        wy2 = yconv(wy2);
                    } else {
                        lines[lineno].gno = cg;
                    }
                    lines[lineno].loctype = line_loctype;
                    lines[lineno].x1 = wx1;
                    lines[lineno].x2 = wx2;
                    lines[lineno].y1 = wy1;
                    lines[lineno].y2 = wy2;
                    lines[lineno].color = line_color;
                    lines[lineno].lines = line_lines;
                    lines[lineno].linew = line_linew;
                    lines[lineno].arrow = line_arrow;
                    lines[lineno].asize = line_asize;
                    lines[lineno].atype = line_atype;
                    draw_line(-2, lineno);
                    set_action(MAKE_LINE_1ST);
                }
                break;
                /*
                 * Edit an existing string
                 */
            case STR_EDIT:
                device2world(x, y, &wx, &wy);
                find_item(cg, wx, wy, &ty, &no);
                if ((ty >= 0) && (ty == OBJECT_STRING)) {
                    int ilenx, ileny;

                    wx1 = pstr[no].x;
                    wy1 = pstr[no].y;
                    if (pstr[no].loctype == COORD_VIEW) { /* in viewport coords */
                        view2world(wx1, wy1, &wx2, &wy2);
                        wx1 = wx2;
                        wy1 = wy2;
                    }
                    world2device(wx1, wy1, &strx, &stry);
                    drawx = strx;
                    drawy = stry;
                    tmpstrins = strcpy(tmpstr, pstr[no].s) + strlen(pstr[no].s);
                    setcharsize(pstr[no].charsize);
                    setfont(pstr[no].font);
                    setcolor(pstr[no].color);
                    string_just = pstr[no].just;
                    justflag = string_just;
                    string_size = pstr[no].charsize;
                    string_font = pstr[no].font;
                    string_color = pstr[no].color;
                    string_linew = pstr[no].linew;
                    string_rot = pstr[no].rot;
                    string_loctype = pstr[no].loctype;
                    updatestrings();
                    kill_string(no);
                    si = sin(M_PI / 180.0 * string_rot) * ((double)win_w) / ((double)win_h);
                    co = cos(M_PI / 180.0 * string_rot);

                    ilenx = stringextentx(charsize * xlibcharsize, tmpstr);
                    ileny = stringextenty(charsize * xlibcharsize, tmpstr);

                    switch (justflag) {
                    case 1:
                        /*
                                    strx = drawx + co * ilenx - si * ileny;
                                    stry = drawy + si * ilenx + co * ileny;
                        */
                        break;
                    case 2:
                        /*
                                    strx = drawx + (co * ilenx - si * ileny) / 2;
                                    stry = drawy + (si * ilenx + co * ileny) / 2;
                        */
                        break;
                    }
                    update_text_cursor(tmpstr, drawx, drawy, tmpstrins);
                    do_text_string(2, 0);
                    action_flag = STR_LOC;
                } else {
                    set_action(0);
                }
                break;
                /*
                 * locate a string on the canvas
                 */
            case STR_LOC:
                if (tmpstr[0]) {
                    device2world(strx, win_h - stry, &wx, &wy);
                    define_string(tmpstr, wx, wy);
                }
                strx = x;
                stry = win_h - y;
                drawx = strx;
                drawy = stry;
                tmpstr[0] = 0;
                tmpstrins = tmpstr;
                define_string_defaults(NULL, NULL, NULL);
                justflag = string_just;
                setcharsize(string_size);
                xlibsetfont(string_font);
                xlibsetcolor(string_color);
                xlibsetlinewidth(string_linew);
                si = sin(M_PI / 180.0 * string_rot) * ((double)win_w) / ((double)win_h);
                co = cos(M_PI / 180.0 * string_rot);
                update_text_cursor(tmpstr, strx, stry, tmpstrins);
                break;
                /*
                 * Kill the set nearest the pointer
                 */
            case KILL_NEAREST: {
                int setno, loc;

                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno, &loc);
                if (setno != -1) {
                    if (verify_action) {
                        char tmpbuf[128];

                        sprintf(tmpbuf, "Kill S%1d?", setno);
                        if (yesno(tmpbuf, NULL, NULL, NULL)) {
                            do_kill(cg, setno, 0);
                            drawgraph();
                        }
                    } else {
                        do_kill(cg, setno, 0);
                        drawgraph();
                    }
                    set_action(KILL_NEAREST);
                } else {
                    errwin("Found no set, cancelling kill");
                    set_action(0);
                }
            } break;
                /*
                 * Deactivate the set nearest the pointer
                 */
            case DEACTIVATE_NEAREST: {
                int setno, loc;

                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno, &loc);
                if (setno != -1) {
                    if (verify_action) {
                        char tmpbuf[128];

                        sprintf(tmpbuf, "Deactivate S%1d?", setno);
                        if (yesno(tmpbuf, NULL, NULL, NULL)) {
                            do_deactivate(cg, setno);
                            drawgraph();
                        }
                    } else {
                        do_deactivate(cg, setno);
                        drawgraph();
                    }
                    set_action(DEACTIVATE_NEAREST);
                } else {
                    errwin("Found no set, cancelling deactivate");
                    set_action(0);
                }
            } break;
                /*
                 * Copy the set nearest the pointer
                 */
            case COPY_NEAREST1ST: {
                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno1, &loc1);
                graphno1 = cg; /* in case focus changes */
                if (setno1 != -1) {
                    set_action(COPY_NEAREST2ND);
                } else {
                    errwin("Found no set, cancelling copy");
                    set_action(0);
                }
            } break;
            case COPY_NEAREST2ND: {
                device2world(x, y, &wx, &wy);
                graphno2 = iscontained(cg, wx, wy);
                if (graphno2 != -1) {
                    if (verify_action) {
                        char tmpbuf[128];

                        sprintf(tmpbuf, "Copy S%1d in graph %d to next set in graph %d?", setno1, graphno1, graphno2);
                        if (yesno(tmpbuf, NULL, NULL, NULL)) {
                            do_copy(setno1, graphno1, SET_SELECT_NEXT, graphno2 + 1);
                        }
                    } else {
                        do_copy(setno1, graphno1, SET_SELECT_NEXT, graphno2 + 1);
                    }
                    set_action(COPY_NEAREST1ST);
                } else {
                    errwin("Found no graph, cancelling copy");
                    set_action(0);
                }
            } break;
                /*
                 * Move the set nearest the pointer
                 */
            case MOVE_NEAREST1ST: {
                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno1, &loc1);
                graphno1 = cg; /* in case focus changes */
                if (setno1 != -1) {
                    set_action(MOVE_NEAREST2ND);
                } else {
                    errwin("Found no set, cancelling move");
                    set_action(0);
                }
            } break;
            case MOVE_NEAREST2ND: {
                device2world(x, y, &wx, &wy);
                graphno2 = iscontained(cg, wx, wy);
                if (graphno2 != -1 && graphno2 + 1 != graphno1) {
                    if (verify_action) {
                        char tmpbuf[128];
                        sprintf(tmpbuf, "Move S%1d in graph %d to next set in graph %d?", setno1, graphno1, graphno2);
                        if (yesno(tmpbuf, NULL, NULL, NULL)) {
                            do_move(setno1, graphno1, SET_SELECT_NEXT, graphno2 + 1);
                        }
                    } else {
                        do_move(setno1, graphno1, SET_SELECT_NEXT, graphno2 + 1);
                    }
                    set_action(MOVE_NEAREST1ST);
                } else if (graphno2 == -1) {
                    errwin("Found no graph, cancelling move");
                    set_action(0);
                }
            } break;
                /*
                 * Break a set at a point
                 */
            case PICK_BREAK: {
                int setno, loc;

                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno, &loc);
                if (setno != -1) {
                    if (verify_action) {
                        char tmpbuf[128];

                        sprintf(tmpbuf, "Reverse S%1d?", setno);
                        if (yesno(tmpbuf, NULL, NULL, NULL)) {
                            do_breakset(cg, setno, loc);
                        }
                    } else {
                        do_breakset(cg, setno, loc);
                    }
                } else {
                    errwin("Found no set, cancelling break");
                }
                set_action(0);
            } break;
                /*
                 * Reverse the set nearest the pointer
                 */
            case REVERSE_NEAREST: {
                int setno, loc;

                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno, &loc);
                if (setno != -1) {
                    if (verify_action) {
                        char tmpbuf[128];

                        sprintf(tmpbuf, "Reverse S%1d?", setno);
                        if (yesno(tmpbuf, NULL, NULL, NULL)) {
                            do_reverse_sets(setno);
                            drawgraph();
                        }
                    } else {
                        do_reverse_sets(setno);
                        drawgraph();
                    }
                } else {
                    errwin("Found no set, cancelling reverse");
                }
                set_action(0);
            } break;
                /*
                 * Join two sets
                 */
            case JOIN_NEAREST1ST: {
                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno1, &loc1);
                graphno1 = cg; /* in case focus changes */
                if (setno1 != -1) {
                    set_action(JOIN_NEAREST2ND);
                } else {
                    errwin("Found no set, cancelling join");
                    set_action(0);
                }
            } break;
            case JOIN_NEAREST2ND: {
                int setno, loc;

                device2world(x, y, &wx, &wy);
                graphno2 = iscontained(cg, wx, wy);
                findpoint(graphno2, wx, wy, &wx, &wy, &setno, &loc);
                if (setno1 == setno && graphno1 == graphno2) {
                    errwin("Can't join the same set");
                } else {
                    if (verify_action) {
                        char tmpbuf[128];

                        sprintf(tmpbuf, "Join S%1d in graph %d to the end of S%1d in graph %d?", setno1, graphno1,
                                setno, graphno2);
                        if (yesno(tmpbuf, NULL, NULL, NULL)) {
                            do_join_sets(graphno1, setno1, graphno2, setno);
                            drawgraph();
                        }
                    } else {
                        do_join_sets(graphno1, setno1, graphno2, setno);
                        drawgraph();
                    }
                }
                set_action(JOIN_NEAREST1ST);
            } break;
                /*
                 * Delete range in a set
                 */
            case DELETE_NEAREST1ST: {
                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno1, &loc1);
                graphno1 = cg; /* in case focus changes */
                if (setno1 != -1) {
                    set_action(DELETE_NEAREST2ND);
                } else {
                    errwin("Found no set, cancelling delete");
                    set_action(0);
                }
            } break;
            case DELETE_NEAREST2ND: {
                device2world(x, y, &wx, &wy);
                graphno2 = iscontained(cg, wx, wy);
                if (graphno1 != graphno2) {
                    errwin("Can't perform operation across 2 graphs");
                    set_action(0);
                } else if (graphno2 != -1) {
                    findpoint(graphno2, wx, wy, &wx, &wy, &setno2, &loc2);
                    if (graphno1 != graphno2) {
                        errwin("Points found are not in the same graph");
                        set_action(0);
                    } else if (setno1 != setno2) {
                        errwin("Points found are not in the same set");
                        set_action(0);
                    } else {
                        if (loc2 < loc1) {
                            iswap(&loc1, &loc2);
                        }
                        if (verify_action) {
                            char tmpbuf[128];

                            sprintf(tmpbuf, "In S%1d, delete points %d through %d?", setno1, loc1, loc2);
                            if (yesno(tmpbuf, NULL, NULL, NULL)) {
                                do_drop_points(setno1, loc1 - 1, loc2 - 1);
                            }
                        } else {
                            do_drop_points(setno1, loc1 - 1, loc2 - 1);
                        }
                        set_action(DELETE_NEAREST1ST);
                    }
                } else {
                    errwin("Found no graph, cancelling delete");
                    set_action(0);
                }
            } break;
            case AUTO_NEAREST: {
                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno1, &loc1);
                if (setno1 != -1) {
                    do_autoscale_set(cg, setno1);
                } else {
                    errwin("Found no set, cancelling autoscale");
                }
                set_action(0);
            } break;
                /*
                 * find a point in a set
                 */
            case FIND_POINT: {
                int setno, loc;

                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno, &loc);
                if (setno != -1) {
                    sprintf(buf, "Set %d, loc %d, (%f, %f)", setno, loc, wx, wy);
                    xv_setstr(locate_point_item, buf);
                    set_action(FIND_POINT);
                } else {
                    errwin("Found no set, cancelling find");
                    set_action(0);
                }
            } break;
                /*
                 * Tracker
                 */
            case TRACKER: {
                extern int track_set, track_point;
                int xtmp, ytmp;
                double *xx, *yy;

                if (track_set == -1) { /* select nearest set, nearest point*/
                    device2world(x, y, &wx, &wy);
                    findpoint(cg, wx, wy, &wx, &wy, &track_set, &track_point);
                } else { /* set selected, find nearest point */
                    if (track_point == -1) {
                        device2world(x, y, &wx, &wy);
                        findpoint_inset(cg, track_set, wx, wy, &track_point);
                    }
                }
                track_point--;
                if (track_point < 0) {
                    track_point = getsetlength(cg, track_set) - 1;
                } else if (track_point >= getsetlength(cg, track_set)) {
                    track_point = 0;
                }
                xx = getx(cg, track_set);
                yy = gety(cg, track_set);
                if (inbounds(cg, xx[track_point], yy[track_point])) {
                    world2deviceabs(xx[track_point], yy[track_point], &xtmp, &ytmp);
                    setpointer(xtmp, ytmp);
                    sprintf(buf, "Set %d, loc %d, (%f, %f)", track_set, track_point + 1, xx[track_point],
                            yy[track_point]);
                } else {
                    sprintf(buf, "OUTSIDE - Set %d, loc %d, (%f, %f)", track_set, track_point + 1, xx[track_point],
                            yy[track_point]);
                }
                xv_setstr(locate_point_item, buf);
                set_action(TRACKER);
            } break;
                /*
                 * define a polygonal region
                 */
            case DEF_REGION:
                device2world(x, y, &area_polyx[region_pts], &area_polyy[region_pts]);
                iax[region_pts] = x;
                iay[region_pts] = y;
                region_pts++;
                rubber_flag = 1;
                sx = x;
                sy = y;
                select_line(sx, sy, x, y);
                set_action(DEF_REGION);
                break;
                /*
                 * define a region by a line, type left, right, above, below
                 */
            case DEF_REGION1ST:
                sx = x;
                sy = y;
                set_action(MAKE_LINE_2ND);
                rubber_flag = 1;
                select_line(sx, sy, x, y);
                set_action(DEF_REGION2ND);
                break;
            case DEF_REGION2ND:
                set_action(0);
                device2world(sx, sy, &wx1, &wy1);
                device2world(x, y, &wx2, &wy2);
                rg[nr].active = TRUE;
                rg[nr].type = regiontype;
                if (regionlinkto) {
                    int i;

                    for (i = 0; i < maxgraph; i++) {
                        rg[nr].linkto[i] = TRUE;
                    }
                } else {
                    int i;

                    for (i = 0; i < maxgraph; i++) {
                        rg[nr].linkto[i] = FALSE;
                    }
                    rg[nr].linkto[cg] = TRUE;
                }
                rg[nr].type = regiontype;
                rg[nr].x1 = wx1;
                rg[nr].x2 = wx2;
                rg[nr].y1 = wy1;
                rg[nr].y2 = wy2;
                draw_region(nr);
                break;

                /*
                 * Compute the area of a polygon
                 */
            case COMP_AREA: {
                double area, comp_area(int n, double* x, double* y);

                device2world(x, y, &area_polyx[narea_pts], &area_polyy[narea_pts]);
                iax[narea_pts] = x;
                iay[narea_pts] = y;
                narea_pts++;
                if (narea_pts <= 2) {
                    area = 0.0;
                } else {
                    area = comp_area(narea_pts, area_polyx, area_polyy);
                }
                sprintf(buf, "[%f]", fabs(area));
                XmStringFree(astring);
                astring = XmStringCreateLtoR(buf, charset);
                XtSetArg(al, XmNlabelString, astring);
                XtSetValues(arealab, &al, 1);
                rubber_flag = 1;
                sx = x;
                sy = y;
                select_line(sx, sy, x, y);
                set_action(COMP_AREA);
            } break;
            case COMP_PERIMETER: {
                double area, comp_perimeter(int n, double* x, double* y);

                device2world(x, y, &area_polyx[narea_pts], &area_polyy[narea_pts]);
                iax[narea_pts] = x;
                iay[narea_pts] = y;
                narea_pts++;
                if (narea_pts <= 1) {
                    area = 0.0;
                } else {
                    area = comp_perimeter(narea_pts, area_polyx, area_polyy);
                }
                sprintf(buf, "[%f]", fabs(area));

                XmStringFree(pstring);
                pstring = XmStringCreateLtoR(buf, charset);
                XtSetArg(al, XmNlabelString, pstring);
                XtSetValues(perimlab, &al, 1);
                rubber_flag = 1;
                sx = x;
                sy = y;
                select_line(sx, sy, x, y);
                set_action(COMP_PERIMETER);
            } break;
                /*
                 * select a reference point for the locator in main_panel
                 */
            case SEL_POINT:
                device2world(x, y, &wx, &wy);
                g[cg].pointset = TRUE;
                g[cg].dsx = wx;
                g[cg].dsy = wy;
                draw_ref_point(cg);
                update_locator_items(cg);
                set_action(0);
                break;
                /*
                 * delete a point in a set
                 */
            case DEL_POINT: {
                int setno, loc;

                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setno, &loc);
                if (setno == -1) {
                    sprintf(buf, "No sets found");
                    xv_setstr(locate_point_item, buf);
                    set_action(0);
                } else {
                    sprintf(buf, "Set %d, loc %d, (%f, %f)", setno, loc, wx, wy);
                    xv_setstr(locate_point_item, buf);
                    if (setno >= 0) {
                        del_point(cg, setno, loc);
                        update_set_status(cg, setno);
                    }
                    set_action(DEL_POINT);
                }
            } break;
                /*
                 * move a point in a set
                 */
            case MOVE_POINT1ST:
                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setnumber, &setindex);
                sprintf(buf, "Set %d, loc %d, (%14g, %14g)", setnumber, setindex, wx, wy);
                xv_setstr(locate_point_item, buf);
                if (setnumber >= 0) {
                    world2deviceabs(wx, wy, &sx, &sy);
                    rubber_flag = 1;
                    select_line(sx, sy, sx, sy);
                    set_action(MOVE_POINT2ND);
                } else {
                    set_action(0);
                }
                break;
            case MOVE_POINT2ND:
                device2world(x, y, &wx, &wy);
                get_point(cg, setnumber, setindex - 1, &wx1, &wy1);
                switch (move_dir) {
                case 0:
                    set_point(cg, setnumber, setindex - 1, wx, wy);
                    break;
                case 1:
                    set_point(cg, setnumber, setindex - 1, wx, wy1);
                    break;
                case 2:
                    set_point(cg, setnumber, setindex - 1, wx1, wy);
                    break;
                }
                sprintf(buf, "Set %d, loc %d, (%14g, %14g)", setnumber, setindex, wx, wy);
                xv_setstr(locate_point_item, buf);
                update_set_status(cg, setnumber);
                set_action(0);
                set_action(MOVE_POINT1ST);
                drawgraph();
                break;
                /*
                 * add a point to a set
                 */
            case ADD_POINT: {
                int ind;

                device2world(x, y, &wx, &wy);
                if (add_setno >= 0) {
                    switch (add_at) {
                    case 0: /* at end */
                        ind = getsetlength(cg, add_setno);
                        add_point(cg, add_setno, wx, wy, 0.0, 0.0, SET_XY);
                        sprintf(buf, "Set %d, loc %d, (%f, %f)", add_setno, ind + 1, wx, wy);
                        break;
                    case 1: /* at beginning */
                        ind = 1;
                        add_point_at(cg, add_setno, 0, FALSE, wx, wy, 0.0, 0.0, SET_XY);
                        sprintf(buf, "Set %d, loc %d, (%f, %f)", add_setno, ind + 1, wx, wy);
                        break;
                    case 2: /* after nearest point */
                        findpoint_inset(cg, add_setno, wx, wy, &ind);
                        if (ind >= 1) {
                            add_point_at(cg, add_setno, ind - 1, TRUE, wx, wy, 0.0, 0.0, SET_XY);
                            sprintf(buf, "Added to Set %d, after loc %d, (%f, %f)", add_setno, ind, wx, wy);
                        }
                        break;
                    case 3: /* before nearest point */
                        findpoint_inset(cg, add_setno, wx, wy, &ind);
                        if (ind >= 1) {
                            add_point_at(cg, add_setno, ind - 1, FALSE, wx, wy, 0.0, 0.0, SET_XY);
                            sprintf(buf, "Added to Set %d, before loc %d, (%f, %f)", add_setno, ind, wx, wy);
                        }
                        break;
                    }
                    xv_setstr(locate_point_item, buf);
                    XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
                    XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
                    update_set_status(cg, add_setno);
                    set_action(ADD_POINT);
                } else {
                    set_action(0);
                }
            } break;
            case ADD_POINT1ST: {
                int ind;
                extern int digit_setno;

                device2world(x, y, &wx, &wy);
                if (digit_setno >= 0) {
                    ind = getsetlength(cg, digit_setno);
                    add_point(cg, digit_setno, wx, wy, 0.0, 0.0, SET_XY);
                    sprintf(buf, "Set %d, loc %d, (%f, %f)", digit_setno, ind + 1, wx, wy);
                    xv_setstr(locate_point_item, buf);
                    XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
                    XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
                    update_set_status(cg, digit_setno);
                    set_action(ADD_POINT2ND);
                } else {
                    set_action(0);
                }
            } break;
            case ADD_POINT2ND: {
                int ind;
                extern int digit_setno;

                device2world(x, y, &wx, &wy);
                if (digit_setno >= 0) {
                    ind = getsetlength(cg, digit_setno);
                    add_point(cg, digit_setno, wx, wy, 0.0, 0.0, SET_XY);
                    sprintf(buf, "Set %d, loc %d, (%f, %f)", digit_setno, ind + 1, wx, wy);
                    xv_setstr(locate_point_item, buf);
                    XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
                    XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
                    update_set_status(cg, digit_setno);
                    set_action(ADD_POINT3RD);
                } else {
                    set_action(0);
                }
            } break;
            case ADD_POINT3RD: {
                int ind;
                extern int digit_setno;

                device2world(x, y, &wx, &wy);
                if (digit_setno >= 0) {
                    ind = getsetlength(cg, digit_setno);
                    add_point(cg, digit_setno, wx, wy, 0.0, 0.0, SET_XY);
                    sprintf(buf, "Set %d, loc %d, (%f, %f)", digit_setno, ind + 1, wx, wy);
                    xv_setstr(locate_point_item, buf);
                    XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
                    XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
                    update_set_status(cg, digit_setno);
                    add_setno = digit_setno;
                    add_at = 0;
                    set_action(ADD_POINT);
                } else {
                    set_action(0);
                }
            } break;
                /*
                 * compute distance, dy/dx, angle
                 */
            case DISLINE1ST:
                device2world(x, y, &wx1, &wy1);
                set_action(DISLINE2ND);
                rubber_flag = 1;
                sx = x;
                sy = y;
                select_line(sx, sy, x, y);
                break;
            case DISLINE2ND:
                device2world(x, y, &wx2, &wy2);
                sprintf(buf, "(%f, %f, %f, %f degrees)", hypot((wx2 - wx1), (wy2 - wy1)), wx2 - wx1, wy2 - wy1,
                        180.0 / M_PI * atan2(wy2 - wy1, wx2 - wx1));
                xv_setstr(locate_point_item, buf);
                set_action(0);
                set_action(DISLINE1ST);
                break;
                /*
                 * place the timestamp
                 */
            case PLACE_TIMESTAMP:
                device2world(x, y, &wx, &wy);
                if (timestamp.loctype == COORD_VIEW) {
                    wx = xconv(wx);
                    wy = yconv(wy);
                }
                if (timestamp_x_item) {
                    sprintf(buf, "%g", wx);
                    xv_setstr(timestamp_x_item, buf);
                    sprintf(buf, "%g", wy);
                    xv_setstr(timestamp_y_item, buf);
                }
                timestamp.x = wx;
                timestamp.y = wy;
                set_action(0);
                drawgraph();
                break;
                /*
                 * pick compute ops
                 */
            case PICK_EXPR:
            case PICK_RUNAVG:
            case PICK_REG:
            case PICK_FOURIER:
            case PICK_HISTO:
            case PICK_SAMPLE:
            case PICK_PRUNE:
            case PICK_DIFF:
            case PICK_INT:
            case PICK_SPLINE:
            case PICK_SEASONAL:
                device2world(x, y, &wx, &wy);
                findpoint(cg, wx, wy, &wx, &wy, &setnumber, &setindex);
                if (setnumber >= 0) {
                    execute_pick_compute(cg, setnumber, action_flag);
                    set_action(action_flag);
                } else {
                    set_action(0);
                }
                break;
                /*
                 * locate the graph legend
                 */
            case LEG_LOC:
                device2world(x, y, &wx, &wy);
                if (g[cg].l.loctype == COORD_VIEW) {
                    wx = xconv(wx);
                    wy = yconv(wy);
                }
                if (legend_x_panel) {
                    sprintf(buf, "%.6g", wx);
                    xv_setstr(legend_x_panel, buf);
                    sprintf(buf, "%.6g", wy);
                    xv_setstr(legend_y_panel, buf);
                } else {
                    g[cg].l.legx = wx;
                    g[cg].l.legy = wy;
                }
                set_action(0);
                define_legends_proc(NULL, NULL, NULL);
                break;
                /*
                 * set one corner of zoom
                 */
            case ZOOM_1ST:
            case ZOOMX_1ST:
            case ZOOMY_1ST:
                switch (action_flag) {
                case ZOOM_1ST:
                    set_action(ZOOM_2ND);
                    break;
                case ZOOMX_1ST:
                    set_action(ZOOMX_2ND);
                    break;
                case ZOOMY_1ST:
                    set_action(ZOOMY_2ND);
                    break;
                }
                rectflag = 1;
                sx = x;
                sy = y;
                select_region(x, y, x, y);
                break;
                /*
                 * set opposing corner of zoom
                 */
            case ZOOM_2ND:
                set_action(0);
                select_region(sx, sy, old_x, old_y);
                device2world(sx, sy, &wx1, &wy1);
                device2world(old_x, old_y, &wx2, &wy2);
                if (sx == old_x || sy == old_y) {
                    errwin("Zoomed rectangle is zero along X or Y, zoom cancelled");
                } else {
                    if (wx1 > wx2) {
                        fswap(&wx1, &wx2);
                    }
                    if (wy1 > wy2) {
                        fswap(&wy1, &wy2);
                    }
                    newworld(cg, linked_zoom, -1, wx1, wy1, wx2, wy2);
                    drawgraph();
                }
                break;
            case ZOOMX_2ND:
                set_action(0);
                select_region(sx, sy, old_x, old_y);
                device2world(sx, sy, &wx1, &wy1);
                device2world(old_x, old_y, &wx2, &wy2);
                if (sx == old_x) {
                    errwin("Zoomed rectangle is zero along X, zoom cancelled");
                } else {
                    if (wx1 > wx2) {
                        fswap(&wx1, &wx2);
                    }
                    newworld(cg, linked_zoom, 0, wx1, wy1, wx2, wy2);
                    drawgraph();
                }
                break;
            case ZOOMY_2ND:
                set_action(0);
                select_region(sx, sy, old_x, old_y);
                device2world(sx, sy, &wx1, &wy1);
                device2world(old_x, old_y, &wx2, &wy2);
                if (sy == old_y) {
                    errwin("Zoomed rectangle is zero along Y, zoom cancelled");
                } else {
                    if (wy1 > wy2) {
                        fswap(&wy1, &wy2);
                    }
                    newworld(cg, linked_zoom, 1, wx1, wy1, wx2, wy2);
                    drawgraph();
                }
                break;
                /*
                 * set one corner of viewport
                 */
            case VIEW_1ST:
                set_action(VIEW_2ND);
                rectflag = 1;
                sx = x;
                sy = y;
                select_region(x, y, x, y);
                break;
                /*
                 * set opposing corner of viewport
                 */
            case VIEW_2ND: {
                double vx1, vx2, vy1, vy2;

                set_action(0);
                select_region(sx, sy, old_x, old_y);
                if (sx == old_x || sy == old_y) {
                    errwin("Viewport size incorrect, not changed");
                } else {
                    device2world(sx, sy, &wx1, &wy1);
                    device2world(old_x, old_y, &wx2, &wy2);
                    world2view(wx1, wy1, &vx1, &vy1);
                    world2view(wx2, wy2, &vx2, &vy2);
                    if (vx1 > vx2) {
                        fswap(&vx1, &vx2);
                    }
                    if (vy1 > vy2) {
                        fswap(&vy1, &vy2);
                    }
                    set_graph_viewport(cg, vx1, vy1, vx2, vy2);
                    update_view(cg);
                    drawgraph();
                }
            } break;
            }
            break;
        case Button2:
            getpoints(x, y);
            switch (action_flag) {
            case TRACKER: {
                extern int track_set, track_point;
                int xtmp, ytmp;
                double *xx, *yy;

                if (track_set == -1) { /* select nearest set, nearest point*/
                    device2world(x, y, &wx, &wy);
                    findpoint(cg, wx, wy, &wx, &wy, &track_set, &track_point);
                    track_point -= 2;
                } else { /* set selected, find nearest point */
                    if (track_point == -1) {
                        device2world(x, y, &wx, &wy);
                        findpoint_inset(cg, track_set, wx, wy, &track_point);
                        track_point -= 2;
                    }
                }
                track_point++;
                if (track_point < 0) {
                    track_point = getsetlength(cg, track_set) - 1;
                } else if (track_point >= getsetlength(cg, track_set)) {
                    track_point = 0;
                }
                xx = getx(cg, track_set);
                yy = gety(cg, track_set);
                if (inbounds(cg, xx[track_point], yy[track_point])) {
                    world2deviceabs(xx[track_point], yy[track_point], &xtmp, &ytmp);
                    setpointer(xtmp, ytmp);
                    sprintf(buf, "Set %d, loc %d, (%f, %f)", track_set, track_point + 1, xx[track_point],
                            yy[track_point]);
                } else {
                    sprintf(buf, "OUTSIDE - Set %d, loc %d, (%f, %f)", track_set, track_point + 1, xx[track_point],
                            yy[track_point]);
                }
                xv_setstr(locate_point_item, buf);
                set_action(TRACKER);
            } break;
            }

            break;
        }
        break;
    case MotionNotify:
        x = event->xmotion.x;
        y = event->xmotion.y;
        /* cross hair cursor function defined at the bottom of this file */
        if (cursortype) {
            motion((XMotionEvent*)event);
        }
        /* allows painting of points */
        if (event->xmotion.state & Button1MotionMask) {
            switch (action_flag) {
            case PAINT_POINTS: {
                int ind;
                extern int paint_skip; /* defined in ptswin.c */
                static int count = 0;

                if (paint_skip < 0) { /* initialize count */
                    count = 0;
                    paint_skip = -paint_skip + 1;
                }
                device2world(x, y, &wx, &wy);
                if (add_setno >= 0) {
                    if (paint_skip == 0 || (count % paint_skip == 0)) {
                        ind = getsetlength(cg, add_setno);
                        add_point(cg, add_setno, wx, wy, 0.0, 0.0, SET_XY);
                        sprintf(buf, "Set %d, loc %d, (%f, %f)", add_setno, ind + 1, wx, wy);
                        xv_setstr(locate_point_item, buf);
                        XDrawLine(disp, xwin, gc, x - 5, y - 5, x + 5, y + 5);
                        XDrawLine(disp, xwin, gc, x - 5, y + 5, x + 5, y - 5);
                        update_set_status(cg, add_setno);
                        set_action(PAINT_POINTS);
                    }
                    count++;
                } else {
                    set_action(0);
                }
            } break;
            }
        }
        getpoints(x, y);
        break;
    default:
        break;
    }
    /*
     * some mouse tracking stuff
     */
    switch (action_flag) {
    case MOVE_OBJECT_2ND:
    case COPY_OBJECT2ND:
        dx = sx - x;
        dy = sy - y;

        switch (ty) {
        case OBJECT_BOX:
        case OBJECT_ELLIPSE:
            select_region(sx, sy, xs, ys);
            sx = x;
            sy = y;
            xs = xs - dx;
            ys = ys - dy;
            select_region(sx, sy, xs, ys);
            break;
        case OBJECT_LINE:
            select_line(sx, sy, xs, ys);
            sx = x;
            sy = y;
            xs = xs - dx;
            ys = ys - dy;
            select_line(sx, sy, xs, ys);
            break;
        case OBJECT_STRING:
            select_region(sx, sy, xs, ys);
            sx = x;
            sy = y;
            xs = xs - dx;
            ys = ys - dy;
            select_region(sx, sy, xs, ys);
            break;
        }
        break;
    case STR_LOC:
        break;
    case LEG_LOC:
        break;
    }
    if (rectflag) {
        select_region(sx, sy, old_x, old_y);
        select_region(sx, sy, x, y);
    }
    if (rubber_flag) {
        select_line(sx, sy, old_x, old_y);
        select_line(sx, sy, x, y);
    }
    old_x = x;
    old_y = y;
}

/*
 * switch on the area calculator
 */
void do_select_area(void) {
    narea_pts = 0;
    set_action(0);
    set_action(COMP_AREA);
}

/*
 * switch on the perimeter calculator
 */
void do_select_peri(void) {
    narea_pts = 0;
    set_action(0);
    set_action(COMP_PERIMETER);
}

/*
 * define a region
 */
void do_select_region(void) {
    region_pts = 0;
    set_action(0);
    set_action(DEF_REGION);
}

/*
 * draw a crosshair cursor
 */
void motion(XMotionEvent* e) {
    if (e->type != MotionNotify) {
        return;
    }
    /* Erase the previous crosshair */
    XDrawLine(disp, xwin, gcxor, 0, cursor_oldy, win_w, cursor_oldy);
    XDrawLine(disp, xwin, gcxor, cursor_oldx, 0, cursor_oldx, win_h);

    /* Draw the new crosshair */
    cursor_oldx = e->x;
    cursor_oldy = e->y;
    XDrawLine(disp, xwin, gcxor, 0, cursor_oldy, win_w, cursor_oldy);
    XDrawLine(disp, xwin, gcxor, cursor_oldx, 0, cursor_oldx, win_h);
}

/*
 * double click detection
 */
#define CLICKINT 400

int double_click(XButtonEvent* e) {
    static Time lastc = 0;

    if (e->time - lastc < CLICKINT) {
        return 1;
    }
    lastc = e->time;
    return 0;
}

void switch_current_graph(int gfrom, int gto) {
    draw_focus(gfrom);
    cg = gto;
    defineworld(g[cg].w.xg1, g[cg].w.yg1, g[cg].w.xg2, g[cg].w.yg2, islogx(cg), islogy(cg));
    viewport(g[cg].v.xv1, g[cg].v.yv1, g[cg].v.xv2, g[cg].v.yv2);
    draw_focus(cg);
    make_format(cg);
    update_all(cg);
}
