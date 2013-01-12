/*
 * define regions and operate on regions
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"

static Widget but1[2];
static Widget but2[2];
static Widget but3[3];

extern int regiontype, regionlinkto;	/* in xmgr.c */

static void do_eval_region(Widget w, XtPointer client_data, XtPointer call_data);
static void do_define_region(Widget w, XtPointer client_data, XtPointer call_data);
static void do_clear_region(Widget w, XtPointer client_data, XtPointer call_data);
static void do_extract_region(Widget w, XtPointer client_data, XtPointer call_data);
static void do_delete_region(Widget w, XtPointer client_data, XtPointer call_data);
static void do_extractsets_region(Widget w, XtPointer client_data, XtPointer call_data);
static void do_deletesets_region(Widget w, XtPointer client_data, XtPointer call_data);


static Widget *define_region_item;
static Widget *define_type_item;
static Widget *define_linkto_item;

static void do_define_region(Widget w, XtPointer client_data, XtPointer call_data)
{
    int rtype = GetChoice(define_type_item);

    nr = GetChoice(define_region_item);
    regionlinkto = GetChoice(define_linkto_item);
    define_region(nr, regionlinkto, rtype);
}

void create_define_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget top, dialog;
    int x, y;

    set_wait_cursor();
    if (top == NULL) {
	char *label2[2];
	label2[0] = "Define";
	label2[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Define region", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	define_region_item = CreatePanelChoice(dialog,
					       "Define region:",
					       6,
					       "0", "1", "2", "3", "4",
					       NULL, 0);

	define_type_item = CreatePanelChoice(dialog,
					     "Region type:",
					     7,
					     "Inside polygon",
					     "Outside polygon",
					     "Above line",
					     "Below line",
					     "Left of line",
					     "Right of line",
					     NULL, 0);

	define_linkto_item = CreatePanelChoice(dialog,
					       "Link to graph(s):",
					       3,
					       "Current", "All",
					       NULL, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc) do_define_region, (XtPointer) NULL);
	XtAddCallback(but2[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

static Widget *clear_region_item;

static void do_clear_region(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i;

    set_wait_cursor();
    if (GetChoice(clear_region_item) == MAXREGION) {
	for (i = 0; i < MAXREGION; i++) {
	    kill_region(i);
	}
    } else {
	kill_region(GetChoice(clear_region_item));
    }
    unset_wait_cursor();
    drawgraph();
}

void create_clear_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Clear region", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	clear_region_item = CreatePanelChoice(dialog,
					      "Clear region:",
					      7,
					      "0", "1", "2", "3", "4", "All",
					      NULL, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_clear_region, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

static Widget *extract_region_item;
static SetChoiceItem extract_fromset_item;
static SetChoiceItem extract_set_item;
static Widget *extract_graph_item;

static void do_extract_region(Widget w, XtPointer client_data, XtPointer call_data)
{
    int gno, fromset, setno, regno;

    gno = GetChoice(extract_graph_item) - 1;
    if (gno == -1) {
	gno = cg;
    }
    fromset = GetSelectedSet(extract_fromset_item);
    if (fromset == SET_SELECT_ERROR) {
        errwin("No source set selected");
        return;
    }
    if (fromset == SET_SELECT_ALL) {
	fromset = -1;
    }
    setno = GetSelectedSet(extract_set_item);
    if (setno == SET_SELECT_ERROR) {
        errwin("No destination set selected");
        return;
    }
    if (setno == SET_SELECT_NEXT) {
	setno = nextset(gno);
	if (setno == -1) {
	    errwin("No more sets in this graph");
	    return;
	}
    }
    regno = (int) GetChoice(extract_region_item);
    set_wait_cursor();
    extract_region(gno, fromset, setno, regno);
    unset_wait_cursor();
}

void create_extract_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Extract region", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	extract_fromset_item = CreateSetSelector(dialog, "Extract points from set:",
                SET_SELECT_ALL,
                FILTER_SELECT_NONE,
                GRAPH_SELECT_CURRENT,
                SELECTION_TYPE_SINGLE);

	extract_region_item = CreatePanelChoice(dialog,
						"found in region:",
						8,
		   "0", "1", "2", "3", "4", "Inside world", "Outside world",
						NULL, 0);

	extract_set_item = CreateSetSelector(dialog, "Load points in region to set:",
	                SET_SELECT_NEXT,
                FILTER_SELECT_NONE,
                GRAPH_SELECT_CURRENT,
                SELECTION_TYPE_SINGLE);

	extract_graph_item = CreatePanelChoice(dialog,
					       "In graph:",
					       12,
				    "Current", "0", "1", "2", "3", "4", "5",
					       "6", "7", "8", "9", NULL, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_extract_region, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

static Widget *delete_region_item;
static Widget *delete_graph_item;
static SetChoiceItem delete_set_item;

static void do_delete_region(Widget w, XtPointer client_data, XtPointer call_data)
{
    int gno, regno, setno;

    gno = GetChoice(delete_graph_item) - 1;
    setno = GetSelectedSet(delete_set_item);
    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    if (setno == SET_SELECT_ALL) {
	setno = -1;
    }
    regno = GetChoice(delete_region_item);

    set_wait_cursor();
    delete_region(gno, setno, regno);
    unset_wait_cursor();
}

void create_delete_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Delete points in region", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	delete_set_item = CreateSetSelector(dialog, "Delete points from set:",
                SET_SELECT_ALL,
                FILTER_SELECT_NONE,
                GRAPH_SELECT_CURRENT,
                SELECTION_TYPE_SINGLE);
	delete_region_item = CreatePanelChoice(dialog,
				 "Found in region:",
					       8,
		   "0", "1", "2", "3", "4", "Inside world", "Outside world",
					       NULL, 0);

	delete_graph_item = CreatePanelChoice(dialog,
					      "In graph:",
					      13,
				    "Current", "0", "1", "2", "3", "4", "5",
					"6", "7", "8", "9", "All", NULL, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_delete_region, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

XmString astring, pstring;
Widget arealab, perimlab;
extern XmStringCharSet charset;

void create_area_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label3[3];
	label3[0] = "Area";
	label3[1] = "Perimeter";
	label3[2] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Area/perimeter", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	arealab = XtVaCreateManagedWidget("label Area", xmLabelWidgetClass, dialog,
					  XmNlabelString, astring = XmStringCreateLtoR("[    Area    ]", charset),
					  NULL);

	perimlab = XtVaCreateManagedWidget("label Perim", xmLabelWidgetClass, dialog,
					   XmNlabelString, pstring = XmStringCreateLtoR("[    Perim    ]", charset),
					   NULL);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 3, but3, label3);
	XtAddCallback(but3[0], XmNactivateCallback, (XtCallbackProc) do_select_area, (XtPointer) NULL);
	XtAddCallback(but3[1], XmNactivateCallback, (XtCallbackProc) do_select_peri, (XtPointer) NULL);
	XtAddCallback(but3[2], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

static Widget *eval_region;
static SetChoiceItem eval_set_item;
static Widget eval_region_item;

static void do_eval_region(Widget w, XtPointer client_data, XtPointer call_data)
{
    int regno, setno, gno = GRAPH_SELECT_CURRENT;
    char buf[256];

    regno = (int) GetChoice(eval_region);
    setno = GetSelectedSet(eval_set_item);
    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    strcpy(buf, (char *) xv_getstr(eval_region_item));
    set_wait_cursor();
/* TODO add gno to parm list */
    evaluate_region(regno, gno, setno, buf);
    drawgraph();
    unset_wait_cursor();
}

void create_evalregion_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Evaluate region", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	eval_set_item = CreateSetSelector(dialog, "Apply expression to all points in set:",
	        SET_SELECT_ALL,
                FILTER_SELECT_NONE,
                GRAPH_SELECT_CURRENT,
                SELECTION_TYPE_SINGLE);

	eval_region = CreatePanelChoice(dialog,
					"found in region:",
					8,
		   "0", "1", "2", "3", "4", "Inside world", "Outside world",
					NULL, 0);

	eval_region_item = CreateTextItem2(dialog, 20, "Expression:");

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_eval_region, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

static Widget *extractsets_region_item;
static Widget *extractsets_graph_item;

static void do_extractsets_region(Widget w, XtPointer client_data, XtPointer call_data)
{
    int gno, regno;

    gno = (int) GetChoice(extractsets_graph_item);
    regno = (int) GetChoice(extractsets_region_item);
    set_wait_cursor();
    extractsets_region(cg, gno, regno);
    unset_wait_cursor();
    drawgraph();
}

void create_extractsets_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Extract sets in region", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	extractsets_region_item = CreatePanelChoice(dialog,
						  "Found in region:",
						    8,
		   "0", "1", "2", "3", "4", "Inside world", "Outside world",
						    NULL, 0);

	extractsets_graph_item = CreateGraphChoice(dialog, "Load to graph:", maxgraph, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_extractsets_region, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

static Widget *deletesets_region_item;

static void do_deletesets_region(Widget w, XtPointer client_data, XtPointer call_data)
{
    int regno = (int) GetChoice(deletesets_region_item);
    set_wait_cursor();
    deletesets_region(cg, regno);
    unset_wait_cursor();
    drawgraph();
}

void create_deletesets_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Delete sets in region", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	deletesets_region_item = CreatePanelChoice(dialog,
						   "Delete sets in region:",
						   8,
		   "0", "1", "2", "3", "4", "Inside world", "Outside world",
						   NULL, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_deletesets_region, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}

static Widget *reporton_region_item;
static Widget *reporton_type_item;
static Widget *reporton_strict_item;

static void do_reporton_region(Widget w, XtPointer client_data, XtPointer call_data)
{
    int regno = (int) GetChoice(reporton_region_item);
    int type = (int) GetChoice(reporton_type_item);
    int strict = (int) GetChoice(reporton_strict_item);
    set_wait_cursor();
    reporton_region(cg, regno, type, strict);
    unset_wait_cursor();
    drawgraph();
}

void create_reporton_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y;
    static Widget top, dialog;

    set_wait_cursor();
    if (top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	top = XmCreateDialogShell(app_shell, "Report on sets in region", NULL, 0);
	handle_close(top);
	XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

	reporton_region_item = CreatePanelChoice(dialog,
						   "Report on sets in region:",
						   8,
		   "0", "1", "2", "3", "4", "Inside world", "Outside world",
						   NULL, 0);

	reporton_type_item = CreatePanelChoice(dialog,
						   "Report type:",
						   3,
		   				"Sets", "Points",
						   NULL, 0);

	reporton_strict_item = CreatePanelChoice(dialog,
						   "Strict containment for sets:",
						   3,
		   				"No", "Yes",
						   NULL, 0);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but1, label1);
	XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc) do_reporton_region, (XtPointer) NULL);
	XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) top);

	XtManageChild(dialog);
    }
    XtRaise(top);
    unset_wait_cursor();
}
