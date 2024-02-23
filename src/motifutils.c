/* $Id: motifutils.c,v 1.2 1995/04/18 15:21:26 pturner Exp pturner $
 *
 * utilities for Motif
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdlib.h>
#include <stdarg.h>

#include "protos.h"

#include <X11/X.h>
#include <X11/Xatom.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/Protocols.h>

#include "defines.h"
#include "globals.h"
#include "motifinc.h"

extern Widget app_shell;
extern XmStringCharSet charset;

extern XtAppContext app_con;

extern Widget canvas;
extern Display* disp;

extern int maxcolors;

void errwin(char* s);
void savewidget(Widget w);

/* This table has color names as gleaned from xvlib.c (PKL) */
static char* NumToColor[] = {
    "white",   /* 0 */
    "black",   /* 1 */
    "red",     /* 2 */
    "green",   /* 3 */
    "blue",    /* 4 */
    "yellow",  /* 5 */
    "brown",   /* 6 */
    "gray",    /* 7 */
    "violet",  /* 8 */
    "cyan",    /* 9 */
    "magenta", /* 10 */
    "orange",  /* 11 */
    "indigo",  /* 12 */
    "maroon",  /* 13 */
    "turqse",  /* 14 */
    "green4"   /* 15 */
};

void SetChoice(Widget* w, int value) {
    Arg a;

    if (w == (Widget*)NULL) {
        errwin("Internal error, SetChoice called with NULL argument");
        return;
    }
    if (w[value + 2] == (Widget)NULL) {
        errwin("Internal error SetChoice: Attempt to set NULL Widget");
        return;
    }
    XtSetArg(a, XmNmenuHistory, w[value + 2]);
    XtSetValues(w[0], &a, 1);
}

int GetChoice(Widget* w) {
    Arg a;
    Widget warg;
    int i;

    if (w == (Widget*)NULL) {
        errwin("Internal error, GetChoice called with NULL argument, returning 0");
        return 0;
    }
    XtSetArg(a, XmNmenuHistory, &warg);
    XtGetValues(w[0], &a, 1);
    i = 0;
    while (w[i + 2] != warg) {
        if (w[i + 2] == (Widget)NULL) {
            errwin("Internal error GetChoice: Found NULL in Widget list, returning 0");
            return 0;
        }
        i++;
    }
    return i;
}

Widget* CreatePanelChoice(Widget parent, char* labelstr, int nchoices, ...) {
    va_list var;
    int i = 0;
    XmString str;
    char* s;
    Widget* retval;

    nchoices--;

    retval = (Widget*)XtMalloc((nchoices + 2) * sizeof(Widget));

    retval[1] = XmCreatePulldownMenu(parent, "pulldown", NULL, 0);

    i = 0;
    va_start(var, nchoices);
    while ((s = va_arg(var, char*)) != NULL && i < nchoices) {
        retval[i + 2] = XmCreatePushButton(retval[1], s, NULL, 0);
        i++;
    }
    if (i != nchoices) {
        fprintf(stderr, "Incorrect number of selections in CreatePanelChoice()\n");
    }
    XtManageChildren(retval + 2, nchoices);
    va_end(var);

    str = XmStringCreate(labelstr, charset);

    retval[0] = XmCreateOptionMenu(parent, "optionmenu", NULL, 0);
    XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1, NULL);
    XtManageChild(retval[0]);
    for (i = 0; i < nchoices + 2; i++) {
        retval[i] = retval[i];
    }
    return retval;
}

Widget* CreatePanelChoice0(Widget parent, char* labelstr, int ncols, int nchoices, ...) {
    va_list var;
    int i = 0;
    XmString str;
    char* s;
    Widget* retval;

    nchoices--;

    retval = (Widget*)XtMalloc((nchoices + 2) * sizeof(Widget));

    retval[1] = XmCreatePulldownMenu(parent, "pulldown", NULL, 0);
    XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, ncols, NULL);

    i = 0;

    va_start(var, nchoices);
    while ((s = va_arg(var, char*)) != NULL && i < nchoices) {
        retval[i + 2] = XmCreatePushButton(retval[1], s, NULL, 0);
        i++;
    }
    if (i != nchoices) {
        fprintf(stderr, "Incorrect number of selections in CreatePanelChoice0()\n");
    }
    va_end(var);

    XtManageChildren(retval + 2, nchoices);

    str = XmStringCreate(labelstr, charset);

    retval[0] = XmCreateOptionMenu(parent, "optionmenu", NULL, 0);
    XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1, NULL);
    XtManageChild(retval[0]);
    for (i = 0; i < nchoices + 2; i++) {
        retval[i] = retval[i];
    }
    return retval;
}

Widget* CreateGraphChoice(Widget parent, char* labelstr, int ngraphs, int type) {
    int i = 0;
    XmString str;
    char *name, buf[32];
    Widget* retval = NULL;

    strcpy(buf, "graphchoice");
    name = buf;

    switch (type) {
    case 0:
        retval = (Widget*)XtMalloc((ngraphs + 2) * sizeof(Widget));
        retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, 4, NULL);
        i = 0;
        for (i = 0; i < ngraphs; i++) {
            sprintf(buf, "%d", i);
            retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
        }
        XtManageChildren(retval + 2, ngraphs);

        str = XmStringCreate(labelstr, charset);

        retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1,
                      NULL);
        XtManageChild(retval[0]);
        break;

    case 1:
        retval = (Widget*)XtMalloc((ngraphs + 3) * sizeof(Widget));
        retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, 4, NULL);
        retval[2] = XmCreatePushButton(retval[1], "Current", NULL, 0);
        for (i = 1; i <= ngraphs; i++) {
            sprintf(buf, "%d", i - 1);
            retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
        }
        XtManageChildren(retval + 2, ngraphs + 1);
        str = XmStringCreate(labelstr, charset);
        retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1,
                      NULL);
        XtManageChild(retval[0]);
        break;
    case 2:
        retval = (Widget*)XtMalloc((ngraphs + 4) * sizeof(Widget));
        retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, 4, NULL);
        retval[2] = XmCreatePushButton(retval[1], "Current", NULL, 0);
        for (i = 1; i <= ngraphs; i++) {
            sprintf(buf, "%d", i - 1);
            retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
        }
        retval[ngraphs + 3] = XmCreatePushButton(retval[1], "All", NULL, 0);

        XtManageChildren(retval + 2, ngraphs + 2);
        str = XmStringCreate(labelstr, charset);
        retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1,
                      NULL);
        XtManageChild(retval[0]);
        break;
    }
    return retval;
}

void UpdateFrameWinColors(unsigned color);
void UpdateLabelWinColors(unsigned color);
void UpdateMiscWinColors(unsigned color);
void UpdateStrWinColors(unsigned color);
void UpdateSymWinColors(unsigned color);
void UpdateTickWinColors(unsigned color);

void UpdateAllColorChoice(unsigned color) {
    UpdateFrameWinColors(color);
    UpdateLabelWinColors(color);
    UpdateMiscWinColors(color);
    UpdateStrWinColors(color);
    UpdateSymWinColors(color);
    UpdateTickWinColors(color);
}

void UpdateColorChoice(Widget* w, unsigned color) {
    extern unsigned long colors[];
    if (w) {
        XtVaSetValues(w[color + 2], XmNbackground, colors[color], NULL);
    }
}

Widget* CreateColorChoice(Widget parent, char* s, int map) {
    int nchoices, ncols, i = 0;
    XmString str;
    char *name, labelstr[32], buf[32];
    Widget* retval;
    extern unsigned long colors[];
    extern int use_colors;

    if (s == NULL) {
        strcpy(labelstr, "");
    } else {
        strcpy(labelstr, s);
    }

    ncols = (int)sqrt((double)maxcolors);
    if (ncols == 0) {
        ncols = 1;
    }
    nchoices = maxcolors;
    retval = (Widget*)XtMalloc((maxcolors + 2) * sizeof(Widget));

    strcpy(buf, "colorchoice");
    name = buf;
    retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
    XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, ncols, NULL);
    for (i = 0; i < maxcolors; i++) {
        if (i < 16) {
            strcpy(buf, NumToColor[i]);
        } else {
            sprintf(buf, "%2d", i);
        }
        retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
        if (use_colors > 2) {
            XtVaSetValues(retval[i + 2], XmNbackground, colors[i], NULL);
        }
    }
    XtManageChildren(retval + 2, nchoices);

    strcpy(buf, "coloroption");
    str = XmStringCreate(labelstr, charset);

    retval[0] = XmCreateOptionMenu(parent, buf, NULL, 0);
    XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1, NULL);
    XtManageChild(retval[0]);
    return retval;
}

Widget* CreateSetChoice(Widget parent, char* labelstr, int nsets, int type) {
    int nmal, i = 0;
    XmString str;
    char* name = "setchoice";
    char buf[10];
    Widget* retval = NULL;

    switch (type) {
    case 0:
        nmal = nsets + 2;
        retval = (Widget*)XtMalloc(nmal * sizeof(Widget));
        retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, nsets / 10,
                      NULL);
        i = 0;
        for (i = 0; i < nsets; i++) {
            sprintf(buf, "%d", i);
            retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
        }
        XtManageChildren(retval + 2, nsets);

        str = XmStringCreate(labelstr, charset);

        retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1,
                      NULL);
        XtManageChild(retval[0]);
        break;
    case 1:
        nmal = nsets + 3;
        retval = (Widget*)XtMalloc(nmal * sizeof(Widget));
        retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, nsets / 10,
                      NULL);
        i = 0;
        for (i = 0; i < nsets; i++) {
            sprintf(buf, "%d", i);
            retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
        }
        retval[nsets + 2] = XmCreatePushButton(retval[1], "All", NULL, 0);
        XtManageChildren(retval + 2, nsets + 1);

        str = XmStringCreate(labelstr, charset);

        retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1,
                      NULL);
        XtManageChild(retval[0]);
        break;
    case 2:
        retval = (Widget*)XtMalloc((nsets + 3) * sizeof(Widget));
        strcpy(buf, "setchoice");
        name = buf;
        retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, nsets / 10,
                      NULL);
        i = 0;
        for (i = 0; i < nsets; i++) {
            sprintf(buf, "%d", i);
            retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
        }
        retval[nsets + 2] = XmCreatePushButton(retval[1], "All", NULL, 0);
        XtManageChildren(retval + 2, nsets + 1);

        str = XmStringCreate(labelstr, charset);

        retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1,
                      NULL);
        XtManageChild(retval[0]);
        break;
        /* 4 is Next */
    case 4:
        retval = (Widget*)XtMalloc((nsets + 3) * sizeof(Widget));
        strcpy(buf, "setchoice");
        name = buf;
        retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, nsets / 10,
                      NULL);
        retval[2] = XmCreatePushButton(retval[1], "Next", NULL, 0);
        for (i = 1; i <= nsets; i++) {
            sprintf(buf, "%d", i - 1);
            retval[i + 2] = XmCreatePushButton(retval[1], buf, NULL, 0);
        }
        XtManageChildren(retval + 2, nsets + 1);

        str = XmStringCreate(labelstr, charset);

        retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1,
                      NULL);
        XtManageChild(retval[0]);
        break;
        /* 5 is Next, Same */
    case 6: /* All, then sets */
        nmal = nsets + 3;
        retval = (Widget*)XtMalloc(nmal * sizeof(Widget));
        retval[1] = XmCreatePulldownMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[1], XmNorientation, XmVERTICAL, XmNpacking, XmPACK_COLUMN, XmNnumColumns, nsets / 10,
                      NULL);
        retval[2] = XmCreatePushButton(retval[1], "All", NULL, 0);
        for (i = 0; i < nsets; i++) {
            sprintf(buf, "%d", i);
            retval[i + 3] = XmCreatePushButton(retval[1], buf, NULL, 0);
        }
        XtManageChildren(retval + 2, nsets + 1);

        str = XmStringCreate(labelstr, charset);

        retval[0] = XmCreateOptionMenu(parent, name, NULL, 0);
        XtVaSetValues(retval[0], XmNlabelString, str, XmNsubMenuId, retval[1], XmNentryBorder, 2, XmNwhichButton, 1,
                      NULL);
        XtManageChild(retval[0]);
        break;
    }
    return retval;
}

Widget CreateTextItem2(Widget parent, int len, char* s) {
    Widget w;
    Widget rc;
    XmString str;
    rc = XmCreateRowColumn(parent, "rc", NULL, 0);
    XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
    str = XmStringCreateLtoR(s, charset);
    XtVaCreateManagedWidget("label", xmLabelWidgetClass, rc, XmNlabelString, str, NULL);
    XmStringFree(str);
    w = XtVaCreateManagedWidget("text", xmTextWidgetClass, rc, XmNtraversalOn, True, XmNcolumns, len, NULL);
    XtManageChild(rc);
    return w;
}

Widget CreateTextItem4(Widget parent, int len, char* label) {
    Widget retval;
    XtVaCreateManagedWidget(label, xmLabelWidgetClass, parent, NULL);
    retval = XtVaCreateManagedWidget("text", xmTextWidgetClass, parent, NULL);
    XtVaSetValues(retval, XmNcolumns, len, NULL);
    return retval;
}

char* xv_getstr(Widget w) {
    char* s;
    static char buf[256];

    strcpy(buf, s = XmTextGetString(w));
    XtFree(s);
    return buf;
}

void xv_setstr(Widget w, char* s) { XmTextSetString(w, s); }

/*
 * set position
 */
void XmGetPos(Widget w, int type, int* x, int* y) {
    Arg a[4];
    int ac = 0;
    static int curpos = 0;
    Position xt, yt;
    Dimension wt, ht;

    /*
     * get the location of the whole app
     */
    ac = 0;
    XtSetArg(a[ac], XmNx, &xt);
    ac++;
    XtSetArg(a[ac], XmNy, &yt);
    ac++;
    XtSetArg(a[ac], XmNheight, &ht);
    ac++;
    XtSetArg(a[ac], XmNwidth, &wt);
    ac++;
    XtGetValues(w, a, ac);

    switch (type) {
    case 0: /* position at upper left */
        *x = xt - 50;
        *y = yt + ht / 4 + (curpos % 5) * 50;
        curpos++;
        if (*x < 0) {
            *x = 0;
        }
        if (*y < 0) {
            *y = 0;
        }
        break;
    case 1: /* position at upper right */
        *x = xt + 50 + wt;
        *y = yt + (curpos % 5 + 1) * 50;
        break;
    case 2: /* center on w */
        *x = xt + wt / 2;
        *y = yt + ht / 2;
        break;
    }
}

void GetWH(Widget w, Dimension* ww, Dimension* wh) {
    Arg args;

    XtSetArg(args, XmNwidth, ww);
    XtGetValues(w, &args, 1);
    XtSetArg(args, XmNheight, wh);
    XtGetValues(w, &args, 1);
}

void GetXY(Widget w, Position* x, Position* y) {
    Arg args;

    XtSetArg(args, XmNx, x);
    XtGetValues(w, &args, 1);
    XtSetArg(args, XmNy, y);
    XtGetValues(w, &args, 1);
}

/**********************************************************************
 * XtFlush - Flushes all Xt events.
 **********************************************************************/
void XtFlush(void) {
    XtAppContext app;

    app = XtWidgetToApplicationContext(app_shell);
    while (XtAppPending(app) & XtIMXEvent) {
        XtAppProcessEvent(app, XtIMXEvent);
        XFlush(XtDisplay(app_shell));
    }
}

/*
 * generic unmanage popup routine, used elswhere
 */
void destroy_dialog(Widget w, XtPointer p) { XtUnmanageChild((Widget)p); }

/*
 * handle the close item on the WM menu
 */
void handle_close(Widget w) {
    Atom WM_DELETE_WINDOW;
    XtVaSetValues(w, XmNdeleteResponse, XmDO_NOTHING, NULL);
    WM_DELETE_WINDOW = XmInternAtom(XtDisplay(app_shell), "WM_DELETE_WINDOW", False);
    XmAddProtocolCallback((Widget)w, (Atom)XM_WM_PROTOCOL_ATOM(w), (Atom)WM_DELETE_WINDOW,
                          (XtCallbackProc)destroy_dialog, (XtPointer)w);
}

/*
 * Manage and raise
 */
void XtRaise(Widget w) {
    XtManageChild(w);
    XMapRaised(XtDisplay(w), XtWindow(w));
    savewidget(w);
}

/*
 * save dialog widgets for cursor control
 */

typedef struct _SaveDialogState {
    Widget w;
    char* name;
    int open;
    int restore;
    int x, y;
    int width, height;
} SaveDialogState;

static Widget* savewidgets = NULL;
static int nsavedwidgets = 0;

void savewidget(Widget w) {
    int i;
    if (savewidgets == NULL) {
        savewidgets = (Widget*)malloc(sizeof(Widget));
    } else {
        for (i = 0; i < nsavedwidgets; i++) {
            if (w == savewidgets[i]) {
                return;
            }
        }
        savewidgets = (Widget*)realloc(savewidgets, (nsavedwidgets + 2) * sizeof(Widget));
    }
    savewidgets[nsavedwidgets] = w;
    nsavedwidgets++;
}

void savewidgetstate(Widget w) {
    int i;
    if (savewidgets == NULL) {
    } else {
        for (i = 0; i < nsavedwidgets; i++) {}
    }
}

void restorewidgetstate(Widget w) {
    int i;
    if (savewidgets == NULL) {
    } else {
        for (i = 0; i < nsavedwidgets; i++) {}
    }
}

void deletewidget(Widget w) {
    int i, j;
    if (savewidgets == NULL || nsavedwidgets == 0) {
        return;
    } else {
        /* find the widget */
        for (i = 0; i < nsavedwidgets; i++) {
            if (w == savewidgets[i]) {
                break;
            }
        }
        /* shouldn't happen, widget not in the saved widget list */
        if (i == nsavedwidgets) {
            return;
        }
        /* remove the widget from the list */
        for (j = i; j < nsavedwidgets - 1; j++) {
            savewidgets[j] = savewidgets[j + 1];
        }
        if (nsavedwidgets - 1 > 0) {
            savewidgets = (Widget*)realloc(savewidgets, (nsavedwidgets - 1) * sizeof(Widget));
            nsavedwidgets--;
        } else {
            free(savewidgets);
            savewidgets = NULL;
        }
    }
}

void DefineDialogCursor(Cursor c) {
    int i;
    for (i = 0; i < nsavedwidgets; i++) {
        XDefineCursor(disp, XtWindow(savewidgets[i]), c);
        XDefineCursor(disp, XtWindow(app_shell), c);
        /*
                attrs.cursor = c;
                XChangeWindowAttributes(disp, XtWindow(savewidgets[i]), CWCursor, &attrs);
        */
    }
    XFlush(disp);
}

void UndefineDialogCursor() {
    int i;
    for (i = 0; i < nsavedwidgets; i++) {
        XUndefineCursor(disp, XtWindow(savewidgets[i]));
        XUndefineCursor(disp, XtWindow(app_shell));
    }
    XFlush(disp);
}

void CreateCommandButtonsNoDefault(Widget parent, int n, Widget* buts, char** l) {
    int i;
    Widget form;
    Dimension h;

    form = XtVaCreateWidget("form", xmFormWidgetClass, parent, XmNfractionBase, n, NULL);

    for (i = 0; i < n; i++) {
        buts[i] = XtVaCreateManagedWidget(l[i], xmPushButtonWidgetClass, form, XmNtopAttachment, XmATTACH_FORM,
                                          XmNbottomAttachment, XmATTACH_FORM, XmNleftAttachment, XmATTACH_POSITION,
                                          XmNleftPosition, i, XmNrightAttachment, XmATTACH_POSITION, XmNrightPosition,
                                          i + 1, XmNleftOffset, (i == 0) ? 2 : 0, XmNrightOffset, 3, XmNtopOffset, 2,
                                          XmNbottomOffset, 3, NULL);
    }
    XtManageChild(form);
    XtVaGetValues(buts[0], XmNheight, &h, NULL);
    XtVaSetValues(form, XmNpaneMaximum, h, XmNpaneMinimum, h, NULL);
}

void CreateCommandButtons(Widget parent, int n, Widget* buts, char** l) {
    int i;
    Widget form;
    Dimension h;

    form = XtVaCreateWidget("form", xmFormWidgetClass, parent, XmNfractionBase, n, NULL);

    for (i = 0; i < n; i++) {
        buts[i] = XtVaCreateManagedWidget(l[i], xmPushButtonWidgetClass, form, XmNtopAttachment, XmATTACH_FORM,
                                          XmNbottomAttachment, XmATTACH_FORM, XmNleftAttachment, XmATTACH_POSITION,
                                          XmNleftPosition, i, XmNrightAttachment, XmATTACH_POSITION, XmNrightPosition,
                                          i + 1, XmNdefaultButtonShadowThickness, 1, XmNshowAsDefault,
                                          (i == 0) ? True : False, NULL);
    }
    XtManageChild(form);
    XtVaGetValues(buts[0], XmNheight, &h, NULL);
    XtVaSetValues(form, XmNpaneMaximum, h, XmNpaneMinimum, h, NULL);
}

static SetChoiceItem* plist = NULL;
static int nplist = 0;

int GetSetFromString(char* buf) {
    int retval = SET_SELECT_ERROR;
    if (strcmp(buf, "New set") == 0) {
        retval = SET_SELECT_NEXT;
    } else if (strcmp(buf, "All sets") == 0) {
        retval = SET_SELECT_ALL;
    } else {
        sscanf(buf, "S%d", &retval);
    }
    return retval;
}

int GetSelectedSet(SetChoiceItem l) {
    int retval = SET_SELECT_ERROR;
    int* pos_list;
    int pos_cnt, cnt;
    char buf[256];
    if (XmListGetSelectedPos(l.list, &pos_list, &pos_cnt)) {
        XmString *s, cs;
        char* cstr;
        XtVaGetValues(l.list, XmNselectedItemCount, &cnt, XmNselectedItems, &s, NULL);
        cs = XmStringCopy(*s);
        if (XmStringGetLtoR(cs, charset, &cstr)) {
            strcpy(buf, cstr);
            if (strcmp(buf, "New set") == 0) {
                retval = SET_SELECT_NEXT;
            } else if (strcmp(buf, "All sets") == 0) {
                retval = SET_SELECT_ALL;
            } else if (strcmp(buf, "Nearest set") == 0) {
                retval = SET_SELECT_NEAREST;
            } else {
                sscanf(buf, "S%d", &retval);
            }
            XtFree(cstr);
        }
        XmStringFree(cs);
    }
    return retval;
}

/* TODO */
int SetSelectedSet(int gno, int setno, SetChoiceItem l) {
    char buf[1024];
    XmString xms;
    sprintf(buf, "S%d (N=%d, %s)", setno, getsetlength(gno, setno), getcomment(gno, setno));
    xms = XmStringCreateLtoR(buf, charset);
    XmListSelectItem(l.list, xms, True);
    XmStringFree(xms);
    return 0;
}

/*
 * if the set selection type is multiple, then get a
 * list of sets, returns the number of selected sets.
 */
int GetSelectedSets(SetChoiceItem l, int** sets) {
    int i;
    int cnt = SET_SELECT_ERROR, retval = SET_SELECT_ERROR;
    int* ptr;
    int* pos_list;
    int pos_cnt, gno;
    if (XmListGetSelectedPos(l.list, &pos_list, &pos_cnt)) {
        char buf[256];
        char* cstr;
        XmString *s, cs;

        XtVaGetValues(l.list, XmNselectedItemCount, &cnt, XmNselectedItems, &s, NULL);
        *sets = (int*)malloc(cnt * sizeof(int));
        ptr = *sets;
        for (i = 0; i < cnt; i++) {
            cs = XmStringCopy(s[i]);
            if (XmStringGetLtoR(cs, charset, &cstr)) {
                strcpy(buf, cstr);
                if (strcmp(buf, "New set") == 0) {
                    retval = SET_SELECT_NEXT;
                    return retval;
                } else if (strcmp(buf, "All sets") == 0) {
                    int j, nsets = 0;
                    retval = SET_SELECT_ALL;
                    if (l.gno == GRAPH_SELECT_CURRENT) {
                        gno = cg;
                    } else {
                        gno = l.gno;
                    }
                    retval = nactive(gno);
                    *sets = (int*)realloc(*sets, retval * sizeof(int));
                    ptr = *sets;
                    for (j = 0; j < g[gno].maxplot; j++) {
                        if (isactive_set(gno, j)) {
                            ptr[nsets] = j;
                            nsets++;
                        }
                    }
                    if (nsets != retval) {
                        errwin("Nsets != reval, can't happen!");
                    }
                    return retval;
                } else {
                    sscanf(buf, "S%d", &retval);
                }
                ptr[i] = retval;
                /*
                        printf("S%d %d\n", retval, ptr[i]);
                */
                XtFree(cstr);
            }
            XmStringFree(cs);
        }
    }
    /*
        printf("Selected sets:");
        for (i = 0; i < cnt; i++) {
        printf(" %d", ptr[i]);
        }
        printf("\n");
    */
    return cnt;
}

void SetSelectorFilterCB(Widget w, XtPointer clientd, XtPointer calld) {
    SetChoiceItem* s = (SetChoiceItem*)clientd;
    if (XmToggleButtonGetState(w)) {
        int i;
        for (i = 0; i < 4; i++) {
            if (w == s->but[i]) {
                break;
            }
        }
        /* update the list in s->list */
        s->fflag = i + 1;
        plist[s->indx].fflag = i + 1;
        update_set_list(cg, *s);
    }
}

void DefineSetSelectorFilter(SetChoiceItem* s) {
    XtAddCallback(s->but[0], XmNvalueChangedCallback, (XtCallbackProc)SetSelectorFilterCB, (XtPointer)s);
    XtAddCallback(s->but[1], XmNvalueChangedCallback, (XtCallbackProc)SetSelectorFilterCB, (XtPointer)s);
    XtAddCallback(s->but[2], XmNvalueChangedCallback, (XtCallbackProc)SetSelectorFilterCB, (XtPointer)s);
    XtAddCallback(s->but[3], XmNvalueChangedCallback, (XtCallbackProc)SetSelectorFilterCB, (XtPointer)s);
}

SetChoiceItem CreateSetSelector(Widget parent, char* label, int type, int ff, int gtype, int stype) {
    Arg args[3];
    Widget rc, rc2, lab;
    SetChoiceItem sel;

    rc2 = XtVaCreateWidget("rc", xmRowColumnWidgetClass, parent, XmNorientation, XmVERTICAL, NULL);
    lab = XmCreateLabel(rc2, label, NULL, 0);
    XtManageChild(lab);
    XtSetArg(args[0], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);
    XtSetArg(args[1], XmNvisibleItemCount, 8);
    sel.list = XmCreateScrolledList(rc2, "list", args, 2);
    if (stype == SELECTION_TYPE_MULTIPLE) { /* multiple select */
        XtVaSetValues(sel.list, XmNselectionPolicy, XmEXTENDED_SELECT, NULL);
    } else { /* single select */
        XtVaSetValues(sel.list, XmNselectionPolicy, XmSINGLE_SELECT, NULL);
    }
    sel.type = type;
    sel.fflag = ff;
    sel.gno = gtype;
    XtManageChild(sel.list);
    sel.indx = save_set_list(sel);
    update_set_list(gtype == GRAPH_SELECT_CURRENT ? cg : sel.gno, sel);

    if (ff) { /* need a filter gadget */
        rc = XmCreateRowColumn(rc2, "rc", NULL, 0);
        XtVaSetValues(rc, XmNorientation, XmHORIZONTAL, NULL);
        lab = XmCreateLabel(rc, "Display:", NULL, 0);
        XtManageChild(lab);
        sel.rb = XmCreateRadioBox(rc, "rb", NULL, 0);
        XtVaSetValues(sel.rb, XmNorientation, XmHORIZONTAL, XmNpacking, XmPACK_TIGHT, NULL);
        sel.but[0] = XtVaCreateManagedWidget("Active", xmToggleButtonWidgetClass, sel.rb, XmNalignment,
                                             XmALIGNMENT_CENTER, XmNindicatorOn, False, XmNshadowThickness, 2, NULL);
        sel.but[1] = XtVaCreateManagedWidget("All", xmToggleButtonWidgetClass, sel.rb, XmNalignment, XmALIGNMENT_CENTER,
                                             XmNindicatorOn, False, XmNshadowThickness, 2, NULL);
        sel.but[2] = XtVaCreateManagedWidget("Inactive", xmToggleButtonWidgetClass, sel.rb, XmNalignment,
                                             XmALIGNMENT_CENTER, XmNindicatorOn, False, XmNshadowThickness, 2, NULL);
        sel.but[3] = XtVaCreateManagedWidget("Deact", xmToggleButtonWidgetClass, sel.rb, XmNalignment,
                                             XmALIGNMENT_CENTER, XmNindicatorOn, False, XmNshadowThickness, 2, NULL);
        XmToggleButtonSetState(sel.but[ff - 1], True, False);
        XtManageChild(sel.rb);
        XtManageChild(rc);
    }
    XtManageChild(rc2);
    return sel;
}

int save_set_list(SetChoiceItem l) {
    nplist++;
    if (plist == NULL) {
        plist = (SetChoiceItem*)malloc(nplist * sizeof(SetChoiceItem));
    } else {
        plist = (SetChoiceItem*)realloc(plist, nplist * sizeof(SetChoiceItem));
    }
    plist[nplist - 1] = l;
    return nplist - 1;
}

void update_save_set_list(SetChoiceItem l, int newgr) {
    plist[l.indx] = l;
    update_set_list(newgr, plist[l.indx]);
}

void update_set_list(int gno, SetChoiceItem l) {
    int i, cnt, scnt = 0;
    char buf[1024];
    XmString* xms;

    XmListDeleteAllItems(l.list);
    for (i = 0; i < g[gno].maxplot; i++) {
        switch (l.fflag) {
        case FILTER_SELECT_NONE: /* Active sets */
            if (isactive_set(gno, i)) {
                scnt++;
            }
            break;
        case FILTER_SELECT_ALL: /* All sets */
            scnt++;
            break;
        case FILTER_SELECT_ACTIVE: /* Active sets */
            if (isactive_set(gno, i)) {
                scnt++;
            }
            break;
        case FILTER_SELECT_INACT: /* Inactive sets */
            if (!isactive_set(gno, i)) {
                scnt++;
            }
            break;
        case FILTER_SELECT_DEACT: /* Deactivated sets */
            if (!isactive_set(gno, i) && g[gno].p[i].deact) {
                scnt++;
            }
            break;
        }
    }
    switch (l.type) { /* TODO */
    case SET_SELECT_ACTIVE:
        xms = (XmString*)malloc(sizeof(XmString) * scnt);
        cnt = 0;
        break;
    case SET_SELECT_ALL:
        xms = (XmString*)malloc(sizeof(XmString) * (scnt + 1));
        xms[0] = XmStringCreateLtoR("All sets", charset);
        cnt = 1;
        break;
    case SET_SELECT_NEXT:
        xms = (XmString*)malloc(sizeof(XmString) * (scnt + 1));
        xms[0] = XmStringCreateLtoR("New set", charset);
        cnt = 1;
        break;
    case SET_SELECT_NEAREST:
        xms = (XmString*)malloc(sizeof(XmString) * (scnt + 1));
        xms[0] = XmStringCreateLtoR("Nearest set", charset);
        cnt = 1;
        break;
    default:
        xms = (XmString*)malloc(sizeof(XmString) * scnt);
        cnt = 0;
        break;
    }

    for (i = 0; i < g[gno].maxplot; i++) {
        switch (l.fflag) {
        case FILTER_SELECT_NONE: /* Active sets */
            if (isactive_set(gno, i)) {
                sprintf(buf, "S%d (N=%d, %s)", i, getsetlength(gno, i), getcomment(gno, i));
                xms[cnt] = XmStringCreateLtoR(buf, charset);
                cnt++;
            }
            break;
        case FILTER_SELECT_ALL: /* All sets */
            if (isactive_set(gno, i)) {
                sprintf(buf, "S%d (N=%d, %s)", i, getsetlength(gno, i), getcomment(gno, i));
            } else if (g[gno].p[i].deact) {
                sprintf(buf, "S%d (DEACTIVATED)", i);
            } else {
                sprintf(buf, "S%d (INACTIVE)", i);
            }
            xms[cnt] = XmStringCreateLtoR(buf, charset);
            cnt++;
            break;
        case FILTER_SELECT_ACTIVE: /* Active sets */
            if (isactive_set(gno, i)) {
                sprintf(buf, "S%d (N=%d, %s)", i, getsetlength(gno, i), getcomment(gno, i));
                xms[cnt] = XmStringCreateLtoR(buf, charset);
                cnt++;
            }
            break;
        case FILTER_SELECT_INACT: /* Inactive sets */
            if (!isactive_set(gno, i)) {
                sprintf(buf, "S%d (INACTIVE)", i);
                xms[cnt] = XmStringCreateLtoR(buf, charset);
                cnt++;
            }
            break;
        case FILTER_SELECT_DEACT: /* Deactivated sets */
            if (!isactive_set(gno, i) && g[gno].p[i].deact) {
                sprintf(buf, "S%d (DEACTIVATED, N=%d, %s)", i, getsetlength(gno, i), getcomment(gno, i));
                xms[cnt] = XmStringCreateLtoR(buf, charset);
                cnt++;
            }
            break;
        }
    }
#if XmVersion > 1001
    XmListAddItemsUnselected(l.list, xms, cnt, 0);
#endif

    /* automatically highlight if only 1 selection */
    if (scnt == 1) {
        XmListSelectItem(l.list, xms[cnt - 1], True);
    }

    for (i = 0; i < cnt; i++) {
#if XmVersion < 1002 /* For Motif 1.1 */
        XmListAddItemUnselected(l.list, xms[i], 0);
#endif
        XmStringFree(xms[i]);
    }
    free(xms);
}

void update_set_lists(int gno) {
    int i;
    /*
    printf("dirty = %d pending = %d\n", lists_dirty(),work_pending());
    */
    if (inwin && lists_dirty() && !work_pending()) {
        for (i = 0; i < nplist; i++) {
            if (plist[i].gno == gno || (gno == cg && plist[i].gno == GRAPH_SELECT_CURRENT))
                update_set_list(gno, plist[i]);
        }
        set_lists_dirty(FALSE);
    }
}

void AddSetToLists(int gno, int setno) {
    if (nplist) {
        int i;
        XmString xms = NULL;
        char buf[256];
        if (isactive_set(gno, setno)) {
            sprintf(buf, "S%d (N=%d, %s)", setno, getsetlength(gno, setno), getcomment(gno, setno));
            xms = XmStringCreateLtoR(buf, charset);
        }
        for (i = 0; i < nplist; i++) {
            XmListAddItemUnselected(plist[i].list, xms, 0);
        }
        XmStringFree(xms);
    }
}

void UpdateSetInLists(int gno, int setno) {
    if (nplist) {
        int i;
        XmString xms = NULL;
        char buf[256];
        if (isactive_set(gno, setno)) {
            sprintf(buf, "S%d (N=%d, %s)", setno, getsetlength(gno, setno), getcomment(gno, setno));
            xms = XmStringCreateLtoR(buf, charset);
        }
        for (i = 0; i < nplist; i++) {
            XmListAddItemUnselected(plist[i].list, xms, 0);
        }
        XmStringFree(xms);
    }
}

Widget CreateMenuBar(Widget parent, char* name, char* help_anchor) {
    Widget menubar;

    menubar = XmCreateMenuBar(parent, name, NULL, 0);

    if (help_anchor) {
        XtAddCallback(menubar, XmNhelpCallback, (XtCallbackProc)HelpCB, (XtPointer)help_anchor);
    }

    return menubar;
}

Widget CreateMenu(Widget parent, char* name, char* label, char mnemonic, Widget* cascade, char* help_anchor) {
    Widget menu, cascadeTmp;
    XmString str;

    str = XmStringCreateSimple(label);
    menu = XmCreatePulldownMenu(parent, name, NULL, 0);
    cascadeTmp = XtVaCreateWidget((String)name, xmCascadeButtonWidgetClass, parent, XmNlabelString, str, XmNmnemonic,
                                  mnemonic, XmNsubMenuId, menu, (char*)NULL);
    XmStringFree(str);
    if (help_anchor) {
        XtAddCallback(menu, XmNhelpCallback, (XtCallbackProc)HelpCB, (XtPointer)help_anchor);
    }
    XtManageChild(cascadeTmp);
    if (cascade != NULL) {
        *cascade = cascadeTmp;
    }

    return menu;
}

Widget CreateMenuButton(Widget parent, char* name, char* label, char mnemonic, XtCallbackProc cb, XtPointer data,
                        char* help_anchor) {
    Widget button;
    XmString str;

    str = XmStringCreateSimple(label);
    button = XtVaCreateManagedWidget((String)name, xmPushButtonWidgetClass, parent, XmNlabelString, str, XmNmnemonic,
                                     mnemonic, NULL);
    XmStringFree(str);
    XtAddCallback(button, XmNactivateCallback, cb, data);
    if (help_anchor) {
        XtAddCallback(button, XmNhelpCallback, (XtCallbackProc)HelpCB, (XtPointer)help_anchor);
    }

    return button;
}

Widget CreateMenuToggle(Widget parent, char* name, char* label, char mnemonic, XtCallbackProc cb, XtPointer data,
                        char* help_anchor) {
    Widget button;
    XmString str;

    str = XmStringCreateSimple(label);
    button = XtVaCreateManagedWidget((String)name, xmToggleButtonWidgetClass, parent, XmNlabelString, str, XmNmnemonic,
                                     mnemonic, XmNvisibleWhenOff, True, XmNindicatorOn, True, NULL);
    XmStringFree(str);
    if (cb) {
        XtAddCallback(button, XmNvalueChangedCallback, cb, data);
    }
    if (help_anchor) {
        XtAddCallback(button, XmNhelpCallback, (XtCallbackProc)HelpCB, (XtPointer)help_anchor);
    }

    return button;
}

Widget CreateMenuSeparator(Widget parent, char* name) {
    Widget sep;

    sep = XmCreateSeparator(parent, name, NULL, 0);
    XtManageChild(sep);
    return sep;
}

static int yesno_retval = 0;
static Boolean keep_grab = True;

void infowin(char* s);

void yesnoCB(Widget w, Boolean* keep_grab, XmAnyCallbackStruct* reason) {
    int why = reason->reason;

    *keep_grab = False;
    XtRemoveGrab(XtParent(w));
    XtUnmanageChild(w);
    switch (why) {
    case XmCR_OK:
        yesno_retval = 1;
        /* process ok action */
        break;
    case XmCR_CANCEL:
        yesno_retval = 0;
        /* process cancel action */
        break;
    }
}

int yesnowin(char* msg, char* s1, char* s2, char* help_anchor) {
    Widget yesno_popup = NULL;
    XmString str;
    XEvent event;

    keep_grab = True;

    if (yesno_popup == NULL) {
        yesno_popup = XmCreateErrorDialog(app_shell, "warndlg", NULL, 0);
        if (msg != NULL) {
            str = XmStringCreateLtoR(msg, charset);
        } else {
            str = XmStringCreateLtoR("Warning", charset);
        }
        XtVaSetValues(yesno_popup, XmNmessageString, str, NULL);
        XmStringFree(str);

        if (s1 != NULL) {
            str = XmStringCreateLtoR(s1, charset);
        } else {
            str = XmStringCreateLtoR("OK", charset);
        }
        XtVaSetValues(yesno_popup, str, XmNokLabelString, NULL);
        XmStringFree(str);

        if (s2 != NULL) {
            str = XmStringCreateLtoR(s2, charset);
        } else {
            str = XmStringCreateLtoR("Cancel", charset);
        }
        XtVaSetValues(yesno_popup, str, XmNcancelLabelString, NULL);
        XmStringFree(str);

        XtAddCallback(yesno_popup, XmNokCallback, (XtCallbackProc)yesnoCB, (XtPointer)&keep_grab);
        XtAddCallback(yesno_popup, XmNcancelCallback, (XtCallbackProc)yesnoCB, (XtPointer)&keep_grab);

        if (help_anchor) {
            XtAddCallback(yesno_popup, XmNhelpCallback, (XtCallbackProc)HelpCB, (XtPointer)help_anchor);
            XtSetSensitive(XtNameToWidget(yesno_popup, "Help"), True);
        } else {
            XtSetSensitive(XtNameToWidget(yesno_popup, "Help"), False);
        }
        XtManageChild(yesno_popup);
    }
    XtRaise(yesno_popup);
    XtAddGrab(XtParent(yesno_popup), True, False);
    while (keep_grab || XtAppPending(app_con)) {
        XtAppNextEvent(app_con, &event);
        XtDispatchEvent(&event);
    }
    return yesno_retval;
}

void errwin(char* s) {
    XmString str;
    Widget error_popup = NULL;

    keep_grab = True;

    log_results(s);

    if (error_popup == NULL) {
        error_popup = XmCreateErrorDialog(app_shell, "errorDialog", NULL, 0);

        str = XmStringCreateLtoR(s, charset);
        XtVaSetValues(error_popup, XmNmessageString, str, NULL);
        XmStringFree(str);

        XtVaSetValues(error_popup, XmNdialogTitle, XmStringCreateLtoR("Error", charset), XmNdialogStyle,
                      XmDIALOG_APPLICATION_MODAL, NULL);
        XtAddCallback(error_popup, XmNhelpCallback, (XtCallbackProc)HelpCB, (XtPointer)NULL);
        XtUnmanageChild(XmMessageBoxGetChild(error_popup, XmDIALOG_CANCEL_BUTTON));
        XtUnmanageChild(XmMessageBoxGetChild(error_popup, XmDIALOG_HELP_BUTTON));
        XtManageChild(error_popup);
    }
    XtRaise(error_popup);
}
