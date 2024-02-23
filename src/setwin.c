/* $Id: setwin.c,v 1.2 1995/04/20 20:40:15 pturner Exp pturner $
 *
 * setops - operations on sets
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
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
#include <Xm/Protocols.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"

static Widget but1[2];

/*
 * char format[128] = "%16lg %16lg";
 */

static void do_activate_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_deactivate_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_reactivate_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_setlength_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_changetype_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_copy_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_setmove_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_drop_points_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_join_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_split_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_kill_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_sort_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_reverse_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_coalesce_sets_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_swap_proc(Widget w, XtPointer client_data, XtPointer call_data);

extern int index_set_types[]; /* declared in setutils.c */
extern int index_set_ncols[];

void define_pickops_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    static Widget top;
    Widget panel, wbut;
    int x, y;

    set_wait_cursor();
    if (top == NULL) {
        XmGetPos(app_shell, 0, &x, &y);
        top = XmCreateDialogShell(app_shell, "Pick ops", NULL, 0);
        handle_close(top);
        XtVaSetValues(top, XmNx, x, XmNy, y, NULL);
        panel = XmCreateRowColumn(top, "pickops_rc", NULL, 0);

        wbut = XtVaCreateManagedWidget("Kill nearest set", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)KILL_NEAREST);

        wbut = XtVaCreateManagedWidget("Copy nearest set", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)COPY_NEAREST1ST);

        wbut = XtVaCreateManagedWidget("Move nearest set", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)MOVE_NEAREST1ST);

        wbut = XtVaCreateManagedWidget("Reverse nearest set", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)REVERSE_NEAREST);

        wbut = XtVaCreateManagedWidget("De-activate nearest set", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)DEACTIVATE_NEAREST);

        wbut = XtVaCreateManagedWidget("Join nearest sets", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)JOIN_NEAREST1ST);

        wbut = XtVaCreateManagedWidget("Delete range in nearest set", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)DELETE_NEAREST1ST);

        wbut = XtVaCreateManagedWidget("Break set", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)PICK_BREAK);

        wbut = XtVaCreateManagedWidget("Cancel operation", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)set_actioncb, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("Close", xmPushButtonWidgetClass, panel, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)top);

        XtManageChild(panel);
    }
    XtRaise(top);
    unset_wait_cursor();
}

typedef struct _Act_ui {
    Widget top;
    SetChoiceItem sel;
    Widget len_item;
    Widget comment_item;
    Widget* type_item;
    Widget* graph_item;
} Act_ui;

static Act_ui aui;

void create_activate_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;
    set_wait_cursor();
    if (aui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        aui.top = XmCreateDialogShell(app_shell, "Activate set", NULL, 0);
        handle_close(aui.top);
        XtVaSetValues(aui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(aui.top, "dialog_rc", NULL, 0);

        aui.sel = CreateSetSelector(dialog, "Activate set:", SET_SELECT_ACTIVE, FILTER_SELECT_INACT,
                                    GRAPH_SELECT_CURRENT, SELECTION_TYPE_MULTIPLE);
        DefineSetSelectorFilter(&aui.sel);

        aui.comment_item = CreateTextItem2(dialog, 25, "Comment:");
        aui.len_item = CreateTextItem2(dialog, 10, "Length:");
        aui.type_item = CreatePanelChoice(dialog, "Set type:", 10, "XY", "XY DX", "XY DY", "XY DX1 DX2", "XY DY1 DY2",
                                          "XY DX DY", "XY Z", "XY HILO", "XY R", NULL, 0);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_activate_proc, (XtPointer)&aui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)aui.top);

        XtManageChild(dialog);
    }
    XtRaise(aui.top);
    unset_wait_cursor();
}

typedef struct _Deact_ui {
    Widget top;
    SetChoiceItem sel;
    Widget allbut_item;
    Widget* graph_item;
} Deact_ui;

static Deact_ui deactui;

void create_deactivate_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (deactui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        deactui.top = XmCreateDialogShell(app_shell, "De-activate set", NULL, 0);
        handle_close(deactui.top);
        XtVaSetValues(deactui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(deactui.top, "dialog_rc", NULL, 0);

        deactui.sel = CreateSetSelector(dialog, "De-activate set:", SET_SELECT_ACTIVE, FILTER_SELECT_NONE,
                                        GRAPH_SELECT_CURRENT, SELECTION_TYPE_MULTIPLE);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);
        deactui.allbut_item =
            XtVaCreateManagedWidget("Deactivate all but selected sets", xmToggleButtonWidgetClass, dialog, NULL);
        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_deactivate_proc, (XtPointer)&deactui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)deactui.top);

        XtManageChild(dialog);
    }
    XtRaise(deactui.top);
    unset_wait_cursor();
}

typedef struct _React_ui {
    Widget top;
    SetChoiceItem sel;
    Widget allbut_item;
    Widget* graph_item;
} React_ui;

static React_ui reactui;

void create_reactivate_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (reactui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        reactui.top = XmCreateDialogShell(app_shell, "Re-activate set", NULL, 0);
        handle_close(reactui.top);
        XtVaSetValues(reactui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(reactui.top, "dialog_rc", NULL, 0);

        reactui.sel = CreateSetSelector(dialog, "Re-activate set:", SET_SELECT_ACTIVE, FILTER_SELECT_DEACT,
                                        GRAPH_SELECT_CURRENT, SELECTION_TYPE_MULTIPLE);
        DefineSetSelectorFilter(&reactui.sel);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);
        reactui.allbut_item =
            XtVaCreateManagedWidget("Reactivate all but selected sets", xmToggleButtonWidgetClass, dialog, NULL);
        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_reactivate_proc, (XtPointer)&reactui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)reactui.top);

        XtManageChild(dialog);
    }
    XtRaise(reactui.top);
    unset_wait_cursor();
}

typedef struct _Type_ui {
    Widget top;
    SetChoiceItem sel;
    Widget comment_item;
    Widget* type_item;
    Widget* graph_item;
} Type_ui;

static Type_ui tui;

static void changetypeCB(Widget w, XtPointer clientd, XtPointer calld) {
    Type_ui* ui = (Type_ui*)clientd;
    XmListCallbackStruct* cbs = (XmListCallbackStruct*)calld;
    char* s;
    XmStringGetLtoR(cbs->item, charset, &s);
    if (cbs->reason == XmCR_SINGLE_SELECT) {
        int setno = GetSelectedSet(ui->sel);
        if (setno == SET_SELECT_ERROR) {
            errwin("No set selected");
            return;
        }
        /*
            int setno = GetSetFromString(s);
        */
        if (setno >= 0) {
            xv_setstr(ui->comment_item, getcomment(cg, setno));
        }
    }
    XtFree(s);
}

void create_change_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (tui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        tui.top = XmCreateDialogShell(app_shell, "Change set type", NULL, 0);
        handle_close(tui.top);
        XtVaSetValues(tui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(tui.top, "dialog_rc", NULL, 0);

        tui.sel = CreateSetSelector(dialog, "Apply to set:", SET_SELECT_ACTIVE, FILTER_SELECT_NONE,
                                    GRAPH_SELECT_CURRENT, SELECTION_TYPE_MULTIPLE);

        XtVaSetValues(tui.sel.list, XmNselectionPolicy, XmSINGLE_SELECT, NULL);
        XtAddCallback(tui.sel.list, XmNdefaultActionCallback, changetypeCB, &tui);
        XtAddCallback(tui.sel.list, XmNsingleSelectionCallback, changetypeCB, &tui);
        tui.comment_item = CreateTextItem2(dialog, 20, "Comment:");
        tui.type_item = CreatePanelChoice(dialog, "Type:", 10, "XY", "XY DX", "XY DY", "XY DX1 DX2", "XY DY1 DY2",
                                          "XY DX DY", "XY Z", "XY HILO", "XY R", NULL, 0);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_changetype_proc, (XtPointer)&tui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)tui.top);

        XtManageChild(dialog);
    }
    XtRaise(tui.top);
    unset_wait_cursor();
}

typedef struct _Length_ui {
    Widget top;
    SetChoiceItem sel;
    Widget length_item;
    Widget* graph_item;
} Length_ui;

static Length_ui lui;

void create_setlength_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (lui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        lui.top = XmCreateDialogShell(app_shell, "Set length", NULL, 0);
        handle_close(lui.top);
        XtVaSetValues(lui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(lui.top, "dialog_rc", NULL, 0);

        lui.sel = CreateSetSelector(dialog, "Set length of set:", SET_SELECT_ACTIVE, FILTER_SELECT_NONE,
                                    GRAPH_SELECT_CURRENT, SELECTION_TYPE_MULTIPLE);
        lui.length_item = CreateTextItem2(dialog, 10, "Length:");

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_setlength_proc, (XtPointer)&lui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)lui.top);

        XtManageChild(dialog);
    }
    XtRaise(lui.top);
    unset_wait_cursor();
}

typedef struct _Copy_ui {
    Widget top;
    SetChoiceItem sel;
    Widget* graph_item;
} Copy_ui;

static Copy_ui cui;

void create_copy_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (cui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        cui.top = XmCreateDialogShell(app_shell, "Copy set", NULL, 0);
        handle_close(cui.top);
        XtVaSetValues(cui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(cui.top, "dialog_rc", NULL, 0);

        cui.sel = CreateSetSelector(dialog, "Copy set:", SET_SELECT_ALL, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
                                    SELECTION_TYPE_MULTIPLE);
        cui.graph_item = CreateGraphChoice(dialog, "To graph:", maxgraph, 1);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_copy_proc, (XtPointer)&cui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)cui.top);

        XtManageChild(dialog);
    }
    XtRaise(cui.top);
    unset_wait_cursor();
}

typedef struct _Move_ui {
    Widget top;
    SetChoiceItem sel;
    Widget* graph_item;
} Move_ui;

static Move_ui mui;

void create_move_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (mui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        mui.top = XmCreateDialogShell(app_shell, "Move set", NULL, 0);
        handle_close(mui.top);
        XtVaSetValues(mui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(mui.top, "dialog_rc", NULL, 0);

        mui.sel = CreateSetSelector(dialog, "Move set:", SET_SELECT_ACTIVE, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
                                    SELECTION_TYPE_MULTIPLE);
        mui.graph_item = CreateGraphChoice(dialog, "To graph:", maxgraph, 1);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_setmove_proc, (XtPointer)&mui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)mui.top);

        XtManageChild(dialog);
    }
    XtRaise(mui.top);
    unset_wait_cursor();
}

typedef struct _Drop_ui {
    Widget top;
    SetChoiceItem sel;
    Widget start_item;
    Widget stop_item;
} Drop_ui;

static Drop_ui dui;

void create_drop_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (dui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        dui.top = XmCreateDialogShell(app_shell, "Drop points", NULL, 0);
        handle_close(dui.top);
        XtVaSetValues(dui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(dui.top, "dialog_rc", NULL, 0);

        dui.sel = CreateSetSelector(dialog, "Drop points from set:", SET_SELECT_ALL, FILTER_SELECT_NONE,
                                    GRAPH_SELECT_CURRENT, SELECTION_TYPE_MULTIPLE);
        dui.start_item = CreateTextItem2(dialog, 6, "Start drop at:");
        dui.stop_item = CreateTextItem2(dialog, 6, "End drop at:");

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_drop_points_proc, (XtPointer)&dui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)dui.top);

        XtManageChild(dialog);
    }
    XtRaise(dui.top);
    unset_wait_cursor();
}

typedef struct _Join_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget* graph_item;
} Join_ui;

static Join_ui jui;

void create_join_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (jui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        jui.top = XmCreateDialogShell(app_shell, "Join sets", NULL, 0);
        handle_close(jui.top);
        XtVaSetValues(jui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(jui.top, "dialog_rc", NULL, 0);

        jui.sel1 = CreateSetSelector(dialog, "Join set:", SET_SELECT_ACTIVE, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
                                     SELECTION_TYPE_SINGLE);
        jui.sel2 = CreateSetSelector(dialog, "To the end of set:", SET_SELECT_ACTIVE, FILTER_SELECT_NONE,
                                     GRAPH_SELECT_CURRENT, SELECTION_TYPE_SINGLE);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_join_sets_proc, (XtPointer)&jui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)jui.top);

        XtManageChild(dialog);
    }
    XtRaise(jui.top);
    unset_wait_cursor();
}

typedef struct _Split_ui {
    Widget top;
    SetChoiceItem sel;
    Widget len_item;
    Widget* graph_item;
} Split_ui;

static Split_ui sui;

void create_split_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (sui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        sui.top = XmCreateDialogShell(app_shell, "Split sets", NULL, 0);
        handle_close(sui.top);
        XtVaSetValues(sui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(sui.top, "dialog_rc", NULL, 0);

        sui.sel = CreateSetSelector(dialog, "Split set:", SET_SELECT_ALL, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
                                    SELECTION_TYPE_MULTIPLE);
        sui.len_item = CreateTextItem2(dialog, 10, "Length:");

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_split_sets_proc, (XtPointer)&sui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)sui.top);

        XtManageChild(dialog);
    }
    XtRaise(sui.top);
    unset_wait_cursor();
}

typedef struct _Kill_ui {
    Widget top;
    SetChoiceItem sel;
    Widget soft_toggle;
    Widget* graph_item;
} Kill_ui;

static Kill_ui kui;

void create_kill_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (kui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        kui.top = XmCreateDialogShell(app_shell, "Kill set", NULL, 0);
        handle_close(kui.top);
        XtVaSetValues(kui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(kui.top, "dialog_rc", NULL, 0);

        kui.sel = CreateSetSelector(dialog, "Kill set:", SET_SELECT_ALL, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
                                    SELECTION_TYPE_MULTIPLE);
        kui.soft_toggle = XtVaCreateManagedWidget("Preserve parameters", xmToggleButtonWidgetClass, dialog, NULL);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_kill_proc, (XtPointer)&kui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)kui.top);

        XtManageChild(dialog);
    }
    XtRaise(kui.top);
    unset_wait_cursor();
}

typedef struct _Sort_ui {
    Widget top;
    SetChoiceItem sel;
    Widget* xy_item;
    Widget* up_down_item;
    Widget* graph_item;
} Sort_ui;

static Sort_ui sortui;

void create_sort_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (sortui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        sortui.top = XmCreateDialogShell(app_shell, "Sort sets", NULL, 0);
        handle_close(sortui.top);
        XtVaSetValues(sortui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(sortui.top, "dialog_rc", NULL, 0);

        sortui.sel = CreateSetSelector(dialog, "Sort set:", SET_SELECT_ACTIVE, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
                                       SELECTION_TYPE_MULTIPLE);
        sortui.xy_item = CreatePanelChoice(dialog, "Sort on:", 7, "X", "Y", "Y1", "Y2", "Y3", "Y4", 0, 0);
        sortui.up_down_item = CreatePanelChoice(dialog, "Order:", 3, "Ascending", "Descending", 0, 0);
        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_sort_proc, (XtPointer)&sortui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)sortui.top);

        XtManageChild(dialog);
    }
    XtRaise(sortui.top);
    unset_wait_cursor();
}

typedef struct _Reverse_ui {
    Widget top;
    SetChoiceItem sel;
    Widget* graph_item;
} Reverse_ui;

static Reverse_ui rui;

void create_reverse_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (rui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        rui.top = XmCreateDialogShell(app_shell, "Reverse sets", NULL, 0);
        handle_close(rui.top);
        XtVaSetValues(rui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(rui.top, "dialog_rc", NULL, 0);

        rui.sel = CreateSetSelector(dialog, "Reverse set:", SET_SELECT_ALL, FILTER_SELECT_NONE, GRAPH_SELECT_CURRENT,
                                    SELECTION_TYPE_MULTIPLE);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_reverse_sets_proc, (XtPointer)&rui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)rui.top);

        XtManageChild(dialog);
    }
    XtRaise(rui.top);
    unset_wait_cursor();
}

typedef struct _Coal_ui {
    Widget top;
    SetChoiceItem sel;
    Widget* graph_item;
} Coal_ui;

static Coal_ui coalui;

void create_coalesce_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    Widget dialog;

    set_wait_cursor();
    if (coalui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        coalui.top = XmCreateDialogShell(app_shell, "Coalesce sets", NULL, 0);
        handle_close(coalui.top);
        XtVaSetValues(coalui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(coalui.top, "dialog_rc", NULL, 0);

        coalui.sel = CreateSetSelector(dialog, "Coalesce active sets to set:", SET_SELECT_ALL, FILTER_SELECT_NONE,
                                       GRAPH_SELECT_CURRENT, SELECTION_TYPE_MULTIPLE);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_coalesce_sets_proc, (XtPointer)&coalui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)coalui.top);

        XtManageChild(dialog);
    }
    XtRaise(coalui.top);
    unset_wait_cursor();
}

typedef struct _Swap_ui {
    Widget top;
    SetChoiceItem sel1;
    SetChoiceItem sel2;
    Widget* graph1_item;
    Widget* graph2_item;
} Swap_ui;

static Swap_ui swapui;

void do_setsel_gr_update(Widget w, XtPointer client_data, XtPointer call_data) {
    if ((int)client_data == 1) {
        swapui.sel1.gno = (int)GetChoice(swapui.graph1_item) - 1;
        if (swapui.sel1.gno == -1) {
            swapui.sel1.gno = GRAPH_SELECT_CURRENT;
            update_save_set_list(swapui.sel1, cg);
        } else
            update_save_set_list(swapui.sel1, swapui.sel1.gno);
    } else {
        swapui.sel2.gno = (int)GetChoice(swapui.graph2_item) - 1;
        if (swapui.sel2.gno == -1) {
            swapui.sel2.gno = GRAPH_SELECT_CURRENT;
            update_save_set_list(swapui.sel2, cg);
        } else
            update_save_set_list(swapui.sel2, swapui.sel2.gno);
    }
}

void create_swap_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y, i;
    Widget dialog;

    set_wait_cursor();
    if (swapui.top == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        swapui.top = XmCreateDialogShell(app_shell, "Swap sets", NULL, 0);
        handle_close(swapui.top);
        XtVaSetValues(swapui.top, XmNx, x, XmNy, y, NULL);
        dialog = XmCreateRowColumn(swapui.top, "dialog_rc", NULL, 0);

        swapui.sel1 = CreateSetSelector(dialog, "Swap set:", SET_SELECT_ACTIVE, FILTER_SELECT_NONE,
                                        GRAPH_SELECT_CURRENT, SELECTION_TYPE_SINGLE);
        swapui.graph1_item = CreateGraphChoice(dialog, "In graph:", maxgraph, 1);
        for (i = 0; i < maxgraph; i++)
            XtAddCallback(swapui.graph1_item[2 + i], XmNactivateCallback, (XtCallbackProc)do_setsel_gr_update,
                          (XtPointer)1);

        swapui.sel2 = CreateSetSelector(dialog, "With set:", SET_SELECT_ACTIVE, FILTER_SELECT_ALL, GRAPH_SELECT_CURRENT,
                                        SELECTION_TYPE_SINGLE);
        DefineSetSelectorFilter(&swapui.sel2);
        swapui.graph2_item = CreateGraphChoice(dialog, "In graph:", maxgraph, 1);
        for (i = 0; i < maxgraph; i++)
            XtAddCallback(swapui.graph2_item[2 + i], XmNactivateCallback, (XtCallbackProc)do_setsel_gr_update,
                          (XtPointer)2);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 2, but1, label1);
        XtAddCallback(but1[0], XmNactivateCallback, (XtCallbackProc)do_swap_proc, (XtPointer)&swapui);
        XtAddCallback(but1[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)swapui.top);

        XtManageChild(dialog);
    }
    XtRaise(swapui.top);
    unset_wait_cursor();
}

/*
 * setops - combine, copy sets - callbacks
 */

/*
 * activate a set and set its length
 */
static void do_activate_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int setno, len, type;
    Act_ui* ui = (Act_ui*)client_data;
    setno = GetSelectedSet(ui->sel);
    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    type = GetChoice(ui->type_item);
    len = atoi((char*)xv_getstr(ui->len_item));
    setcomment(cg, setno, (char*)xv_getstr(ui->comment_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    do_activate(setno, type, len);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
}

/*
 * de-activate a set
 */
static void do_deactivate_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int* selsets;
    int i, cnt, allbut;
    int setno;
    Deact_ui* ui = (Deact_ui*)client_data;
    allbut = XmToggleButtonGetState(ui->allbut_item);
    cnt = GetSelectedSets(ui->sel, &selsets);
    if ((cnt == SET_SELECT_ERROR) && !allbut) {
        errwin("No sets selected");
        return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    if (allbut) {
        for (i = 0; i < g[cg].maxplot; i++) {
            if (isactive_set(cg, i)) {
                do_deactivate(cg, i);
            }
        }
        if (cnt != SET_SELECT_ERROR) {
            for (i = 0; i < cnt; i++) {
                setno = selsets[i];
                do_reactivate(cg, setno);
            }
        }
    } else {
        for (i = 0; i < cnt; i++) {
            setno = selsets[i];
            do_deactivate(cg, setno);
        }
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    if (cnt != SET_SELECT_ERROR) {
        free(selsets);
    }
    drawgraph();
}

/*
 * re-activate a set
 */
static void do_reactivate_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int* selsets;
    int i, cnt, allbut;
    int setno;
    React_ui* ui = (React_ui*)client_data;
    allbut = XmToggleButtonGetState(ui->allbut_item);
    cnt = GetSelectedSets(ui->sel, &selsets);
    if ((cnt == SET_SELECT_ERROR) && !allbut) {
        errwin("No sets selected");
        return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    if (allbut) {
        for (setno = i = 0; setno < g[cg].maxplot; setno++) {
            if ((cnt != SET_SELECT_ERROR) && i < cnt) {
                if (setno == selsets[i]) {
                    i++;
                } else {
                    if (g[cg].p[setno].deact) {
                        do_reactivate(cg, setno);
                    }
                }
            } else {
                if (g[cg].p[setno].deact) {
                    do_reactivate(cg, setno);
                }
            }
        }
    } else {
        for (i = 0; i < cnt; i++) {
            setno = selsets[i];
            do_reactivate(cg, setno);
        }
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    if (cnt != SET_SELECT_ERROR) {
        free(selsets);
    }
    drawgraph();
}

/*
 * change the type of a set
 */
static void do_changetype_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int setno, type;
    Type_ui* ui = (Type_ui*)client_data;
    setno = GetSelectedSet(ui->sel);
    if (setno == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    }
    type = GetChoice(ui->type_item);
    setcomment(cg, setno, (char*)xv_getstr(ui->comment_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    do_changetype(setno, type);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
}

/*
 * set the length of an active set - contents are destroyed
 */
static void do_setlength_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int* selsets;
    int i, cnt;
    int setno, len;
    Length_ui* ui = (Length_ui*)client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    len = atoi((char*)xv_getstr(ui->length_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        setno = selsets[i];
        do_setlength(setno, len);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/*
 * copy a set to another set, if the to set doesn't exist
 * get a new one, if it does, ask if it is okay to overwrite
 */
static void do_copy_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int j1, j2, gto, i, *selsets;
    Copy_ui* ui = (Copy_ui*)client_data;
    int cnt;
    set_wait_cursor();
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        j1 = selsets[i];
        j2 = SET_SELECT_NEXT;
        gto = GetChoice(ui->graph_item);
        do_copy(j1, cg, j2, gto);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/*
 * move a set to another set, if the to set doesn't exist
 * get a new one, if it does, ask if it is okay to overwrite
 */
static void do_setmove_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int j1, j2, gto, i, *selsets;
    Move_ui* ui = (Move_ui*)client_data;
    int cnt;
    set_wait_cursor();
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        j1 = selsets[i];
        j2 = SET_SELECT_NEXT;
        gto = GetChoice(ui->graph_item);
        do_move(j1, cg, j2, gto);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/*
 * swap a set with another set
 */
static void do_swap_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int j1, j2, gto, gfrom;

    Swap_ui* ui = (Swap_ui*)client_data;
    j1 = GetSelectedSet(ui->sel1);
    j2 = GetSelectedSet(ui->sel2);
    if (j1 == SET_SELECT_ERROR || j2 == SET_SELECT_ERROR) {
        errwin("Select 2 sets");
        return;
    }
    gfrom = (int)GetChoice(ui->graph1_item);
    gto = (int)GetChoice(ui->graph2_item);
    set_wait_cursor();
    set_work_pending(TRUE);
    do_swap(j1, gfrom, j2, gto);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    drawgraph();
}

/*
 * drop points from an active set
 */
static void do_drop_points_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i, *selsets;
    int cnt;
    int startno, endno, setno;
    Drop_ui* ui = (Drop_ui*)client_data;
    startno = atoi((char*)xv_getstr(ui->start_item)) - 1;
    endno = atoi((char*)xv_getstr(ui->stop_item)) - 1;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        setno = selsets[i];
        do_drop_points(setno, startno, endno);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/*
 * append one set to another
 */
static void do_join_sets_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int j1, j2;
    Join_ui* ui = (Join_ui*)client_data;
    j1 = GetSelectedSet(ui->sel1);
    j2 = GetSelectedSet(ui->sel2);
    if (j1 == SET_SELECT_ERROR || j2 == SET_SELECT_ERROR) {
        errwin("Select 2 sets");
        return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    do_join_sets(cg, j1, cg, j2);
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    drawgraph();
}

/*
 * reverse the order of a set
 */
static void do_reverse_sets_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int setno;
    int cnt, i, *selsets;
    Reverse_ui* ui = (Reverse_ui*)client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        setno = selsets[i];
        do_reverse_sets(setno);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/*
 * coalesce sets
 */
static void do_coalesce_sets_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int* selsets;
    int i, cnt;
    int setno;
    Coal_ui* ui = (Coal_ui*)client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        setno = selsets[i];
        do_coalesce_sets(setno);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/*
 * kill a set
 */
static void do_kill_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int* selsets;
    int i, cnt;
    int setno, soft;
    Kill_ui* ui = (Kill_ui*)client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    soft = (int)XmToggleButtonGetState(ui->soft_toggle);
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        setno = selsets[i];
        do_kill(cg, setno, soft);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    free(selsets);
    unset_wait_cursor();
    drawgraph();
}

/*
 sort sets
*/
static void do_sort_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int* selsets;
    int i, cnt;
    int setno, sorton, stype;
    Sort_ui* ui = (Sort_ui*)client_data;
    static int son[MAX_SET_COLS] = {DATA_X, DATA_Y, DATA_Y1, DATA_Y2, DATA_Y3, DATA_Y4};

    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    sorton = son[(int)GetChoice(ui->xy_item)];
    stype = (int)GetChoice(ui->up_down_item);

    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        setno = selsets[i];
        do_sort(setno, sorton, stype);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}

/*
 * split sets split by itmp, remainder in last set.
 */
static void do_split_sets_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int* selsets;
    int i, cnt;
    int setno, lpart;
    Split_ui* ui = (Split_ui*)client_data;
    cnt = GetSelectedSets(ui->sel, &selsets);
    if (cnt == SET_SELECT_ERROR) {
        errwin("No sets selected");
        return;
    }
    lpart = atoi((char*)xv_getstr(ui->len_item));
    set_wait_cursor();
    set_work_pending(TRUE);
    for (i = 0; i < cnt; i++) {
        setno = selsets[i];
        do_splitsets(cg, setno, lpart);
    }
    set_work_pending(FALSE);
    update_set_lists(cg);
    unset_wait_cursor();
    free(selsets);
    drawgraph();
}
