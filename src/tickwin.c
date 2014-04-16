/* $Id: tickwin.c,v 1.3 1995/06/08 01:37:21 pturner Exp pturner $
 *
 * ticks / tick labels / axes labels
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ScrolledW.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"

static Widget ticks_frame;
static Widget ticks_panel;

static Widget *editaxis;	/* which axis to edit */
static Widget *axis_applyto;	/* ovverride */
static Widget offx;		/* x offset of axis in viewport coords */
static Widget offy;		/* y offset of axis in viewport coords */
static Widget tonoff;		/* toggle display of axis ticks */
static Widget tlonoff;		/* toggle display of tick labels */
static Widget axislabel;	/* axis label */
static Widget *axislabellayout;	/* axis label layout (perp or parallel) */
static Widget *axislabelplace;	/* axis label placement, auto or specified */
static Widget axislabelspec;	/* location of axis label if specified (viewport coords) */
static Widget *axislabelfont;	/* axis label font */
static Widget axislabelcharsize;/* axis label charsize */
static Widget *axislabelcolor;	/* axis label color */
static Widget *axislabellinew;	/* axis label linew */
static Widget tmajor;		/* major tick spacing */
static Widget tminor;		/* minor tick spacing */
static Widget *tickop;		/* ticks opposite */
static Widget *ticklop;		/* tick labels opposite */
static Widget *ticklabel_applyto;	/* ovverride */
static Widget *tlform;		/* format for labels */
static Widget *tlprec;		/* precision for labels */
static Widget *tlfont;		/* tick label font */
static Widget tlcharsize;	/* tick label charsize */
static Widget *tlcolor;		/* tick label color */
static Widget *tllinew;		/* tick label color */
static Widget tlappstr;		/* tick label append string */
static Widget tlprestr;		/* tick label prepend string */
static Widget *tlskip;		/* tick marks to skip */
static Widget *tlstarttype;	/* use graph min or starting value */
static Widget tlstart;		/* value to start tick labels */
static Widget *tlstoptype;	/* use graph max or stop value */
static Widget tlstop;		/* value to stop tick labels */
static Widget *tlloc;		/* Where to place the label relative to the tick */
static Widget *tllayout;	/* tick labels perp or horizontal or use the angle */
static Widget tlangle;		/* angle */
static Widget *tlstagger;	/* stagger */
static Widget *tlsign;		/* sign of tick label (normal, negate, *
				 * absolute) */
static Widget *tick_applyto;	/* override */
static Widget tgrid;		/* major ticks grid */
static Widget *tgridcol;
static Widget *tgridlinew;
static Widget *tgridlines;
static Widget tmgrid;		/* minor ticks grid */
static Widget *tmgridcol;
static Widget *tmgridlinew;
static Widget *tmgridlines;
static Widget tlen;		/* tick length */
static Widget tmlen;
static Widget *tinout;		/* ticks in out or both */
static Widget baronoff;		/* axis bar */
static Widget *barcolor;
static Widget *barlinew;
static Widget *barlines;

static Widget specticks;	/* special ticks and tick labels */
static Widget specticklabels;
static Widget nspec;
static Widget specnum[MAX_TICK_LABELS];	/* label denoting which tick/label */
static Widget specloc[MAX_TICK_LABELS];
static Widget speclabel[MAX_TICK_LABELS];

static void set_axis_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void ticks_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void accept_axis_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_axis_items(int gno);
static void do_axis_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void accept_axislabel_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_axislabel_items(int gno);
static void do_axislabel_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void accept_ticklabel_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_ticklabel_items(int gno);
static void do_ticklabels_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_tickmark_items(int gno);
static void accept_tickmark_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_tickmarks_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void accept_axisbar_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_axisbar_items(int gno);
static void do_axisbar_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void accept_special_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_special_items(int gno);
static void load_special(int gno, int a);
static void do_special_proc(Widget w, XtPointer client_data, XtPointer call_data);

static Widget but1[2];

void update_ticks(int gno)
{
    update_ticks_items(gno);
    update_axis_items(gno);
    update_axislabel_items(gno);
    update_ticklabel_items(gno);
    update_tickmark_items(gno);
    update_axisbar_items(gno);
    load_special(gno, curaxis);
    update_special_items(gno);
}

void update_ticks_items(int gno)
{
    tickmarks t;

    if (ticks_frame) {
	SetChoice(editaxis, curaxis);
	get_graph_tickmarks(gno, &t, curaxis);
	XmToggleButtonSetState(tlonoff, t.tl_flag == TRUE, False);
	XmToggleButtonSetState(tonoff, t.t_flag == TRUE, False);
	XmToggleButtonSetState(baronoff, t.t_drawbar == TRUE, False);
	XmTextSetString(axislabel, t.label.s);

	if (islogx(gno) && (curaxis % 2 == 0)) {
	    t.tmajor = (int) t.tmajor;
	    if (t.tmajor == 0) {
		t.tmajor = 1;
	    }
	    sprintf(buf, "%.0f", t.tmajor);
	} else if (islogy(gno) && (curaxis % 2 == 1)) {
	    t.tmajor = (int) t.tmajor;
	    if (t.tmajor == 0) {
		t.tmajor = 1;
	    }
	    sprintf(buf, "%.0f", t.tmajor);
	} else if (t.tmajor > 0) {
	    sprintf(buf, "%.5g", t.tmajor);
	} else {
	    strcpy(buf, "UNDEFINED");
	}
	XmTextSetString(tmajor, buf);
	if (islogx(gno) && (curaxis % 2 == 0)) {
	    t.tminor = (int) t.tminor;
	    if (t.tminor < 0 || t.tminor > 5) {
		t.tminor = 0;
	    }
	    sprintf(buf, "%.0f", t.tminor);
	} else if (islogy(gno) && (curaxis % 2 == 1)) {
	    t.tminor = (int) t.tminor;
	    if (t.tminor < 0 || t.tminor > 5) {
		t.tminor = 0;
	    }
	    sprintf(buf, "%.0f", t.tminor);
	} else if (t.tminor > 0) {
	    sprintf(buf, "%.5g", t.tminor);
	} else {
	    strcpy(buf, "UNDEFINED");
	}
	XmTextSetString(tminor, buf);
    }
}

static void set_axis_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int cd = (int) client_data;
    curaxis = cd;
    update_ticks(cg);
}

/*
 * Create the ticks popup
 */
void create_ticks_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wbut, rc;
    int x, y;
    int i;
    set_wait_cursor();
    if (ticks_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	ticks_frame = XmCreateDialogShell(app_shell, "Axes", NULL, 0);
	handle_close(ticks_frame);
	XtVaSetValues(ticks_frame, XmNx, x, XmNy, y, NULL);
	ticks_panel = XmCreateRowColumn(ticks_frame, "ticks_rc", NULL, 0);

	editaxis = (Widget *) CreatePanelChoice(ticks_panel, "Edit:",
						5,
						"X axis",
						"Y axis",
						"Zero X axis",
						"Zero Y axis",
						NULL,
						NULL);
	for (i = 0; i < 4; i++) {
	    XtAddCallback(editaxis[2 + i], XmNactivateCallback, (XtCallbackProc) set_axis_proc, (XtPointer) (intptr_t) i);
	}

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticks_panel, NULL);
	axislabel = CreateTextItem2(ticks_panel, 30, "Axis label:");
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticks_panel, NULL);
	tmajor = CreateTextItem2(ticks_panel, 10, "Major tick spacing:");
	tminor = CreateTextItem2(ticks_panel, 10, "Minor tick spacing:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticks_panel, NULL);
	tlonoff = XtVaCreateManagedWidget("Display tick labels",
				     xmToggleButtonWidgetClass, ticks_panel,
					  NULL);
	tonoff = XtVaCreateManagedWidget("Display tick marks",
				     xmToggleButtonWidgetClass, ticks_panel,
					 NULL);
	baronoff = XtVaCreateManagedWidget("Display axis bar",
				     xmToggleButtonWidgetClass, ticks_panel,
					   NULL);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticks_panel, NULL);
	rc = XmCreateRowColumn(ticks_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	wbut = XtVaCreateManagedWidget("Axis props...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_axis_proc, (XtPointer) 0);
	wbut = XtVaCreateManagedWidget("Axis label...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_axislabel_proc, (XtPointer) 0);
	wbut = XtVaCreateManagedWidget("Tick labels...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_ticklabels_proc, (XtPointer) 0);
	XtManageChild(rc);

	rc = XmCreateRowColumn(ticks_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	wbut = XtVaCreateManagedWidget("Tick marks...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_tickmarks_proc, (XtPointer) 0);
	wbut = XtVaCreateManagedWidget("Axis bar...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_axisbar_proc, (XtPointer) 0);
	wbut = XtVaCreateManagedWidget("User ticks/tick labels...",
				       xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_special_proc, (XtPointer) 0);
	XtManageChild(rc);
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticks_panel, NULL);
	axis_applyto = (Widget *) CreatePanelChoice(ticks_panel,
						    "Apply to:",
						    5,
						    "Current axis",
						    "All axes, current graph",
						    "Current axis, all graphs",
						    "All axes, all graphs",
						    NULL,
						    NULL);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticks_panel, NULL);

	CreateCommandButtons(ticks_panel, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) ticks_define_notify_proc, (XtPointer) 0);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) ticks_frame);

	XtManageChild(ticks_panel);
    }
    XtRaise(ticks_frame);
    update_ticks(cg);
    unset_wait_cursor();
}

/*
 * define tick marks
 */
static void ticks_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char val[80];
    int i, j;
    int applyto;
    extern double result;
    double x = (g[cg].w.xg2 - g[cg].w.xg1), y = (g[cg].w.yg2 - g[cg].w.yg1), a = g[cg].w.xg1, b = g[cg].w.yg1, c = g[cg].w.xg2, d = g[cg].w.yg2;
    int errpos;
    int axis_start, axis_stop, graph_start, graph_stop;
    tickmarks t;

    get_graph_tickmarks(cg, &t, curaxis);
    t.label.s = NULL;

    applyto = GetChoice(axis_applyto);
    strcpy(val, (char *) xv_getstr(tmajor));
    scanner(val, &x, &y, 1, &a, &b, &c, &d, 1, 0, 0, &errpos);
    if (errpos) {
	return;
    }
    t.tmajor = result;
    if (islogx(cg) && (curaxis % 2 == 0)) {
	t.tmajor = (int) t.tmajor;
    } else if (islogy(cg) && (curaxis % 2 == 1)) {
	t.tmajor = (int) t.tmajor;
    }
    strcpy(val, (char *) xv_getstr(tminor));
    scanner(val, &x, &y, 1, &a, &b, &c, &d, 1, 0, 0, &errpos);
    if (errpos) {
	return;
    }
    t.tminor = result;
    if (islogx(cg) && (curaxis % 2 == 0)) {
	t.tminor = (int) t.tminor;
	if (t.tminor < 0 || t.tminor > 5) {
	    t.tminor = 0;
	}
    } else if (islogy(cg) && (curaxis % 2 == 1)) {
	t.tminor = (int) t.tminor;
	if (t.tminor < 0 || t.tminor > 5) {
	    t.tminor = 0;
	}
    }
    t.tl_flag = XmToggleButtonGetState(tlonoff) ? TRUE : FALSE;
    t.t_flag = XmToggleButtonGetState(tonoff) ? TRUE : FALSE;
    t.t_drawbar = XmToggleButtonGetState(baronoff) ? TRUE : FALSE;
    set_plotstr_string(&t.label, (char *) xv_getstr(axislabel));
    
    switch (applyto) {
    case 0:			/* current axis */
        axis_start = curaxis;
        axis_stop  = curaxis;
        graph_start = cg;
        graph_stop  = cg;
        break;
    case 1:			/* all axes, current graph */
        axis_start = 0;
        axis_stop  = MAXAXES - 1;
        graph_start = cg;
        graph_stop  = cg;
        break;
    case 2:			/* current axis, all graphs */
        axis_start = curaxis;
        axis_stop  = curaxis;
        graph_start = 0;
        graph_stop  = maxgraph - 1;
        break;
    case 3:			/* all axes, all graphs */
        axis_start = 0;
        axis_stop  = MAXAXES - 1;
        graph_start = 0;
        graph_stop  = maxgraph - 1;
        break;
    default:
        axis_start = curaxis;
        axis_stop  = curaxis;
        graph_start = cg;
        graph_stop  = cg;
        break;        
    }
        
/*
 * 	set_graph_tickmarks(cg, &t, curaxis);
 * 	set_plotstr_string(&g[cg].t[curaxis].label, t.label.s);
 * 	break;
 */
    for (i = graph_start; i <= graph_stop; i++) {
        for (j = axis_start; j <= axis_stop; j++) {
            g[i].t[j].tl_flag = t.tl_flag;
            g[i].t[j].t_flag = t.t_flag;
            g[i].t[j].t_drawbar = t.t_drawbar;
            g[i].t[j].tmajor = t.tmajor;
            g[i].t[j].tminor = t.tminor;
            set_plotstr_string(&g[i].t[j].label, t.label.s);
        }
    }
    
    drawgraph();
    set_dirtystate();
}

static Widget axis_frame;
static Widget axis_panel;

static void accept_axis_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    tickmarks t;

    get_graph_tickmarks(cg, &t, curaxis);
    t.alt = FALSE;
    t.tmin = 0.0;
    t.tmax = 1.0;
    t.offsx = atof((char *) xv_getstr(offx));
    t.offsy = atof((char *) xv_getstr(offy));
    set_graph_tickmarks(cg, &t, curaxis);
    drawgraph();
    set_dirtystate();
}

static void update_axis_items(int gno)
{
    tickmarks t;

    if (axis_frame) {
	get_graph_tickmarks(gno, &t, curaxis);
	sprintf(buf, "%.5g", t.offsx);
	XmTextSetString(offx, buf);
	sprintf(buf, "%.5g", t.offsy);
	XmTextSetString(offy, buf);
    }
}

static void do_axis_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wlabel;
    int x, y;
    set_wait_cursor();
    if (axis_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	axis_frame = XmCreateDialogShell(app_shell, "Axis props", NULL, 0);
	handle_close(axis_frame);
	XtVaSetValues(axis_frame, XmNx, x, XmNy, y, NULL);
	axis_panel = XmCreateRowColumn(axis_frame, "axis_rc", NULL, 0);


	wlabel = XtVaCreateManagedWidget("Axis offset (viewport coordinates):",
					 xmLabelWidgetClass, axis_panel,
					 NULL);
	offx = CreateTextItem2(axis_panel, 10, "Left or bottom:");
	offy = CreateTextItem2(axis_panel, 10, "Right or top:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, axis_panel, NULL);

	CreateCommandButtons(axis_panel, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) accept_axis_proc, (XtPointer) 0);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) axis_frame);

	XtManageChild(axis_panel);
    }
    XtRaise(axis_frame);
    update_axis_items(cg);
    unset_wait_cursor();
}

static Widget axislabel_frame;
static Widget axislabel_panel;

static void accept_axislabel_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg a;
    tickmarks t;
    int iv;

    get_graph_tickmarks(cg, &t, curaxis);
    t.label_layout = GetChoice(axislabellayout) ? LAYOUT_PERPENDICULAR : LAYOUT_PARALLEL;
    t.label_place = GetChoice(axislabelplace) ? TYPE_SPEC : TYPE_AUTO;
    if (t.label_place == TYPE_SPEC) {
	char buf[128];
	strcpy(buf, xv_getstr(axislabelspec));
	sscanf(buf, "%lf %lf", &t.label.x, &t.label.y);
    }
    t.label.font = GetChoice(axislabelfont);
    t.label.color = GetChoice(axislabelcolor);
    t.label.linew = GetChoice(axislabellinew) + 1;
    XtSetArg(a, XmNvalue, &iv);
    XtGetValues(axislabelcharsize, &a, 1);
    t.label.charsize = iv / 100.0;
    set_graph_tickmarks(cg, &t, curaxis);
    drawgraph();
    set_dirtystate();
}

static void update_axislabel_items(int gno)
{
    Arg a;
    tickmarks t;
    int iv;
    char buf[128];

    if (axislabel_frame) {
	get_graph_tickmarks(gno, &t, curaxis);
	SetChoice(axislabellayout, t.label_layout == LAYOUT_PERPENDICULAR ? 1 : 0);
	SetChoice(axislabelplace, t.label_place == TYPE_AUTO ? 0 : 1);
	sprintf(buf, "%.2f %.2f", t.label.x, t.label.y);
	xv_setstr(axislabelspec, buf);
	SetChoice(axislabelfont, t.label.font);
	SetChoice(axislabelcolor, t.label.color);
	SetChoice(axislabellinew, t.label.linew - 1);
	iv = (int) (100 * t.label.charsize);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(axislabelcharsize, &a, 1);
    }
}

static void do_axislabel_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wlabel, rc;
    int x, y;
    set_wait_cursor();
    if (axislabel_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	axislabel_frame = XmCreateDialogShell(app_shell, "Axis label", NULL, 0);
	handle_close(axislabel_frame);
	XtVaSetValues(axislabel_frame, XmNx, x, XmNy, y, NULL);
	axislabel_panel = XmCreateRowColumn(axislabel_frame, "axislabel_rc", NULL, 0);

	axislabellayout = (Widget *) CreatePanelChoice(axislabel_panel, "Axis layout:",
						       3,
						       "Parallel to axis",
						    "Perpendicular to axis",
						       NULL,
						       NULL);

	axislabelplace = (Widget *) CreatePanelChoice(axislabel_panel, "Axis label location:",
						       3,
						       "Auto",
						    "Specified",
						       NULL,
						       NULL);
	axislabelspec = CreateTextItem2(axislabel_panel, 10, "Location:");

	axislabelfont = CreatePanelChoice(axislabel_panel, "Font:",
					  11,
				"Times-Roman", "Times-Bold", "Times-Italic",
					  "Times-BoldItalic", "Helvetica",
				      "Helvetica-Bold", "Helvetica-Oblique",
				 "Helvetica-BoldOblique", "Greek", "Symbol",
					  0,
					  0);

 
	rc = XmCreateRowColumn(axislabel_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	axislabelcolor = CreateColorChoice(rc, "Color:", 0);
	axislabellinew = CreatePanelChoice(rc, "Line width:",
					   10,
			     "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
					   0);
	XtManageChild(rc);

	wlabel = XtVaCreateManagedWidget("Size:", xmLabelWidgetClass, axislabel_panel,
					 NULL);
	axislabelcharsize = XtVaCreateManagedWidget("stringsize", xmScaleWidgetClass, axislabel_panel,
						    XmNminimum, 0,
						    XmNmaximum, 400,
						    XmNvalue, 100,
						    XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						    NULL);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, axislabel_panel, NULL);

	CreateCommandButtons(axislabel_panel, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) accept_axislabel_proc, (XtPointer) 0);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) axislabel_frame);

	XtManageChild(axislabel_panel);
    }
    XtRaise(axislabel_frame);
    update_axislabel_items(cg);
    unset_wait_cursor();
}

static Widget ticklabel_frame;
static Widget ticklabel_panel;

static void accept_ticklabel_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg a;
    tickmarks t;
    int iv;
    int i, j, applyto, gstart, gstop, astart, astop;;
    applyto = GetChoice(ticklabel_applyto);
    switch (applyto) {
    case 0:
	gstart = gstop = cg;
	astart = astop = curaxis;
	break;
    case 1:
	gstart = gstop = cg;
	astart = 0;
	astop = MAXAXES - 1;
	break;
    case 2:
	gstart = 0;
	gstop = maxgraph - 1;
	astart = astop = curaxis;
	break;
    case 3:
	gstart = 0;
	gstop = maxgraph - 1;
	astart = 0;
	astop = MAXAXES - 1;
	break;
    default:
	/* error */
	gstart = gstop = astart = astop = 0;
	errmsg("Internal error, accept_ticklabel_proc called with wrong argument");
    }
    for (i = gstart; i <= gstop; i++) {
	for (j = astart; j <= astop; j++) {
	    get_graph_tickmarks(i, &t, j);
	    t.tl_font = GetChoice(tlfont);
	    t.tl_color = GetChoice(tlcolor);
	    t.tl_linew = GetChoice(tllinew) + 1;
	    t.tl_skip = GetChoice(tlskip);
	    t.tl_prec = GetChoice(tlprec);
	    t.tl_staggered = (int) GetChoice(tlstagger);
	    strcpy(t.tl_appstr, xv_getstr(tlappstr));
	    strcpy(t.tl_prestr, xv_getstr(tlprestr));
	    t.tl_starttype = (int) GetChoice(tlstarttype) == 0 ? TYPE_AUTO : TYPE_SPEC;
	    if (t.tl_starttype == TYPE_SPEC) {
		t.tl_start = atof((char *) xv_getstr(tlstart));
	    }
	    t.tl_stoptype = (int) GetChoice(tlstoptype) == 0 ? TYPE_AUTO : TYPE_SPEC;
	    if (t.tl_stoptype == TYPE_SPEC) {
		t.tl_stop = atof((char *) xv_getstr(tlstop));
	    }
	    t.tl_format = format_types[(int) GetChoice(tlform)];
	    switch (GetChoice(ticklop)) {
	    case 0:
		if (j % 2) {
		    t.tl_op = PLACE_LEFT;
		} else {
		    t.tl_op = PLACE_BOTTOM;
		}
		break;
	    case 1:
		if (j % 2) {
		    t.tl_op = PLACE_RIGHT;
		} else {
		    t.tl_op = PLACE_TOP;
		}
		break;
	    case 2:
		t.tl_op = PLACE_BOTH;
		break;
	    }
	    switch ((int) GetChoice(tlsign)) {
	    case 0:
		t.tl_sign = SIGN_NORMAL;
		break;
	    case 1:
		t.tl_sign = SIGN_ABSOLUTE;
		break;
	    case 2:
		t.tl_sign = SIGN_NEGATE;
		break;
	    }
	    switch ((int) GetChoice(tllayout)) {
	    case 0:
		t.tl_layout = TICKS_HORIZONTAL;
		break;
	    case 1:
		t.tl_layout = TICKS_VERTICAL;
		break;
	    case 2:
		t.tl_layout = TYPE_SPEC;
		XtSetArg(a, XmNvalue, &iv);
		XtGetValues(tlangle, &a, 1);
		t.tl_angle = iv;
		break;
	    }
	    switch ((int) GetChoice(tlloc)) {
	    case 0:
		t.tl_loc = LABEL_ONTICK;
		break;
	    case 1:
		t.tl_loc = LABEL_BETWEEN;
		break;
	    }
	    XtSetArg(a, XmNvalue, &iv);
	    XtGetValues(tlcharsize, &a, 1);
	    t.tl_charsize = iv / 100.0;
	    set_graph_tickmarks(i, &t, j);
	}
    }
    drawgraph();
    set_dirtystate();
}

static void update_ticklabel_items(int gno)
{
    Arg a;
    tickmarks t;
    int iv;

    if (ticklabel_frame) {
	get_graph_tickmarks(gno, &t, curaxis);
	SetChoice(tlfont, t.tl_font);
	SetChoice(tlcolor, t.tl_color);
	SetChoice(tllinew, t.tl_linew - 1);
	SetChoice(tlskip, t.tl_skip);
	SetChoice(tlstagger, t.tl_staggered);
	xv_setstr(tlappstr, t.tl_appstr);
	xv_setstr(tlprestr, t.tl_prestr);
	SetChoice(tlstarttype, t.tl_starttype == TYPE_SPEC);
	if (t.tl_starttype == TYPE_SPEC) {
	    sprintf(buf, "%f", t.tl_start);
	    xv_setstr(tlstart, buf);
	    sprintf(buf, "%f", t.tl_stop);
	    xv_setstr(tlstop, buf);
	}
	SetChoice(tlstoptype, t.tl_stoptype == TYPE_SPEC);
	if (t.tl_stoptype == TYPE_SPEC) {
	    sprintf(buf, "%f", t.tl_stop);
	    xv_setstr(tlstop, buf);
	}
	iv = get_format_index(t.tl_format);
	SetChoice(tlform, iv);
	switch (t.tl_op) {
	case PLACE_LEFT:
	    SetChoice(ticklop, 0);
	    break;
	case PLACE_RIGHT:
	    SetChoice(ticklop, 1);
	    break;
	case PLACE_BOTTOM:
	    SetChoice(ticklop, 0);
	    break;
	case PLACE_TOP:
	    SetChoice(ticklop, 1);
	    break;
	case PLACE_BOTH:
	    SetChoice(ticklop, 2);
	    break;
	}
	switch (t.tl_sign) {
	case SIGN_NORMAL:
	    SetChoice(tlsign, 0);
	    break;
	case SIGN_ABSOLUTE:
	    SetChoice(tlsign, 1);
	    break;
	case SIGN_NEGATE:
	    SetChoice(tlsign, 2);
	    break;
	}
	SetChoice(tlprec, t.tl_prec);
	iv = (int) (100 * t.tl_charsize);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(tlcharsize, &a, 1);
	switch (t.tl_layout) {
	case TICKS_HORIZONTAL:
	    SetChoice(tllayout, 0);
	    break;
	case TICKS_VERTICAL:
	    SetChoice(tllayout, 1);
	    break;
	case TYPE_SPEC:
	    SetChoice(tllayout, 2);
	    break;
	}
	switch (t.tl_loc) {
	case LABEL_ONTICK:
	    SetChoice(tlloc, 0);
	    break;
	case LABEL_BETWEEN:
	    SetChoice(tlloc, 1);
	    break;
	}
	iv = (int) t.tl_angle % 360;
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(tlangle, &a, 1);
    }
}

static void do_ticklabels_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wlabel, rc;
    int x, y;
    set_wait_cursor();
    if (ticklabel_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	ticklabel_frame = XmCreateDialogShell(app_shell, "Tick labels", NULL, 0);
	handle_close(ticklabel_frame);
	XtVaSetValues(ticklabel_frame, XmNx, x, XmNy, y, NULL);
	ticklabel_panel = XmCreateRowColumn(ticklabel_frame, "ticklabel_rc", NULL, 0);

	tlfont = CreatePanelChoice(ticklabel_panel, "Font:",
				   11,
				"Times-Roman", "Times-Bold", "Times-Italic",
				   "Times-BoldItalic", "Helvetica",
				   "Helvetica-Bold", "Helvetica-Oblique",
				 "Helvetica-BoldOblique", "Greek", "Symbol",
				   0,
				   0);

	rc = XmCreateRowColumn(ticklabel_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	tlcolor = CreateColorChoice(rc, "Color:", 0);

	tllinew = CreatePanelChoice(rc, "Line width:",
				    10,
			     "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
				    0);
	XtManageChild(rc);

	rc = XmCreateRowColumn(ticklabel_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	wlabel = XtVaCreateManagedWidget("Char size:", xmLabelWidgetClass, rc,
					 NULL);
	tlcharsize = XtVaCreateManagedWidget("stringsize", xmScaleWidgetClass, rc,
					     XmNminimum, 0,
					     XmNmaximum, 400,
					     XmNvalue, 0,
					     XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					     XmNorientation, XmHORIZONTAL,
					     NULL);
	XtManageChild(rc);
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticklabel_panel, NULL);

	tlform = CreatePanelChoice0(ticklabel_panel,
				    "Format:", 4,
				    30,
				    "Decimal",
				    "Exponential",
				    "Power",
				    "General",
				    "DD-MM-YY",
				    "MM-DD-YY",
				    "YY-MM-DD",
				    "MM-YY",
				    "MM-DD",
				    "Month-DD",
				    "DD-Month",
				    "Month (abrev.)",
				    "Month (abrev.)-YY",
				    "Month",
				    "Day of week (abrev.)",
				    "Day of week",
				    "Day of year",
				    "HH:MM:SS.s",
				    "MM-DD HH:MM:SS.s",
				    "MM-DD-YY HH:MM:SS.s",
				    "YY-MM-DD HH:MM:SS.s",
				    "Degrees (lon)",
				    "DD MM' (lon)",
				    "DD MM' SS.s\" (lon)",
				    "MM' SS.s\" (lon)",
				    "Degrees (lat)",
				    "DD MM' (lat)",
				    "DD MM' SS.s\" (lat)",
				    "MM' SS.s\" (lat)", 0,
				    0);

	tlprec = CreatePanelChoice(ticklabel_panel, "Precision:",
				   11,
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
				   0);
	tlappstr = CreateTextItem2(ticklabel_panel, 10, "Append to labels:");
	tlprestr = CreateTextItem2(ticklabel_panel, 10, "Prepend to labels:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticklabel_panel, NULL);

	tlstagger = CreatePanelChoice(ticklabel_panel, "Stagger labels:",
				      11,
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
				      0);

	tlskip = CreatePanelChoice(ticklabel_panel, "Skip every:",
				   11,
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
				   0);
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticklabel_panel, NULL);

	rc = XmCreateRowColumn(ticklabel_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	tlstarttype = CreatePanelChoice(rc, "Start labels at:",
					3,
					"Graph min", "Specified:", 0,
					0);
	tlstart = XtVaCreateManagedWidget("tlstart", xmTextWidgetClass, rc,
					  XmNtraversalOn, True,
					  XmNcolumns, 10,
					  NULL);
	XtManageChild(rc);

	rc = XmCreateRowColumn(ticklabel_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	tlstoptype = CreatePanelChoice(rc, "Stop labels at:",
				       3,
				       "Graph max", "Specified:", 0,
				       0);
	tlstop = XtVaCreateManagedWidget("tlstop", xmTextWidgetClass, rc,
					 XmNtraversalOn, True,
					 XmNcolumns, 10,
					 NULL);
	XtManageChild(rc);

	tlloc = (Widget *) CreatePanelChoice(ticklabel_panel, "Location:",
						3,
						"On ticks",
						"Between ticks",
						NULL,
						NULL);

	rc = XmCreateRowColumn(ticklabel_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	tllayout = (Widget *) CreatePanelChoice(rc, "Layout:",
						4,
						"Horizontal",
						"Vertical",
						"Specified (degrees):",
						NULL,
						NULL);
	tlangle = XtVaCreateManagedWidget("ticklangle", xmScaleWidgetClass, rc,
					  XmNminimum, 0,
					  XmNmaximum, 360,
					  XmNvalue, 100,
					  XmNshowValue, True,
					  XmNprocessingDirection, XmMAX_ON_RIGHT,
					  XmNorientation, XmHORIZONTAL,
					  NULL);
	XtManageChild(rc);
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticklabel_panel, NULL);

	ticklop = CreatePanelChoice(ticklabel_panel, "Draw tick labels on:",
				    4,
				  "Normal side", "Opposite side", "Both", 0,
				    0);

	tlsign = CreatePanelChoice(ticklabel_panel, "Sign of label:",
				   4,
				   "As is", "Absolute value", "Negate",
				   NULL,
				   0);

	ticklabel_applyto = CreatePanelChoice(ticklabel_panel, "Apply to:",
					      5,
					      "Current axis",
					      "All axes, current graph",
					      "Current axis, all graphs",
					      "All axes, all graphs",
					      NULL,
					      0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, ticklabel_panel, NULL);

	CreateCommandButtons(ticklabel_panel, 2, but1, label1);

	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) accept_ticklabel_proc, (XtPointer) 0);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) ticklabel_frame);

	XtManageChild(ticklabel_panel);
    }
    XtRaise(ticklabel_frame);
    update_ticklabel_items(cg);
    unset_wait_cursor();
}

static Widget tickmark_frame;
static Widget tickmark_panel;

static void update_tickmark_items(int gno)
{
    Arg a;
    tickmarks t;
    int iv;

    if (tickmark_frame) {
	get_graph_tickmarks(gno, &t, curaxis);
	switch (t.t_inout) {
	case TICKS_IN:
	    SetChoice(tinout, 0);
	    break;
	case TICKS_OUT:
	    SetChoice(tinout, 1);
	    break;
	case TICKS_BOTH:
	    SetChoice(tinout, 2);
	    break;
	}
	switch (t.t_op) {
	case PLACE_LEFT:
	    SetChoice(tickop, 0);
	    break;
	case PLACE_RIGHT:
	    SetChoice(tickop, 1);
	    break;
	case PLACE_BOTTOM:
	    SetChoice(tickop, 0);
	    break;
	case PLACE_TOP:
	    SetChoice(tickop, 1);
	    break;
	case PLACE_BOTH:
	    SetChoice(tickop, 2);
	    break;
	}
	SetChoice(tgridcol, t.t_color);
	SetChoice(tgridlinew, t.t_linew - 1);
	SetChoice(tgridlines, t.t_lines - 1);
	SetChoice(tmgridcol, t.t_mcolor);
	SetChoice(tmgridlinew, t.t_mlinew - 1);
	SetChoice(tmgridlines, t.t_mlines - 1);
	iv = (int) (100 * t.t_size);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(tlen, &a, 1);
	iv = (int) (100 * t.t_msize);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(tmlen, &a, 1);
	XmToggleButtonSetState(tgrid, t.t_gridflag == TRUE, False);
	XmToggleButtonSetState(tmgrid, t.t_mgridflag == TRUE, False);
    }
}

static void accept_tickmark_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg a;
    tickmarks t;
    int iv;
    int i, j, applyto, gstart, gstop, astart, astop;;
    applyto = GetChoice(tick_applyto);
    switch (applyto) {
    case 0:
	gstart = gstop = cg;
	astart = astop = curaxis;
	break;
    case 1:
	gstart = gstop = cg;
	astart = 0;
	astop = MAXAXES - 1;
	break;
    case 2:
	gstart = 0;
	gstop = maxgraph - 1;
	astart = astop = curaxis;
	break;
    case 3:
	gstart = 0;
	gstop = maxgraph - 1;
	astart = 0;
	astop = MAXAXES - 1;
	break;
    default:
	/* error */
	gstart = gstop = astart = astop = 0;
	errmsg("Internal error, accept_tickmark_proc called with wrong argument");
    }
    for (i = gstart; i <= gstop; i++) {
	for (j = astart; j <= astop; j++) {
	    get_graph_tickmarks(i, &t, j);
	    switch ((int) GetChoice(tinout)) {
	    case 0:
		t.t_inout = TICKS_IN;
		break;
	    case 1:
		t.t_inout = TICKS_OUT;
		break;
	    case 2:
		t.t_inout = TICKS_BOTH;
		break;
	    }
	    switch (GetChoice(tickop)) {
	    case 0:
		if (j % 2) {
		    t.t_op = PLACE_LEFT;
		} else {
		    t.t_op = PLACE_BOTTOM;
		}
		break;
	    case 1:
		if (j % 2) {
		    t.t_op = PLACE_RIGHT;
		} else {
		    t.t_op = PLACE_TOP;
		}
		break;
	    case 2:
		t.t_op = PLACE_BOTH;
		break;
	    }
	    t.t_color = GetChoice(tgridcol);
	    t.t_linew = GetChoice(tgridlinew) + 1;
	    t.t_lines = GetChoice(tgridlines) + 1;
	    t.t_mcolor = GetChoice(tmgridcol);
	    t.t_mlinew = GetChoice(tmgridlinew) + 1;
	    t.t_mlines = GetChoice(tmgridlines) + 1;
	    XtSetArg(a, XmNvalue, &iv);
	    XtGetValues(tlen, &a, 1);
	    t.t_size = iv / 100.0;
	    XtSetArg(a, XmNvalue, &iv);
	    XtGetValues(tmlen, &a, 1);
	    t.t_msize = iv / 100.0;
	    t.t_gridflag = XmToggleButtonGetState(tgrid) ? TRUE : FALSE;
	    t.t_mgridflag = XmToggleButtonGetState(tmgrid) ? TRUE : FALSE;
	    set_graph_tickmarks(i, &t, j);
	}
    }
    drawgraph();
    set_dirtystate();
}

static void do_tickmarks_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wlabel, rc, rc2, rc3, fr;
    int x, y;
    set_wait_cursor();
    if (tickmark_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	tickmark_frame = XmCreateDialogShell(app_shell, "Tick marks", NULL, 0);
	handle_close(tickmark_frame);
	XtVaSetValues(tickmark_frame, XmNx, x, XmNy, y, NULL);
	tickmark_panel = XmCreateRowColumn(tickmark_frame, "tickmark_rc", NULL, 0);

	tinout = CreatePanelChoice(tickmark_panel, "Tick marks pointing:",
				   4,
				   "In", "Out", "Both", 0,
				   0);

	tickop = CreatePanelChoice(tickmark_panel, "Draw tick marks on:",
				   4,
			    "Normal side", "Opposite side", "Both sides", 0,
				   0);

	rc2 = XmCreateRowColumn(tickmark_panel, "rc2", NULL, 0);
	XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);

/* major tick marks */
	fr = XmCreateFrame(rc2, "fr", NULL, 0);
	rc = XmCreateRowColumn(fr, "rc", NULL, 0);

	tgrid = XtVaCreateManagedWidget("Major ticks grid lines",
					xmToggleButtonWidgetClass, rc,
					NULL);

	rc3 = XmCreateRowColumn(rc, "rc3", NULL, 0);
	wlabel = XtVaCreateManagedWidget("Major tick length:", xmLabelWidgetClass, rc3,
					 NULL);
	tlen = XtVaCreateManagedWidget("ticklength", xmScaleWidgetClass, rc3,
				       XmNminimum, 0,
				       XmNmaximum, 400,
				       XmNvalue, 100,
				       XmNshowValue, True,
				       XmNprocessingDirection, XmMAX_ON_RIGHT,
				       XmNorientation, XmHORIZONTAL,
				       NULL);
	XtManageChild(rc3);

	tgridcol = CreateColorChoice(rc, "Color:", 0);

	tgridlinew = CreatePanelChoice(rc, "Line width:",
				       10,
			     "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
				       0);
	tgridlines = (Widget *) CreatePanelChoice(rc, "Line style:",
						  6,
						  "Solid line",
						  "Dotted line",
						  "Dashed line",
						  "Long Dashed",
						  "Dot-dashed",
						  NULL,
						  NULL);
	XtManageChild(rc);
	XtManageChild(fr);

	fr = XmCreateFrame(rc2, "fr", NULL, 0);
	rc = XmCreateRowColumn(fr, "rc", NULL, 0);

	tmgrid = XtVaCreateManagedWidget("Minor ticks grid lines", xmToggleButtonWidgetClass, rc,
					 NULL);
	rc3 = XmCreateRowColumn(rc, "rc", NULL, 0);
	wlabel = XtVaCreateManagedWidget("Minor tick length:", xmLabelWidgetClass, rc3,
					 NULL);
	tmlen = XtVaCreateManagedWidget("mticklength", xmScaleWidgetClass, rc3,
					XmNminimum, 0,
					XmNmaximum, 400,
					XmNvalue, 100,
					XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					XmNorientation, XmHORIZONTAL,
					NULL);
	XtManageChild(rc3);

	tmgridcol = CreateColorChoice(rc, "Color:", 0);
	tmgridlinew = CreatePanelChoice(rc, "Line width:",
					10,
			     "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
					0);
	tmgridlines = (Widget *) CreatePanelChoice(rc, "Line style:",
						   6,
						   "Solid line",
						   "Dotted line",
						   "Dashed line",
						   "Long Dashed",
						   "Dot-dashed",
						   NULL,
						   NULL);
	XtManageChild(rc);
	XtManageChild(fr);
	XtManageChild(rc2);

	tick_applyto = CreatePanelChoice(tickmark_panel, "Apply to:",
					 5,
					 "Current axis",
					 "All axes, current graph",
					 "Current axis, all graphs",
					 "All axes, all graphs",
					 NULL,
					 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, tickmark_panel, NULL);

	CreateCommandButtons(tickmark_panel, 2, but1, label1);

	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) accept_tickmark_proc, (XtPointer) 0);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) tickmark_frame);


	XtManageChild(tickmark_panel);
    }
    XtRaise(tickmark_frame);
    update_tickmark_items(cg);
    unset_wait_cursor();
}

static Widget axisbar_frame;
static Widget axisbar_panel;

static void accept_axisbar_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    tickmarks t;

    get_graph_tickmarks(cg, &t, curaxis);
    t.t_drawbarcolor = GetChoice(barcolor);
    t.t_drawbarlinew = GetChoice(barlinew) + 1;
    t.t_drawbarlines = GetChoice(barlines) + 1;
    set_graph_tickmarks(cg, &t, curaxis);
    drawgraph();
    set_dirtystate();
}

static void update_axisbar_items(int gno)
{
    tickmarks t;

    if (axisbar_frame) {
	get_graph_tickmarks(gno, &t, curaxis);
	SetChoice(barcolor, t.t_drawbarcolor);
	SetChoice(barlinew, t.t_drawbarlinew - 1);
	SetChoice(barlines, t.t_drawbarlines - 1);
    }
}

static void do_axisbar_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    set_wait_cursor();
    if (axisbar_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	axisbar_frame = XmCreateDialogShell(app_shell, "Axis bar", NULL, 0);
	handle_close(axisbar_frame);
	XtVaSetValues(axisbar_frame, XmNx, x, XmNy, y, NULL);
	axisbar_panel = XmCreateRowColumn(axisbar_frame, "axisbar_rc", NULL, 0);

	barcolor = CreateColorChoice(axisbar_panel, "Color:", 0);

	barlinew = CreatePanelChoice(axisbar_panel, "Line width:",
				     10,
			     "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
				     0);

	barlines = (Widget *) CreatePanelChoice(axisbar_panel, "Line style:",
						6,
						"Solid line",
						"Dotted line",
						"Dashed line",
						"Long Dashed",
						"Dot-dashed",
						NULL,
						NULL);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, axisbar_panel, NULL);

	CreateCommandButtons(axisbar_panel, 2, but1, label1);

	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) accept_axisbar_proc, (XtPointer) 0);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) axisbar_frame);

	XtManageChild(axisbar_panel);
    }
    XtRaise(axisbar_frame);
    update_axisbar_items(cg);
    unset_wait_cursor();
}

static Widget special_frame;
static Widget special_panel;

#define TPAGESIZE 5
#define NPAGES (MAX_TICK_LABELS/TPAGESIZE)

static void accept_special_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    tickmarks t;
    int iv, i;

    get_graph_tickmarks(cg, &t, curaxis);
    t.t_type = XmToggleButtonGetState(specticks) ? TYPE_SPEC : TYPE_AUTO;
    t.tl_type = XmToggleButtonGetState(specticklabels) ? TYPE_SPEC : TYPE_AUTO;
    iv = atoi((char *) xv_getstr(nspec));
    if (iv > MAX_TICK_LABELS) {
	sprintf(buf, "Number of ticks/tick labels exceeds %d", MAX_TICK_LABELS);
	errwin(buf);
	return;
    }
    t.t_spec = iv;
    for (i = 0; i < MAX_TICK_LABELS; i++) {
	t.t_specloc[i] = atof((char *) xv_getstr(specloc[i]));
	set_plotstr_string(&t.t_speclab[i], (char *) xv_getstr(speclabel[i]));
    }
    set_graph_tickmarks(cg, &t, curaxis);
    drawgraph();
    set_dirtystate();
}

static void update_special_items(int gno)
{
    tickmarks t;

    if (special_frame) {
	get_graph_tickmarks(gno, &t, curaxis);
	XmToggleButtonSetState(specticks, t.t_type == TYPE_SPEC, False);
	XmToggleButtonSetState(specticklabels, t.tl_type == TYPE_SPEC, False);
    }
}

static void load_special(int gno, int a)
{
    int i;
    char buf[128];
    tickmarks t;

    if (special_frame) {
	get_graph_tickmarks(gno, &t, a);
	sprintf(buf, "%d", t.t_spec);
	xv_setstr(nspec, buf);
	for (i = 0; i < t.t_spec; i++) {
	    sprintf(buf, "%f", t.t_specloc[i]);
	    xv_setstr(specloc[i], buf);
	    if (t.t_speclab[i].s != NULL) {
		xv_setstr(speclabel[i], t.t_speclab[i].s);
	    }
	}
    }
}

static void do_special_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wlabel, rc, rc3, sw;
    int i, x, y;
    char buf[10];
    set_wait_cursor();
    if (special_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	special_frame = XmCreateDialogShell(app_shell, "Specified ticks/ticklabels", NULL, 0);
	handle_close(special_frame);
	XtVaSetValues(special_frame, XmNx, x, XmNy, y, NULL);
	special_panel = XmCreateForm(special_frame, "special_form", NULL, 0);

	rc = XmCreateRowColumn(special_panel, "rc", NULL, 0);
	specticks = XtVaCreateManagedWidget("Use special tick locations",
					    xmToggleButtonWidgetClass, rc,
					    NULL);
	specticklabels = XtVaCreateManagedWidget("Use special tick labels",
					      xmToggleButtonWidgetClass, rc,
						 NULL);

	nspec = CreateTextItem2(rc, 10, "# of user ticks/labels to use:");
	wlabel = XtVaCreateManagedWidget("Tick location - Label:", xmLabelWidgetClass,
					 rc, NULL);
	XtManageChild(rc);
	XtVaSetValues(rc,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      XmNtopAttachment, XmATTACH_FORM,
		      NULL);

	sw = XtVaCreateManagedWidget("sw",
				 xmScrolledWindowWidgetClass, special_panel,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     XmNtopAttachment, XmATTACH_WIDGET,
				     XmNtopWidget, rc,
				     XmNleftAttachment, XmATTACH_FORM,
				     XmNrightAttachment, XmATTACH_FORM,
				     NULL);
	rc = XmCreateRowColumn(sw, "rc", NULL, 0);
	XtVaSetValues(sw,
		      XmNworkWindow, rc,
		      NULL);

	for (i = 0; i < MAX_TICK_LABELS; i++) {
	    rc3 = XmCreateRowColumn(rc, "rc3", NULL, 0);
	    XtVaSetValues(rc3, XmNorientation, XmHORIZONTAL, NULL);
	    sprintf(buf, "%2d", i + 1);
	    specnum[i] = XtVaCreateManagedWidget(buf, xmLabelWidgetClass, rc3,
						 NULL);
	    specloc[i] = XtVaCreateManagedWidget("tickmark", xmTextFieldWidgetClass, rc3,
						 XmNcolumns, 10,
						 NULL);
	    speclabel[i] = XtVaCreateManagedWidget("ticklabel", xmTextFieldWidgetClass, rc3,
						   XmNcolumns, 35,
						   NULL);
	    XtManageChild(rc3);
	}
	XtManageChild(rc);
	XtManageChild(sw);

	rc = XmCreateRowColumn(special_panel, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);

	CreateCommandButtons(rc, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) accept_special_proc, (XtPointer) 0);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) special_frame);
	XtManageChild(rc);
	XtVaSetValues(rc,
		      XmNleftAttachment, XmATTACH_FORM,
		      XmNrightAttachment, XmATTACH_FORM,
		      XmNbottomAttachment, XmATTACH_FORM,
		      NULL);
	XtVaSetValues(sw,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, rc,
		      NULL);

	load_special(cg, curaxis);
	XtManageChild(special_panel);
    }
    XtRaise(special_frame);
    update_special_items(cg);
    unset_wait_cursor();
}

void UpdateTickWinColors(unsigned color) {
  UpdateColorChoice(axislabelcolor,color);
  UpdateColorChoice(tlcolor,color);
  UpdateColorChoice(tgridcol,color);
  UpdateColorChoice(tmgridcol,color);
  UpdateColorChoice(barcolor,color);
}
