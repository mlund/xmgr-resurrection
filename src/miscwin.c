/* $Id: miscwin.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * Timestamp, save images, misc properties
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"

static Widget misc_frame;
static Widget props_frame;

/*
 * Panel item declarations
 */
static Widget timestamp_active_item;
static Widget* timestamp_font_item;
static Widget timestamp_size_item;
static Widget timestamp_rotate_item;
static Widget* timestamp_color_item;
static Widget* timestamp_linew_item;
Widget timestamp_x_item;
Widget timestamp_y_item;

static Widget* debug_item;
static Widget verify_item;
static Widget dc_item;
static Widget auto_item;

static void misc_place_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void misc_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_props_items(void);
static void props_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);

void update_misc_items(void) {
    int iv;
    Arg a;

    if (misc_frame) {
        XmToggleButtonSetState(timestamp_active_item, timestamp.active == TRUE, False);
        SetChoice(timestamp_font_item, timestamp.font);
        SetChoice(timestamp_color_item, timestamp.color);
        SetChoice(timestamp_linew_item, timestamp.linew - 1);

        iv = (int)(100 * timestamp.charsize);
        XtSetArg(a, XmNvalue, iv);
        XtSetValues(timestamp_size_item, &a, 1);

        iv = (int)(timestamp.rot % 360);
        XtSetArg(a, XmNvalue, iv);
        XtSetValues(timestamp_rotate_item, &a, 1);

        sprintf(buf, "%g", timestamp.x);
        xv_setstr(timestamp_x_item, buf);
        sprintf(buf, "%g", timestamp.y);
        xv_setstr(timestamp_y_item, buf);
    }
}

static void misc_place_notify_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    set_action(0);
    set_action(PLACE_TIMESTAMP);
}

void create_misc_frame(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget panel;
    Widget buts[3];
    set_wait_cursor();
    if (misc_frame == NULL) {
        char* label1[3];
        label1[0] = "Accept";
        label1[1] = "Place";
        label1[2] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        misc_frame = XmCreateDialogShell(app_shell, "Time stamp", NULL, 0);
        handle_close(misc_frame);
        XtVaSetValues(misc_frame, XmNx, x, XmNy, y, NULL);
        panel = XmCreateRowColumn(misc_frame, "misc_rc", NULL, 0);

        timestamp_active_item = XtVaCreateManagedWidget("Display Time stamp", xmToggleButtonWidgetClass, panel, NULL);

        timestamp_font_item = CreatePanelChoice(panel, "Font:", 11, "Times-Roman", "Times-Bold", "Times-Italic",
                                                "Times-BoldItalic", "Helvetica", "Helvetica-Bold", "Helvetica-Oblique",
                                                "Helvetica-BoldOblique", "Greek", "Symbol", NULL, NULL);
        timestamp_color_item = CreateColorChoice(panel, "Color:", 0);
        timestamp_linew_item =
            CreatePanelChoice0(panel, "Line width:", 3, 10, "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL, NULL);

        XtVaCreateManagedWidget("Character size:", xmLabelWidgetClass, panel, NULL);
        timestamp_size_item = XtVaCreateManagedWidget("size", xmScaleWidgetClass, panel, XmNminimum, 0, XmNmaximum, 400,
                                                      XmNvalue, 100, XmNshowValue, True, XmNprocessingDirection,
                                                      XmMAX_ON_RIGHT, XmNorientation, XmHORIZONTAL, NULL);

        XtVaCreateManagedWidget("Place at angle:", xmLabelWidgetClass, panel, NULL);
        timestamp_rotate_item = XtVaCreateManagedWidget(
            "tstampangle", xmScaleWidgetClass, panel, XmNminimum, 0, XmNmaximum, 360, XmNvalue, 100, XmNshowValue, True,
            XmNprocessingDirection, XmMAX_ON_RIGHT, XmNorientation, XmHORIZONTAL, NULL);

        timestamp_x_item = CreateTextItem2(panel, 10, "Timestamp X:");
        timestamp_y_item = CreateTextItem2(panel, 10, "Timestamp Y:");

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

        CreateCommandButtons(panel, 3, buts, label1);
        XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)misc_define_notify_proc, (XtPointer)0);
        XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc)misc_place_notify_proc, (XtPointer)0);
        XtAddCallback(buts[2], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)misc_frame);

        XtManageChild(panel);
    }
    XtRaise(misc_frame);
    update_misc_items();
    unset_wait_cursor();
}

static void misc_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int value;
    Arg a;

    timestamp.active = XmToggleButtonGetState(timestamp_active_item) ? TRUE : FALSE;
    timestamp.font = GetChoice(timestamp_font_item);
    timestamp.color = GetChoice(timestamp_color_item);
    timestamp.linew = GetChoice(timestamp_linew_item) + 1;

    XtSetArg(a, XmNvalue, &value);
    XtGetValues(timestamp_size_item, &a, 1);
    timestamp.charsize = value / 100.0;

    XtSetArg(a, XmNvalue, &value);
    XtGetValues(timestamp_rotate_item, &a, 1);
    timestamp.rot = value;

    timestamp.x = atof((char*)xv_getstr(timestamp_x_item));
    timestamp.y = atof((char*)xv_getstr(timestamp_y_item));
    set_dirtystate();
    drawgraph();
}

void create_props_frame(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget panel;
    Widget buts[2];

    set_wait_cursor();
    if (props_frame == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        props_frame = XmCreateDialogShell(app_shell, "Misc", NULL, 0);
        handle_close(props_frame);
        XtVaSetValues(props_frame, XmNx, x, XmNy, y, NULL);
        panel = XmCreateRowColumn(props_frame, "props_rc", NULL, 0);
        debug_item =
            CreatePanelChoice0(panel, "Debug level:", 3, 10, "Off", "1", "2", "3", "4", "5", "6", "7", "8", NULL, NULL);
        verify_item = XtVaCreateManagedWidget("Verify Pick sets operations", xmToggleButtonWidgetClass, panel, NULL);
        dc_item = XtVaCreateManagedWidget("Allow double clicks on canvas", xmToggleButtonWidgetClass, panel, NULL);
        auto_item = XtVaCreateManagedWidget("Allow autoscale on read", xmToggleButtonWidgetClass, panel, NULL);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

        CreateCommandButtons(panel, 2, buts, label1);
        XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)props_define_notify_proc, (XtPointer)0);
        XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)props_frame);

        XtManageChild(panel);
    }
    XtRaise(props_frame);
    update_props_items();
    unset_wait_cursor();
}

static void update_props_items(void) {
    if (props_frame) {
        if (debuglevel > 8) {
            errwin("Debug level > 8, resetting to 0");
            debuglevel = 0;
        }
        SetChoice(debug_item, debuglevel);
        XmToggleButtonSetState(verify_item, verify_action, False);
        XmToggleButtonSetState(dc_item, allow_dc, False);
        XmToggleButtonSetState(auto_item, autoscale_onread, False);
    }
}

static void props_define_notify_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    debuglevel = (int)GetChoice(debug_item);
    /*
        invert = (int) xv_getstr(invert_item);
        backingstore = (int) xv_getstr(bs_item);
        allow_refresh = (int) xv_getstr(refresh_item);
        revflag = (int) xv_getstr(rvideo_item);
        maxplot = (int) xv_getstr(maxplot_item);
        maxgraph = (int) xv_getstr(maxgraph_item);
        maxcolors = (int) xv_getstr(maxcolors_item);
    */
    verify_action = XmToggleButtonGetState(verify_item);
    allow_dc = XmToggleButtonGetState(dc_item);
    autoscale_onread = XmToggleButtonGetState(auto_item);
}

void UpdateMiscWinColors(unsigned color) { UpdateColorChoice(timestamp_color_item, color); }
