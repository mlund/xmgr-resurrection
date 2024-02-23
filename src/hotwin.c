/* $Id: hotwin.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * hot links
 *
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/Separator.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"

static Widget hotlink_frame = (Widget)NULL;
static SetChoiceItem hotlink_set_item;
static Widget hotlink_list_item;
static Widget hotlink_file_item;
static Widget* hotlink_source_item;

void create_hotfiles_popup(Widget w, XtPointer client_data, XtPointer call_data);

static void do_hotlink_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int setno, src;
    char fname[256];
    char buf[256];
    XmString xms;

    set_wait_cursor();

    setno = GetSelectedSet(hotlink_set_item);
    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        unset_wait_cursor();
        return;
    }
    src = GetChoice(hotlink_source_item);
    strcpy(fname, xv_getstr(hotlink_file_item));

    sprintf(buf, "S%02d -> %s -> %s", setno, src == 0 ? "DISK" : "PIPE", fname);

    xms = XmStringCreateLtoR(buf, charset);
    XmListAddItemUnselected(hotlink_list_item, xms, 0);

    set_hotlink(cg, setno, TRUE, fname, src == 0 ? SOURCE_DISK : SOURCE_PIPE);

    XmStringFree(xms);

    unset_wait_cursor();
}

static void do_hotunlink_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    XmString *s, cs;
    int* pos_list;
    int j, pos_cnt, cnt;
    char* cstr;
    int setno;

    set_wait_cursor();

    if (XmListGetSelectedPos(hotlink_list_item, &pos_list, &pos_cnt)) {
        j = pos_list[0];
        XtVaGetValues(hotlink_list_item, XmNselectedItemCount, &cnt, XmNselectedItems, &s, NULL);
        cs = XmStringCopy(*s);
        if (XmStringGetLtoR(cs, charset, &cstr)) {
            sscanf(cstr, "S%d", &setno);
            if (setno >= 0 && setno < g[cg].maxplot) {
                set_hotlink(cg, setno, FALSE, NULL, 0);
            }
            XtFree(cstr);
        }
        XmStringFree(cs);
        update_hotlinks();
    }

    unset_wait_cursor();
}

void update_hotlinks(void) {
    int i, j;
    char buf[256];
    XmString xms;

    if (hotlink_frame != NULL) {
        set_wait_cursor();
        XmListDeleteAllItems(hotlink_list_item);
        for (i = 0; i < maxgraph; i++) {
            for (j = 0; j < g[i].maxplot; j++) {
                if (is_hotlinked(i, j)) {
                    sprintf(buf, "S%02d -> %s -> %s", j, get_hotlink_src(i, j) == SOURCE_DISK ? "DISK" : "PIPE",
                            get_hotlink_file(i, j));
                    xms = XmStringCreateLtoR(buf, charset);
                    XmListAddItemUnselected(hotlink_list_item, xms, 0);
                    XmStringFree(xms);
                }
            }
        }
        unset_wait_cursor();
    }
}

static void do_hotupdate_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;

    set_wait_cursor();

    for (i = 0; i < g[cg].maxplot; i++) {
        if (is_hotlinked(cg, i)) {
            do_update_hotlink(cg, i);
        }
    }

    unset_wait_cursor();
    drawgraph();
}

void create_hotlinks_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    static Widget top, dialog;
    Arg args[3];
    set_wait_cursor();
    if (top == NULL) {
        char* label1[5];
        Widget but1[5];
        label1[0] = "Link";
        label1[1] = "Files...";
        label1[2] = "Unlink";
        label1[3] = "Update";
        label1[4] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        top = XmCreateDialogShell(app_shell, "Hot links", NULL, 0);
        handle_close(top);
        XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(top, "dialog_rc", NULL, 0);

        XtSetArg(args[0], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);
        XtSetArg(args[1], XmNvisibleItemCount, 5);
        hotlink_list_item = XmCreateScrolledList(dialog, "list", args, 2);
        XtManageChild(hotlink_list_item);

        hotlink_set_item = CreateSetSelector(dialog, "Link set:", SET_SELECT_ACTIVE, FILTER_SELECT_ALL,
                                             GRAPH_SELECT_CURRENT, SELECTION_TYPE_MULTIPLE);
        DefineSetSelectorFilter(&hotlink_set_item);

        hotlink_file_item = CreateTextItem2(dialog, 30, "To file or SOURCE_PIPE:");
        hotlink_source_item = CreatePanelChoice(dialog, "Source: ", 3, "Disk file", "Pipe", NULL, NULL);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 5, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_hotlink_proc, (XtPointer)NULL);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)create_hotfiles_popup, (XtPointer)NULL);
        XtAddCallback(but1[2], XmNactivateCallback, (XtCallbackProc)do_hotunlink_proc, (XtPointer)NULL);
        XtAddCallback(but1[3], XmNactivateCallback, (XtCallbackProc)do_hotupdate_proc, (XtPointer)NULL);
        XtAddCallback(but1[4], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)top);

        XtManageChild(dialog);
        hotlink_frame = top;
    }
    XtRaise(top);
    update_hotlinks();
    unset_wait_cursor();
}

static void do_hotlinkfile_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    Widget dialog = (Widget)client_data;
    char* s;
    XmFileSelectionBoxCallbackStruct* cbs = (XmFileSelectionBoxCallbackStruct*)call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
        errwin("Error converting XmString to char string in do_hotlinkfile_proc()");
        return;
    }
    xv_setstr(hotlink_file_item, s);
    XtFree(s);
    XtUnmanageChild(dialog);
}

void create_hotfiles_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    static Widget top;

    set_wait_cursor();
    if (top == NULL) {
        top = XmCreateFileSelectionDialog(app_shell, "hotlinks", NULL, 0);
        XtVaSetValues(XtParent(top), XmNtitle, "Select hot link file", NULL);

        XtAddCallback(top, XmNokCallback, (XtCallbackProc)do_hotlinkfile_proc, (XtPointer)top);
        XtAddCallback(top, XmNcancelCallback, (XtCallbackProc)destroy_dialog, (XtPointer)top);
    }
    XtRaise(top);
    unset_wait_cursor();
}
