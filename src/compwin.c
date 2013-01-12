/* $Id: compwin.c,v 1.2 1995/04/19 03:10:31 pturner Exp pturner $
 *
 * transformations, curve fitting, etc.
 *
 * formerly, this was all one big popup, now it is several.
 * All are created as needed
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/List.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"

extern int nonlflag;		/* true if nonlinear curve fitting module is
				 * to be included */
static int pick_gno;		/* Used in pick operations */
static int pick_set;		/* Used in pick operations */

static Widget but1[2];
static Widget but2[3];

static void do_compute_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_load_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_compute_proc2(Widget w, XtPointer client_data, XtPointer call_data);
static void do_digfilter_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_linearc_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_xcor_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_spline_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_int_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_differ_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_seasonal_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_interp_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_regress_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_runavg_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_fourier_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_fft_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_window_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_histo_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_sample_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_prune_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void do_prune_proc(Widget w, XtPointer client_data, XtPointer call_data);

void do_pick_compose(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_action(0);
    set_action((int) client_data);
}

typedef struct _Eval_ui {
    Widget top;
    SetChoiceItem sel;
    Widget formula_item;
    Widget *load_item;
    Widget *loadgraph_item;
    Widget *region_item;
    Widget rinvert_item;
} Eval_ui;

static Eval_ui eui;

void create_eval_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog, rc;
    set_wait_cursor();
    if (eui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	eui.top = XmCreateDialogShell(app_shell, "Evaluate expression", NULL, 0);
	handle_close(eui.top);
	XtVaSetValues(eui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(eui.top, "dialog_rc", NULL, 0);

	eui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	rc = XmCreateRowColumn(dialog, "rc", NULL, 0);
	XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
	eui.load_item = CreatePanelChoice(rc,
					  "Result to:", 3,
					  "Same set", "New set", NULL, 0);
	eui.loadgraph_item = CreateGraphChoice(rc, "In graph: ", maxgraph, 1);
	XtManageChild(rc);

	eui.formula_item = CreateTextItem2(dialog, 30, "Formula:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_compute_proc, (XtPointer) & eui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_EXPR);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) eui.top);

	XtManageChild(dialog);
    }
    XtRaise(eui.top);
    unset_wait_cursor();
}

/*
 * evaluate a formula
 */
static void do_compute_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, loadto, graphto, resno;
    char fstr[256];
    Eval_ui *ui = (Eval_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    loadto = (int) GetChoice(ui->load_item);
    graphto = (int) GetChoice(ui->loadgraph_item) - 1;
    strcpy(fstr, (char *) xv_getstr(ui->formula_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	resno = do_compute(setno, loadto, graphto, fstr);
	if (resno < 0) {
	    errwin("Error in  do_compute(), check expression");
	    break;
	}
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

typedef struct _Load_ui {
    Widget top;
    SetChoiceItem sel;
    Widget start_item;
    Widget step_item;
    Widget *load_item;
} Load_ui;

static Load_ui lui;

/* load a set */

void create_load_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (lui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	lui.top = XmCreateDialogShell(app_shell, "Load values", NULL, 0);
	handle_close(lui.top);
	XtVaSetValues(lui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(lui.top, "dialog_rc", NULL, 0);

	lui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 3,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Load to: ", xmLabelWidgetClass, rc, NULL);
	lui.load_item = CreatePanelChoice(rc,
					  " ",
					  7,
					  "Set X",
					  "Set Y",
					  "Scratch A",
					  "Scratch B",
					  "Scratch C",
					  "Scratch D", 0,
					  0);
	lui.start_item = CreateTextItem4(rc, 10, "Start:");
	lui.step_item = CreateTextItem4(rc, 10, "Step:");
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_load_proc, (XtPointer) & lui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) lui.top);

	XtManageChild(dialog);
    }
    XtRaise(lui.top);
    unset_wait_cursor();
}

/*
 * load a set
 */
static void do_load_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, toval;
    char startstr[256], stepstr[256];
    Load_ui *ui = (Load_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    toval = (int) GetChoice(ui->load_item) + 1;
    strcpy(stepstr, (char *) xv_getstr(ui->step_item));
    strcpy(startstr, (char *) xv_getstr(ui->start_item));

    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_load(setno, toval, startstr, stepstr);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* histograms */

typedef struct _Histo_ui {
    Widget top;
    SetChoiceItem sel;
    Widget binw_item;
    Widget hxmin_item;
    Widget hxmax_item;
    Widget *type_item;
    Widget *graph_item;
} Histo_ui;

static Histo_ui hui;

void create_histo_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (hui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	hui.top = XmCreateDialogShell(app_shell, "Histograms", NULL, 0);
	handle_close(hui.top);
	XtVaSetValues(hui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(hui.top, "dialog_rc", NULL, 0);

	hui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 4,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Bin width: ", xmLabelWidgetClass, rc, NULL);
	hui.binw_item = XtVaCreateManagedWidget("binwidth", xmTextWidgetClass, rc, NULL);
	XtVaSetValues(hui.binw_item, XmNcolumns, 10, NULL);
	XtVaCreateManagedWidget("Start value: ", xmLabelWidgetClass, rc, NULL);
	hui.hxmin_item = XtVaCreateManagedWidget("xmin", xmTextWidgetClass, rc, NULL);
	XtVaSetValues(hui.hxmin_item, XmNcolumns, 10, NULL);
	XtVaCreateManagedWidget("Ending value: ", xmLabelWidgetClass, rc, NULL);
	hui.hxmax_item = XtVaCreateManagedWidget("xmax", xmTextWidgetClass, rc, NULL);
	XtVaSetValues(hui.hxmax_item, XmNcolumns, 10, NULL);
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);
	hui.type_item = CreatePanelChoice(dialog, "Compute: ",
					  3,
					  "Histogram",
					  "Cumulative histogram",
					  0,
					  0);
	hui.graph_item = CreateGraphChoice(dialog, "Load result to graph:", maxgraph, 1);
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_histo_proc, (XtPointer) & hui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_HISTO);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) hui.top);

	XtManageChild(dialog);
    }
    XtRaise(hui.top);
    unset_wait_cursor();
}

/*
 * histograms
 */
static void do_histo_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int fromset, toset, tograph, hist_type;
    double binw, xmin, xmax;
    Histo_ui *ui = (Histo_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    toset = SET_SELECT_NEXT;
    tograph = (int) GetChoice(ui->graph_item) - 1;
    binw = atof((char *) xv_getstr(ui->binw_item));
    xmin = atof((char *) xv_getstr(ui->hxmin_item));
    xmax = atof((char *) xv_getstr(ui->hxmax_item));
    hist_type = (int) GetChoice(ui->type_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	fromset = selsets[i];
	do_histo(fromset, toset, tograph, binw, xmin, xmax, hist_type);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* DFTs */

typedef struct _Four_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *load_item;
    Widget *window_item;
    Widget *loadx_item;
    Widget *inv_item;
    Widget *type_item;
    Widget *graph_item;
} Four_ui;

static Four_ui fui;

void create_fourier_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;
    Widget rc;
    Widget buts[5];

    set_wait_cursor();
    if (fui.top == NULL) {
	char *l[5];
	l[0] = "DFT";
	l[1] = "FFT";
	l[2] = "Window only";
	l[3] = "Pick";
	l[4] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	fui.top = XmCreateDialogShell(app_shell, "Fourier transforms", NULL, 0);
	handle_close(fui.top);
	XtVaSetValues(fui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(fui.top, "dialog_rc", NULL, 0);

	fui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 5,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Data window: ", xmLabelWidgetClass, rc, NULL);
	fui.window_item = CreatePanelChoice(rc,
					    " ",
					    8,
					    "None (Rectangular)",
					    "Triangular",
					    "Hanning",
					    "Welch",
					    "Hamming",
					    "Blackman",
					    "Parzen",
					    NULL,
					    NULL);

	XtVaCreateManagedWidget("Load result as: ", xmLabelWidgetClass, rc, NULL);

	fui.load_item = CreatePanelChoice(rc,
					  " ",
					  4,
					  "Magnitude",
					  "Phase",
					  "Coefficients",
					  0,
					  0);

	XtVaCreateManagedWidget("Let result X = ", xmLabelWidgetClass, rc, NULL);
	fui.loadx_item = CreatePanelChoice(rc,
					   " ",
					   4,
					   "Index",
					   "Frequency",
					   "Period",
					   0,
					   0);

	XtVaCreateManagedWidget("Perform: ", xmLabelWidgetClass, rc, NULL);
	fui.inv_item = CreatePanelChoice(rc,
					 " ",
					 3,
					 "Transform",
					 "Inverse transform",
					 0,
					 0);

	XtVaCreateManagedWidget("Data is: ", xmLabelWidgetClass, rc, NULL);
	fui.type_item = CreatePanelChoice(rc,
					  " ",
					  3,
					  "Real",
					  "Complex",
					  0,
					  0);
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);
	CreateCommandButtons(dialog, 5, buts, l);
	XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc) do_fourier_proc, (XtPointer) & fui);
	XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc) do_fft_proc, (XtPointer) & fui);
	XtAddCallback(buts[2], XmNactivateCallback, (XtCallbackProc) do_window_proc, (XtPointer) & fui);
	XtAddCallback(buts[3], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_FOURIER);
	XtAddCallback(buts[4], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) fui.top);

	XtManageChild(dialog);
    }
    XtRaise(fui.top);
    unset_wait_cursor();
}

/*
 * DFT
 */
static void do_fourier_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, load, loadx, invflag, type, wind;
    Four_ui *ui = (Four_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    wind = GetChoice(ui->window_item);
    load = GetChoice(ui->load_item);
    loadx = GetChoice(ui->loadx_item);
    invflag = GetChoice(ui->inv_item);
    type = GetChoice(ui->type_item);
    set_wait_cursor();
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_fourier(0, setno, load, loadx, invflag, type, wind);
    }
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/*
 * DFT by FFT
 */
static void do_fft_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, load, loadx, invflag, type, wind;
    Four_ui *ui = (Four_ui *) client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
	errwin("No sets selected");
	return;
    }
    wind = GetChoice(ui->window_item);
    load = GetChoice(ui->load_item);
    loadx = GetChoice(ui->loadx_item);
    invflag = GetChoice(ui->inv_item);
    type = GetChoice(ui->type_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_fourier(1, setno, load, loadx, invflag, type, wind);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/*
 * Apply data window only
 */
static void do_window_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, type, wind;
    Four_ui *ui = (Four_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    wind = GetChoice(ui->window_item);
    type = GetChoice(ui->type_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_window(setno, type, wind);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/* running averages */

typedef struct _Run_ui {
    Widget top;
    SetChoiceItem sel;
    Widget len_item;
    Widget *type_item;
    Widget *region_item;
    Widget rinvert_item;
} Run_ui;

static Run_ui rui;

void create_run_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (rui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	rui.top = XmCreateDialogShell(app_shell, "Running averages", NULL, 0);
	handle_close(rui.top);
	XtVaSetValues(rui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(rui.top, "dialog_rc", NULL, 0);

	rui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 5,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Running:", xmLabelWidgetClass, rc, NULL);
	rui.type_item = CreatePanelChoice(rc,
					  " ",
					  6,
					  "Average",
					  "Median",
					  "Minimum",
					  "Maximum",
					  "Std. dev.", 0,
					  0);
	rui.len_item = CreateTextItem4(rc, 10, "Length of average:");

	XtVaCreateManagedWidget("Restrictions:", xmLabelWidgetClass, rc, NULL);
	rui.region_item = CreatePanelChoice(rc,
					    " ",
					    9,
					    "None",
					    "Region 0",
					    "Region 1",
					    "Region 2",
					    "Region 3",
					    "Region 4",
					    "Inside graph",
					    "Outside graph",
					    0,
					    0);

	XtVaCreateManagedWidget("Invert region:", xmLabelWidgetClass, rc, NULL);
	rui.rinvert_item = XmCreateToggleButton(rc, " ", NULL, 0);
	XtManageChild(rui.rinvert_item);

	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_runavg_proc, (XtPointer) & rui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_RUNAVG);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) rui.top);

	XtManageChild(dialog);
    }
    XtRaise(rui.top);
    unset_wait_cursor();
}

/*
 * running averages, medians, min, max, std. deviation
 */
static void do_runavg_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int runlen, runtype, setno, rno, invr;
    Run_ui *ui = (Run_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    runlen = atoi((char *) xv_getstr(ui->len_item));
    runtype = GetChoice(ui->type_item);
    rno = GetChoice(ui->region_item) - 1;
    invr = XmToggleButtonGetState(ui->rinvert_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_runavg(setno, runlen, runtype, rno, invr);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* TODO finish this */
void do_eval_regress()
{
}

typedef struct _Reg_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *degree_item;
    Widget zero_item;
    Widget *resid_item;
    Widget *region_item;
    Widget rinvert_item;
    Widget start_item;
    Widget stop_item;
    Widget step_item;
    Widget method_item;
} Reg_ui;

static Reg_ui regui;

void create_reg_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;
    Widget rc, rc2;
    Widget buts[4];

    set_wait_cursor();
    if (regui.top == NULL) {
	char *label1[4];
	label1[0] = "Accept";
	label1[1] = "Pick";
/*
	label1[2] = "Eval...";
*/
	label1[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	regui.top = XmCreateDialogShell(app_shell, "Regression", NULL, 0);
	handle_close(regui.top);
	XtVaSetValues(regui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(regui.top, "dialog_rc", NULL, 0);

	regui.sel = CreateSetSelector(dialog, "Apply to set:",
				      SET_SELECT_ALL,
				      FILTER_SELECT_NONE,
				      GRAPH_SELECT_CURRENT,
				      SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNorientation, XmVERTICAL,
 			      NULL);

	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, rc,
			      XmNorientation, XmHORIZONTAL,
 			      NULL);
			      
	XtVaCreateManagedWidget("Type of fit:", xmLabelWidgetClass, rc2, NULL);
	regui.degree_item = CreatePanelChoice(rc2,
					      " ",
					      16,
					      "Linear",
					      "Quadratic",
					      "Cubic",
					      "4th degree",
					      "5th degree",
					      "6th degree",
					      "7th degree",
					      "8th degree",
					      "9th degree",
					      "10th degree",
					      "1-10",
					      "Power y=A*x^B",
					      "Exponential y=A*exp(B*x)",
					      "Logarithmic y=A+B*ln(x)",
					      "Inverse y=1/(A+Bx)",
					      0,
					      0);
	XtManageChild(rc2);
	
	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, rc,
			      XmNorientation, XmHORIZONTAL,
 			      NULL);
			      
	XtVaCreateManagedWidget("Load:", xmLabelWidgetClass, rc2, NULL);
	regui.resid_item = CreatePanelChoice(rc2,
					     " ",
					     3,
					     "Fitted values",
					     "Residuals",
					     0,
					     0);
        XtManageChild(rc2);
	
	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, rc,
			      XmNorientation, XmHORIZONTAL,
 			      NULL);
	XtVaCreateManagedWidget("Restrictions:", xmLabelWidgetClass, rc2, NULL);
	regui.region_item = CreatePanelChoice(rc2,
					      " ",
					      9,
					      "None",
					      "Region 0",
					      "Region 1",
					      "Region 2",
					      "Region 3",
					      "Region 4",
					      "Inside graph",
					      "Outside graph",
					      0,
					      0);
	XtManageChild(rc2);
	
	rc2 = XtVaCreateWidget("rc2", xmRowColumnWidgetClass, rc,
			      XmNorientation, XmHORIZONTAL,
 			      NULL);
	XtVaCreateManagedWidget("Invert region:", xmLabelWidgetClass, rc2, NULL);
	regui.rinvert_item = XmCreateToggleButton(rc2, " ", NULL, 0);
	XtManageChild(regui.rinvert_item);
	XtManageChild(rc2);
	
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc) do_regress_proc, (XtPointer) & regui);
	XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_REG);
	XtAddCallback(buts[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) regui.top);

	XtManageChild(dialog);
    }
    XtRaise(regui.top);
    unset_wait_cursor();
}

/*
 * regression
 */
static void do_regress_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int cnt;
    Reg_ui *ui = (Reg_ui *) client_data;
    int setno, ideg, iresid, i, j;
    int rno = GetChoice(ui->region_item) - 1;
    int invr = XmToggleButtonGetState(ui->rinvert_item);

    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    ideg = (int) GetChoice(ui->degree_item) + 1;
    iresid = (int) GetChoice(ui->resid_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (j = 0; j < cnt; j++) {
	setno = selsets[j];
	if (ideg == 11) {
	    for (i = 1; i <= ideg - 1; i++) {
		do_regress(setno, i, iresid, rno, invr);
	    }
	} else {
	    do_regress(setno, ideg, iresid, rno, invr);
	}
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* finite differencing */

typedef struct _Diff_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget *region_item;
    Widget rinvert_item;
} Diff_ui;

static Diff_ui dui;

void create_diff_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (dui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	dui.top = XmCreateDialogShell(app_shell, "Differences", NULL, 0);
	handle_close(dui.top);
	XtVaSetValues(dui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(dui.top, "dialog_rc", NULL, 0);

	dui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	dui.type_item = CreatePanelChoice(dialog,
					  "Method:",
					  4,
					  "Forward difference",
					  "Backward difference",
					  "Centered difference",
					  0,
					  0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_differ_proc, (XtPointer) & dui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_DIFF);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) dui.top);

	XtManageChild(dialog);
    }
    XtRaise(dui.top);
    unset_wait_cursor();
}

/*
 * finite differences
 */
static void do_differ_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, itype;
    Diff_ui *ui = (Diff_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    itype = (int) GetChoice(ui->type_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_differ(setno, itype);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* numerical integration */

typedef struct _Int_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget sum_item;
    Widget *region_item;
    Widget rinvert_item;
} Int_ui;

static Int_ui iui;

void create_int_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (iui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	iui.top = XmCreateDialogShell(app_shell, "Integration", NULL, 0);
	handle_close(iui.top);
	XtVaSetValues(iui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(iui.top, "dialog_rc", NULL, 0);
	iui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	iui.type_item = CreatePanelChoice(dialog,
					  "Load:",
					  3,
					  "Cumulative sum",
					  "Sum only",
					  0,
					  0);
	iui.sum_item = CreateTextItem2(dialog, 10, "Sum:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_int_proc, (XtPointer) & iui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_INT);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) iui.top);

	XtManageChild(dialog);
    }
    XtRaise(iui.top);
    unset_wait_cursor();
}

/*
 * numerical integration
 */
static void do_int_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, itype;
    double sum, do_int(int setno, int itype);
    Int_ui *ui = (Int_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    itype = GetChoice(ui->type_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	sum = do_int(setno, itype);
	sprintf(buf, "%g", sum);
	xv_setstr(ui->sum_item, buf);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* seasonal differencing */

typedef struct _Seas_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget period_item;
    Widget *region_item;
    Widget rinvert_item;
} Seas_ui;

static Seas_ui sui;

void create_seasonal_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (sui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	sui.top = XmCreateDialogShell(app_shell, "Seasonal differences", NULL, 0);
	handle_close(sui.top);
	XtVaSetValues(sui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(sui.top, "dialog_rc", NULL, 0);

	sui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);
	sui.period_item = CreateTextItem2(dialog, 10, "Period:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_seasonal_proc, (XtPointer) & sui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_SEASONAL);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) sui.top);

	XtManageChild(dialog);
    }
    XtRaise(sui.top);
    unset_wait_cursor();
}

/*
 * seasonal differences
 */
static void do_seasonal_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, period;
    Seas_ui *ui = (Seas_ui *) client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    period = atoi(xv_getstr(ui->period_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_seasonal_diff(setno, period);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/* interpolation */

typedef struct _Interp_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *type_item;
    Widget *region_item;
    Widget *meth_item;
    Widget rinvert_item;
} Interp_ui;

static Interp_ui interpui;

void create_interp_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (interpui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	interpui.top = XmCreateDialogShell(app_shell, "Interpolation", NULL, 0);
	handle_close(interpui.top);
	XtVaSetValues(interpui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(interpui.top, "dialog_rc", NULL, 0);

	interpui.sel1 = CreateSetSelector(dialog, "Interpolate on set:",
					  SET_SELECT_ACTIVE,
					  FILTER_SELECT_NONE,
					  GRAPH_SELECT_CURRENT,
					  SELECTION_TYPE_SINGLE);
	interpui.sel2 = CreateSetSelector(dialog, "At points from set:",
					  SET_SELECT_ACTIVE,
					  FILTER_SELECT_NONE,
					  GRAPH_SELECT_CURRENT,
					  SELECTION_TYPE_SINGLE);

	interpui.meth_item = CreatePanelChoice(dialog,
					  "Method:",
					  4,
					  "Linear",
					  "Spline",
					  "Akima",
					  NULL, 0);
					  
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_interp_proc, (XtPointer) & interpui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) interpui.top);

	XtManageChild(dialog);
    }
    XtRaise(interpui.top);
    unset_wait_cursor();
}

/*
 * interpolation
 */
static void do_interp_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
/* TODO
    int *selsets;
    int i, cnt;
    int setno;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
*/
    int set1, set2, method;
    Interp_ui *ui = (Interp_ui *) client_data;
    set1 = GetSelectedSet(ui->sel1);
    set2 = GetSelectedSet(ui->sel2);
    if (set1 == SET_SELECT_ERROR || set2 == SET_SELECT_ERROR) {
		errwin("Select 2 sets");
		return;
    }
    method = (int) GetChoice(ui->meth_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    do_interp(set1, set2, method);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
}

/* cross correlation */

typedef struct _Cross_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget lag_item;
    Widget *region_item;
    Widget rinvert_item;
} Cross_ui;

static Cross_ui crossui;

void create_xcor_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (crossui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	crossui.top = XmCreateDialogShell(app_shell, "X-correlation", NULL, 0);
	handle_close(crossui.top);
	XtVaSetValues(crossui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(crossui.top, "dialog_rc", NULL, 0);

	crossui.sel1 = CreateSetSelector(dialog, "Select set:",
					 SET_SELECT_ACTIVE,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_SINGLE);
	crossui.sel2 = CreateSetSelector(dialog, "Select set:",
					 SET_SELECT_ACTIVE,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_SINGLE);
	crossui.lag_item = CreateTextItem2(dialog, 10, "Maximum lag:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_xcor_proc, (XtPointer) & crossui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) crossui.top);

	XtManageChild(dialog);
    }
    XtRaise(crossui.top);
    unset_wait_cursor();
}

/*
 * cross correlation
 */
static void do_xcor_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int set1, set2, maxlag;
    Cross_ui *ui = (Cross_ui *) client_data;
    set1 = GetSelectedSet(ui->sel1);
    set2 = GetSelectedSet(ui->sel2);
    if (set1 == SET_SELECT_ERROR || set2 == SET_SELECT_ERROR) {
	errwin("Select 2 sets");
	return;
    }
    maxlag = atoi((char *) xv_getstr(ui->lag_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    do_xcor(set1, set2, maxlag);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
}

/* splines */

typedef struct _Spline_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget start_item;
    Widget stop_item;
    Widget step_item;
    Widget *region_item;
    Widget rinvert_item;
} Spline_ui;

static Spline_ui splineui;

void create_spline_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget dialog;

    set_wait_cursor();
    if (splineui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	splineui.top = XmCreateDialogShell(app_shell, "Splines", NULL, 0);
	handle_close(splineui.top);
	XtVaSetValues(splineui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(splineui.top, "dialog_rc", NULL, 0);

	splineui.sel = CreateSetSelector(dialog, "Apply to set:",
					 SET_SELECT_ALL,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_MULTIPLE);


	splineui.start_item = CreateTextItem2(dialog, 10, "Start:");
	splineui.stop_item = CreateTextItem2(dialog, 10, "Stop:");
	splineui.step_item = CreateTextItem2(dialog, 6, "Number of points:");
	splineui.type_item = CreatePanelChoice(dialog,
					  "Spline type:",
					  3,
					  "Cubic",
					  "Akima",
					  0, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_spline_proc, (XtPointer) & splineui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_SPLINE);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) splineui.top);

	XtManageChild(dialog);
    }
    XtRaise(splineui.top);
    unset_wait_cursor();
}

/*
 * splines
 */
static void do_spline_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, n;
    int stype;
    double start, stop;
    Spline_ui *ui = (Spline_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    start = atof((char *) xv_getstr(ui->start_item));
    stop = atof((char *) xv_getstr(ui->stop_item));
    n = atoi((char *) xv_getstr(ui->step_item));

    stype = GetChoice(ui->type_item);
    
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
	do_spline(setno, start, stop, n, stype+1);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();

    free(selsets);
    drawgraph();
}

/* sample a set */

typedef struct _Samp_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget start_item;
    Widget step_item;
    Widget expr_item;
    Widget *region_item;
    Widget rinvert_item;
} Samp_ui;

static Samp_ui sampui;

void create_samp_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (sampui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	sampui.top = XmCreateDialogShell(app_shell, "Sample points", NULL, 0);
	handle_close(sampui.top);
	XtVaSetValues(sampui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(sampui.top, "dialog_rc", NULL, 0);

	sampui.sel = CreateSetSelector(dialog, "Apply to set:",
				       SET_SELECT_ALL,
				       FILTER_SELECT_NONE,
				       GRAPH_SELECT_CURRENT,
				       SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 5,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	XtVaCreateManagedWidget("Sample type:", xmLabelWidgetClass, rc, NULL);
	sampui.type_item = CreatePanelChoice(rc,
					     " ",
					     3,
					     "Start/step",
					     "Expression",
					     0,
					     0);
	sampui.start_item = CreateTextItem4(rc, 10, "Start:");
	sampui.step_item = CreateTextItem4(rc, 10, "Step:");
	sampui.expr_item = CreateTextItem4(rc, 10, "Logical expression:");
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_sample_proc, (XtPointer) & sampui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_SAMPLE);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) sampui.top);

	XtManageChild(dialog);
    }
    XtRaise(sampui.top);
    unset_wait_cursor();
}

/*
 * sample a set, by start/step or logical expression
 */
static void do_sample_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, typeno;
    char exprstr[256];
    int startno, stepno;
    Samp_ui *ui = (Samp_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    typeno = (int) GetChoice(ui->type_item);
    startno = atoi((char *) xv_getstr(ui->start_item));
    stepno = atoi((char *) xv_getstr(ui->step_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
/* exprstr gets clobbered */
	strcpy(exprstr, (char *) xv_getstr(ui->expr_item));
	do_sample(setno, typeno, exprstr, startno, stepno);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* Prune data */

typedef struct _Prune_ui {
    Widget top;
    SetChoiceItem sel;
    Widget *type_item;
    Widget *dxtype_item;
    Widget *dytype_item;
    Widget *deltatype_item;
    Widget dx_rc;
    Widget dy_rc;
    Widget dx_item;
    Widget dy_item;
} Prune_ui;

static Prune_ui pruneui;

void create_prune_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y, i;
    static Widget dialog;

    set_wait_cursor();
    if (pruneui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Pick";
	label2[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	pruneui.top = XmCreateDialogShell(app_shell, "Prune data", NULL, 0);
	handle_close(pruneui.top);
	XtVaSetValues(pruneui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(pruneui.top, "dialog_rc", NULL, 0);

	pruneui.sel = CreateSetSelector(dialog, "Apply to set:",
	    SET_SELECT_ALL, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
	    SELECTION_TYPE_MULTIPLE);

	pruneui.type_item = CreatePanelChoice(dialog,
	    "Prune type: ", 5,
	    "Interpolation", "Circle", "Ellipse", "Rectangle",
	    NULL, 0);

	pruneui.dx_rc = XtVaCreateWidget("dx_rc",
            xmRowColumnWidgetClass, dialog,
            XmNorientation, XmHORIZONTAL,
	    NULL);
	pruneui.dx_item = CreateTextItem4(pruneui.dx_rc, 17, "Delta X:");
        XtManageChild(pruneui.dx_rc);

	pruneui.dy_rc = XtVaCreateWidget("dy_rc",
            xmRowColumnWidgetClass, dialog,
            XmNorientation, XmHORIZONTAL,
	    NULL);
	pruneui.dy_item = CreateTextItem4(pruneui.dy_rc, 17, "Delta Y:");
        XtManageChild(pruneui.dy_rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        pruneui.deltatype_item = CreatePanelChoice(dialog,
	    "Type of Delta coordinates:", 3, "Viewport", "World", NULL, 0);
	
	pruneui.dxtype_item = CreatePanelChoice(dialog,
            "Scaling of Delta X:", 3, "Linear", "Logarithmic", NULL, 0);
	
	pruneui.dytype_item = CreatePanelChoice(dialog,
            "Scaling of Delta Y:", 3, "Linear", "Logarithmic", NULL, 0);

        update_prune_frame();

        for (i = 0; i <= 3; i++) {
            XtAddCallback(pruneui.type_item[2 + i], XmNactivateCallback,
                (XtCallbackProc) do_prune_toggle, (XtPointer) &pruneui);
        }
	for (i = 0; i <= 1; i++) {
            XtAddCallback(pruneui.deltatype_item[2 + i], XmNactivateCallback,
                (XtCallbackProc) do_prune_toggle, (XtPointer) &pruneui);
        }
        do_prune_toggle ((Widget) NULL, (XtPointer) &pruneui, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_prune_proc, (XtPointer) & pruneui);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) do_pick_compose, (XtPointer) PICK_PRUNE);
	XtAddCallback(but2[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) pruneui.top);

	XtManageChild(dialog);
    }
    XtRaise(pruneui.top);
    unset_wait_cursor();
}

void update_prune_frame(void)
{
    if (pruneui.top != NULL) {
        SetChoice(pruneui.dxtype_item,
            (g[cg].type == GRAPH_LOGX || g[cg].type == GRAPH_LOGXY) ? 1 : 0);
        SetChoice(pruneui.dytype_item,
            (g[cg].type == GRAPH_LOGY || g[cg].type == GRAPH_LOGXY) ? 1 : 0);
    }
}

/*
 * Toggle prune type
 */
static void do_prune_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    Prune_ui *ui = (Prune_ui *) client_data;
    int typeno = (int) GetChoice(ui->type_item);
    int deltatypeno = (int) GetChoice(ui->deltatype_item);

    switch (typeno) {
        case PRUNE_CIRCLE:
	    XtSetSensitive(pruneui.dx_rc, TRUE);
	    XtSetSensitive(pruneui.dy_rc, FALSE);
	    switch (deltatypeno) {
		case PRUNE_VIEWPORT:
		    XtSetSensitive(*pruneui.dxtype_item, FALSE);
		    XtSetSensitive(*pruneui.dytype_item, FALSE);
		    break;
		case PRUNE_WORLD:
		    XtSetSensitive(*pruneui.dxtype_item, TRUE);
		    XtSetSensitive(*pruneui.dytype_item, FALSE);
		    break;
	    }
	    break;
        case PRUNE_ELLIPSE:
        case PRUNE_RECTANGLE:
	    XtSetSensitive(pruneui.dx_rc, TRUE);
	    XtSetSensitive(pruneui.dy_rc, TRUE);
	    switch (deltatypeno) {
		case PRUNE_VIEWPORT:
		    XtSetSensitive(*pruneui.dxtype_item, FALSE);
		    XtSetSensitive(*pruneui.dytype_item, FALSE);
		    break;
		case PRUNE_WORLD:
		    XtSetSensitive(*pruneui.dxtype_item, TRUE);
		    XtSetSensitive(*pruneui.dytype_item, TRUE);
		    break;
	    }
	    break;
        case PRUNE_INTERPOLATION:
	    XtSetSensitive(pruneui.dx_rc, FALSE);
	    XtSetSensitive(pruneui.dy_rc, TRUE);
	    switch (deltatypeno) {
		case PRUNE_VIEWPORT:
		    XtSetSensitive(*pruneui.dxtype_item, FALSE);
		    XtSetSensitive(*pruneui.dytype_item, FALSE);
		    break;
		case PRUNE_WORLD:
		    XtSetSensitive(*pruneui.dxtype_item, FALSE);
		    XtSetSensitive(*pruneui.dytype_item, TRUE);
		    break;
	    }
	    break;
    }
}

/*
 * Prune data
 */
static void do_prune_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int *selsets;
    int i, cnt;
    int setno, typeno, deltatypeno;
    int dxtype, dytype;
    char dxstr[256];
    char dystr[256];

    Prune_ui *ui = (Prune_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    typeno = (int) GetChoice(ui->type_item);
    deltatypeno = (int) GetChoice(ui->deltatype_item);
    dxtype = (int) GetChoice(ui->dxtype_item);
    dytype = (int) GetChoice(ui->dytype_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
	setno = selsets[i];
        /* dxstr and dystr get clobbered */
	strcpy(dxstr, (char *) xv_getstr(ui->dx_item));
	strcpy(dystr, (char *) xv_getstr(ui->dy_item));
	do_prune(setno, typeno, deltatypeno, dxstr, dystr, dxtype, dytype);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/* apply a digital filter in set 2 to set 1 */

typedef struct _Digf_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *type_item;
    Widget *region_item;
    Widget rinvert_item;
} Digf_ui;

static Digf_ui digfui;

void create_digf_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (digfui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	digfui.top = XmCreateDialogShell(app_shell, "Digital filter", NULL, 0);
	handle_close(digfui.top);
	XtVaSetValues(digfui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(digfui.top, "dialog_rc", NULL, 0);

	digfui.sel1 = CreateSetSelector(dialog, "Filter set:",
					SET_SELECT_ACTIVE,
					FILTER_SELECT_NONE,
					GRAPH_SELECT_CURRENT,
					SELECTION_TYPE_SINGLE);
	digfui.sel2 = CreateSetSelector(dialog, "With weights from set:",
					SET_SELECT_ACTIVE,
					FILTER_SELECT_NONE,
					GRAPH_SELECT_CURRENT,
					SELECTION_TYPE_SINGLE);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_digfilter_proc, (XtPointer) & digfui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) digfui.top);

	XtManageChild(dialog);
    }
    XtRaise(digfui.top);
    unset_wait_cursor();
}

/*
 * apply a digital filter
 */
static void do_digfilter_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int set1, set2;
    Digf_ui *ui = (Digf_ui *) client_data;
    set1 = GetSelectedSet(ui->sel1);
    set2 = GetSelectedSet(ui->sel2);
    if (set1 == SET_SELECT_ERROR || set2 == SET_SELECT_ERROR) {
	errwin("Select 2 sets");
	return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    do_digfilter(set1, set2);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
}

/* linear convolution */

typedef struct _Lconv_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *type_item;
    Widget lag_item;
    Widget *region_item;
    Widget rinvert_item;
} Lconv_ui;

static Lconv_ui lconvui;

void create_lconv_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (lconvui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	lconvui.top = XmCreateDialogShell(app_shell, "Linear convolution", NULL, 0);
	handle_close(lconvui.top);
	XtVaSetValues(lconvui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(lconvui.top, "dialog_rc", NULL, 0);

	lconvui.sel1 = CreateSetSelector(dialog, "Convolve set:",
					 SET_SELECT_ACTIVE,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_SINGLE);
	lconvui.sel2 = CreateSetSelector(dialog, "With set:",
					 SET_SELECT_ACTIVE,
					 FILTER_SELECT_NONE,
					 GRAPH_SELECT_CURRENT,
					 SELECTION_TYPE_SINGLE);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_linearc_proc, (XtPointer) & lconvui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) lconvui.top);

	XtManageChild(dialog);
    }
    XtRaise(lconvui.top);
    unset_wait_cursor();
}

/*
 * linear convolution
 */
static void do_linearc_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int set1, set2;
    Lconv_ui *ui = (Lconv_ui *) client_data;
    set1 = GetSelectedSet(ui->sel1);
    set2 = GetSelectedSet(ui->sel2);
    if (set1 == SET_SELECT_ERROR || set2 == SET_SELECT_ERROR) {
	errwin("Select 2 sets");
	return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    do_linearc(set1, set2);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
}

/* evaluate a formula - load the next set */

typedef struct _Leval_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget *load_item;
    Widget x_item;
    Widget y_item;
    Widget start_item;
    Widget stop_item;
    Widget npts_item;
    Widget *region_item;
    Widget rinvert_item;
} Leval_ui;

static Leval_ui levalui;

void create_leval_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (levalui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	levalui.top = XmCreateDialogShell(app_shell, "Load & evaluate", NULL, 0);
	handle_close(levalui.top);
	XtVaSetValues(levalui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(levalui.top, "dialog_rc", NULL, 0);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 6,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);
 
	levalui.x_item = CreateTextItem2(dialog, 30, "X = ");
	levalui.y_item = CreateTextItem2(dialog, 30, "Y = ");

	XtVaCreateManagedWidget("Load:", xmLabelWidgetClass, rc, NULL);
	levalui.load_item = CreatePanelChoice(rc,
					      "",
					      7,
					      "Set X",
					      "Set Y",
					      "Scratch A",
					      "Scratch B",
					      "Scratch C",
					      "Scratch D", 0,
					      0);
	levalui.start_item = CreateTextItem4(rc, 10, "Start load at:");
	levalui.stop_item = CreateTextItem4(rc, 10, "Stop load at:");
	levalui.npts_item = CreateTextItem4(rc, 10, "# of points:");
	XtManageChild(rc);
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_compute_proc2, (XtPointer) & levalui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) levalui.top);

	XtManageChild(dialog);
    }
    XtRaise(levalui.top);
    unset_wait_cursor();
}

/*
 * evaluate a formula loading the next set
 */
static void do_compute_proc2(Widget w, XtPointer client_data, XtPointer call_data)
{
    int npts, toval;
    char fstrx[256], fstry[256];
    char startstr[256], stopstr[256];
    Leval_ui *ui = (Leval_ui *) client_data;
    npts = atoi((char *) xv_getstr(ui->npts_item));
    strcpy(fstrx, (char *) xv_getstr(ui->x_item));
    strcpy(fstry, (char *) xv_getstr(ui->y_item));
    strcpy(startstr, (char *) xv_getstr(ui->start_item));
    strcpy(stopstr, (char *) xv_getstr(ui->stop_item));
    toval = (int) GetChoice(ui->load_item) + 1;
    set_wait_cursor();
    set_work_pending(TRUE);
    do_compute2(fstrx, fstry, startstr, stopstr, npts, toval);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
}


/*
 * Rotate, scale, translate
 */

typedef struct _Geom_ui {
    Widget top;
    SetChoiceItem sel;
    SetChoiceItem sel2;
    Widget *order_item;
    Widget degrees_item;
    Widget rotx_item;
    Widget roty_item;
    Widget scalex_item;
    Widget scaley_item;
    Widget transx_item;
    Widget transy_item;
    Widget *region_item;
    Widget rinvert_item;
} Geom_ui;

static Geom_ui gui;

static void do_geom_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void reset_geom_proc(Widget, XtPointer, XtPointer);

void create_geom_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    Widget dialog;
    Widget rc;

    set_wait_cursor();
    if (gui.top == NULL) {
	char *label1[3];
	label1[0] = "Accept";
	label1[1] = "Reset";
	label1[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	gui.top = XmCreateDialogShell(app_shell, "Geometric transformations", NULL, 0);
	handle_close(gui.top);
	XtVaSetValues(gui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(gui.top, "dialog_rc", NULL, 0);

	gui.sel = CreateSetSelector(dialog, "Apply to set:",
				    SET_SELECT_ALL,
				    FILTER_SELECT_NONE,
				    GRAPH_SELECT_CURRENT,
				    SELECTION_TYPE_MULTIPLE);

	rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, dialog,
			      XmNpacking, XmPACK_COLUMN,
			      XmNnumColumns, 8,
			      XmNorientation, XmHORIZONTAL,
			      XmNisAligned, True,
			      XmNadjustLast, False,
			      XmNentryAlignment, XmALIGNMENT_END,
			      NULL);

	gui.order_item = CreatePanelChoice(dialog,
					   "Apply in order:",
					   7,
					   "Rotate, translate, scale",
					   "Rotate, scale, translate",
					   "Translate, scale, rotate",
					   "Translate, rotate, scale",
					   "Scale, translate, rotate",
					   "Scale, rotate, translate",
					   0,
					   0);

	gui.degrees_item = CreateTextItem4(rc, 10, "Rotation (degrees):");
	gui.rotx_item = CreateTextItem4(rc, 10, "Rotate about X = :");
	gui.roty_item = CreateTextItem4(rc, 10, "Rotate about Y = :");
	gui.scalex_item = CreateTextItem4(rc, 10, "Scale X:");
	gui.scaley_item = CreateTextItem4(rc, 10, "Scale Y:");
	gui.transx_item = CreateTextItem4(rc, 10, "Translate X:");
	gui.transy_item = CreateTextItem4(rc, 10, "Translate Y:");
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_geom_proc, (XtPointer) & gui);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) reset_geom_proc, (XtPointer)
	& gui.top);
	XtAddCallback(but1[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) gui.top);

	XtManageChild(dialog);
	xv_setstr(gui.degrees_item, "0.0");
	xv_setstr(gui.rotx_item, "0.0");
	xv_setstr(gui.roty_item, "0.0");
	xv_setstr(gui.scalex_item, "1.0");
	xv_setstr(gui.scaley_item, "1.0");
	xv_setstr(gui.transx_item, "0.0");
	xv_setstr(gui.transy_item, "0.0");
    }
    XtRaise(gui.top);
    unset_wait_cursor();
}

/*
 * compute geom
 */
static void do_geom_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, j, k, cnt, order[3], setno, ord;
    int *selsets;
    double degrees, sx, sy, rotx, roty, tx, ty, xtmp, ytmp, *x, *y;
    char buf[256];
    Geom_ui *ui = (Geom_ui *) client_data;
    if (w == NULL) {
	cnt = 1;
	selsets = (int *) malloc(sizeof(int));
	selsets[0] = pick_set;
    } else {
	cnt = GetSelectedSets(ui->sel, &selsets);
	if (cnt == SET_SELECT_ERROR) {
	    errwin("No sets selected");
	    return;
	}
    }
    ord = (int) GetChoice(ui->order_item);
    switch (ord) {
    case 0:
	order[0] = 0;		/* rotate */
	order[1] = 1;		/* translate */
	order[2] = 2;		/* scale */
	break;
    case 1:
	order[0] = 0;
	order[1] = 2;
	order[2] = 1;
    case 2:
	order[0] = 1;
	order[1] = 2;
	order[2] = 0;
	break;
    case 3:
	order[0] = 1;
	order[1] = 0;
	order[2] = 2;
	break;
    case 4:
	order[0] = 2;
	order[1] = 1;
	order[2] = 0;
	break;
    case 5:
	order[0] = 2;
	order[1] = 0;
	order[2] = 1;
	break;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    for (k = 0; k < cnt; k++) {
	setno = selsets[k];
	if (isactive_set(cg, setno)) {
	    x = getx(cg, setno);
	    y = gety(cg, setno);
	    for (j = 0; j < 3; j++) {
		switch (order[j]) {
		case 0:
		    strcpy(buf, (char *) xv_getstr(ui->degrees_item));
		    degrees = atof(buf);
		    if (degrees == 0.0) {
			break;
		    }
		    degrees = M_PI / 180.0 * degrees;
		    strcpy(buf, (char *) xv_getstr(ui->rotx_item));
		    rotx = atof(buf);
		    strcpy(buf, (char *) xv_getstr(ui->roty_item));
		    roty = atof(buf);
		    for (i = 0; i < getsetlength(cg, setno); i++) {
			xtmp = x[i] - rotx;
			ytmp = y[i] - roty;
			x[i] = rotx + cos(degrees) * xtmp - sin(degrees) * ytmp;
			y[i] = roty + sin(degrees) * xtmp + cos(degrees) * ytmp;
		    }
		    break;
		case 1:
		    strcpy(buf, (char *) xv_getstr(ui->transx_item));
		    tx = atof(buf);
		    strcpy(buf, (char *) xv_getstr(ui->transy_item));
		    ty = atof(buf);
		    for (i = 0; i < getsetlength(cg, setno); i++) {
			x[i] += tx;
			y[i] += ty;
		    }
		    break;
		case 2:
		    strcpy(buf, (char *) xv_getstr(ui->scalex_item));
		    sx = atof(buf);
		    strcpy(buf, (char *) xv_getstr(ui->scaley_item));
		    sy = atof(buf);
		    for (i = 0; i < getsetlength(cg, setno); i++) {
			x[i] *= sx;
			y[i] *= sy;
		    }
		    break;
		}		/* end case */
	    }			/* end for j */
	    updatesetminmax(cg, setno);
	    update_set_status(cg, setno);
	}			/* end if */
    }				/* end for k */
    set_work_pending(FALSE);
    update_set_lists(cg);
    free(selsets);
    set_dirtystate();
    unset_wait_cursor();
    drawgraph();
}

static void reset_geom_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Geom_ui *tui = (Geom_ui *) client_data;
	xv_setstr(tui->degrees_item, "0.0");
	xv_setstr(tui->rotx_item, "0.0");
	xv_setstr(tui->roty_item, "0.0");
	xv_setstr(tui->scalex_item, "1.0");
	xv_setstr(tui->scaley_item, "1.0");
	xv_setstr(tui->transx_item, "0.0");
	xv_setstr(tui->transy_item, "0.0");
}



void execute_pick_compute(int gno, int setno, int function)
{
    pick_gno = gno;
    pick_set = setno;
    switch (function) {
    case PICK_EXPR:
	do_compute_proc((Widget) NULL, (XtPointer) & eui, (XtPointer) 0);
	break;
    case PICK_RUNAVG:
	do_runavg_proc((Widget) NULL, (XtPointer) & rui, (XtPointer) 0);
	break;
    case PICK_REG:
	do_regress_proc((Widget) NULL, (XtPointer) & regui, (XtPointer) 0);
	break;
    case PICK_SEASONAL:
	do_seasonal_proc((Widget) NULL, (XtPointer) & sui, (XtPointer) 0);
	break;
    case PICK_SPLINE:
	do_spline_proc((Widget) NULL, (XtPointer) & splineui, (XtPointer) 0);
	break;
    case PICK_DIFF:
	do_differ_proc((Widget) NULL, (XtPointer) & dui, (XtPointer) 0);
	break;
    case PICK_INT:
	do_int_proc((Widget) NULL, (XtPointer) & iui, (XtPointer) 0);
	break;
    case PICK_SAMPLE:
	do_sample_proc((Widget) NULL, (XtPointer) & sampui, (XtPointer) 0);
	break;
    case PICK_PRUNE:
        do_prune_proc((Widget) NULL, (XtPointer) & pruneui, (XtPointer) 0);
        break;
    case PICK_FOURIER:
	do_fourier_proc((Widget) NULL, (XtPointer) & fui, (XtPointer) 0);
	break;
    case PICK_HISTO:
	do_histo_proc((Widget) NULL, (XtPointer) & hui, (XtPointer) 0);
	break;
    }
}
