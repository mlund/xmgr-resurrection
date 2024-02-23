/* $Id: symwin.c,v 1.2 1995/04/18 15:21:26 pturner Exp pturner $
 *
 * symbols, legends, and error bars
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"
#include "draw.h"

int cset = 0; /* the current set from the symbols panel */

static Widget define_symbols_frame;

static Widget define_legend_frame;

static Widget* toggle_symbols_item;
static Widget symchar_item;
static Widget symsize_item;
static Widget* symfill_item;
static Widget symskip_item;
static Widget* symcolor_item;
static Widget* symlinew_item;
static Widget* symlines_item;
static Widget* symbols_apply_item;
static Widget* toggle_color_item;
static Widget* toggle_width_item;
static Widget* toggle_lines_item;
static Widget* toggle_fill_item;
static Widget* toggle_fillusing_item;
static Widget* toggle_fillpat_item;
static Widget* toggle_fillcol_item;
static SetChoiceItem toggle_symset_item;

static Widget define_errbar_frame;
static Widget errbar_size_item;
static Widget* errbar_width_item;
static Widget* errbar_lines_item;
static Widget* errbar_type_item;
static Widget* errbar_riser_item;
static Widget* errbar_riserlinew_item;
static Widget* errbar_riserlines_item;
static Widget* errbar_apply_item;

Widget legend_x_panel; /* needed in the canvas event proc */
Widget legend_y_panel;
static Widget toggle_legends_item;
static Widget* toggle_legendloc_item;
static Widget legend_str_panel;
static Widget* legends_gap_item;
static Widget* legends_len_item;
static Widget* legend_font_item;
static Widget legend_charsize_item;
static Widget* legend_color_item;
static Widget legend_box_item;
static Widget legend_boxfill_item;
static Widget* legend_boxfillusing_item;
static Widget* legend_boxfillcolor_item;
static Widget* legend_boxfillpat_item;
static Widget* legend_boxlinew_item;
static Widget* legend_boxlines_item;
static Widget* legend_boxcolor_item;
static Widget legend_pstex_width_item;

static void define_symbols();
static void define_symbols_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void setall_colors_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void setall_sym_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void setall_linew_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void set_cset_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void define_errbar_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void accept_ledit_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void update_ledit_items(int gno);
static void create_ledit_frame(Widget w, XtPointer client_data, XtPointer call_data);

/*
 * define symbols for the current set
 */
static void define_symbols() {
    int sym, symchar, symskip, symfill, symcolor, symlinew, symlines;
    int line, pen, wid, fill, fillusing, fillpat, fillcol, i;
    double symsize;
    char s[30];
    int value;
    Arg a;
    int *selset, cd;

    XtSetArg(a, XmNvalue, &value);
    XtGetValues(symsize_item, &a, 1);
    symsize = value / 100.0;
    sym = (int)GetChoice(toggle_symbols_item);
    pen = (int)GetChoice(toggle_color_item);
    wid = (int)GetChoice(toggle_width_item);
    line = (int)GetChoice(toggle_lines_item);
    fill = (int)GetChoice(toggle_fill_item);
    fillusing = (int)GetChoice(toggle_fillusing_item) ? PTNFILLED : CLRFILLED;
    fillpat = (int)GetChoice(toggle_fillpat_item);
    fillcol = (int)GetChoice(toggle_fillcol_item);
    symskip = atoi((char*)xv_getstr(symskip_item));
    symfill = (int)GetChoice(symfill_item);
    symcolor = (int)GetChoice(symcolor_item);
    symlinew = (int)GetChoice(symlinew_item);
    symlines = (int)GetChoice(symlines_item);
    strcpy(s, (char*)xv_getstr(symchar_item));
    symchar = s[0];
    cd = GetSelectedSets(toggle_symset_item, &selset);
    if (cd == SET_SELECT_ERROR) {
        errwin("No set selected");
        return;
    } else {
        for (i = 0; i < cd; i++) {
            cset = selset[i];
            g[cg].p[cset].symskip = symskip;
            g[cg].p[cset].symsize = symsize;
            g[cg].p[cset].symchar = symchar;
            g[cg].p[cset].symfill = symfill;
            g[cg].p[cset].symlinew = symlinew;
            g[cg].p[cset].symlines = symlines;
            g[cg].p[cset].fill = fill;
            g[cg].p[cset].fillusing = fillusing;
            g[cg].p[cset].fillpattern = fillpat;
            g[cg].p[cset].fillcolor = fillcol;
            strcpy(g[cg].p[cset].lstr, (char*)xv_getstr(legend_str_panel));
            setplotsym(cg, cset, sym);
            setplotlines(cg, cset, line);
            setplotlinew(cg, cset, wid);
            setplotcolor(cg, cset, pen);
            setplotsymcolor(cg, cset, symcolor);
        }
    }
    updatesymbols(cg, cset);
    set_dirtystate();
    drawgraph();
}

static void define_symbols_proc(Widget w, XtPointer client_data, XtPointer call_data) { define_symbols(); }

/*
 * define colors incrementally
 */
static void setall_colors_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;

    for (i = 0; i < g[cg].maxplot; i++) {
        if (isactive_set(cg, i)) {
            setplotcolor(cg, i, (i % 15) + 1);
            setplotsymcolor(cg, i, (i % 15) + 1);
        }
    }
    updatesymbols(cg, cset);
    drawgraph();
}

/*
 * define symbols incrementally mod 10
 */
static void setall_sym_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;

    for (i = 0; i < g[cg].maxplot; i++) {
        if (isactive_set(cg, i)) {
            setplotsym(cg, i, (i % 10) + 2);
        }
    }
    updatesymbols(cg, cset);
    drawgraph();
}

/*
 * define linewidths incrementally mod 7
 */
static void setall_linew_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;

    for (i = 0; i < g[cg].maxplot; i++) {
        if (isactive_set(cg, i)) {
            setplotlinew(cg, i, (i % 7) + 1);
        }
    }
    updatesymbols(cg, cset);
    drawgraph();
}

/*
 * define line styles incrementally mod 5
 */
static void setall_linesty_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;

    for (i = 0; i < g[cg].maxplot; i++) {
        if (isactive_set(cg, i)) {
            setplotlines(cg, i, (i % 5) + 1);
        }
    }
    updatesymbols(cg, cset);
    drawgraph();
}

/*
 * make all lines black
 */
static void blackwhite(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;

    for (i = 0; i < g[cg].maxplot; i++) {
        if (isactive_set(cg, i)) {
            setplotcolor(cg, i, 1);
            setplotsymcolor(cg, i, 1);
        }
    }
    updatesymbols(cg, cset);
    drawgraph();
}

/*
 * freshen up symbol items, generally after a parameter
 * file has been read
 */
void updatesymbols(int gno, int value) {
    Arg a;
    int iv;
    char s[2], val[24];

    if (define_symbols_frame && (cset == value) && (value != -1)) {
        iv = 100.0 * g[gno].p[value].symsize;
        XtSetArg(a, XmNvalue, iv);
        XtSetValues(symsize_item, &a, 1);
        if (value < maxplot) {
            /*
                    SetChoice(toggle_symset_item, value);
            */
        }
        sprintf(val, "%d", g[gno].p[value].symskip);
        xv_setstr(symskip_item, val);
        SetChoice(symfill_item, g[gno].p[value].symfill);
        if (g[gno].p[value].symchar > ' ' && g[gno].p[value].symchar < 127) {
            s[0] = g[gno].p[value].symchar;
            s[1] = 0;
        } else {
            s[0] = 0;
        }
        xv_setstr(symchar_item, s);
        SetChoice(toggle_symbols_item, getsetplotsym(gno, value));

        if (symcolorbug || (getsetplotsymcolor(gno, value) == -1)) {
            setplotsymcolor(gno, value, getsetcolor(gno, value));
        }
        SetChoice(symcolor_item, getsetplotsymcolor(gno, value));

        SetChoice(symlinew_item, g[gno].p[value].symlinew);
        SetChoice(symlines_item, g[gno].p[value].symlines);
        SetChoice(toggle_color_item, getsetcolor(gno, value));
        SetChoice(toggle_width_item, getsetlinew(gno, value));
        SetChoice(toggle_lines_item, getsetlines(gno, value));
        SetChoice(toggle_fill_item, g[gno].p[value].fill);
        SetChoice(toggle_fillusing_item, g[gno].p[value].fillusing == CLRFILLED ? 0 : 1);
        SetChoice(toggle_fillcol_item, g[gno].p[value].fillcolor);
        SetChoice(toggle_fillpat_item, g[gno].p[value].fillpattern);
        updatelegendstr(gno);
        update_ledit_items(gno);
        updateerrbar(gno, value);
    }
}

void change_colors_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    if (define_symbols_frame) {
        SetChoice(symcolor_item, GetChoice(toggle_color_item));
    }
}

static void set_set_selection_type(Widget w, XtPointer client_data, XtPointer call_data) {
    int seltype;

    seltype = GetChoice(symbols_apply_item);
    if (!seltype) {
        XtVaSetValues(toggle_symset_item.list, XmNselectionPolicy, XmSINGLE_SELECT, NULL);
    } else {
        XtVaSetValues(toggle_symset_item.list, XmNselectionPolicy, XmEXTENDED_SELECT, NULL);
    }
    update_set_list(cg, toggle_symset_item);
}

static void set_cset_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int seltype = GetChoice(symbols_apply_item);
    int cd;

    if (!seltype) { /* single select */
        cd = GetSelectedSet(toggle_symset_item);
        if (cd != SET_SELECT_ERROR) {
            cset = cd;
            updatesymbols(cg, cd);
        }
    } else {
        int* selsets;
        cd = GetSelectedSets(toggle_symset_item, &selsets);
        if (cd != SET_SELECT_ERROR) {
            cset = selsets[0];
            updatesymbols(cg, selsets[0]);
        }
    }
}

void set_symbols_set(int gno, int setno) {
    int cd = SetSelectedSet(gno, setno, toggle_symset_item);
    if (cd != SET_SELECT_ERROR) {
        cset = setno;
        updatesymbols(cg, setno);
    } else {
    }
}

/*
 * legends
 */

/*
 * freshen up legend items, generally after a parameter
 * file has been read
 */

void updatelegendstr(int gno) {
    if (define_symbols_frame) {
        xv_setstr(legend_str_panel, g[gno].p[cset].lstr);
    }
}

/*
 * create the symbols popup
 */
void define_symbols_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    Widget dialog, wbut, fr, rc, rc2, rc3;
    Widget wlabel;
    Widget buts[4];
    int i;

    set_wait_cursor();
    if (define_symbols_frame == NULL) {
        char* label1[4];
        label1[0] = "Accept";
        label1[1] = "Error bars...";
        label1[2] = "Legends...";
        label1[3] = "Close";

        define_symbols_frame = XmCreateDialogShell(app_shell, "Symbols/legends", NULL, 0);
        handle_close(define_symbols_frame);
        dialog = XmCreateRowColumn(define_symbols_frame, "symbols_rc", NULL, 0);

        toggle_symset_item = CreateSetSelector(dialog, "Select set:", SET_SELECT_ALL, FILTER_SELECT_NONE,
                                               GRAPH_SELECT_CURRENT, SELECTION_TYPE_SINGLE);

        XtVaSetValues(toggle_symset_item.list, XmNselectionPolicy, XmSINGLE_SELECT, NULL);

        XtAddCallback(toggle_symset_item.list, XmNsingleSelectionCallback, (XtCallbackProc)set_cset_proc, (XtPointer)0);

        XtAddCallback(toggle_symset_item.list, XmNmultipleSelectionCallback, (XtCallbackProc)set_cset_proc,
                      (XtPointer)0);

        symbols_apply_item = CreatePanelChoice(dialog, "Select type:", 3, "Single", "Multiple", NULL, NULL);
        XtAddCallback(symbols_apply_item[2], XmNactivateCallback, (XtCallbackProc)set_set_selection_type,
                      (XtPointer)NULL);
        XtAddCallback(symbols_apply_item[3], XmNactivateCallback, (XtCallbackProc)set_set_selection_type,
                      (XtPointer)NULL);

        rc2 = XmCreateRowColumn(dialog, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);

        fr = XtVaCreateManagedWidget("symframe", xmFrameWidgetClass, rc2, NULL);
        rc = XtVaCreateWidget("symbolsbb", xmRowColumnWidgetClass, fr, NULL);
        toggle_symbols_item = CreatePanelChoice0(rc, " ", 4, 47, "No symbol", /* 0 */
                                                 "Dot",                       /* 1 */
                                                 "Circle",                    /* 2 */
                                                 "Square",                    /* 3 */
                                                 "Diamond",                   /* 4 */
                                                 "Triangle up",               /* 5 */
                                                 "Triangle left",             /* 6 */
                                                 "Triangle down",             /* 7 */
                                                 "Triangle right",            /* 8 */
                                                 "Plus",                      /* 9 */
                                                 "X",                         /* 10 */
                                                 "Star",                      /* 11 */
                                                 "Impulse at X",              /* 12 */
                                                 "Impulse at Y",              /* 13 */
                                                 "Vert line at X",            /* 14 */
                                                 "Horiz line at Y",           /* 15 */
                                                 "Histogram X",               /* 16 */
                                                 "Histogram Y",               /* 17 */
                                                 "Stair step X",              /* 18 */
                                                 "Stair step Y",              /* 19 */
                                                 "Bar X",                     /* 20 */
                                                 "Bar Y",                     /* 21 */
                                                 "Range",                     /* 22 */
                                                 "Index",                     /* 23 */
                                                 "Set #",                     /* 24 */
                                                 "Set #, index",              /* 25 */
                                                 "Bar and whisker (n/a)",     /* 26 */
                                                 "Segments",                  /* 27 */
                                                 "Character",                 /* 28 */
                                                 "Tag first point",           /* 29 */
                                                 "Tag last point",            /* 30 */
                                                 "Tag center point",          /* 31 */
                                                 "String (n/a)",              /* 32 */
                                                 "Hi low X",                  /* 33 */
                                                 "Hi low Y",                  /* 34 */
                                                 "Open/close X",              /* 35 */
                                                 "Open/close Y",              /* 36 */
                                                 "Box plot X",                /* 37 */
                                                 "Box plot Y",                /* 38 */
                                                 "Average Y",                 /* 39 */
                                                 "Average Y+-1*std",          /* 40 */
                                                 "Average Y+-2*std",          /* 41 */
                                                 "Average Y+-3*std",          /* 42 */
                                                 "Location X",                /* 43 */
                                                 "Location Y",                /* 44 */
                                                 "Location (X,Y)",            /* 45 */
                                                 0, 0);

        symfill_item = CreatePanelChoice(rc, "Sym fill:", 4, "None", "Filled", "Opaque", NULL, NULL);

        wlabel = XtVaCreateManagedWidget("Sym size:", xmLabelWidgetClass, rc, NULL);
        symsize_item = XtVaCreateManagedWidget("SymSize", xmScaleWidgetClass, rc, XmNminimum, 0, XmNmaximum, 800,
                                               XmNvalue, 100, XmNshowValue, True, XmNprocessingDirection,
                                               XmMAX_ON_RIGHT, XmNorientation, XmHORIZONTAL, NULL);
        symcolor_item = CreateColorChoice(rc, "Color:", 0);

        symchar_item = CreateTextItem2(rc, 2, "Sym char:");
        symskip_item = CreateTextItem2(rc, 5, "Sym skip:");

        symlines_item = CreatePanelChoice(rc, "Style:", 7, "None", "Solid line", "Dotted line", "Dashed line",
                                          "Long Dashed", "Dot-dashed", NULL, 0);
        symlinew_item =
            CreatePanelChoice(rc, "Width:", 11, "None", "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL, 0);
        XtManageChild(rc);

        rc3 = XmCreateRowColumn(rc2, "rc", NULL, 0);
        fr = XtVaCreateManagedWidget("lineframe", xmFrameWidgetClass, rc3, NULL);
        rc = XtVaCreateWidget("linesrc", xmRowColumnWidgetClass, fr, NULL);
        wlabel = XtVaCreateManagedWidget("Line properties:", xmLabelWidgetClass, rc, NULL);
        toggle_lines_item = CreatePanelChoice(rc, "Style:", 7, "None", "Solid line", "Dotted line", "Dashed line",
                                              "Long Dashed", "Dot-dashed", NULL, 0);
        toggle_width_item =
            CreatePanelChoice(rc, "Width:", 11, "None", "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL, 0);
        toggle_color_item = CreateColorChoice(rc, "Color:", 0);
        for (i = 0; i < MAXCOLORS; i++) {
            XtAddCallback(toggle_color_item[2 + i], XmNactivateCallback, (XtCallbackProc)change_colors_proc, NULL);
        }

        XtManageChild(rc);

        fr = XtVaCreateManagedWidget("fillframe", xmFrameWidgetClass, rc3, NULL);
        rc = XtVaCreateWidget("fillsbb", xmRowColumnWidgetClass, fr, NULL);
        wlabel = XtVaCreateManagedWidget("Fill properties:", xmLabelWidgetClass, rc, NULL);
        toggle_fill_item = CreatePanelChoice(rc, "Fill: ", 9, "None", "As polygon", "To Y=0.0", "To X=0.0", "To X min",
                                             "To X max", "To Y min", "To Y max", 0, 0);
        toggle_fillusing_item = CreatePanelChoice(rc, "Using:", 3, "Colors", "Patterns", 0, 0);
        toggle_fillpat_item = CreatePanelChoice0(rc, "Pattern:", 4, 17, "0", "1", "2", "3", "4", "5", "6", "7", "8",
                                                 "9", "10", "11", "12", "13", "14", "15", NULL, 0);
        toggle_fillcol_item = CreateColorChoice(rc, "Color:", 0);
        XtManageChild(rc);
        XtManageChild(rc3);
        XtManageChild(rc2);

        legend_str_panel = CreateTextItem2(dialog, 30, "Legend:");

        rc2 = XmCreateRowColumn(dialog, "rc", NULL, 0);
        XtVaSetValues(rc2, XmNorientation, XmHORIZONTAL, NULL);
        wbut = XtVaCreateManagedWidget("All colors", xmPushButtonWidgetClass, rc2, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)setall_colors_proc, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("All symbols", xmPushButtonWidgetClass, rc2, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)setall_sym_proc, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("All widths", xmPushButtonWidgetClass, rc2, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)setall_linew_proc, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("All line styles", xmPushButtonWidgetClass, rc2, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)setall_linesty_proc, (XtPointer)0);

        wbut = XtVaCreateManagedWidget("B/W", xmPushButtonWidgetClass, rc2, NULL);
        XtAddCallback(wbut, XmNactivateCallback, (XtCallbackProc)blackwhite, (XtPointer)0);

        XtManageChild(rc2);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

        CreateCommandButtons(dialog, 4, buts, label1);

        XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)define_symbols_proc, (XtPointer)0);
        XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc)define_errbar_popup, (XtPointer)0);
        XtAddCallback(buts[2], XmNactivateCallback, (XtCallbackProc)define_legend_popup, (XtPointer)0);
        XtAddCallback(buts[3], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)define_symbols_frame);

        XtManageChild(dialog);
    }
    XtRaise(define_symbols_frame);
    set_symbols_set(cg, cset);
    updatesymbols(cg, cset);
    unset_wait_cursor();
}

/*
 * legend popup
 */
void updatelegends(int gno) {
    Arg a;
    int iv;

    if (define_legend_frame) {
        iv = 100.0 * g[gno].l.charsize;
        XtSetArg(a, XmNvalue, iv);
        XtSetValues(legend_charsize_item, &a, 1);
        XmToggleButtonSetState(toggle_legends_item, g[gno].l.active == TRUE, False);
        sprintf(buf, "%.9g", g[gno].l.legx);
        xv_setstr(legend_x_panel, buf);
        sprintf(buf, "%.9g", g[gno].l.legy);
        xv_setstr(legend_y_panel, buf);
        SetChoice(legends_gap_item, g[gno].l.vgap - 1);
        SetChoice(legends_len_item, g[gno].l.len);
        SetChoice(toggle_legendloc_item, g[gno].l.loctype == COORD_VIEW);
        SetChoice(legend_font_item, g[gno].l.font);
        SetChoice(legend_color_item, g[gno].l.color);
        XmToggleButtonSetState(legend_box_item, g[gno].l.box == TRUE, False);
        XmToggleButtonSetState(legend_boxfill_item, g[gno].l.boxfill == TRUE, False);
        SetChoice(legend_boxfillusing_item, g[gno].l.boxfillusing == PTNFILLED);
        SetChoice(legend_boxfillcolor_item, g[gno].l.boxfillcolor);
        SetChoice(legend_boxfillpat_item, g[gno].l.boxfillpat);
        SetChoice(legend_boxcolor_item, g[gno].l.boxlcolor);
        SetChoice(legend_boxlinew_item, g[gno].l.boxlinew - 1);
        SetChoice(legend_boxlines_item, g[gno].l.boxlines - 1);
        XtSetArg(a, XmNvalue, g[gno].l.hgap);
        XtSetValues(legend_pstex_width_item, &a, 1);
    }
}

/*
 * define legends for the current graph
 */
void define_legends_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    Arg a;
    char val[80];
    int value;

    if (define_legend_frame) {
        XtSetArg(a, XmNvalue, &value);
        XtGetValues(legend_charsize_item, &a, 1);
        g[cg].l.charsize = value / 100.0;
        g[cg].l.active = XmToggleButtonGetState(toggle_legends_item) ? TRUE : FALSE;
        g[cg].l.vgap = GetChoice(legends_gap_item) + 1;
        g[cg].l.len = (int)GetChoice(legends_len_item);
        g[cg].l.loctype = (int)GetChoice(toggle_legendloc_item) ? COORD_VIEW : COORD_WORLD;
        strcpy(val, (char*)xv_getstr(legend_x_panel));
        g[cg].l.legx = atof(val);
        strcpy(val, (char*)xv_getstr(legend_y_panel));
        g[cg].l.legy = atof(val);
        g[cg].l.font = (int)GetChoice(legend_font_item);
        g[cg].l.color = (int)GetChoice(legend_color_item);
        g[cg].l.box = XmToggleButtonGetState(legend_box_item) ? TRUE : FALSE;
        g[cg].l.boxfill = XmToggleButtonGetState(legend_boxfill_item) ? TRUE : FALSE;
        g[cg].l.boxfillusing = (int)GetChoice(legend_boxfillusing_item) ? PTNFILLED : CLRFILLED;
        g[cg].l.boxfillcolor = (int)GetChoice(legend_boxfillcolor_item);
        g[cg].l.boxfillpat = (int)GetChoice(legend_boxfillpat_item);
        g[cg].l.boxlcolor = (int)GetChoice(legend_boxcolor_item);
        g[cg].l.boxlinew = (int)GetChoice(legend_boxlinew_item) + 1;
        g[cg].l.boxlines = (int)GetChoice(legend_boxlines_item) + 1;
        XtSetArg(a, XmNvalue, &value);
        XtGetValues(legend_pstex_width_item, &a, 1);
        g[cg].l.hgap = value;
        update_ledit_items(cg);
    }
    drawgraph();
    set_dirtystate();
}

/*
 * strip leading pathname from comments
 */
void strip_leg_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;

    for (i = 0; i < g[cg].maxplot; i++) {
        if (isactive_set(cg, i)) {
            strcpy(g[cg].p[i].lstr, mybasename(g[cg].p[i].lstr));
        }
    }
    update_ledit_items(cg);
}

/*
 * activate the legend location flag
 */
void legend_loc_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    if (define_legend_frame) {
        g[cg].l.loctype = (int)GetChoice(toggle_legendloc_item) ? COORD_VIEW : COORD_WORLD;
    }
    set_action(0);
    set_action(LEG_LOC);
    set_dirtystate();
}

/*
 * load legend strings from set comments
 */
void legend_load_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i;

    for (i = 0; i < g[cg].maxplot; i++) {
        if (isactive_set(cg, i)) {
            strcpy(g[cg].p[i].lstr, g[cg].p[i].comments);
        }
    }
    update_ledit_items(cg);
    set_dirtystate();
}

/*
 * create the legend popup
 */
void define_legend_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    Widget panel, fr, rc, rc0, rc1;
    int x, y;
    Widget buts[4];
    set_wait_cursor();
    if (define_legend_frame == NULL) {
        char* label1[4];
        label1[0] = "Accept";
        label1[1] = "Place";
        label1[2] = "Edit...";
        label1[3] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        define_legend_frame = XmCreateDialogShell(app_shell, "Legends", NULL, 0);
        handle_close(define_legend_frame);
        XtVaSetValues(define_legend_frame, XmNx, x, XmNy, y, NULL);
        panel = XmCreateRowColumn(define_legend_frame, "legend_rc", NULL, 0);

        rc0 = XmCreateRowColumn(panel, "rc0", NULL, 0);
        XtVaSetValues(rc0, XmNorientation, XmHORIZONTAL, NULL);

        fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, rc0, NULL);
        rc = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        toggle_legends_item = XtVaCreateManagedWidget("Display legend", xmToggleButtonWidgetClass, rc, NULL);

        toggle_legendloc_item = CreatePanelChoice(rc, "Locate in:", 3, "World coords", "Viewport coords", 0, 0);
        legend_font_item = CreatePanelChoice(rc, "Font:", 11, "Times-Roman", "Times-Bold", "Times-Italic",
                                             "Times-BoldItalic", "Helvetica", "Helvetica-Bold", "Helvetica-Oblique",
                                             "Helvetica-BoldOblique", "Greek", "Symbol", 0, 0);

        XtVaCreateManagedWidget("Char size:", xmLabelWidgetClass, rc, NULL);

        legend_charsize_item = XtVaCreateManagedWidget("charsize", xmScaleWidgetClass, rc, XmNminimum, 0, XmNmaximum,
                                                       400, XmNvalue, 100, XmNshowValue, True, XmNprocessingDirection,
                                                       XmMAX_ON_RIGHT, XmNorientation, XmHORIZONTAL, NULL);
        legend_color_item = CreateColorChoice(rc, "Color:", 0);

        legends_gap_item = CreatePanelChoice(rc, "Legend gap:", 5, "1", "2", "3", "4", 0, 0);
        legends_len_item =
            CreatePanelChoice(rc, "Legend length:", 10, "0", "1", "2", "3", "4", "5", "6", "7", "8", 0, 0);
        legend_x_panel = CreateTextItem2(rc, 10, "X location:");
        legend_y_panel = CreateTextItem2(rc, 10, "Y location:");
        XtManageChild(fr);
        XtManageChild(rc);

        rc1 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rc0, NULL);
        fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, rc1, NULL);
        rc = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        legend_box_item = XtVaCreateManagedWidget("Legend frame", xmToggleButtonWidgetClass, rc, NULL);
        legend_boxcolor_item = CreateColorChoice(rc, "Line color:", 0);
        legend_boxlinew_item =
            CreatePanelChoice(rc, "Line width:", 10, "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL, NULL);
        legend_boxlines_item = CreatePanelChoice(rc, "Line style:", 6, "Solid", "Dotted", "Dashed", "Long Dashed",
                                                 "Dot-dashed", NULL, NULL);
        XtManageChild(fr);
        XtManageChild(rc);

        fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, rc1, NULL);
        rc = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        legend_boxfill_item = XtVaCreateManagedWidget("Fill frame", xmToggleButtonWidgetClass, rc, NULL);
        legend_boxfillusing_item = CreatePanelChoice(rc, "Fill with:", 3, "Color", "Pattern", NULL, NULL);
        legend_boxfillcolor_item = CreateColorChoice(rc, "Color:", 0);

        legend_boxfillpat_item = CreatePanelChoice0(rc, "Pattern:", 4, 17, "0", "1", "2", "3", "4", "5", "6", "7", "8",
                                                    "9", "10", "11", "12", "13", "14", "15", NULL, 0);
        XtManageChild(fr);
        XtManageChild(rc);

        fr = XtVaCreateManagedWidget("frame", xmFrameWidgetClass, rc1, NULL);
        rc = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, fr, NULL);
        XtVaCreateManagedWidget("PSTeX legend width:", xmLabelWidgetClass, rc, NULL);
        legend_pstex_width_item = XtVaCreateManagedWidget(
            "Legend width (pstex):", xmScaleWidgetClass, rc, XmNminimum, 0, XmNmaximum, 2000, XmNvalue, 200,
            XmNshowValue, True, XmNprocessingDirection, XmMAX_ON_RIGHT, XmNorientation, XmHORIZONTAL, NULL);
        XtManageChild(fr);
        XtManageChild(rc);

        XtManageChild(rc0);

        CreateCommandButtons(panel, 4, buts, label1);
        XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)define_legends_proc, (XtPointer)0);
        XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc)legend_loc_proc, (XtPointer)0);
        XtAddCallback(buts[2], XmNactivateCallback, (XtCallbackProc)create_ledit_frame, (XtPointer)0);
        XtAddCallback(buts[3], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)define_legend_frame);

        XtManageChild(panel);
    }
    XtRaise(define_legend_frame);
    updatelegends(cg);
    unset_wait_cursor();
}

/*
 * define errbars for the current set
 */
static void define_errbar_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i, itmp, applyto, nstart, nstop;
    Arg a;
    int value;

    applyto = GetChoice(errbar_apply_item);
    if (applyto) {
        nstart = 0;
        nstop = g[cg].maxplot - 1;
    } else {
        nstart = nstop = cset;
    }
    for (i = nstart; i <= nstop; i++) {

        XtSetArg(a, XmNvalue, &value);
        XtGetValues(errbar_size_item, &a, 1);
        g[cg].p[i].errbarper = value / 100.0;

        itmp = (int)GetChoice(errbar_type_item);
        switch (dataset_type(cg, i)) {
        case SET_XYDX:
        case SET_XYDXDX:
            if (itmp == 0) {
                itmp = PLACE_BOTH;
                ;
            } else if (itmp == 1) {
                itmp = PLACE_LEFT;
            } else {
                itmp = PLACE_RIGHT;
            }
            break;
        case SET_XYDY:
        case SET_XYDYDY:
            if (itmp == 0) {
                itmp = PLACE_BOTH;
                ;
            } else if (itmp == 1) {
                itmp = PLACE_TOP;
            } else {
                itmp = PLACE_BOTTOM;
            }
            break;
        default:
            itmp = PLACE_BOTH;
            break;
        }

        g[cg].p[i].errbarxy = itmp;
        g[cg].p[i].errbar_linew = (int)GetChoice(errbar_width_item) + 1;
        g[cg].p[i].errbar_lines = (int)GetChoice(errbar_lines_item) + 1;
        g[cg].p[i].errbar_riser = (int)GetChoice(errbar_riser_item) ? TRUE : FALSE;
        g[cg].p[i].errbar_riser_linew = (int)GetChoice(errbar_riserlinew_item) + 1;
        g[cg].p[i].errbar_riser_lines = (int)GetChoice(errbar_riserlines_item) + 1;
    }
    drawgraph();
    set_dirtystate();
}

/*
 */
void updateerrbar(int gno, int value) {
    int itmp = 0;
    Arg a;
    int iv;

    if (value == -1) {
        value = cset;
    }
    if (define_errbar_frame && cset == value) {

        iv = (int)(100.0 * g[gno].p[value].errbarper);
        XtSetArg(a, XmNvalue, iv);
        XtSetValues(errbar_size_item, &a, 1);

        switch (g[gno].p[value].errbarxy) {
        case PLACE_BOTH:
            itmp = 0;
            break;
        case PLACE_TOP:
        case PLACE_LEFT:
            itmp = 1;
            break;
        case PLACE_BOTTOM:
        case PLACE_RIGHT:
            itmp = 2;
            break;
        }
        SetChoice(errbar_type_item, itmp);
        SetChoice(errbar_width_item, g[gno].p[value].errbar_linew - 1);
        SetChoice(errbar_lines_item, g[gno].p[value].errbar_lines - 1);
        SetChoice(errbar_riser_item, g[gno].p[value].errbar_riser == TRUE ? 1 : 0);
        SetChoice(errbar_riserlinew_item, g[gno].p[value].errbar_riser_linew - 1);
        SetChoice(errbar_riserlines_item, g[gno].p[value].errbar_riser_lines - 1);
    }
}

/*
 * create the errbar popup
 */
void define_errbar_popup(Widget w, XtPointer client_data, XtPointer call_data) {
    Widget panel;
    int x, y;
    Widget buts[2];
    set_wait_cursor();
    if (define_errbar_frame == NULL) {
        char* label1[2];
        label1[0] = "Accept";
        label1[1] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        define_errbar_frame = XmCreateDialogShell(app_shell, "Error bars", NULL, 0);
        handle_close(define_errbar_frame);
        XtVaSetValues(define_errbar_frame, XmNx, x, XmNy, y, NULL);
        panel = XmCreateRowColumn(define_errbar_frame, "errbar_rc", NULL, 0);

        XtVaCreateManagedWidget("Size:", xmLabelWidgetClass, panel, NULL);
        errbar_size_item = XtVaCreateManagedWidget("Size", xmScaleWidgetClass, panel, XmNminimum, 0, XmNmaximum, 400,
                                                   XmNvalue, 100, XmNshowValue, True, XmNprocessingDirection,
                                                   XmMAX_ON_RIGHT, XmNorientation, XmHORIZONTAL, NULL);

        errbar_width_item =
            CreatePanelChoice(panel, "Line width:", 10, "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL, NULL);
        errbar_lines_item = CreatePanelChoice(panel, "Line style:", 6, "Solid", "Dotted", "Dashed", "Long Dashed",
                                              "Dot-dashed", NULL, NULL);
        errbar_riser_item = CreatePanelChoice(panel, "Riser:", 3, "OFF", "ON", NULL, NULL);
        errbar_riserlinew_item =
            CreatePanelChoice(panel, "Riser line width:", 10, "1", "2", "3", "4", "5", "6", "7", "8", "9", NULL, NULL);
        errbar_riserlines_item = CreatePanelChoice(panel, "Riser line style:", 6, "Solid", "Dotted", "Dashed",
                                                   "Long Dashed", "Dot-dashed", NULL, NULL);
        errbar_type_item = CreatePanelChoice(panel, "Display:", 4, "Both", "Top/left", "Bottom/right", NULL, NULL);
        errbar_apply_item = CreatePanelChoice(panel, "Apply to:", 3, "selected set", "all sets", NULL, NULL);

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

        CreateCommandButtons(panel, 2, buts, label1);
        XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)define_errbar_proc, (XtPointer)0);
        XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)define_errbar_frame);

        XtManageChild(panel);
    }
    updateerrbar(cg, cset);
    XtRaise(define_errbar_frame);
    unset_wait_cursor();
}

static Widget ledit_frame;
static Widget leglabel;      /* */
static Widget leg_list_item; /* */
static int labsetno;
static int labcursel = 1;

static void accept_ledit_proc(Widget w, XtPointer client_data, XtPointer call_data) {
    int i, cnt, setno;
    XmStringTable xmstr;
    char *cstr, *lab, name[512];
    XtVaGetValues(leg_list_item, XmNitemCount, &cnt, XmNitems, &xmstr, NULL);
    for (i = 0; i < cnt; i++) {
        if (XmStringGetLtoR(xmstr[i], charset, &cstr)) {
            strcpy(name, cstr);
            sscanf(name, "S%d", &setno);
            lab = strchr(name, ':');
            lab += 2; /* skip the : and ' ' */
            /*printf("Set %d: %s\n", setno, lab); */
            strcpy(g[cg].p[setno].lstr, lab);
            XtFree(cstr);
        }
    }
    updatelegendstr(cg);
    drawgraph();
    set_dirtystate();
}

static void update_ledit_items(int gno) {
    char buf[512];
    int i;
    XmString xms;

    if (ledit_frame) {
        XmListDeleteAllItems(leg_list_item);
        for (i = 0; i < g[cg].maxplot; i++) {
            if (isactive_set(cg, i)) {
                sprintf(buf, "S%0d: %s", i, g[gno].p[i].lstr);
                xms = XmStringCreateLtoR(buf, charset);
                XmListAddItemUnselected(leg_list_item, xms, 0);
                XmStringFree(xms);
            }
        }
    }
}

static void set_leglab(Widget w, XtPointer client_data, XtPointer call_data) {
    char name[512], *lab;
    XmString *s, cs;
    int* pos_list;
    int pos_cnt, cnt;
    char* cstr;
    XmListCallbackStruct* cbs = (XmListCallbackStruct*)call_data;

    labcursel = cbs->item_position;
    if (XmListGetSelectedPos(leg_list_item, &pos_list, &pos_cnt)) {
        XtVaGetValues(leg_list_item, XmNselectedItemCount, &cnt, XmNselectedItems, &s, NULL);
        cs = XmStringCopy(*s);
        if (XmStringGetLtoR(cs, charset, &cstr)) {
            strcpy(name, cstr);
            sscanf(name, "S%d", &labsetno);
            lab = strchr(name, ':');
            lab += 2; /* skip the : and ' ' */
            xv_setstr(leglabel, lab);
            XtFree(cstr);
            XmStringFree(cs);
        }
    }
}

extern XtAppContext app_con;

void leglab_down(Widget w, XKeyEvent* e, String* p, Cardinal* c) {
    int numitems;

    XtVaGetValues(leg_list_item, XmNitemCount, &numitems, NULL);
    if (++labcursel > numitems)
        labcursel = 1;
    XmListSelectPos(leg_list_item, labcursel, True);
}

void leglab_up(Widget w, XKeyEvent* e, String* p, Cardinal* c) {
    int numitems;

    XtVaGetValues(leg_list_item, XmNitemCount, &numitems, NULL);
    if (--labcursel == 0)
        labcursel = numitems;
    XmListSelectPos(leg_list_item, labcursel, True);
}

static XtActionsRec actions[] = {
    {"leglab_down", (XtActionProc)leglab_down},
    {"leglab_up", (XtActionProc)leglab_up},
};

static char leglab_table[] = "#override\n\
	 <Key>osfUp: leglab_up()\n\
	 <Key>osfDown: leglab_down()";

static void tCB() {
    char buf[512];
    XmString cstr;
    int numitems;

    sprintf(buf, "S%0d: %s", labsetno, xv_getstr(leglabel));
    cstr = XmStringCreateLtoR(buf, charset);
    XmListReplaceItemsPos(leg_list_item, &cstr, 1, labcursel);
    /* TODO free cstr? */
    XtVaGetValues(leg_list_item, XmNitemCount, &numitems, NULL);
    if (++labcursel > numitems)
        labcursel = 1;
    XmListSelectPos(leg_list_item, labcursel, True);
}

static void create_ledit_frame(Widget w, XtPointer client_data, XtPointer call_data) {
    Widget panel;
    int x, y;
    Widget buts[4];
    Arg args[10];
    set_wait_cursor();
    if (ledit_frame == NULL) {
        char* label1[4];
        label1[0] = "Accept";
        label1[1] = "Load comments";
        label1[2] = "Strip labels";
        label1[3] = "Close";
        XmGetPos(app_shell, 0, &x, &y);
        ledit_frame = XmCreateDialogShell(app_shell, "Edit legend labels", NULL, 0);
        handle_close(ledit_frame);
        XtVaSetValues(ledit_frame, XmNx, x, XmNy, y, NULL);
        panel = XmCreateRowColumn(ledit_frame, "ledit_rc", NULL, 0);

        XtSetArg(args[0], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);
        XtSetArg(args[1], XmNvisibleItemCount, 5);
        XtSetArg(args[2], XmNselectionPolicy, XmSINGLE_SELECT);
        leg_list_item = XmCreateScrolledList(panel, "list", args, 3);
        XtAddCallback(leg_list_item, XmNsingleSelectionCallback, set_leglab, (XtPointer)0);
        XtManageChild(leg_list_item);

        leglabel = CreateTextItem2(panel, 35, "Label (<Enter> to record):");
        XtAddCallback(leglabel, XmNactivateCallback, (XtCallbackProc)tCB, (XtPointer)0);
        /* up and down arrow functioning */
        XtAppAddActions(app_con, actions, XtNumber(actions));
        XtOverrideTranslations(leglabel, XtParseTranslationTable(leglab_table));

        XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, panel, NULL);

        CreateCommandButtons(panel, 4, buts, label1);
        XtAddCallback(buts[0], XmNactivateCallback, (XtCallbackProc)accept_ledit_proc, (XtPointer)0);
        XtAddCallback(buts[1], XmNactivateCallback, (XtCallbackProc)legend_load_proc, (XtPointer)0);
        XtAddCallback(buts[2], XmNactivateCallback, (XtCallbackProc)strip_leg_proc, (XtPointer)ledit_frame);
        XtAddCallback(buts[3], XmNactivateCallback, (XtCallbackProc)destroy_dialog, (XtPointer)ledit_frame);

        XtManageChild(panel);
    }
    XtRaise(ledit_frame);
    update_ledit_items(cg);
    XmListSelectPos(leg_list_item, labcursel = 1, True);
    unset_wait_cursor();
}

void UpdateSymWinColors(unsigned color) {
    UpdateColorChoice(symcolor_item, color);
    UpdateColorChoice(toggle_color_item, color);
    UpdateColorChoice(toggle_fillcol_item, color);
    UpdateColorChoice(legend_color_item, color);
    UpdateColorChoice(legend_boxcolor_item, color);
    UpdateColorChoice(legend_boxfillcolor_item, color);
}
