/* $Id: comwin.c,v 1.1 1995/04/13 16:25:49 pturner Exp pturner $
 *
 * Command Panel
 *
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/Frame.h>
#include <Xm/FileSB.h>
#include <Xm/Command.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/Separator.h>
#include <Xm/List.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"

/*
 * Widget item declarations
 */

static Widget command;
static Widget comshell;
static Widget hl; /* command history list */

static void comcall(Widget w, XtPointer cd, XtPointer calld);
static void clear_history(Widget w, XtPointer client_data, XtPointer call_data);
static void replay_history(Widget w, XtPointer client_data, XtPointer call_data);
static void whist_apply_notify_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void add_com(Widget w, XtPointer client_data, XtPointer call_data);
static void replace_com(Widget w, XtPointer client_data, XtPointer call_data);
static void delete_com(Widget w, XtPointer client_data, XtPointer call_data);
static void move_com(Widget w, XtPointer client_data, XtPointer call_data);
void close_rhist_popup(Widget w, XtPointer client_data, XtPointer call_data);
void comwin_add_act(Widget w, XKeyEvent* e, String* p, Cardinal* c);
void comwin_delete_act(Widget w, XKeyEvent* e, String* p, Cardinal* c);
void comwin_down_act(Widget w, XKeyEvent* e, String* p, Cardinal* c);
void comwin_replace_act(Widget w, XKeyEvent* e, String* p, Cardinal* c);
void comwin_up_act(Widget w, XKeyEvent* e, String* p, Cardinal* c);
void create_rhist_popup(Widget w, XtPointer client_data, XtPointer call_data);
void create_whist_frame(Widget w, XtPointer client_data, XtPointer call_data);
void do_rhist_proc(Widget w, XtPointer client_data, XtPointer call_data);
void exec_cmd(char* s);
void open_command(Widget w, XtPointer client_data, XtPointer call_data);

extern XtAppContext app_con;

void comwin_add_act(Widget w, XKeyEvent* e, String* p, Cardinal* c) {
    add_com((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
}

void comwin_replace_act(Widget w, XKeyEvent* e, String* p, Cardinal* c) {
    replace_com((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
}

void comwin_delete_act(Widget w, XKeyEvent* e, String* p, Cardinal* c) {
    delete_com((Widget)NULL, (XtPointer)NULL, (XtPointer)NULL);
}

void comwin_up_act(Widget w, XKeyEvent* e, String* p, Cardinal* c) {
    move_com((Widget)NULL, (XtPointer)0, (XtPointer)NULL);
}

void comwin_down_act(Widget w, XKeyEvent* e, String* p, Cardinal* c) {
    move_com((Widget)NULL, (XtPointer)1, (XtPointer)NULL);
}

static XtActionsRec actions[] = {{"comwin_add", (XtActionProc)comwin_add_act},
                                 {"comwin_replace", (XtActionProc)comwin_replace_act},
                                 {"comwin_delete", (XtActionProc)comwin_delete_act},
                                 {"comwin_up", (XtActionProc)comwin_up_act},
                                 {"comwin_down", (XtActionProc)comwin_down_act}};

static char comwin_table[] = "#override\n\
    Shift ~Ctrl ~Meta ~Alt<Key>Return: comwin_add()\n\
    Shift Ctrl ~Meta ~Alt<Key>Return: comwin_replace()\n\
    Shift ~Ctrl ~Meta ~Alt<Key>osfBackSpace: comwin_delete()\n\
    Shift ~Ctrl ~Meta ~Alt<Key>osfUp: comwin_up()\n\
    Shift ~Ctrl ~Meta ~Alt<Key>osfDown: comwin_down()";

void exec_cmd(char* s) {
    static int errpos, errorcount;
    errorcount = errpos = 0;

    if (s == NULL) {
        return;
    }

    scanner(s, getx(cg, curset), gety(cg, curset), getsetlength(cg, curset), ax, bx, cx, dx, MAXARR, 0, curset,
            &errpos);
    if (errpos) {
        errorcount++;
    }
}

static void comcall(Widget w, XtPointer cd, XtPointer calld) {
    static int errpos, errorcount;
    static char val[256];
    char* ts;
    XmCommandCallbackStruct* s = (XmCommandCallbackStruct*)calld;
    errorcount = errpos = 0;
    XmStringGetLtoR(s->value, charset, &ts);
    strcpy(val, ts);
    XtFree(ts);
    scanner(val, getx(cg, curset), gety(cg, curset), getsetlength(cg, curset), ax, bx, cx, dx, MAXARR, 0, curset,
            &errpos);
    if (errpos) {
        errorcount++;
    }
}

static void delete_com(Widget w, XtPointer client_data, XtPointer call_data)
/* delete a entry from the history list */
{
    int npos, *pos;

    if (XmListGetSelectedPos(hl, &pos, &npos) == True) {
        XmListDeletePos(hl, pos[0]);
        free(pos);
    }
}

static void move_com(Widget w, XtPointer client_data, XtPointer call_data)
/* move a entry up(0) or down(1) in the history list */
{
    int npos, *pos, numit;
    XmString selit, *selitemlst;

    if (XmListGetSelectedPos(hl, &pos, &npos) == True) {
        XtVaGetValues(hl, XmNselectedItems, &selitemlst, XmNitemCount, &numit, NULL);
        selit = XmStringCopy(selitemlst[0]);
        XmListDeletePos(hl, pos[0]);
        if (client_data == 0) {
            pos[0]--;
        } else {
            if (pos[0] < numit) {
                pos[0]++;
            } else {
                pos[0] = 1;
            }
        }
        XmListAddItem(hl, selit, pos[0]);
        XmListSelectPos(hl, pos[0], False);
        XmStringFree(selit);
        free(pos);
    }
}

static void add_com(Widget w, XtPointer client_data, XtPointer call_data)
/*
 * copy the contents of the command line to thestory list without executing it
 */
{
    int npos, *pos, numit = -1, newpos;
    XmString comtxt;

    XtVaGetValues(command, XmNcommand, &comtxt, NULL);
    if (!XmStringEmpty(comtxt)) {
        if (XmListGetSelectedPos(hl, &pos, &npos) == True) {
            XtVaGetValues(hl, XmNitemCount, &numit, NULL);
            newpos = ++pos[0];
        } else {
            newpos = 0;
        }
        XmListAddItem(hl, comtxt, newpos);
        XmListSelectPos(hl, newpos, False);
        if (numit > 0) {
            free(pos);
        }
    }
    XmStringFree(comtxt);

    comtxt = XmStringCreateLocalized("");
    XmCommandSetValue(command, comtxt);
    XmStringFree(comtxt);
}

static void replace_com(Widget w, XtPointer client_data, XtPointer call_data)
/*
 * replace a entry in the history list with the command line
 * without executing it
 */
{
    int npos, *pos;
    XmString comtxt;

    XtVaGetValues(command, XmNcommand, &comtxt, NULL);
    if (!XmStringEmpty(comtxt) && XmListGetSelectedPos(hl, &pos, &npos) == True) {
        XmListDeletePos(hl, pos[0]);
        XmListAddItem(hl, comtxt, pos[0]);
        XmListSelectPos(hl, pos[0], False);
        free(pos);
        XmStringFree(comtxt);

        comtxt = XmStringCreateLocalized("");
        XmCommandSetValue(command, comtxt);
        XmStringFree(comtxt);
    }
}

static void clear_history(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;
    int ac = 0, hc;
    Arg al[5];
    Widget h = XmCommandGetChild(command, XmDIALOG_HISTORY_LIST);
    ac = 0;
    XtSetArg(al[ac], XmNhistoryItemCount, &hc);
    ac++;
    XtGetValues(command, al, ac);
    for (i = 0; i < hc; i++) {
        XmListDeletePos(h, 0);
    }
}

static void replay_history(Widget w, XtPointer client_data, XtPointer call_data) {
    static int errpos, errcount;
    char buf[256], *ts;
    int i;
    int ac = 0, hc;
    XmStringTable xmstrs;
    Arg al[5];
    ac = 0;
    XtSetArg(al[ac], XmNhistoryItems, &xmstrs);
    ac++;
    XtSetArg(al[ac], XmNhistoryItemCount, &hc);
    ac++;
    XtGetValues(command, al, ac);
    errcount = 0;
    for (i = 0; i < hc; i++) {
        XmStringGetLtoR(xmstrs[i], charset, &ts);
        strcpy(buf, ts);
        XtFree(ts);

        errpos = 0;
        scanner(buf, getx(cg, curset), gety(cg, curset), getsetlength(cg, curset), ax, bx, cx, dx, MAXARR, 0, curset,
                &errpos);
        if (errpos) {
            errcount++;
        }
        if (errcount > 3) {
            if (yesno("Lots of errors, cancel?", NULL, NULL, NULL)) {
                break;
            } else {
                errcount = 0;
            }
        }
    }
}

void open_command(Widget w, XtPointer client_data, XtPointer call_data) {
    Widget form, fr1, fr2, but[6];
    int x, y;
    char* labrow1[5] = {"Add", "Delete", "Replace", "Up", "Down"};
    char* labrow2[6] = {"Read...", "Save...", "Clear", "Replay", "Close", "Help"};
    set_wait_cursor();
    if (command == NULL) {
        XmGetPos(app_shell, 0, &x, &y);
        comshell = XmCreateDialogShell(app_shell, "Commands", NULL, 0);
        handle_close(comshell);
        XtVaSetValues(comshell, XmNx, x, XmNy, y, NULL);
        command = XmCreateCommand(comshell, "command", NULL, 0);
        hl = XmCommandGetChild(command, XmDIALOG_HISTORY_LIST);
        XtVaSetValues(command, XmNpromptString, XmStringCreateLtoR("Command", charset), NULL);

        form = XmCreateForm(command, "commandform", NULL, 0);

        fr1 = XmCreateFrame(form, "commandframe1", NULL, 0);
        XtVaSetValues(fr1, XmNtopAttachment, XmATTACH_FORM, XmNleftAttachment, XmATTACH_FORM, XmNrightAttachment,
                      XmATTACH_FORM, NULL);
        CreateCommandButtonsNoDefault(fr1, 5, but, labrow1);
        XtAddCallback(but[0], XmNactivateCallback, (XtCallbackProc)add_com, (XtPointer)NULL);
        XtAddCallback(but[1], XmNactivateCallback, (XtCallbackProc)delete_com, (XtPointer)NULL);
        XtAddCallback(but[2], XmNactivateCallback, (XtCallbackProc)replace_com, (XtPointer)NULL);
        XtAddCallback(but[3], XmNactivateCallback, (XtCallbackProc)move_com, (XtPointer)0);
        XtAddCallback(but[4], XmNactivateCallback, (XtCallbackProc)move_com, (XtPointer)1);

        fr2 = XmCreateFrame(form, "commandframe2", NULL, 0);
        XtVaSetValues(fr2, XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, fr1, XmNleftAttachment, XmATTACH_FORM,
                      XmNrightAttachment, XmATTACH_FORM, XmNbottomAttachment, XmATTACH_FORM, NULL);
        CreateCommandButtonsNoDefault(fr2, 6, but, labrow2);
        XtAddCallback(but[0], XmNactivateCallback, (XtCallbackProc)create_rhist_popup, (XtPointer)NULL);
        XtAddCallback(but[1], XmNactivateCallback, (XtCallbackProc)create_whist_frame, (XtPointer)NULL);
        XtAddCallback(but[2], XmNactivateCallback, (XtCallbackProc)clear_history, (XtPointer)NULL);
        XtAddCallback(but[3], XmNactivateCallback, (XtCallbackProc)replay_history, (XtPointer)NULL);
        XtAddCallback(but[4], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)comshell);
        XtAddCallback(but[5], XmNactivateCallback, (XtCallbackProc)HelpCB, (XtPointer) "data.html#commands");

        XtAddCallback(command, XmNcommandEnteredCallback, (XtCallbackProc)comcall, (XtPointer)NULL);
        XtManageChild(fr1);
        XtManageChild(fr2);
        XtManageChild(form);
        XtManageChild(command);
        XtManageChild(comshell);
        XtAppAddActions(app_con, actions, XtNumber(actions));
        XtOverrideTranslations(XmCommandGetChild(command, XmDIALOG_COMMAND_TEXT),
                               XtParseTranslationTable(comwin_table));
    }
    XtRaise(comshell);
    unset_wait_cursor();
}

static Widget rhist_dialog;

void close_rhist_popup(Widget w, XtPointer client_data, XtPointer call_data) { XtUnmanageChild(rhist_dialog); }

void do_rhist_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    char buf[512];
    char* s;
    int sl;
    FILE* fp;
    XmString list_item;
    Widget h = XmCommandGetChild(command, XmDIALOG_HISTORY_LIST);
    XmFileSelectionBoxCallbackStruct* cbs = (XmFileSelectionBoxCallbackStruct*)call_data;
    if (!XmStringGetLtoR(cbs->value, charset, &s)) {
        errwin("Error converting XmString to char string in rdata_proc()");
        return;
    }

    strcpy(buf, s);
    XtFree(s);
    if ((fp = fopen(buf, "r")) != NULL) {
        while (fgets(buf, 255, fp) != NULL) {
            sl = strlen(buf);
            buf[sl - 1] = 0;
            if (strlen(buf) == 0) {
                continue;
            }
            list_item = XmStringCreateLtoR(buf, charset);
            XmListAddItemUnselected(h, list_item, 0);
            XmStringFree(list_item);
        }
        fclose(fp);
    } else {
        errwin("Unable to open file");
    }
    XtUnmanageChild(rhist_dialog);
}

void create_rhist_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    set_wait_cursor();
    if (rhist_dialog == NULL) {
        rhist_dialog = XmCreateFileSelectionDialog(app_shell, "Read history", NULL, 0);
        XtAddCallback(rhist_dialog, XmNcancelCallback, (XtCallbackProc)close_rhist_popup, (XtPointer)NULL);
        XtAddCallback(rhist_dialog, XmNokCallback, (XtCallbackProc)do_rhist_proc, (XtPointer)NULL);
        XtManageChild(rhist_dialog);
    }
    XtRaise(rhist_dialog);
    unset_wait_cursor();
}

/*
 * Panel item declarations
 */
static Widget whist_frame;
static Widget whist_panel;
static Widget whist_text_item;

/*
 * Create the whist Frame and the whist Panel
 */
void create_whist_frame(Widget w, XtPointer client_data, XtPointer call_data) {
    int x, y;
    set_wait_cursor();
    if (whist_frame == NULL) {
        Widget buts[2];
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Cancel";
        XmGetPos(app_shell, 0, &x, &y);
        whist_frame = XmCreateDialogShell(app_shell, "Write history", NULL, 0);
        handle_close(whist_frame);
        XtVaSetValues(whist_frame, XmNx, x, XmNy, y, NULL);
        whist_panel = XmCreateRowColumn(whist_frame, "whist_rc", NULL, 0);

        whist_text_item = CreateTextItem2(whist_panel, 30, "Write history to:");

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, whist_panel, NULL);

        CreateCommandButtons(whist_panel, 2, buts, label1);
        XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)whist_apply_notify_proc, (XtPointer)NULL);
        XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)whist_frame);
        XtManageChild(whist_panel);
    }
    XtRaise(whist_frame);
    unset_wait_cursor();
}

static void whist_apply_notify_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i, ac = 0, hc;
    char s[256], *ts;
    XmStringTable xmstrs;
    Arg al[5];
    strcpy(s, (char*)xv_getstr(whist_text_item));
    if (!fexists(s)) {
        FILE* pp = fopen(s, "w");
        if (pp != NULL) {
            ac = 0;
            XtSetArg(al[ac], XmNhistoryItems, &xmstrs);
            ac++;
            XtSetArg(al[ac], XmNhistoryItemCount, &hc);
            ac++;
            XtGetValues(command, al, ac);
            for (i = 0; i < hc; i++) {
                XmStringGetLtoR(xmstrs[i], charset, &ts);
                fprintf(pp, "%s\n", ts);
                XtFree(ts);
            }
            fclose(pp);
        } else {
            errwin("Unable to open file");
        }
    }
    XtUnmanageChild(whist_frame);
}
