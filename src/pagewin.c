/* $Id: pagewin.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * Set page size, orientation and background color
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"

extern int canvasw, canvash;
extern int bgcolor;

static Widget page_frame;
static Widget page_panel;

/*
 * Panel item declarations
 */
static Widget* page_layout_item;
static Widget* page_color_item;
static Widget page_width_item;
static Widget page_height_item;

/*
 * Event and Notify proc declarations
 */

void update_page(void) {
    char buf[256];
    if (page_frame) {
        SetChoice(page_layout_item, get_pagelayout());
        sprintf(buf, "%d", canvasw);
        xv_setstr(page_width_item, buf);
        sprintf(buf, "%d", canvash);
        xv_setstr(page_height_item, buf);
        if (page_layout == PAGE_FIXED) {
            XtSetSensitive(page_width_item, True);
            XtSetSensitive(page_height_item, True);
        } else {
            XtSetSensitive(page_width_item, False);
            XtSetSensitive(page_height_item, False);
        }
        SetChoice(page_color_item, bgcolor);
    }
}

void do_pagelayout_toggle(Widget w, XtPointer client_data, XtPointer call_data) {
    int value = (int)client_data;
    if (page_frame) {
        if (value == PAGE_FIXED) {
            XtSetSensitive(page_width_item, True);
            XtSetSensitive(page_height_item, True);
        } else {
            XtSetSensitive(page_width_item, False);
            XtSetSensitive(page_height_item, False);
        }
    }
}

/*
 * define the draw options
 */
static void define_page_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    char buf[256];
    set_page(NULL, (XtPointer)(intptr_t)get_pagelayout(), NULL);
    switch (GetChoice(page_layout_item)) {
    case 1:
        page_layout = PAGE_LANDSCAPE;
        break;
    case 2:
        page_layout = PAGE_PORTRAIT;
        break;
    case 3:
        page_layout = PAGE_FIXED;
        break;
    case 0: /* falls through */
    default:
        page_layout = PAGE_FREE;
        break;
    }

    if (page_layout == PAGE_FIXED) {
        strcpy(buf, (char*)xv_getstr(page_width_item));
        canvasw = atoi(buf);
        strcpy(buf, (char*)xv_getstr(page_height_item));
        canvash = atoi(buf);
    }
    bgcolor = (int)GetChoice(page_color_item);
    set_page(NULL, (XtPointer)(intptr_t)page_layout, NULL);
    XtUnmanageChild(page_frame);
    drawgraph();
}

/*
 * Create the draw Frame and the draw Panel
 */
void create_page_frame(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget buts[2];

    set_wait_cursor();
    if (page_frame == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        page_frame = XmCreateDialogShell(app_shell, "Page options", NULL, 0);
        handle_close(page_frame);
        XtVaSetValues(page_frame, XmNx, x, XmNy, y, NULL);
        page_panel = XmCreateRowColumn(page_frame, "page_rc", NULL, 0);
        page_layout_item =
            CreatePanelChoice(page_panel, "Page layout", 5, "Free", "Landscape", "Portrait", "Fixed", NULL, NULL);
        XtAddCallback(page_layout_item[2], XmNactivateCallback, (XtCallbackProc)do_pagelayout_toggle,
                      (XtPointer)PAGE_FREE);
        XtAddCallback(page_layout_item[3], XmNactivateCallback, (XtCallbackProc)do_pagelayout_toggle,
                      (XtPointer)PAGE_LANDSCAPE);
        XtAddCallback(page_layout_item[4], XmNactivateCallback, (XtCallbackProc)do_pagelayout_toggle,
                      (XtPointer)PAGE_PORTRAIT);
        XtAddCallback(page_layout_item[5], XmNactivateCallback, (XtCallbackProc)do_pagelayout_toggle,
                      (XtPointer)PAGE_FIXED);
        page_width_item = CreateTextItem2(page_panel, 10, "Page width (pixels)");
        page_height_item = CreateTextItem2(page_panel, 10, "Page height (pixels)");
        page_color_item = CreateColorChoice(page_panel, "Page colour", 0);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, page_panel, NULL);

        CreateCommandButtons(page_panel, 2, buts, label1);
        XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)define_page_proc, (XtPointer)0);
        XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)page_frame);

        XtManageChild(page_panel);
    }
    XtRaise(page_frame);
    update_page();
    unset_wait_cursor();
}
