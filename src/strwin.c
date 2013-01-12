/* $Id: strwin.c,v 1.3 1995/05/25 04:03:57 pturner Exp pturner $
 *
 * strings, lines, and boxes
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>

#include "globals.h"
#include "draw.h"
#include "protos.h"
#include "motifinc.h"

static Widget objects_frame;
static Widget strings_frame;
static Widget lines_frame;
static Widget boxes_frame;
static Widget ellip_frame;

static Widget *strings_font_item;
static Widget strings_rot_item;
static Widget strings_size_item;
static Widget *strings_loc_item;
static Widget *strings_pen_item;
static Widget *strings_just_item;
Widget strings_x_item;
Widget strings_y_item;

static Widget *lines_arrow_item;
static Widget lines_asize_item;
static Widget *lines_atype_item;
static Widget *lines_pen_item;
static Widget *lines_style_item;
static Widget *lines_width_item;
static Widget *lines_loc_item;
static Widget *boxes_pen_item;
static Widget *boxes_lines_item;
static Widget *boxes_linew_item;
static Widget *boxes_fill_item;
static Widget *boxes_fillpat_item;
static Widget *boxes_fillcol_item;
static Widget *boxes_loc_item;

static Widget *ellip_pen_item;
static Widget *ellip_lines_item;
static Widget *ellip_linew_item;
static Widget *ellip_fill_item;
static Widget *ellip_fillpat_item;
static Widget *ellip_fillcol_item;
static Widget *ellip_loc_item;
void define_ellip_popup(Widget w, XtPointer client_data, XtPointer call_data);

void ellip_def_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    defellip.color = (int) GetChoice(ellip_pen_item);
    defellip.loctype = (int) GetChoice(ellip_loc_item) ? COORD_VIEW : COORD_WORLD;
    defellip.lines = (int) GetChoice(ellip_lines_item) + 1;
    defellip.linew = (int) GetChoice(ellip_linew_item) + 1;
    switch (GetChoice(ellip_fill_item)) {
    case 0:
	defellip.fill = UNFILLED;
	break;
    case 1:
	defellip.fill = CLRFILLED;
	break;
    case 2:
	defellip.fill = PTNFILLED;
	break;
    }
    defellip.fillcolor = (int) GetChoice(ellip_fillcol_item);
    defellip.fillpattern = (int) GetChoice(ellip_fillpat_item);
	defellip.active = TRUE;
}

void update_ellip(void)
{
    if (ellip_frame) {
	SetChoice(ellip_pen_item, defellip.color);
	SetChoice(ellip_lines_item, defellip.lines - 1);
	SetChoice(ellip_linew_item, defellip.linew - 1);
	switch (defellip.fill) {
	case UNFILLED:
	    SetChoice(ellip_fill_item, 0);
	    break;
	case CLRFILLED:
	    SetChoice(ellip_fill_item, 1);
	    break;
	case PTNFILLED:
	    SetChoice(ellip_fill_item, 2);
	    break;
	}
	SetChoice(ellip_fillpat_item, defellip.fillpattern);
	SetChoice(ellip_fillcol_item, defellip.fillcolor);
	SetChoice(ellip_loc_item, defellip.loctype == COORD_VIEW ? 1 : 0);
    }
}

void boxes_def_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    box_color = (int) GetChoice(boxes_pen_item);
    box_loctype = (int) GetChoice(boxes_loc_item) ? COORD_VIEW : COORD_WORLD;
    box_lines = (int) GetChoice(boxes_lines_item) + 1;
    box_linew = (int) GetChoice(boxes_linew_item) + 1;
    switch (GetChoice(boxes_fill_item)) {
    case 0:
	box_fill = UNFILLED;
	break;
    case 1:
	box_fill = CLRFILLED;
	break;
    case 2:
	box_fill = PTNFILLED;
	break;
    }
    box_fillcolor = (int) GetChoice(boxes_fillcol_item);
    box_fillpat = (int) GetChoice(boxes_fillpat_item);
}

void lines_def_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg a;
    int value;
    XtSetArg(a, XmNvalue, &value);
    XtGetValues(lines_asize_item, &a, 1);
    line_color = (int) GetChoice(lines_pen_item);
    line_asize = value / 50.0;
    line_arrow = (int) GetChoice(lines_arrow_item);
    line_atype = (int) GetChoice(lines_atype_item);
    line_lines = (int) GetChoice(lines_style_item) + 1;
    line_linew = (int) GetChoice(lines_width_item) + 1;
    line_loctype = (int) GetChoice(lines_loc_item) ? COORD_VIEW : COORD_WORLD;
}

void updatestrings(void)
{
    Arg a;
    int iv;
    if (strings_frame) {
	SetChoice(strings_font_item, string_font);
	SetChoice(strings_pen_item, string_color);
	iv = rint(100 * string_size);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(strings_size_item, &a, 1);
	XtSetArg(a, XmNvalue, string_rot);
	XtSetValues(strings_rot_item, &a, 1);
	SetChoice(strings_loc_item, string_loctype == COORD_VIEW ? 1 : 0);
	SetChoice(strings_just_item, string_just);
    }
}

void update_lines(void)
{
    Arg a;
    int iv;
    if (lines_frame) {
	SetChoice(lines_pen_item, line_color);
	SetChoice(lines_style_item, line_lines - 1);
	SetChoice(lines_width_item, line_linew - 1);
	SetChoice(lines_arrow_item, line_arrow);
	SetChoice(lines_atype_item, line_atype);
	iv = (int) (50 * line_asize);
	XtSetArg(a, XmNvalue, iv);
	XtSetValues(lines_asize_item, &a, 1);
	SetChoice(lines_loc_item, line_loctype == COORD_VIEW ? 1 : 0);
    }
}

void update_boxes(void)
{
    if (boxes_frame) {
	SetChoice(boxes_pen_item, box_color);
	SetChoice(boxes_lines_item, box_lines - 1);
	SetChoice(boxes_linew_item, box_linew - 1);
	switch (box_fill) {
	case UNFILLED:
	    SetChoice(boxes_fill_item, 0);
	    break;
	case CLRFILLED:
	    SetChoice(boxes_fill_item, 1);
	    break;
	case PTNFILLED:
	    SetChoice(boxes_fill_item, 2);
	    break;
	}
	SetChoice(boxes_fillpat_item, box_fillpat);
	SetChoice(boxes_fillcol_item, box_fillcolor);
	SetChoice(boxes_loc_item, box_loctype == COORD_VIEW ? 1 : 0);
    }
}

void define_string_defaults(Widget w, XtPointer client_data, XtPointer call_data)
{
    Arg a;
    int value;

    if (strings_frame) {
	string_font = (int) GetChoice(strings_font_item);
	string_color = (int) GetChoice(strings_pen_item);
	XtSetArg(a, XmNvalue, &value);
	XtGetValues(strings_size_item, &a, 1);
	string_size = value / 100.0;
	XtSetArg(a, XmNvalue, &value);
	XtGetValues(strings_rot_item, &a, 1);
	string_rot = value;
	string_loctype = (int) GetChoice(strings_loc_item) ? COORD_VIEW : COORD_WORLD;
	string_just = (int) GetChoice(strings_just_item);
    }
}

void define_objects_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget wbut;
    Widget panel, rc;
    int x, y;
    set_wait_cursor();
    if (objects_frame == NULL) {
	XmGetPos(app_shell, 0, &x, &y);
	objects_frame = XmCreateDialogShell(app_shell, "Objects", NULL, 0);
	handle_close(objects_frame);
	XtVaSetValues(objects_frame, XmNx, x, XmNy, y, NULL);
	panel = XmCreateRowColumn(objects_frame, "ticks_rc", NULL, 0);
        XtVaSetValues(panel, XmNorientation, XmHORIZONTAL, NULL);

        rc = XmCreateRowColumn(panel, "rc", NULL, 0);

	wbut = XtVaCreateManagedWidget("Text", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) STR_LOC);

	wbut = XtVaCreateManagedWidget("Text at angle", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) STR_LOC1ST);

	wbut = XtVaCreateManagedWidget("Edit Text", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) STR_EDIT);

	wbut = XtVaCreateManagedWidget("Text props...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) define_strings_popup, (XtPointer) NULL);

	wbut = XtVaCreateManagedWidget("Line", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) MAKE_LINE_1ST);

	wbut = XtVaCreateManagedWidget("Line props...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) define_lines_popup, (XtPointer) NULL);

	wbut = XtVaCreateManagedWidget("Box", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) MAKE_BOX_1ST);

	wbut = XtVaCreateManagedWidget("Box props...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) define_boxes_popup, (XtPointer) NULL);

	wbut = XtVaCreateManagedWidget("Ellipse", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) MAKE_ELLIP_1ST);

	wbut = XtVaCreateManagedWidget("Ellipse props...", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) define_ellip_popup, (XtPointer) NULL);
	XtManageChild(rc);

        rc = XmCreateRowColumn(panel, "rc", NULL, 0);

	wbut = XtVaCreateManagedWidget("Edit object", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) EDIT_OBJECT);

	wbut = XtVaCreateManagedWidget("Move object", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) MOVE_OBJECT_1ST);

	wbut = XtVaCreateManagedWidget("Copy object", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) COPY_OBJECT1ST);

	wbut = XtVaCreateManagedWidget("Delete object", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) DEL_OBJECT);

	wbut = XtVaCreateManagedWidget("Clear all text", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_clear_text, (XtPointer) NULL);

	wbut = XtVaCreateManagedWidget("Clear all lines", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_clear_lines, (XtPointer) NULL);

	wbut = XtVaCreateManagedWidget("Clear all boxes", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_clear_boxes, (XtPointer) NULL);

	wbut = XtVaCreateManagedWidget("Clear all ellip", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) do_clear_ellipses, (XtPointer) NULL);

	wbut = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, rc,
				       NULL);
	XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc) destroy_dialog, (XtPointer) objects_frame);
	XtManageChild(rc);
	XtManageChild(panel);
    }
    XtRaise(objects_frame);
    unset_wait_cursor();
}

void define_ellip_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget rc;
    int x, y;
    Widget buts[2];
    Widget panel;

    set_wait_cursor();
    if (ellip_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	ellip_frame = XmCreateDialogShell(app_shell, "Ellipses", NULL, 0);
	handle_close(ellip_frame);
	XtVaSetValues(ellip_frame, XmNx, x, XmNy, y, NULL);
	panel = XmCreateRowColumn(ellip_frame, "ellip_rc", NULL, 0);

        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
                              XmNpacking, XmPACK_COLUMN,
                              XmNnumColumns, 7,
                              XmNorientation, XmHORIZONTAL,
                              XmNisAligned, True,
                              XmNadjustLast, False,
                              XmNentryAlignment, XmALIGNMENT_END,
                              NULL);

	XtVaCreateManagedWidget("Color:", xmLabelWidgetClass, rc, NULL);
	ellip_pen_item = CreateColorChoice(rc, " ", 0);

	XtVaCreateManagedWidget("Line width:", xmLabelWidgetClass, rc, NULL);
	ellip_linew_item = CreatePanelChoice(rc, " ",
					     10,
			     "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
					     NULL,
					     0);

	XtVaCreateManagedWidget("Line style:", xmLabelWidgetClass, rc, NULL);
	ellip_lines_item = (Widget *) CreatePanelChoice(rc, " ",
							6,
							"Solid line",
							"Dotted line",
							"Dashed line",
							"Long Dashed",
							"Dot-dashed",
							NULL,
							NULL);

	XtVaCreateManagedWidget("Fill:", xmLabelWidgetClass, rc, NULL);
	ellip_fill_item = CreatePanelChoice(rc, " ",
					    4,
					    "None",
					    "Color",
					    "Pattern",
					    NULL,
					    0);

	XtVaCreateManagedWidget("Fill pattern:", xmLabelWidgetClass, rc, NULL);
	ellip_fillpat_item = CreatePanelChoice0(rc,
						"Pattern:", 4, 17,
			   "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
				  "10", "11", "12", "13", "14", "15", 0, 0);
	XtVaCreateManagedWidget("Fill color:", xmLabelWidgetClass, rc, NULL);
	ellip_fillcol_item = CreateColorChoice(rc, " ", 0);
	XtVaCreateManagedWidget("Position in:", xmLabelWidgetClass, rc, NULL);
	ellip_loc_item = CreatePanelChoice(rc, " ",
					   3,
					   "World coordinates",
					   "Viewport coordinates",
					   0,
					   0);
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		      (XtCallbackProc) ellip_def_proc, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		  (XtCallbackProc) destroy_dialog, (XtPointer) ellip_frame);

	XtManageChild(panel);
    }
    XtRaise(ellip_frame);
    update_ellip();
    unset_wait_cursor();
}

void define_strings_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget rc;
    Widget wlabel;
    int x, y;
    Widget buts[2];
    Widget panel;

    set_wait_cursor();
    if (strings_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	strings_frame = XmCreateDialogShell(app_shell, "Strings", NULL, 0);
	handle_close(strings_frame);
	XtVaSetValues(strings_frame, XmNx, x, XmNy, y, NULL);
	panel = XmCreateRowColumn(strings_frame, "strings_rc", NULL, 0);

        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
                              XmNpacking, XmPACK_COLUMN,
                              XmNnumColumns, 4,
                              XmNorientation, XmHORIZONTAL,
                              XmNisAligned, True,
                              XmNadjustLast, False,
                              XmNentryAlignment, XmALIGNMENT_END,
                              NULL);

        XtVaCreateManagedWidget("Font:", xmLabelWidgetClass, rc, NULL);
	strings_font_item = CreatePanelChoice(rc, " ",
					      11,
				"Times-Roman", "Times-Bold", "Times-Italic",
					    "Times-BoldItalic", "Helvetica",
				      "Helvetica-Bold", "Helvetica-Oblique",
				 "Helvetica-BoldOblique", "Greek", "Symbol",
					      0,
					      0);

        XtVaCreateManagedWidget("Color:", xmLabelWidgetClass, rc, NULL);
	strings_pen_item = CreateColorChoice(rc, " ", 0);

        XtVaCreateManagedWidget("Justification:", xmLabelWidgetClass, rc, NULL);
	strings_just_item = CreatePanelChoice(rc, " ",
					      4,
					      "Left",
					      "Right",
					      "Centered",
					      0,
					      0);

        XtVaCreateManagedWidget("Position in:", xmLabelWidgetClass, rc, NULL);
	strings_loc_item = CreatePanelChoice(rc, " ",
					     3,
					     "World coordinates",
					     "Viewport coordinates",
					     0,
					     0);
	XtManageChild(rc);

	wlabel = XtVaCreateManagedWidget("Rotation:", xmLabelWidgetClass, panel, NULL);
	strings_rot_item = XtVaCreateManagedWidget("rotation", xmScaleWidgetClass, panel,
						   XmNminimum, 0,
						   XmNmaximum, 360,
						   XmNvalue, 0,
						   XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						   NULL);

	wlabel = XtVaCreateManagedWidget("Size:", xmLabelWidgetClass, panel, NULL);
	strings_size_item = XtVaCreateManagedWidget("stringsize", xmScaleWidgetClass, panel,
						    XmNminimum, 0,
						    XmNmaximum, 400,
						    XmNvalue, 100,
						    XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						    NULL);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		    (XtCallbackProc) define_string_defaults, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		(XtCallbackProc) destroy_dialog, (XtPointer) strings_frame);

	XtManageChild(panel);
    }
    XtRaise(strings_frame);
    updatestrings();
    unset_wait_cursor();
}

void define_lines_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget rc;
    int x, y;
    Widget buts[2];
    Widget panel;

    set_wait_cursor();
    if (lines_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	lines_frame = XmCreateDialogShell(app_shell, "Lines", NULL, 0);
	handle_close(lines_frame);
	XtVaSetValues(lines_frame, XmNx, x, XmNy, y, NULL);
	panel = XmCreateRowColumn(lines_frame, "lines_rc", NULL, 0);

        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
                              XmNpacking, XmPACK_COLUMN,
                              XmNnumColumns, 7,
                              XmNorientation, XmHORIZONTAL,
                              XmNisAligned, True,
                              XmNadjustLast, False,
                              XmNentryAlignment, XmALIGNMENT_END,
                              NULL);

	XtVaCreateManagedWidget("Color:", xmLabelWidgetClass, rc, NULL);
	lines_pen_item = CreateColorChoice(rc, " ", 0);

	XtVaCreateManagedWidget("Width:", xmLabelWidgetClass, rc, NULL);
	lines_width_item = (Widget *) CreatePanelChoice(rc, " ",
							10,
				"1", "2", "3", "4", "5", "6", "7", "8", "9",
							NULL,
							NULL);

	XtVaCreateManagedWidget("Style:", xmLabelWidgetClass, rc, NULL);
	lines_style_item = (Widget *) CreatePanelChoice(rc, " ",
							6,
							"Solid line",
							"Dotted line",
							"Dashed line",
							"Long Dashed",
							"Dot-dashed",
							NULL,
							NULL);

	XtVaCreateManagedWidget("Arrow:", xmLabelWidgetClass, rc, NULL);
	lines_arrow_item = CreatePanelChoice(rc, " ",
					     5,
					     "None",
					     "At start",
					     "At end",
					     "Both ends",
					     0,
					     0);

	XtVaCreateManagedWidget("Arrow type:", xmLabelWidgetClass, rc, NULL);
	lines_atype_item = CreatePanelChoice(rc, " ",
					     4,
					     "Line",
					     "Filled",
					     "Hollow",
					     0,
					     0);

	XtVaCreateManagedWidget("Arrow head size:", xmLabelWidgetClass, rc, NULL);
	lines_asize_item = XtVaCreateManagedWidget("arrowsize", xmScaleWidgetClass, rc,
						   XmNminimum, 0,
						   XmNmaximum, 400,
						   XmNvalue, 100,
						   XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						   NULL);

	XtVaCreateManagedWidget("Position in:", xmLabelWidgetClass, rc, NULL);
	lines_loc_item = CreatePanelChoice(rc, " ",
					   3,
					   "World coordinates",
					   "Viewport coordinates",
					   0,
					   0);
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		      (XtCallbackProc) lines_def_proc, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		  (XtCallbackProc) destroy_dialog, (XtPointer) lines_frame);

	XtManageChild(panel);
    }
    update_lines();
    XtRaise(lines_frame);
    unset_wait_cursor();
}

void define_boxes_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    Widget rc;
    int x, y;
    Widget buts[2];
    Widget panel;

    set_wait_cursor();
    if (boxes_frame == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	boxes_frame = XmCreateDialogShell(app_shell, "Boxes", NULL, 0);
	handle_close(boxes_frame);
	XtVaSetValues(boxes_frame, XmNx, x, XmNy, y, NULL);
	panel = XmCreateRowColumn(boxes_frame, "boxes_rc", NULL, 0);

        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
                              XmNpacking, XmPACK_COLUMN,
                              XmNnumColumns, 7,
                              XmNorientation, XmHORIZONTAL,
                              XmNisAligned, True,
                              XmNadjustLast, False,
                              XmNentryAlignment, XmALIGNMENT_END,
                              NULL);

	XtVaCreateManagedWidget("Color:", xmLabelWidgetClass, rc, NULL);
	boxes_pen_item = CreateColorChoice(rc, " ", 0);

	XtVaCreateManagedWidget("Line width:", xmLabelWidgetClass, rc, NULL);
	boxes_linew_item = CreatePanelChoice(rc, " ",
					     10,
			     "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
					     NULL,
					     0);

	XtVaCreateManagedWidget("Line style:", xmLabelWidgetClass, rc, NULL);
	boxes_lines_item = (Widget *) CreatePanelChoice(rc, " ",
							6,
							"Solid line",
							"Dotted line",
							"Dashed line",
							"Long Dashed",
							"Dot-dashed",
							NULL,
							NULL);

	XtVaCreateManagedWidget("Fill:", xmLabelWidgetClass, rc, NULL);
	boxes_fill_item = CreatePanelChoice(rc, " ",
					    4,
					    "None",
					    "Color",
					    "Pattern",
					    NULL,
					    0);

	XtVaCreateManagedWidget("Fill pattern:", xmLabelWidgetClass, rc, NULL);
	boxes_fillpat_item = CreatePanelChoice0(rc,
						"Pattern:", 4, 17,
			   "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
				  "10", "11", "12", "13", "14", "15", 0, 0);
	XtVaCreateManagedWidget("Fill color:", xmLabelWidgetClass, rc, NULL);
	boxes_fillcol_item = CreateColorChoice(rc, " ", 0);
	XtVaCreateManagedWidget("Position in:", xmLabelWidgetClass, rc, NULL);
	boxes_loc_item = CreatePanelChoice(rc, " ",
					   3,
					   "World coordinates",
					   "Viewport coordinates",
					   0,
					   0);
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		      (XtCallbackProc) boxes_def_proc, (XtPointer) 0);
	XtAddCallback(buts[1], XmNactivateCallback,
		  (XtCallbackProc) destroy_dialog, (XtPointer) boxes_frame);

	XtManageChild(panel);
    }
    XtRaise(boxes_frame);
    update_boxes();
    unset_wait_cursor();
}

typedef struct {
    Widget top;
    Widget *color_item;
    Widget *linew_item;
    Widget *lines_item;
    Widget *fill_item;
    Widget *fill_color_item;
    Widget *fill_pattern_item;
    Widget *loc_item;
    Widget x1_item;
    Widget x2_item;
    Widget y1_item;
    Widget y2_item;
    int boxno;
} EditBoxUI;

static EditBoxUI box_ui,ellip_ui;

void update_box_edit(EditBoxUI *ui)
{
    if (ui->top) {
	int boxno = ui->boxno;
	SetChoice(ui->color_item, boxes[boxno].color);
	SetChoice(ui->lines_item, boxes[boxno].lines - 1);
	SetChoice(ui->linew_item, boxes[boxno].linew - 1);
	switch (boxes[boxno].fill) {
	case UNFILLED:
	    SetChoice(ui->fill_item, 0);
	    break;
	case CLRFILLED:
	    SetChoice(ui->fill_item, 1);
	    break;
	case PTNFILLED:
	    SetChoice(ui->fill_item, 2);
	    break;
	}
	SetChoice(ui->fill_pattern_item, boxes[boxno].fillpattern);
	SetChoice(ui->fill_color_item, boxes[boxno].fillcolor);
	SetChoice(ui->loc_item, boxes[boxno].loctype == COORD_VIEW ? 1 : 0);
	sprintf(buf, "%.12f", boxes[boxno].x1);
	xv_setstr(ui->x1_item, buf);
	sprintf(buf, "%.12f", boxes[boxno].x2);
	xv_setstr(ui->x2_item, buf);
	sprintf(buf, "%.12f", boxes[boxno].y1);
	xv_setstr(ui->y1_item, buf);
	sprintf(buf, "%.12f", boxes[boxno].y2);
	xv_setstr(ui->y2_item, buf);
    }
}

void update_ellipse_edit(EditBoxUI *ui)
{
    if (ui->top) {
	int ellipno = ui->boxno;
	SetChoice(ui->color_item, ellip[ellipno].color);
	SetChoice(ui->lines_item, ellip[ellipno].lines - 1);
	SetChoice(ui->linew_item, ellip[ellipno].linew - 1);
	switch (ellip[ellipno].fill) {
	case UNFILLED:
	    SetChoice(ui->fill_item, 0);
	    break;
	case CLRFILLED:
	    SetChoice(ui->fill_item, 1);
	    break;
	case PTNFILLED:
	    SetChoice(ui->fill_item, 2);
	    break;
	}
	SetChoice(ui->fill_pattern_item, ellip[ellipno].fillpattern);
	SetChoice(ui->fill_color_item, ellip[ellipno].fillcolor);
	SetChoice(ui->loc_item, ellip[ellipno].loctype == COORD_VIEW ? 1 : 0);
	sprintf(buf, "%.12f", 0.5*(ellip[ellipno].x1+ellip[ellipno].x2));
	xv_setstr(ui->x1_item, buf);
	sprintf(buf, "%.12f", 0.5*(ellip[ellipno].y1+ellip[ellipno].y2));
	xv_setstr(ui->y1_item, buf);
	sprintf(buf, "%.12f", fabs(ellip[ellipno].x1-ellip[ellipno].x2) );
	xv_setstr(ui->x2_item, buf);
	sprintf(buf, "%.12f", fabs(ellip[ellipno].y1-ellip[ellipno].y2) );
	xv_setstr(ui->y2_item, buf);
    }
}


void swap_ellipwv_coords(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditBoxUI *ui = (EditBoxUI *)client_data;
    int ellipno = ui->boxno;
    double x1, x2, y1, y2;
	
    if(((int)GetChoice(ui->loc_item)?COORD_VIEW : COORD_WORLD) == ellip[ellipno].loctype) {
        return;
    }
               

    x1 = atof(xv_getstr(ui->x1_item));
    x2 = atof(xv_getstr(ui->x2_item));
    y1 = atof(xv_getstr(ui->y1_item));
    y2 = atof(xv_getstr(ui->y2_item));
	
    if( ellip[ellipno].loctype == COORD_VIEW ) {
    ellip[ellipno].gno = cg;
	ellip[ellipno].loctype = COORD_WORLD;
	view2world( x1-x2/2., y1-y2/2., &ellip[ellipno].x1,&ellip[ellipno].y1 );
	view2world( x1+x2/2., y1+y2/2., &ellip[ellipno].x2,&ellip[ellipno].y2 );
    } else {
	ellip[ellipno].loctype = COORD_VIEW;
	world2view( x1-x2/2., y1-y2/2., &ellip[ellipno].x1,&ellip[ellipno].y1 );
	world2view( x1+x2/2., y1+y2/2., &ellip[ellipno].x2,&ellip[ellipno].y2 );
    }
    update_ellipse_edit( ui );
}


void ellipse_edit_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditBoxUI *ui = (EditBoxUI *) client_data;
    int ellipno = ui->boxno;
 	set_write_mode(0);
	draw_ellipse(-2, ellipno);
   	ellip[ellipno].color = (int) GetChoice(ui->color_item);
    ellip[ellipno].loctype = (int) GetChoice(ui->loc_item) ? COORD_VIEW : COORD_WORLD;
    ellip[ellipno].lines = (int) GetChoice(ui->lines_item) + 1;
    ellip[ellipno].linew = (int) GetChoice(ui->linew_item) + 1;
    switch (GetChoice(ui->fill_item)) {
    case 0:
	ellip[ellipno].fill = UNFILLED;
	break;
    case 1:
	ellip[ellipno].fill = CLRFILLED;
	break;
    case 2:
	ellip[ellipno].fill = PTNFILLED;
	break;
    }
    ellip[ellipno].fillcolor = (int) GetChoice(ui->fill_color_item);
    ellip[ellipno].fillpattern = (int) GetChoice(ui->fill_pattern_item);
    ellip[ellipno].x1 = atof(xv_getstr(ui->x1_item)) - 
    				atof(xv_getstr(ui->x2_item))/2.;
    ellip[ellipno].x2 = atof(xv_getstr(ui->x1_item)) + 
    				atof(xv_getstr(ui->x2_item))/2.;
    ellip[ellipno].y1 = atof(xv_getstr(ui->y1_item)) - 
    				atof(xv_getstr(ui->y2_item))/2.;
    ellip[ellipno].y2 = atof(xv_getstr(ui->y1_item)) + 
    				atof(xv_getstr(ui->y2_item))/2.;
	set_write_mode(1);
	draw_ellipse(-2, ellipno);
}


void swap_boxwv_coords(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditBoxUI *ui = (EditBoxUI *)client_data;
    int boxno = ui->boxno;
	

    if( ((int)GetChoice(ui->loc_item)?COORD_VIEW:COORD_WORLD) == boxes[boxno].loctype ) {
        return;
    }
	
    if( boxes[boxno].loctype == COORD_VIEW ) {
    boxes[boxno].gno = cg;
	boxes[boxno].loctype = COORD_WORLD;
	view2world( atof(xv_getstr(ui->x1_item)), atof(xv_getstr(ui->y1_item)),
				&boxes[boxno].x1,&boxes[boxno].y1 );
	view2world( atof(xv_getstr(ui->x2_item)), atof(xv_getstr(ui->y2_item)),
				&boxes[boxno].x2,&boxes[boxno].y2 );
    } else {
	boxes[boxno].loctype = COORD_VIEW;
	world2view( atof(xv_getstr(ui->x1_item)), atof(xv_getstr(ui->y1_item)),
				&boxes[boxno].x1,&boxes[boxno].y1 );
	world2view( atof(xv_getstr(ui->x2_item)), atof(xv_getstr(ui->y2_item)),
				&boxes[boxno].x2,&boxes[boxno].y2 );
    }
    update_box_edit( ui );
}


void box_edit_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditBoxUI *ui = (EditBoxUI *) client_data;
    int boxno = ui->boxno;
	set_write_mode(0);
	draw_box(-2, boxno);
    boxes[boxno].color = (int) GetChoice(ui->color_item);
    boxes[boxno].loctype = (int) GetChoice(ui->loc_item) ? COORD_VIEW : COORD_WORLD;
    boxes[boxno].lines = (int) GetChoice(ui->lines_item) + 1;
    boxes[boxno].linew = (int) GetChoice(ui->linew_item) + 1;
    switch (GetChoice(ui->fill_item)) {
    case 0:
		boxes[boxno].fill = UNFILLED;
		break;
    case 1:
		boxes[boxno].fill = CLRFILLED;
		break;
    case 2:
		boxes[boxno].fill = PTNFILLED;
		break;
    }
    boxes[boxno].fillcolor = (int) GetChoice(ui->fill_color_item);
    boxes[boxno].fillpattern = (int) GetChoice(ui->fill_pattern_item);
    boxes[boxno].x1 = atof(xv_getstr(ui->x1_item));
    boxes[boxno].x2 = atof(xv_getstr(ui->x2_item));
    boxes[boxno].y1 = atof(xv_getstr(ui->y1_item));
    boxes[boxno].y2 = atof(xv_getstr(ui->y2_item));
	set_write_mode(1);
	draw_box(-2, boxno);
}

void box_edit_popup(int boxno)
{
    Widget rc;
    Widget panel;
    int x, y;
    Widget buts[2];

    set_wait_cursor();
    if (box_ui.top == NULL) {
	char *label1[3];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	box_ui.top = XmCreateDialogShell(app_shell, "Edit box", NULL, 0);

	handle_close(box_ui.top);
	XtVaSetValues(box_ui.top, XmNx, x, XmNy, y, NULL);
	panel = XmCreateRowColumn(box_ui.top, "boxes_rc", NULL, 0);

        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
                           XmNpacking, XmPACK_COLUMN,
                           XmNnumColumns, 11,
                           XmNorientation, XmHORIZONTAL,
                           XmNisAligned, True,
                           XmNadjustLast, False,
                           XmNentryAlignment, XmALIGNMENT_END,
                           NULL);

	XtVaCreateManagedWidget("Colour:", xmLabelWidgetClass, rc, NULL);
	box_ui.color_item = CreateColorChoice(rc, " ", 0);

	XtVaCreateManagedWidget("Line width:", xmLabelWidgetClass, rc, NULL);
	box_ui.linew_item = CreatePanelChoice(rc, " ",
				    	 10,
		    	 "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
				    	 NULL,
				    	 0);

	XtVaCreateManagedWidget("Line style:", xmLabelWidgetClass, rc, NULL);
	box_ui.lines_item = (Widget *) CreatePanelChoice(rc, " ",
							6,
							"Solid line",
							"Dotted line",
							"Dashed line",
							"Long Dashed",
							"Dot-dashed",
							NULL,
							NULL);

	XtVaCreateManagedWidget("Fill:", xmLabelWidgetClass, rc, NULL);
	box_ui.fill_item = CreatePanelChoice(rc, " ",
				    	4,
				    	"None",
				    	"Color",
				    	"Pattern",
				    	NULL,
				    	0);

	XtVaCreateManagedWidget("Fill pattern:", xmLabelWidgetClass, rc, NULL);
	box_ui.fill_pattern_item = CreatePanelChoice0(rc,
							" ", 4, 17,
			   "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
			   "10", "11", "12", "13", "14", "15", 0, 0);
	XtVaCreateManagedWidget("Fill color:", xmLabelWidgetClass, rc, NULL);
	box_ui.fill_color_item = CreateColorChoice(rc, " ", 0);
	XtVaCreateManagedWidget("Position in:", xmLabelWidgetClass, rc, NULL);
	box_ui.loc_item = CreatePanelChoice(rc, " ",
					   3,
					   "World coordinates",
					   "Viewport coordinates",
					   0,
					   0);
                                          
        XtAddCallback(box_ui.loc_item[2], XmNactivateCallback, 
               (XtCallbackProc) swap_boxwv_coords, (XtPointer) &box_ui);
        XtAddCallback(box_ui.loc_item[3], XmNactivateCallback, 
               (XtCallbackProc) swap_boxwv_coords, (XtPointer) &box_ui);


	box_ui.x1_item = CreateTextItem4(rc, 8, "Xmin = ");
	box_ui.y1_item = CreateTextItem4(rc, 8, "Ymin = ");
	box_ui.x2_item = CreateTextItem4(rc, 8, "Xmax = ");
	box_ui.y2_item = CreateTextItem4(rc, 8, "Ymax = ");
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
	  	(XtCallbackProc) box_edit_proc, (XtPointer) &box_ui);
	XtAddCallback(buts[1], XmNactivateCallback,
		(XtCallbackProc) destroy_dialog, (XtPointer) box_ui.top);
	XtManageChild(panel);
    }
    box_ui.boxno = boxno;
    update_box_edit(&box_ui);
   	XtRaise(box_ui.top);
    unset_wait_cursor();
}

void ellipse_edit_popup(int boxno)
{
/*    static EditBoxUI ui;*/
    Widget rc;
    Widget panel;
    int x, y;
    Widget buts[2];

    set_wait_cursor();
    if (ellip_ui.top == NULL) {
		char *label1[2];
		label1[0] = "Accept";
		label1[1] = "Close";
		XmGetPos(app_shell, 0, &x, &y);
		ellip_ui.top = XmCreateDialogShell(app_shell, "Edit ellipse", NULL, 0);
		handle_close(ellip_ui.top);
		XtVaSetValues(ellip_ui.top, XmNx, x, XmNy, y, NULL);
		panel = XmCreateRowColumn(ellip_ui.top, "ellipses_rc", NULL, 0);

    	 rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
                    	  XmNpacking, XmPACK_COLUMN,
                    	  XmNnumColumns, 11,
                    	  XmNorientation, XmHORIZONTAL,
                    	  XmNisAligned, True,
                    	  XmNadjustLast, False,
                    	  XmNentryAlignment, XmALIGNMENT_END,
                    	  NULL);

	XtVaCreateManagedWidget("Colour:", xmLabelWidgetClass, rc, NULL);
	ellip_ui.color_item = CreateColorChoice(rc, " ", 0);

	XtVaCreateManagedWidget("Line width:", xmLabelWidgetClass, rc, NULL);
	ellip_ui.linew_item = CreatePanelChoice(rc, " ",
				    	 10,
		    	 "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
				    	 NULL,
				    	 0);

	XtVaCreateManagedWidget("Line style:", xmLabelWidgetClass, rc, NULL);
	ellip_ui.lines_item = (Widget *) CreatePanelChoice(rc, " ",
							6,
							"Solid line",
							"Dotted line",
							"Dashed line",
							"Long Dashed",
							"Dot-dashed",
							NULL,
							NULL);

	XtVaCreateManagedWidget("Fill:", xmLabelWidgetClass, rc, NULL);
	ellip_ui.fill_item = CreatePanelChoice(rc, " ",
				    	4,
				    	"None",
				    	"Color",
				    	"Pattern",
				    	NULL,
				    	0);

	XtVaCreateManagedWidget("Fill pattern:", xmLabelWidgetClass, rc, NULL);
	ellip_ui.fill_pattern_item = CreatePanelChoice0(rc,
						" ", 4, 17,
			   "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
				  "10", "11", "12", "13", "14", "15", 0, 0);
	XtVaCreateManagedWidget("Fill color:", xmLabelWidgetClass, rc, NULL);
	ellip_ui.fill_color_item = CreateColorChoice(rc, " ", 0);
	XtVaCreateManagedWidget("Position in:", xmLabelWidgetClass, rc, NULL);
	ellip_ui.loc_item = CreatePanelChoice(rc, " ",
					   3,
					   "World coordinates",
					   "Viewport coordinates",
					   0,
					   0);
        XtAddCallback(ellip_ui.loc_item[2], XmNactivateCallback, 
               (XtCallbackProc) swap_ellipwv_coords, (XtPointer) &ellip_ui );
        XtAddCallback(ellip_ui.loc_item[3], XmNactivateCallback, 
               (XtCallbackProc) swap_ellipwv_coords, (XtPointer) &ellip_ui );


	ellip_ui.x1_item = CreateTextItem4(rc, 8, "Xcentre = ");
	ellip_ui.y1_item = CreateTextItem4(rc, 8, "Ycentre = ");
	ellip_ui.x2_item = CreateTextItem4(rc, 8, "Width = ");
	ellip_ui.y2_item = CreateTextItem4(rc, 8, "Height = ");
    	XtManageChild(rc);

    	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

    	ellip_ui.boxno = boxno;
    	CreateCommandButtons(panel, 2, buts, label1);
    	XtAddCallback(buts[0], XmNactivateCallback,
		    	  (XtCallbackProc) ellipse_edit_proc, (XtPointer) &ellip_ui);
    	XtAddCallback(buts[1], XmNactivateCallback,
			  (XtCallbackProc) destroy_dialog, (XtPointer) ellip_ui.top);
    	XtManageChild(panel);
    }
    ellip_ui.boxno = boxno;
    update_ellipse_edit(&ellip_ui);
    XtRaise(ellip_ui.top);
    unset_wait_cursor();
}

typedef struct {
    Widget top;
    Widget *color_item;
    Widget *linew_item;
    Widget *lines_item;
    Widget *loc_item;
    Widget *arrow_item;
    Widget *atype_item;
    Widget asize_item;
    Widget x1_item;
    Widget y1_item;
    Widget x2_item;
    Widget y2_item;
    int lineno;
} EditLineUI;

void update_line_edit(EditLineUI *ui)
{
    int iv;
    if (ui->top) {
	int lineno = ui->lineno;
	SetChoice(ui->color_item, lines[lineno].color);
	SetChoice(ui->lines_item, lines[lineno].lines - 1);
	SetChoice(ui->linew_item, lines[lineno].linew - 1);
	SetChoice(ui->arrow_item, lines[lineno].arrow);
	SetChoice(ui->atype_item, lines[lineno].atype);
	iv = (int) (50 * lines[lineno].asize);
	XtVaSetValues(ui->asize_item, XmNvalue, iv, NULL);
	SetChoice(ui->loc_item, lines[lineno].loctype == COORD_VIEW ? 1 : 0);
	sprintf(buf, "%.12f", lines[lineno].x1);
	xv_setstr(ui->x1_item, buf);
	sprintf(buf, "%.12f", lines[lineno].y1);
	xv_setstr(ui->y1_item, buf);
	sprintf(buf, "%.12f", lines[lineno].x2);
	xv_setstr(ui->x2_item, buf);
	sprintf(buf, "%.12f", lines[lineno].y2);
	xv_setstr(ui->y2_item, buf);
    }
}

void swap_linewv_coords(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditLineUI *ui = (EditLineUI *)client_data;
    int lineno = ui->lineno;
	
    if( lines[lineno].loctype == ((int) GetChoice(ui->loc_item)?COORD_VIEW:COORD_WORLD) ) {
        return;
    }
	   
	if( lines[lineno].loctype == COORD_VIEW ) {
		lines[lineno].gno = cg;
		lines[lineno].loctype = COORD_WORLD;
		view2world( atof(xv_getstr(ui->x1_item)), atof(xv_getstr(ui->y1_item)),
										&lines[lineno].x1,&lines[lineno].y1 );
		view2world( atof(xv_getstr(ui->x2_item)), atof(xv_getstr(ui->y2_item)),
										&lines[lineno].x2,&lines[lineno].y2 );
	} else {
		lines[lineno].loctype = COORD_VIEW;
		world2view( atof(xv_getstr(ui->x1_item)), atof(xv_getstr(ui->y1_item)),
										&lines[lineno].x1,&lines[lineno].y1 );
		world2view( atof(xv_getstr(ui->x2_item)), atof(xv_getstr(ui->y2_item)),
										&lines[lineno].x2,&lines[lineno].y2 );
	}
	update_line_edit( ui );
}


void line_edit_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditLineUI *ui = (EditLineUI *) client_data;
    int value;
    int lineno = ui->lineno;
    set_write_mode(0);
    draw_line(-2,lineno);
    lines[lineno].color = (int) GetChoice(ui->color_item);
    lines[lineno].loctype = (int) GetChoice(ui->loc_item) ? COORD_VIEW : COORD_WORLD;
    lines[lineno].lines = (int) GetChoice(ui->lines_item) + 1;
    lines[lineno].linew = (int) GetChoice(ui->linew_item) + 1;
    lines[lineno].x1 = atof(xv_getstr(ui->x1_item));
    lines[lineno].y1 = atof(xv_getstr(ui->y1_item));
    lines[lineno].x2 = atof(xv_getstr(ui->x2_item));
    lines[lineno].y2 = atof(xv_getstr(ui->y2_item));
    XtVaGetValues(ui->asize_item, XmNvalue, &value, NULL);
    lines[lineno].asize = value / 50.0;
    lines[lineno].arrow = (int) GetChoice(ui->arrow_item);
    lines[lineno].atype = (int) GetChoice(ui->atype_item);
    set_write_mode(1);
    draw_line(-2,lineno);
}

static EditLineUI line_ui;

void line_edit_popup(int lineno)
{
    Widget rc;
    Widget panel;
    int x, y;
    Widget buts[2];

    set_wait_cursor();
    if (line_ui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	line_ui.top = XmCreateDialogShell(app_shell, "Edit Line", NULL, 0);
	handle_close(line_ui.top);
	XtVaSetValues(line_ui.top, XmNx, x, XmNy, y, NULL);
	panel = XmCreateRowColumn(line_ui.top, "lines_rc", NULL, 0);

        rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
                              XmNpacking, XmPACK_COLUMN,
                              XmNnumColumns, 11,
                              XmNorientation, XmHORIZONTAL,
                              XmNisAligned, True,
                              XmNadjustLast, False,
                              XmNentryAlignment, XmALIGNMENT_END,
                              NULL);

	XtVaCreateManagedWidget("Color:", xmLabelWidgetClass, rc, NULL);
	line_ui.color_item = CreateColorChoice(rc, " ", 0);

	XtVaCreateManagedWidget("Line width:", xmLabelWidgetClass, rc, NULL);
	line_ui.linew_item = CreatePanelChoice(rc, " ",
					     10,
			     "1", "2", "3", "4", "5", "6", "7", "8", "9", 0,
					     NULL,
					     0);

	XtVaCreateManagedWidget("Line style:", xmLabelWidgetClass, rc, NULL);
	line_ui.lines_item = (Widget *) CreatePanelChoice(rc, " ",
							6,
							"Solid line",
							"Dotted line",
							"Dashed line",
							"Long Dashed",
							"Dot-dashed",
							NULL,
							NULL);
	XtVaCreateManagedWidget("Arrow:", xmLabelWidgetClass, rc, NULL);
	line_ui.arrow_item = CreatePanelChoice(rc, " ",
					     5,
					     "None",
					     "At start",
					     "At end",
					     "Both ends",
					     0,
					     0);

	XtVaCreateManagedWidget("Arrow type:", xmLabelWidgetClass, rc, NULL);
	line_ui.atype_item = CreatePanelChoice(rc, " ",
					     4,
					     "Line",
					     "Filled",
					     "Hollow",
					     0,
					     0);

	XtVaCreateManagedWidget("Arrow head size:", xmLabelWidgetClass, rc, NULL);
	line_ui.asize_item = XtVaCreateManagedWidget("arrowsize", xmScaleWidgetClass, rc,
						   XmNminimum, 0,
						   XmNmaximum, 400,
						   XmNvalue, 100,
						   XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						   NULL);
	XtVaCreateManagedWidget("Position in:", xmLabelWidgetClass, rc, NULL);
	line_ui.loc_item = CreatePanelChoice(rc, " ",
					   3,
					   "World coordinates",
					   "Viewport coordinates",
					   0,
					   0);
        XtAddCallback(line_ui.loc_item[2], XmNactivateCallback,
                   (XtCallbackProc) swap_linewv_coords, (XtPointer) &line_ui);
        XtAddCallback(line_ui.loc_item[3], XmNactivateCallback,
                   (XtCallbackProc) swap_linewv_coords, (XtPointer) &line_ui);


	line_ui.x1_item = CreateTextItem4(rc, 8, "X1 = ");
	line_ui.y1_item = CreateTextItem4(rc, 8, "Y1 = ");
	line_ui.x2_item = CreateTextItem4(rc, 8, "X2 = ");
	line_ui.y2_item = CreateTextItem4(rc, 8, "Y2 = ");
	XtManageChild(rc);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		(XtCallbackProc) line_edit_proc, (XtPointer) &line_ui);
	XtAddCallback(buts[1], XmNactivateCallback,
		(XtCallbackProc) destroy_dialog, (XtPointer) line_ui.top);

	XtManageChild(panel);
    }
    XtRaise(line_ui.top);
    line_ui.lineno = lineno;
    update_line_edit(&line_ui);
    unset_wait_cursor();
}

typedef struct {
    Widget top;
    Widget string_item;
    Widget *color_item;
    Widget *loc_item;
    Widget *font_item;
    Widget size_item;
    Widget rot_item;
    Widget *just_item;
  	Widget x1_item;
    Widget y1_item;
    int stringno;
} EditStringUI;


void update_string_edit(EditStringUI *ui)
{
    int iv;
    if (ui->top) {
		plotstr *pstring = &pstr[ui->stringno];
		xv_setstr(ui->string_item, pstring->s);
		SetChoice(ui->color_item, pstring->color);
		SetChoice(ui->just_item, pstring->just);
		SetChoice(ui->font_item, pstring->font );
		iv = (int) (100 * pstring->charsize);
		XtVaSetValues(ui->size_item, XmNvalue, iv, NULL);
    	XtVaSetValues(ui->rot_item, XmNvalue, pstring->rot, NULL);
		SetChoice(ui->loc_item, pstring->loctype == COORD_VIEW ? 1 : 0);
		sprintf(buf, "%.12f", pstring->x);
		xv_setstr(ui->x1_item, buf);
		sprintf(buf, "%.12f", pstring->y);
		xv_setstr(ui->y1_item, buf);
    }
}

void swap_stringwv_coords(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditStringUI *ui = (EditStringUI *)client_data;
    int stringno = ui->stringno;
	
    if( pstr[stringno].loctype == ((int)GetChoice(ui->loc_item)?COORD_VIEW:COORD_WORLD) ) {
        return;
    }
	   
	if( pstr[stringno].loctype == COORD_VIEW ) {
		pstr[stringno].gno = cg;
		pstr[stringno].loctype = COORD_WORLD;
		view2world( atof(xv_getstr(ui->x1_item)), atof(xv_getstr(ui->y1_item)),
								&pstr[stringno].x,&pstr[stringno].y );
	} else {
		pstr[stringno].loctype = COORD_VIEW;
		world2view( atof(xv_getstr(ui->x1_item)), atof(xv_getstr(ui->y1_item)),
								&pstr[stringno].x,&pstr[stringno].y );
	}
	update_string_edit( ui );
}


void string_edit_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    EditStringUI *ui = (EditStringUI *) client_data;
    char *tmpstr;
    int value;
    int stringno = ui->stringno;
    set_write_mode(0);
    draw_string(-2,stringno);
    tmpstr = xv_getstr(ui->string_item);
   	if( strlen(pstr[stringno].s )< strlen(tmpstr) ) {
       	if( pstr[stringno].s == NULL )
       		free( pstr[stringno].s );
		pstr[stringno].s = (char *)malloc(strlen(tmpstr));        		
    }
    strcpy( pstr[stringno].s, tmpstr );
    pstr[stringno].color = (int) GetChoice(ui->color_item);
    pstr[stringno].loctype = (int) GetChoice(ui->loc_item) ? COORD_VIEW : COORD_WORLD;
    pstr[stringno].font = (int) GetChoice(ui->font_item);
    pstr[stringno].just = (int) GetChoice(ui->just_item);
    pstr[stringno].x = atof(xv_getstr(ui->x1_item));
    pstr[stringno].y = atof(xv_getstr(ui->y1_item));
    XtVaGetValues(ui->rot_item, XmNvalue, &value, NULL);
    pstr[stringno].rot = value;
    XtVaGetValues(ui->size_item, XmNvalue, &value, NULL);
    pstr[stringno].charsize = (double)value/100.;
    set_write_mode(1);
    draw_string(-2,stringno);
}

static EditStringUI string_ui;

void string_edit_popup(int stringno)
{
    Widget rc;
    Widget wlabel;
    int x, y;
    Widget buts[2];
    Widget panel;

    set_wait_cursor();
    if (string_ui.top == NULL) {
	char *label1[2];
	label1[0] = "Accept";
	label1[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	string_ui.top = XmCreateDialogShell(app_shell, "Edit String", NULL, 0);
	handle_close(string_ui.top);
	XtVaSetValues(string_ui.top, XmNx, x, XmNy, y, NULL);
	panel = XmCreateRowColumn(string_ui.top, "strings_rc", NULL, 0);

    XtVaCreateManagedWidget("String:", xmLabelWidgetClass, panel, NULL);
	string_ui.string_item = CreateTextItem2(panel, 40, "");

    rc = XtVaCreateWidget("rc", xmRowColumnWidgetClass, panel,
                              XmNpacking, XmPACK_COLUMN,
                              XmNnumColumns, 6,
                              XmNorientation, XmHORIZONTAL,
                              XmNisAligned, True,
                              XmNadjustLast, False,
                              XmNentryAlignment, XmALIGNMENT_END,
                              NULL);

    XtVaCreateManagedWidget("Color:", xmLabelWidgetClass, rc, NULL);
	string_ui.color_item = CreateColorChoice(rc, " ", 0);
	
	
    XtVaCreateManagedWidget("Font:", xmLabelWidgetClass, rc, NULL);
	string_ui.font_item = CreatePanelChoice(rc, " ",
					      11,
				"Times-Roman", "Times-Bold", "Times-Italic",
					    "Times-BoldItalic", "Helvetica",
				      "Helvetica-Bold", "Helvetica-Oblique",
				 "Helvetica-BoldOblique", "Greek", "Symbol",
					      0,
					      0);

        XtVaCreateManagedWidget("Justification:", xmLabelWidgetClass, rc, NULL);
		string_ui.just_item = CreatePanelChoice(rc, " ",
					      4,
					      "Left",
					      "Right",
					      "Centered",
					      0,
					      0);

        XtVaCreateManagedWidget("Position in:", xmLabelWidgetClass, rc, NULL);
		string_ui.loc_item = CreatePanelChoice(rc, " ",
					     3,
					     "World coordinates",
					     "Viewport coordinates",
					     0,
					     0);
        XtAddCallback(string_ui.loc_item[2], XmNactivateCallback,
                  (XtCallbackProc) swap_stringwv_coords, (XtPointer) &string_ui);
        XtAddCallback(string_ui.loc_item[3], XmNactivateCallback,
                  (XtCallbackProc) swap_stringwv_coords, (XtPointer) &string_ui);
					     
	string_ui.x1_item = CreateTextItem4(rc, 8, "X = ");
	string_ui.y1_item = CreateTextItem4(rc, 8, "Y = ");	

	XtManageChild(rc);

	wlabel = XtVaCreateManagedWidget("Rotation:", xmLabelWidgetClass, panel, NULL);
	string_ui.rot_item = XtVaCreateManagedWidget("rotation", xmScaleWidgetClass,
	panel,
						   XmNminimum, 0,
						   XmNmaximum, 360,
						   XmNvalue, 0,
						   XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						   NULL);

	wlabel = XtVaCreateManagedWidget("Size:", xmLabelWidgetClass, panel, NULL);
	string_ui.size_item = XtVaCreateManagedWidget("stringsize", xmScaleWidgetClass,
	panel,
						    XmNminimum, 0,
						    XmNmaximum, 400,
						    XmNvalue, 100,
						    XmNshowValue, True,
				     XmNprocessingDirection, XmMAX_ON_RIGHT,
					       XmNorientation, XmHORIZONTAL,
						    NULL);
						    
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

	CreateCommandButtons(panel, 2, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		    (XtCallbackProc) string_edit_proc, (XtPointer) &string_ui );
	XtAddCallback(buts[1], XmNactivateCallback,
		(XtCallbackProc) destroy_dialog, (XtPointer) string_ui.top);

	XtManageChild(panel);
    }
    XtRaise(string_ui.top);
    string_ui.stringno = stringno;
    update_string_edit(&string_ui);
    unset_wait_cursor();
}

void UpdateStrWinColors(unsigned color) {
  UpdateColorChoice(ellip_pen_item,color);
  UpdateColorChoice(ellip_fillcol_item,color);
  UpdateColorChoice(strings_pen_item,color);
  UpdateColorChoice(lines_pen_item,color);
  UpdateColorChoice(boxes_pen_item,color);
  UpdateColorChoice(boxes_fillcol_item,color);
  UpdateColorChoice(box_ui.color_item,color);
  UpdateColorChoice(box_ui.fill_color_item,color);
  UpdateColorChoice(ellip_ui.color_item,color);
  UpdateColorChoice(ellip_ui.fill_color_item,color);
  UpdateColorChoice(line_ui.color_item,color);
  UpdateColorChoice(string_ui.color_item,color);
}
