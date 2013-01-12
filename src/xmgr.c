/* $Id: xmgr.c,v 1.3 1995/06/04 20:00:26 pturner Exp pturner $
 *
 * main loop
 *
 * Has Motif and X specific variable declarations
 *
 */

#include <config.h>

#ifdef VMS
#  include <unixlib.h>
#  include <unixio.h>
#  include "vms_unix.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/keysym.h>
#include <X11/StringDefs.h>

#ifdef WITH_EDITRES
#include <X11/Xmu/Editres.h>
#endif

#include <Xm/Xm.h>
#include <Xm/ArrowB.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/BulletinB.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/ToggleB.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>
#if XmVersion >= 1002
#  include <Xm/RepType.h>
#endif

#if defined(HAVE_XPM_H)
#  include <xpm.h>
#else
#  if defined (HAVE_X11_XPM_H)
#    include <X11/xpm.h>
#  else
int XpmCreatePixmapFromData (Display *display,
                             Drawable d,
                             char **data,
                             Pixmap *pixmap_return,
                             Pixmap *shapemask_return,
                             char *attributes);
#  endif
#endif

#include "globals.h"
#include "protos.h"
#include "motifinc.h"
#include "bitmaps.h"

void create_workingdir_popup(Widget w, XtPointer client_data, XtPointer call_data);

#ifdef DRAGnDROP
/* testing drag n drop */
#include <Xm/DragDrop.h>
void (*drop_proc) ();
Cardinal numImportTargets;
Atom FILE_CONTENTS, FILE_NAME;
Atom *importTargets, *newTargets;
Atom importList[2];
void HandleDropLabel(), HandleDropText();
void init_dragndrop(void);
#endif

/*
 * used to set up XmStrings
 * Seems to be some problems under AIX, the #ifdef is supposed to
 * take care of the problem.
 */
#ifdef XmFONTLIST_DEFAULT_TAG
XmStringCharSet charset = (XmStringCharSet) XmFONTLIST_DEFAULT_TAG;
#else
XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
#endif

XtAppContext app_con;

/* used globally */
Widget app_shell;
Widget canvas;

static Widget scrollw;		/* container for drawing area */

Widget loclab;			/* locator label */
Widget statlab;			/* status line at the bottom */
Widget stack_depth_item;	/* stack depth item on the main panel */
Widget curw_item;		/* current world stack item on the main panel */
XmString string;		/* string for current location */
XmString sdstring;		/* string for stack depth */
XmString cystring;		/* string for stack cycle */
XmString statstring;		/* string for pointer status */

extern Colormap mycmap;		/* colormap for canvas */
extern Display *disp;
extern GC gc;
extern GC gcxor;
extern GC gcclr;
extern XGCValues gc_val;
extern Window xwin;
extern unsigned long colors[];

/* used locally */
static Widget main_frame;
static Widget menu_bar;
static Widget frleft, frtop, frbot;	/* dialogs along canvas edge */
static Widget form;		/* form for mainwindow */

static void MenuCB(Widget w, XtPointer client_data, XtPointer call_data);
static Widget CreateMainMenuBar(Widget parent);
static void init_pm(Widget wref);
static void set_pipetimer(void);

/*
 * for buttons on front panel
 */

static Pixmap zoompm, shrinkpm, expandpm, autopm;
static Pixmap uppm, leftpm, downpm, rightpm;

/*
 * establish resource stuff
 */
typedef struct {
    Boolean invert;
    Boolean revflag;
    Boolean backingstore;
    Boolean allow_dc;
    Boolean autoscale_onread;
    Boolean verify_action;
    Boolean auto_redraw;
    Boolean status_auto_redraw;
    Boolean allow_refresh;
    int maxplot;
    int maxgraph;
    int maxcolors;
    Boolean noask;
    Boolean logwindow;
    Boolean free_colors;
}
ApplicationData, *ApplicationDataPtr;

static XtResource resources[] =
{
    {"invertDraw", "InvertDraw", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, invert), XtRImmediate,
     (XtPointer) FALSE},
    {"reverseVideo", "ReverseVideo", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, revflag), XtRImmediate,
     (XtPointer) FALSE},
    {"backingstore", "Backingstore", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, backingstore), XtRImmediate,
     (XtPointer) FALSE},
    {"allowDoubleClick", "AllowDoubleClick", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, allow_dc), XtRImmediate,
     (XtPointer) TRUE},
    {"autoscaleOnRead", "AutoscaleOnRead", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, autoscale_onread), XtRImmediate,
     (XtPointer) FALSE},
    {"verifyAction", "VerifyAction", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, verify_action), XtRImmediate,
     (XtPointer) FALSE},
    {"maxSets", "MaxSets", XtRInt, sizeof(int),
     XtOffset(ApplicationDataPtr, maxplot), XtRImmediate,
     (XtPointer) MAXPLOT},
    {"maxGraphs", "MaxGraphs", XtRInt, sizeof(int),
     XtOffset(ApplicationDataPtr, maxgraph), XtRImmediate,
     (XtPointer) MAXGRAPH},
    {"maxColors", "MaxColors", XtRInt, sizeof(int),
     XtOffset(ApplicationDataPtr, maxcolors), XtRImmediate,
     (XtPointer) MAXCOLORS},
    {"allowRefresh", "AllowRefresh", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, allow_refresh), XtRImmediate,
     (XtPointer) TRUE},
    {"allowRedraw", "AllowRedraw", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, auto_redraw), XtRImmediate,
     (XtPointer) TRUE},
    {"allowStatusRedraw", "AllowStatusRedraw", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, status_auto_redraw), XtRImmediate,
     (XtPointer) TRUE},
    {"noAsk", "NoAsk", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, noask), XtRImmediate,
     (XtPointer) FALSE},
    {"logWindow", "LogWindow", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, logwindow), XtRImmediate,
     (XtPointer) FALSE},
    {"freeColors", "FreeColors", XtRBoolean, sizeof(Boolean),
     XtOffset(ApplicationDataPtr, free_colors), XtRImmediate,
     (XtPointer) TRUE},
};

String fallbackResources[] = {
    "XMgr*fontList:-adobe-helvetica-bold-r-normal-*-14-*-*-*-*-*-*-*",
    "XMgr*background: #e5e5e5",
    "XMgr*foreground: #000000",
    "XMgr*XmPushButton.background: #b0c4de",
    "XMgr*XmMenuShell*XmPushButton.background: #e5e5e5",
    "XMgr*XmText*background: #cfe7e7",
    "XMgr*XmToggleButton.selectColor: #ff0000",
    "*XmToggleButton.fillOnSelect: true",
    "*XmSeparator.margin: 0",
#if ((XmVersion >= 1002) || defined (LesstifVersion) && (LesstifVersion >= 1000))
/*
 * Lesstif-0.80 sometimes crashes with tear-off menus; let's hope version 1.0
 * will fix it :-)
 */
    "*fileMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*readMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*writeMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*dataMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*transformationsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*setOperationsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*regionOperationsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*graphOperationsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*plotMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*optionsMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*pageMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*viewMenu.tearOffModel: XmTEAR_OFF_ENABLED",
    "*helpMenu.tearOffModel: XmTEAR_OFF_ENABLED",
#endif
    "*fileMenu.open.acceleratorText: Ctrl+O",
    "*fileMenu.open.accelerator: Ctrl<Key>o",
    "*fileMenu.save.acceleratorText: Ctrl+S",
    "*fileMenu.save.accelerator: Ctrl<Key>s",
    "*fileMenu.exit.acceleratorText: Ctrl+Q",
    "*fileMenu.exit.accelerator: Ctrl<Key>q",
    "*fileMenu.print.acceleratorText: Ctrl+P",
    "*fileMenu.print.accelerator: Ctrl<Key>p",
    "*helpMenu.onContext.acceleratorText: Shift+F1",
    "*helpMenu.onContext.accelerator: Shift<Key>F1",
    NULL
};



/*
 * put the current working directory in the title bar
 */
void set_title(char *ts)
{
    if (ts == NULL) {
	if (getcwd(buf, 1023) != NULL) {
	    strcpy(workingdir, buf);
	    XtVaSetValues(app_shell, XtNtitle, buf, NULL);
	    /* free(buf); */
	}
    } else {
	XtVaSetValues(app_shell, XtNtitle, ts, NULL);
    }
}

/*
 * initialize the X-Toolkit
 */
void initialize_screen(int *argc, char **argv)
{
    ApplicationData rd;

    app_shell = XtVaAppInitialize(&app_con, "XMgr", NULL, 0, argc, argv, 
    	fallbackResources, NULL);

#if (XmVersion >= 1002)
    /* Allow users to change tear off menus with X resources */
    XmRepTypeInstallTearOffModelConverter();
#endif

#ifdef WITH_EDITRES    
    XtAddEventHandler(app_shell, (EventMask) 0, True,
    			_XEditResCheckMessages, NULL);
#endif

    XtAddEventHandler(app_shell, StructureNotifyMask, False,
        		     (XtEventHandler) resize, NULL);

    savewidget(app_shell);
    disp = XtDisplay(app_shell);
    if (!disp) {
	sprintf(buf, "%s: can't open display, exiting...", argv[0]);
	XtWarning(buf);
	exit(0);
    }
#ifdef DRAGnDROP
    FILE_CONTENTS = XmInternAtom(disp, "FILE_CONTENTS", False);
    FILE_NAME = XmInternAtom(disp, "FILE_NAME", False);
#endif

    use_colors = DisplayPlanes(disp, DefaultScreen(disp));
    if (use_colors < 8) {
	use_colors = 1;
    }
    
    
    XtGetApplicationResources(app_shell, &rd, resources,
  			    XtNumber(resources), NULL, 0);
    
    invert = rd.invert;
    revflag = rd.revflag;
    backingstore = rd.backingstore;
    allow_dc = rd.allow_dc;
    autoscale_onread = rd.autoscale_onread;
    verify_action = rd.verify_action;
    maxplot = rd.maxplot;
    maxgraph = rd.maxgraph;
    maxcolors = rd.maxcolors;
    logwindow = rd.logwindow;
    auto_redraw = rd.auto_redraw;
    status_auto_redraw = rd.status_auto_redraw;
    allow_refresh = rd.allow_refresh;
    free_colors = rd.free_colors;
}


/*
 * exit xmgr
 */
void bailout(void)
{
    if (!is_dirtystate() || yesno("Exit losing unsaved changes?", NULL, NULL, "file.html#exit")) {
         if (resfp) {
             fclose(resfp);
         }
         exit(0);
    }
}

/*
 * Reread config (TODO)
 */
void rereadConfig(void)
{
}

/*
 * Warn about bug (TODO X message)
 */
void bugwarn(char *signame)
{
    fprintf(stderr, "\a\nOops! Got %s. Please use \"Help/Comments\" to report the bug.\n", signame);
    exit(1);
}

/*
 * Signal-handling routines
 */
 
void installSignal();

static RETSIGTYPE actOnSignal(int signo)
{
    char signame[16]; 
    installSignal();
    switch (signo) {
#ifdef SIGHUP
        case SIGHUP:
            rereadConfig();
            break;
#endif
#ifdef SIGINT
        case SIGINT:
#endif
#ifdef SIGQUIT
        case SIGQUIT:
#endif
#ifdef SIGTERM
        case SIGTERM:
#endif
            bailout();
            break;
#ifdef SIGILL
        case SIGILL:
        strcpy(signame, "SIGILL");
#endif
#ifdef SIGABRT
        case SIGABRT:
        strcpy(signame, "SIGABRT");
#endif
#ifdef SIGFPE
        case SIGFPE:
        strcpy(signame, "SIGFPE");
#endif
#ifdef SIGBUS
        case SIGBUS:
        strcpy(signame, "SIGBUS");
#endif
#ifdef SIGSEGV
        case SIGSEGV:
        strcpy(signame, "SIGSEGV");
#endif
#ifdef SIGSYS
        case SIGSYS:
        strcpy(signame, "SIGSYS");
#endif
            bugwarn(signame);
            break;
        default:
            break;
    }
    
}

void installSignal(void){

#ifdef SIGHUP
    signal(SIGHUP,  actOnSignal);   /* hangup */
#endif

#ifdef SIGINT
    signal(SIGINT,  actOnSignal);   /* interrupt */
#endif

#ifdef SIGQUIT
    signal(SIGQUIT, actOnSignal);   /* quit */
#endif

#ifdef SIGILL
    signal(SIGILL,  actOnSignal);   /* illegal instruction */
#endif

#ifdef SIGABRT
    signal(SIGABRT, actOnSignal);   /* abort */
#endif

#ifdef SIGFPE
    signal(SIGFPE,  actOnSignal);   /* floating point exception */
#endif

#ifdef SIGBUS
    signal(SIGBUS,  actOnSignal);   /* bus error */
#endif

#ifdef SIGSEGV
    signal(SIGSEGV, actOnSignal);   /* segmentation violation */
#endif

#ifdef SIGSYS
    signal(SIGSYS,  actOnSignal);   /* bad argument to system call */
#endif

#ifdef SIGTERM
    signal(SIGTERM, actOnSignal);   /* software termination signal */
#endif

}


/*
 * main menubar
 */
/* #define MENU_HELP	200 */
#define MENU_EXIT	201
#define MENU_CLEAR	202
/* #define MENU_NEW	203 */
#define MENU_OPEN	204
#define MENU_SAVE	205
#define MENU_SAVEAS	206
#define MENU_PRINT	207

static void MenuCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    switch ((int) client_data) {
    case MENU_EXIT:
	bailout();
	break;
    case MENU_CLEAR:
	wipeout(1);
	set_graph_active(cg = 0);
	update_all(cg);
	drawgraph();
	break;
    case MENU_OPEN:
	create_openproject_popup();
	break;
    case MENU_SAVE:
	if (strcmp (docname, NONAME) != 0) {
	    set_wait_cursor();
	    
	    if (do_writesets(maxgraph, -1, 1, docname, sformat) != 1) {
	        clear_dirtystate();
	    }
	    
	    unset_wait_cursor();
	    drawgraph();
	} else {
	    create_saveproject_popup();
	}
	break;
    case MENU_SAVEAS:
	create_saveproject_popup();
	break;
    case MENU_PRINT:
	set_wait_cursor();
	do_hardcopy();
	unset_wait_cursor();
	break;
    default:
	break;
    }
}



/*
 * service the autoscale buttons on the main panel
 */
void autoscale_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (activeset(cg)) {
	switch ((int) client_data) {
	case 0:
	    autoscale_graph(cg, -3);
	    break;
	case 1:
	    autoscale_graph(cg, -2);
	    break;
	case 2:
	    autoscale_graph(cg, -1);
	    break;
	}
	drawgraph();
    } else {
	errwin("No active sets!");
    }
}

void autoon_proc(void)
{
    set_action(0);
    set_action(AUTO_NEAREST);
}

/*
 * service the auticks button on the main panel
 */
void autoticks_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    default_axis(cg, g[cg].auto_type, X_AXIS);
    default_axis(cg, g[cg].auto_type, ZX_AXIS);
    default_axis(cg, g[cg].auto_type, Y_AXIS);
    default_axis(cg, g[cg].auto_type, ZY_AXIS);
    update_all(cg);
    drawgraph();
}

/*
 * set the message in the left footer
 */
void set_left_footer(char *s)
{
    Arg al;

    XmStringFree(statstring);
    statstring = XmStringCreateLtoR(s, charset);
    XtSetArg(al, XmNlabelString, statstring);
    XtSetValues(statlab, &al, 1);
    if (logwindow) {
	log_results(s);
    }
    XmUpdateDisplay(statlab);
}


/*
 * clear the locator reference point
 */
void do_clear_point(Widget w, XtPointer client_data, XtPointer call_data)
{
    g[cg].pointset = FALSE;
    g[cg].pt_type = 0;
    g[cg].dsx = g[cg].dsy = 0.0;
}

/*
 * set visibility of the toolbars
 */
int toolbar_visible = 1;
int statusbar_visible = 1;
int locbar_visible = 1;

static Widget windowbarw[3];

static void set_view_items()
{
    if (statusbar_visible) {
	XmToggleButtonSetState(windowbarw[1], True, False);
	XtManageChild(frbot);
	XtVaSetValues(scrollw,
		      XmNbottomAttachment, XmATTACH_WIDGET,
		      XmNbottomWidget, frbot,
		      NULL);
	if (toolbar_visible) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, frbot,
			  NULL);
	}
    } else {
	XmToggleButtonSetState(windowbarw[1], False, False);
	XtVaSetValues(scrollw,
		      XmNbottomAttachment, XmATTACH_FORM,
		      NULL);
	XtUnmanageChild(frbot);
	if (toolbar_visible) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
    if (toolbar_visible) {
	XmToggleButtonSetState(windowbarw[2], True, False);
	XtManageChild(frleft);
	if (statusbar_visible) {
	    XtVaSetValues(frleft,
			  XmNbottomAttachment, XmATTACH_WIDGET,
			  XmNbottomWidget, frbot,
			  NULL);
	}
	if (locbar_visible) {
	    XtVaSetValues(frleft,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, frtop,
			  NULL);
	}
	XtVaSetValues(scrollw,
		      XmNleftAttachment, XmATTACH_WIDGET,
		      XmNleftWidget, frleft,
		      NULL);
    } else {
	XmToggleButtonSetState(windowbarw[2], False, False);
	XtUnmanageChild(frleft);
	XtVaSetValues(scrollw,
		      XmNleftAttachment, XmATTACH_FORM,
		      NULL);
    }
    if (locbar_visible) {
	XmToggleButtonSetState(windowbarw[0], True, False);
	XtManageChild(frtop);
	XtVaSetValues(scrollw,
		      XmNtopAttachment, XmATTACH_WIDGET,
		      XmNtopWidget, frtop,
		      NULL);
	if (toolbar_visible) {
	    XtVaSetValues(frleft,
			  XmNtopAttachment, XmATTACH_WIDGET,
			  XmNtopWidget, frtop,
			  NULL);
	}
    } else {
	XmToggleButtonSetState(windowbarw[0], False, False);
	XtUnmanageChild(frtop);
	XtVaSetValues(scrollw,
		      XmNtopAttachment, XmATTACH_FORM,
		      NULL);
	if (toolbar_visible) {
	    XtVaSetValues(frleft,
			  XmNtopAttachment, XmATTACH_FORM,
			  NULL);
	}
    }
}

/*
 * called from the parser
 */
void set_toolbars(int bar, int onoff)
{
    switch (bar) {
    case BAR_TOOLBAR:
	toolbar_visible = onoff;
	break;
    case BAR_STATUSBAR:
	statusbar_visible = onoff;
	break;
    case BAR_LOCATORBAR:
	locbar_visible = onoff;
	break;
    }
    if (inwin) {
	set_view_items();
    }
}

/*
 * service routines for the View pulldown
 */
void set_statusbar(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (XmToggleButtonGetState(w)) {
	statusbar_visible = 1;
    } else {
	statusbar_visible = 0;
    }
    set_view_items();
}

void set_toolbar(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (XmToggleButtonGetState(w)) {
	toolbar_visible = 1;
    } else {
	toolbar_visible = 0;
    }
    set_view_items();
}

void set_locbar(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (XmToggleButtonGetState(w)) {
	locbar_visible = 1;
    } else {
	locbar_visible = 0;
    }
    set_view_items();
}

/*
 * set the canvas size
 */
void set_canvas_size(int w, int h, int o)
{
    Dimension px, py;
    px = w;
    py = h;
    XtVaSetValues(canvas,
		  XmNwidth, px,
		  XmNheight, py,
		  NULL);
}

void get_default_canvas_size(int *w, int *h)
{
    Dimension ww, wh;
    Arg args;
    XtSetArg(args, XmNwidth, &ww);
    XtGetValues(scrollw, &args, 1);
    XtSetArg(args, XmNheight, &wh);
    XtGetValues(scrollw, &args, 1);
    *w = ww - 5;
    *h = wh - 5;
}

/*
 * service the Page pulldown item
 */
void set_page(Widget w, XtPointer client_data, XtPointer call_data)
{
    double wx1, wx2, wy1, wy2;
    Dimension px, py;
    int pageorient = (int) client_data;
    wx1 = DisplayWidth(disp, DefaultScreen(disp));
    wx2 = DisplayWidthMM(disp, DefaultScreen(disp));
    wy1 = DisplayHeight(disp, DefaultScreen(disp));
    wy2 = DisplayHeightMM(disp, DefaultScreen(disp));
    px = (Dimension) (wx1 / wx2 * (8.5 * 25.4));
    py = (Dimension) (wy1 / wy2 * (11.5 * 25.4));

    switch (pageorient) {
    case PAGE_LANDSCAPE:
	page_layout = PAGE_LANDSCAPE;
	XtVaSetValues(canvas,
		      XmNwidth, py,
		      XmNheight, px,
		      NULL);
	break;
    case PAGE_PORTRAIT:
	page_layout = PAGE_PORTRAIT;
	XtVaSetValues(canvas,
		      XmNwidth, px,
		      XmNheight, py,
		      NULL);
	break;
    case PAGE_FIXED:
	page_layout = PAGE_FIXED;
	XtVaSetValues(canvas,
		      XmNwidth, canvasw,
		      XmNheight, canvash,
		      NULL);
	break;
    case PAGE_FREE:			/* falls through */
    default:
	page_layout = PAGE_FREE;
	{
	    int w, h;
	    get_default_canvas_size(&w, &h);
	    px = w;
	    py = h;
	    XtVaSetValues(canvas,
			  XmNwidth, px,
			  XmNheight, py,
			  NULL);
	}
	break;
    }
}

/*
 * get/set page layouts - returns the index into the array
 * of ToggleButton widgets
 */
int get_pagelayout()
{
    switch (page_layout) {
    case PAGE_FREE:
	return 0;
    case PAGE_LANDSCAPE:
	return 1;
    case PAGE_PORTRAIT:
	return 2;
    case PAGE_FIXED:
	return 3;
    default:
    	return 0;
    }
}

int set_pagelayout(int layout)
{
    page_layout = layout;
    if (inwin) {
	set_page(NULL, (XtPointer) page_layout, NULL);
    }
    return page_layout;
}


/*
 * create the main menubar
 */
static Widget CreateMainMenuBar(Widget parent)
{
    Widget menubar;
    Widget menupane, submenupane;

    Widget cascade;
    Widget button;


    menubar = CreateMenuBar(parent, "menuBar", "main.html#menubar");

/*
 * File menu
 */
    menupane = CreateMenu(menubar, "fileMenu", "File", 'F', NULL, NULL);

    CreateMenuButton(menupane, "open", "Open...", 'O',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_OPEN, "file.html#open");

    CreateMenuButton(menupane, "save", "Save", 'S',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_SAVE, "file.html#save");

    CreateMenuButton(menupane, "saveAs", "Save as...", 'a',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_SAVEAS, "file.html#saveas");

    CreateMenuButton(menupane, "describe", "Describe...", 'D',
    	(XtCallbackProc) create_describe_popup, NULL, "file.html#describe");

    CreateMenuSeparator(menupane, "sep1");

/*
 * Read submenu
 */

    submenupane = CreateMenu(menupane, "readMenu", "Read", 'R', NULL, NULL);

    CreateMenuButton(submenupane, "sets", "Sets...", 'S',
    	(XtCallbackProc) create_file_popup, (XtPointer) NULL, "file.html#readsets");

#ifdef HAVE_MFHDF
    CreateMenuButton(submenupane, "netCDF", "NetCDF/HDF...", 'N',
    	(XtCallbackProc) create_netcdfs_popup, (XtPointer) NULL, "file.html#readnetcdf");
#else

#ifdef HAVE_NETCDF
    CreateMenuButton(submenupane, "netCDF", "NetCDF...", 'N', 
    	(XtCallbackProc) create_netcdfs_popup, (XtPointer) NULL, "file.html#readnetcdf");
#endif

#endif
    CreateMenuButton(submenupane, "parameters", "Parameters...", 'P',
    	(XtCallbackProc) create_rparams_popup, (XtPointer) NULL, "file.html#readpars");

    CreateMenuButton(submenupane, "blockData", "Block data...", 'B',
    	(XtCallbackProc) create_block_popup, (XtPointer) NULL, "file.html#readblock");

    CreateMenuButton(submenupane, "image", "Image...", 'I',
    	(XtCallbackProc) create_image_frame, (XtPointer) NULL, 0);
   
/*
 * Write submenu
 */  
    submenupane = CreateMenu(menupane, "writeMenu", "Write", 'W', NULL, NULL);

    CreateMenuButton(submenupane, "sets", "Sets...", 'S',
    	(XtCallbackProc) create_write_popup, (XtPointer) NULL, "file.html#writesets");

    CreateMenuButton(submenupane, "parameters", "Parameters...", 'P', 
    	(XtCallbackProc) create_wparam_frame, (XtPointer) NULL, "file.html#writeparams");


    CreateMenuSeparator(menupane, "sep2");

    CreateMenuButton(menupane, "clearAll", "Clear all", 'C',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_CLEAR, "file.html#clearall");

    CreateMenuSeparator(menupane, "sep3");

    CreateMenuButton(menupane, "print", "Print", 'P',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_PRINT, "file.html#print");

    CreateMenuButton(menupane, "printerSetup", "Printer setup...", 't',
    	(XtCallbackProc) create_printer_setup, (XtPointer) NULL, "file.html#printersetup");


    CreateMenuSeparator(menupane, "sep4");

    CreateMenuButton(menupane, "exit", "Exit", 'x',
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_EXIT, "file.html#exit");


/*
 * Data menu
 */
    menupane = CreateMenu(menubar, "dataMenu", "Data", 'D', NULL, NULL);

    CreateMenuButton(menupane, "status", "Status...", 'S',
    	(XtCallbackProc) define_status_popup, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "results", "Results...", 'R',
    	(XtCallbackProc) create_monitor_frame, (XtPointer) NULL, 0);
    
    CreateMenuButton(menupane, "commands", "Commands...", 'C',
    	(XtCallbackProc) open_command, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane, "sep1");


    submenupane = CreateMenu(menupane, "transformationsMenu", "Transformations", 'T', NULL, NULL);

    CreateMenuButton(submenupane, "evaluateExpression", "Evaluate expression...", 'E',
    	    (XtCallbackProc) create_eval_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "loadValues", "Load values...", 'L',
    	    (XtCallbackProc) create_load_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "loadEvaluate", "Load & evaluate...", '&',
    	    (XtCallbackProc) create_leval_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "histograms", "Histograms...", 'H',
    	    (XtCallbackProc) create_histo_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "fourierTransforms", "Fourier transforms...", 'u',
    	    (XtCallbackProc) create_fourier_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "runningAverages", "Running averages...", 'a',
    	    (XtCallbackProc) create_run_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "regression", "Regression...", 'R',
    	    (XtCallbackProc) create_reg_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "nonLinearFit", "Non-linear curve fitting...", 'N',
    	    (XtCallbackProc) create_nonl_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "differences", "Differences...", 'D',
    	    (XtCallbackProc) create_diff_frame, (XtPointer) NULL, 0);
    	    
    CreateMenuButton(submenupane, "seasonalDifferences", "Seasonal differences...", 'o',
    	    (XtCallbackProc) create_seasonal_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "integration", "Integration...", 'I',
    	    (XtCallbackProc) create_int_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "correlation", "Cross/auto correlation...", 'C',
    	    (XtCallbackProc) create_xcor_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "interpolation", "Interpolation...", 't',
    	    (XtCallbackProc) create_interp_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "splines", "Splines...", 'S',
    	    (XtCallbackProc) create_spline_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "samplePoints", "Sample points...", 'm',
    	    (XtCallbackProc) create_samp_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "pruneData", "Prune data...", 'P',
    	    (XtCallbackProc) create_prune_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "digitalFilter", "Digital filter...", 'f',
    	    (XtCallbackProc) create_digf_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "linearConvolution", "Linear convolution...", 'v',
    	    (XtCallbackProc) create_lconv_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "geometricTransforms", "Geometric transforms...", 'G',
    	    (XtCallbackProc) create_geom_frame, (XtPointer) NULL, 0);
    	    
    CreateMenuButton(submenupane, "featureExtraction", "Feature extraction...", 'x',
    	    (XtCallbackProc) create_featext_frame, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane, "sep2");

    CreateMenuButton(menupane, "pointOperations", "Point operations...", 'P',
    	(XtCallbackProc) create_points_frame, (XtPointer) NULL, 0);


    submenupane = CreateMenu(menupane, "setOperationsMenu", "Set operations", 'o', NULL, NULL);

    CreateMenuButton(submenupane, "pickOperations", "Pick operations...", 'P',
    	    (XtCallbackProc) define_pickops_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "activate", "Activate...", 'i',
    	    (XtCallbackProc) create_activate_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "deactivate", "De-activate...", 'D',
    	    (XtCallbackProc) create_deactivate_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "reactivate", "Re-activate...", 'R',
    	    (XtCallbackProc) create_reactivate_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "setLength", "Set length...", 'g',
    	    (XtCallbackProc) create_setlength_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "changeType", "Set type...", 'y',
    	    (XtCallbackProc) create_change_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "copy", "Copy...", 'C',
    	    (XtCallbackProc) create_copy_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "move", "Move...", 'M',
    	    (XtCallbackProc) create_move_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "dropPoints", "Drop points...", 'n',
    	    (XtCallbackProc) create_drop_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "join", "Join...", 'J',
    	    (XtCallbackProc) create_join_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "split", "Split...", 'S',
    	    (XtCallbackProc) create_split_popup, (XtPointer) NULL, 0);
    	    
    CreateMenuButton(submenupane, "kill", "Kill...", 'K',
    	    (XtCallbackProc) create_kill_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "killAll", "Kill all", 'l',
    	    (XtCallbackProc) do_flush, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "Sort", "Sort...", 'o',
    	    (XtCallbackProc) create_sort_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "reverse", "Reverse...", 'v',
    	    (XtCallbackProc) create_reverse_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "coalesce", "Coalesce...", 'e',
    	    (XtCallbackProc) create_coalesce_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "swap", "Swap...", 'w',
    	    (XtCallbackProc) create_swap_popup, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "pack", "Pack...", 'a',
    	    (XtCallbackProc) do_packsets, (XtPointer) NULL, 0);

    
    CreateMenuButton(menupane, "editCreateSet", "Edit/create set...", 'E',
    	(XtCallbackProc) create_editp_frame, (XtPointer) NULL, 0);


    submenupane = CreateMenu(menupane, "regionOperationsMenu", 
    				"Region operations", 'i', NULL, NULL);

    CreateMenuButton(submenupane, "define", "Define...", 'D',
    	    (XtCallbackProc) create_define_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "clear", "Clear...", 'C',
    	    (XtCallbackProc) create_clear_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "evaluate", "Evaluate...", 'E',
    	    (XtCallbackProc) create_evalregion_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "extractPoints", "Extract points...", 'p',
    	    (XtCallbackProc) create_extract_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "extractSets", "Extract sets...", 's',
    	    (XtCallbackProc) create_extractsets_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "deletePoints", "Delete points...", 'o',
    	    (XtCallbackProc) create_delete_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "killSets", "Kill sets...", 'K',
    	    (XtCallbackProc) create_deletesets_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "reportOn", "Report on...", 'R',
    	    (XtCallbackProc) create_reporton_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "areaPerimeter", "Area/perimeter...", 'A',
    	    (XtCallbackProc) create_area_frame, (XtPointer) NULL, 0);


    submenupane = CreateMenu(menupane, "graphOperationsMenu", 
    				"Graph operations", 'G', NULL, NULL);

    CreateMenuButton(submenupane, "activate", "Activate...", 'i',
    	    (XtCallbackProc) create_gactive_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "copy", "Copy...", 'C',
    	    (XtCallbackProc) create_gcopy_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "swap", "Swap...", 'w',
    	    (XtCallbackProc) create_gswap_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "kill", "Kill...", 'K',
    	    (XtCallbackProc) create_gkill_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "focus", "Focus...", 'F',
    	    (XtCallbackProc) create_gfocus_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "show", "Show...", 'S',
    	    (XtCallbackProc) create_gshow_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "setType", "Set type...", 'y',
    	    (XtCallbackProc) create_gtype_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "arrange", "Arrange...", 'r',
    	    (XtCallbackProc) create_arrange_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "overlay", "Overlay...", 'O',
    	    (XtCallbackProc) create_overlay_frame, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "invertFlipAxes", "Invert/flip axes...", 'v',
    	    (XtCallbackProc) create_ginvert_frame, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "blockData", "Block data...", 'B',
    	(XtCallbackProc) create_eblock_frame, (XtPointer) NULL, 0);
    
    CreateMenuButton(menupane, "hotLinks", "Hot links...", 'l',
    	(XtCallbackProc) create_hotlinks_popup, (XtPointer) NULL, 0);



/* Plot menu */
    menupane = CreateMenu(menubar, "plotMenu", "Plot", 'P', NULL, NULL);

    CreateMenuButton(menupane, "worldScaling", "World scaling...", 'W',
    	(XtCallbackProc) create_world_frame, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "viewport", "Viewport...", 'V',
    	(XtCallbackProc) create_view_frame, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "autoscale", "Autoscale...", 'A',
    	(XtCallbackProc) create_autos_frame, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane, "sep1");

    CreateMenuButton(menupane, "titles", "Titles...", 'T',
    	(XtCallbackProc) create_label_frame, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "tickLabelsMarks", "Tick labels/tick marks...", 'i',
    	(XtCallbackProc) create_ticks_frame, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "frame", "Frame...", 'F',
    	(XtCallbackProc) create_frame_frame, (XtPointer) NULL, 0);

    button = XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, menupane, NULL);

    CreateMenuButton(menupane, "symbols", "Symbols...", 'y',
    	(XtCallbackProc) define_symbols_popup, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "errorBars", "Error bars...", 'E',
    	(XtCallbackProc) define_errbar_popup, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "legends", "Legends...", 'L',
    	(XtCallbackProc) define_legend_popup, (XtPointer) NULL, 0);

    CreateMenuSeparator(menupane, "sep2");

    CreateMenuButton(menupane, "stringsThings", "Strings & things...", 'S',
    	(XtCallbackProc) define_objects_popup, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "timeStamp", "Time stamp...", 'p',
    	(XtCallbackProc) create_misc_frame, (XtPointer) NULL, 0);


/* Options menu */
    menupane = CreateMenu(menubar, "optionsMenu", "Options", 'O', NULL, NULL);
   

    CreateMenuButton(menupane, "page", "Page...", 'P',
    	    (XtCallbackProc) create_page_frame, (XtPointer) NULL, 0);

	
    submenupane = CreateMenu(menupane, "viewMenu", "View", 'V', NULL, NULL);

    windowbarw[0] = CreateMenuToggle(submenupane, "locatorBar", "Locator bar", 'L',
	    (XtCallbackProc) set_locbar, (XtPointer) &frtop, NULL);
    windowbarw[1] = CreateMenuToggle(submenupane, "statusBar", "Status bar", 'S',
	    (XtCallbackProc) set_statusbar, (XtPointer) &frbot, NULL);
    windowbarw[2] = CreateMenuToggle(submenupane, "toolBar", "Tool bar", 'T',
	    (XtCallbackProc) set_toolbar, (XtPointer) &frleft, NULL);

    CreateMenuSeparator(submenupane, "sep1");

    CreateMenuButton(submenupane, "setLocatorFixedPoint", "Set locator fixed point", 'f',
    	(XtCallbackProc) set_actioncb, (XtPointer) SEL_POINT, 0);

    CreateMenuButton(submenupane, "clearLocatorFixedPoint", "Clear locator fixed point", 'C',
    	(XtCallbackProc) do_clear_point, (XtPointer) NULL, 0);

    CreateMenuButton(submenupane, "locatorProps", "Locator props...", 'p',
    	(XtCallbackProc) create_locator_frame, (XtPointer) NULL, 0);
    
    CreateMenuButton(menupane, "draw", "Draw...", 'D',
    	(XtCallbackProc) create_draw_frame, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "workingDirectory", "Working directory...", 'W',
    	(XtCallbackProc) create_workingdir_popup, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "misc", "Misc...", 'M',
    	(XtCallbackProc) create_props_frame, (XtPointer) NULL, 0);


/* help menu */

    menupane = CreateMenu(menubar, "helpMenu", "Help", 'H', &cascade, NULL);
    XtVaSetValues(menubar, XmNmenuHelpWidget, cascade, NULL);

    CreateMenuButton(menupane, "onContext", "On context", 'x',
    	(XtCallbackProc) ContextHelpCB, (XtPointer) NULL, 0);

    CreateMenuButton(menupane, "userGiude", "User Guide", 'G',
    	(XtCallbackProc) HelpCB, (XtPointer) "xmgr.html", 0);
    
    CreateMenuButton(menupane, "faq", "FAQ", 'Q',
    	(XtCallbackProc) HelpCB, (XtPointer) "FAQ.html", 0);

    CreateMenuButton(menupane, "changes", "Changes", 'C',
    	(XtCallbackProc) HelpCB, (XtPointer) "CHANGES.html", 0);

    CreateMenuButton(menupane, "comments", "Comments", 'm',
    	(XtCallbackProc) HelpCB, (XtPointer) "http://plasma-gate.weizmann.ac.il/Xmgr/comments.html", 0);
   	    
    CreateMenuSeparator(menupane, "sep1");

    CreateMenuButton(menupane, "about", "About...", 'A',
    	(XtCallbackProc) create_about_grtool, (XtPointer) NULL, 0);


    return (menubar);
}


/*
 * build the UI here
 */
void do_main_winloop(void)
{
    Widget bt, rc3, rcleft, rctop, formbot;
    Widget shell;
    Pixmap icon, shape;
    XSetWindowAttributes sw;
    Atom WM_DELETE_WINDOW;

    main_frame = XtVaCreateManagedWidget("main", xmMainWindowWidgetClass, app_shell,
					 XmNshadowThickness, 0,
					 XmNwidth, 800,
					 XmNheight, 700,
					 NULL);

    XtAddCallback(main_frame, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "index.html");
/*
 * We handle important WM events ourselves
 */
    WM_DELETE_WINDOW = XmInternAtom(XtDisplay(main_frame), "WM_DELETE_WINDOW", False);
    shell=XtParent(main_frame);
    XmAddWMProtocolCallback(shell, WM_DELETE_WINDOW, 
    	(XtCallbackProc) MenuCB, (XtPointer) MENU_EXIT);
    XtVaSetValues(shell, XmNdeleteResponse, XmDO_NOTHING, NULL);
    
    menu_bar = CreateMainMenuBar(main_frame);
    XtManageChild(menu_bar);

    form = XmCreateForm(main_frame, "form", NULL, 0);

    frleft = XtVaCreateManagedWidget("fr", xmFrameWidgetClass, form,
				     NULL);
    rcleft = XtVaCreateManagedWidget("toolBar", xmRowColumnWidgetClass, frleft,
				     XmNorientation, XmVERTICAL,
				     XmNpacking, XmPACK_TIGHT,
				     XmNspacing, 0,
				     XmNentryBorder, 0,
				     XmNmarginWidth, 0,
				     XmNmarginHeight, 0,
				     NULL);
    XtAddCallback(rcleft, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#toolbar");

    frtop = XtVaCreateManagedWidget("frtop", xmFrameWidgetClass, form,
				    NULL);
    rctop = XtVaCreateManagedWidget("rctop", xmRowColumnWidgetClass, frtop,
				    XmNorientation, XmHORIZONTAL,
				    XmNpacking, XmPACK_TIGHT,
				    XmNspacing, 0,
				    XmNentryBorder, 0,
				    XmNmarginWidth, 0,
				    XmNmarginHeight, 0,
				    NULL);

    frbot = XtVaCreateManagedWidget("frbot", xmFrameWidgetClass, form, NULL);
    XtManageChild(frbot);
    /* formbot = XmCreateForm(frbot, "form", NULL, 0); */
    formbot = XmCreateRowColumn(frbot, "rc", NULL, 0);
    set_default_message(buf);
    statstring = XmStringCreateLtoR(buf, charset);
    statlab = XtVaCreateManagedWidget("statlab", xmLabelWidgetClass, formbot,
				      XmNlabelString, statstring,
				      XmNalignment, XmALIGNMENT_BEGINNING,
				      XmNrecomputeSize, True,
				      NULL);
    XtAddCallback(statlab, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#statbar");

    string = XmStringCreateLtoR("G0:[X, Y] =                                           ",
				charset);
    loclab = XtVaCreateManagedWidget("label Locate", xmLabelWidgetClass, rctop,
				     XmNlabelString, string,
				     XmNalignment, XmALIGNMENT_END,
				     XmNrecomputeSize, True,
				     NULL);
    XtAddCallback(loclab, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#locbar");

    XtManageChild(formbot);

    scrollw = XtVaCreateManagedWidget("scrollw",
				     xmScrolledWindowWidgetClass, form,
				     XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
				     XmNscrollingPolicy, XmAUTOMATIC,
				     XmNvisualPolicy, XmVARIABLE,
				     NULL);

    canvas = XtVaCreateManagedWidget("canvas", xmDrawingAreaWidgetClass, scrollw,
				     XmNwidth, (Dimension) canvasw,
				     XmNheight, (Dimension) canvash,
				     XmNbackground,
				     WhitePixel(XtDisplay(main_frame),
				     DefaultScreen(XtDisplay(main_frame))),
				     NULL);
    XtAddCallback(canvas, XmNexposeCallback, (XtCallbackProc) refresh, NULL);
    XtAddCallback(canvas, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#canvas");

    XtAddEventHandler(canvas, EnterWindowMask
		      | LeaveWindowMask
		      | ButtonPressMask
		      | PointerMotionMask
		      | KeyPressMask
		      | ColormapChangeMask,
		      False,
		      (XtEventHandler) my_proc, NULL);

    XtVaSetValues(frleft,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, frtop,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, frbot,
		  XmNleftAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(frtop,
		  XmNtopAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  NULL);
    XtVaSetValues(scrollw,
		  XmNtopAttachment, XmATTACH_WIDGET,
		  XmNtopWidget, frtop,
		  XmNbottomAttachment, XmATTACH_WIDGET,
		  XmNbottomWidget, frbot,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_WIDGET,
		  XmNleftWidget, frleft,
		  NULL);
    XtVaSetValues(frbot,
		  XmNbottomAttachment, XmATTACH_FORM,
		  XmNrightAttachment, XmATTACH_FORM,
		  XmNleftAttachment, XmATTACH_FORM,
		  NULL);

    XtManageChild(form);


/*
 * We need it to get right (same) background color for pixmaps.
 * There should be more clever way of doing that, of course.
 */    
    bt = XtVaCreateManagedWidget("Draw", xmPushButtonWidgetClass, rcleft,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) doforce_redraw, (XtPointer) NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#draw");

/*
 * initialize pixmaps for buttons on front
 */
    init_pm(bt);

/* zoom and autoscale */
    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("Zoom", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, zoompm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) ZOOM_1ST);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#zoom");

    bt = XtVaCreateManagedWidget("AS", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, autopm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) autoscale_proc, (XtPointer) 0);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#as");

/* expand/shrink */
    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("Z", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, expandpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) gwindshrink_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#shrink");

    bt = XtVaCreateManagedWidget("z", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, shrinkpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) gwindexpand_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#expand");

/*
 * scrolling buttons
 */
    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("Left", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, leftpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) gwindleft_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#left");

    bt = XtVaCreateManagedWidget("Right", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, rightpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) gwindright_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#right");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);

    bt = XtVaCreateManagedWidget("Down", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, downpm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) gwinddown_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#down");
    
    bt = XtVaCreateManagedWidget("Up", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtVaSetValues(bt,
		  XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap, uppm,
		  NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) gwindup_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#up");

    XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, rcleft,
			    NULL);


    bt = XtVaCreateManagedWidget("AutoT", xmPushButtonWidgetClass, rcleft,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) autoticks_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#autoticks");

    bt = XtVaCreateManagedWidget("AutoO", xmPushButtonWidgetClass, rcleft,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) autoon_proc, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#autoon");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("ZX", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) ZOOMX_1ST);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#zoomx");

    bt = XtVaCreateManagedWidget("ZY", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) set_actioncb, (XtPointer) ZOOMY_1ST);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#zoomy");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("AX", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) autoscale_proc, (XtPointer) 1);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#autox");

    bt = XtVaCreateManagedWidget("AY", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) autoscale_proc, (XtPointer) 2);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#autoy");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("PZ", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) push_and_zoom, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#pz");

    bt = XtVaCreateManagedWidget("Pu", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) push_world, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#pu");

    rc3 = XtVaCreateManagedWidget("rc", xmRowColumnWidgetClass, rcleft,
				  XmNorientation, XmHORIZONTAL,
				  XmNpacking, XmPACK_TIGHT,
				  XmNspacing, 0,
				  XmNentryBorder, 0,
				  XmNmarginWidth, 0,
				  XmNmarginHeight, 0,
				  NULL);
    bt = XtVaCreateManagedWidget("Po", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) pop_world, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#po");

    bt = XtVaCreateManagedWidget("Cy", xmPushButtonWidgetClass, rc3,
				 NULL);
    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) cycle_world_stack, NULL);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#cy");

    sdstring = XmStringCreateLtoR("SD:1 ", charset);
    stack_depth_item = XtVaCreateManagedWidget("stackdepth", xmLabelWidgetClass, rcleft,
					       XmNlabelString, sdstring,
					       NULL);
    XtAddCallback(stack_depth_item, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#sd");

    cystring = XmStringCreateLtoR("CW:0 ", charset);
    curw_item = XtVaCreateManagedWidget("curworld", xmLabelWidgetClass, rcleft,
					XmNlabelString, cystring,
					NULL);
    XtAddCallback(curw_item, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#cw");

    bt = XtVaCreateManagedWidget("Exit", xmPushButtonWidgetClass, rcleft,
				 NULL);
    if (named_pipe>0) XtSetSensitive(bt, False);

    XtAddCallback(bt, XmNactivateCallback, (XtCallbackProc) MenuCB, (XtPointer) MENU_EXIT);
    XtAddCallback(bt, XmNhelpCallback, (XtCallbackProc) HelpCB, (XtPointer) "main.html#exit");


/*
 * initialize the tool bars
 */
    set_view_items();

    XmMainWindowSetAreas(main_frame, menu_bar, NULL, NULL, NULL, form);
    XtRealizeWidget(app_shell);

    xwin = XtWindow(canvas);
    disp = XtDisplay(canvas);

    sw.backing_store = Always;
    XChangeWindowAttributes(disp, xwin, CWBackingStore, &sw);

    set_page(NULL, (XtPointer) page_layout, NULL);

/*
 * A named pipe
 */
    if (named_pipe) {
	set_pipetimer();
    }
/*
 * set colors
 */
    xlibinitcmap();
    if (use_colors > 2) {
	XtVaSetValues(canvas, XmNcolormap, mycmap, NULL);
	XSetWindowColormap(disp, xwin, mycmap);
    }
/*
 * set GCs
 */
    gc = DefaultGC(disp, DefaultScreen(disp));
    gc_val.foreground = WhitePixel(disp, DefaultScreen(disp));
    gc_val.foreground = BlackPixel(disp, DefaultScreen(disp)) ^ WhitePixel(disp, DefaultScreen(disp));
    if (invert) {
	gc_val.function = GXinvert;
    } else {
	gc_val.function = GXxor;
    }
    gcxor = XCreateGC(disp, xwin, GCFunction | GCForeground, &gc_val);
    gc_val.foreground = WhitePixel(disp, DefaultScreen(disp));
    gc_val.function = GXcopy;
    gcclr = XCreateGC(disp, xwin, GCFunction | GCForeground, &gc_val);
/* 
 * set icon
 */
#if defined(HAVE_XPM)
    if (use_colors > 1) {
        XpmCreatePixmapFromData(XtDisplay(app_shell), 
                            DefaultRootWindow(XtDisplay(app_shell)),
			    xmgr_icon_xpm, &icon, &shape, NULL);
    } else {
        icon = XCreateBitmapFromData(XtDisplay(app_shell),
				 DefaultRootWindow(XtDisplay(app_shell)),
				 (char *) xmgr_icon_bits, xmgr_icon_width,
				 xmgr_icon_height);
        shape = icon;
    }
#else
    icon = XCreateBitmapFromData(XtDisplay(app_shell),
				 DefaultRootWindow(XtDisplay(app_shell)),
				 (char *) xmgr_icon_bits, xmgr_icon_width,
				 xmgr_icon_height);
    shape = icon;
#endif /* HAVE_XPM */
    XtVaSetValues(app_shell, XtNiconPixmap, icon, XtNiconMask, shape, NULL);

    /*
     * initialize cursors
     */
    init_cursors();

    /*
     * if an image was placed on the command line, read it in
     */
    if (readimage) {
	read_image(image_filename);
    }
    /*
     * If logging is on, initialize
     */
    inwin = 1;
    log_results("Startup");
    inwin = 0;

    /*
     * set the title to the working directory
     */
    set_title(mybasename(docname));

#ifdef DRAGnDROP
    /*
     * initialize drag n drop
     */
    init_dragndrop();
#endif

    /*
     * Process events.
     */
    if (sigcatch) {
        installSignal();
    }
    
    XtAppMainLoop(app_con);
}

/*
 * initialize pixmaps for buttons on front
 */
static void init_pm(Widget wref)
{
    Display *disp = XtDisplay(app_shell);
    Window cwin = RootWindowOfScreen(XtScreen(app_shell));
    GC gc;
    Pixmap ptmp;
    Pixel fg, bg;

    XtVaGetValues(wref,
		  XmNforeground, &fg,
		  XmNbackground, &bg,
		  NULL);

    gc = XCreateGC(disp, cwin, 0, NULL);
    XSetForeground(disp, gc, fg);
    XSetBackground(disp, gc, bg);

    zoompm = XCreatePixmap(disp, cwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    ptmp = XCreateBitmapFromData(disp, cwin, (char *) zoom_bits, 16, 16);
    XCopyPlane(disp, ptmp, zoompm, gc, 0, 0, 16, 16, 0, 0, 1);
    autopm = XCreatePixmap(disp, cwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    ptmp = XCreateBitmapFromData(disp, cwin, (char *) auto_bits, 16, 16);
    XCopyPlane(disp, ptmp, autopm, gc, 0, 0, 16, 16, 0, 0, 1);
    shrinkpm = XCreatePixmap(disp, cwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    ptmp = XCreateBitmapFromData(disp, cwin, (char *) shrink_bits, 16, 16);
    XCopyPlane(disp, ptmp, shrinkpm, gc, 0, 0, 16, 16, 0, 0, 1);
    expandpm = XCreatePixmap(disp, cwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    ptmp = XCreateBitmapFromData(disp, cwin, (char *) expand_bits, 16, 16);
    XCopyPlane(disp, ptmp, expandpm, gc, 0, 0, 16, 16, 0, 0, 1);
    rightpm = XCreatePixmap(disp, cwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    ptmp = XCreateBitmapFromData(disp, cwin, (char *) right_bits, 16, 16);
    XCopyPlane(disp, ptmp, rightpm, gc, 0, 0, 16, 16, 0, 0, 1);
    leftpm = XCreatePixmap(disp, cwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    ptmp = XCreateBitmapFromData(disp, cwin, (char *) left_bits, 16, 16);
    XCopyPlane(disp, ptmp, leftpm, gc, 0, 0, 16, 16, 0, 0, 1);
    uppm = XCreatePixmap(disp, cwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    ptmp = XCreateBitmapFromData(disp, cwin, (char *) up_bits, 16, 16);
    XCopyPlane(disp, ptmp, uppm, gc, 0, 0, 16, 16, 0, 0, 1);
    downpm = XCreatePixmap(disp, cwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    ptmp = XCreateBitmapFromData(disp, cwin, (char *) down_bits, 16, 16);
    XCopyPlane(disp, ptmp, downpm, gc, 0, 0, 16, 16, 0, 0, 1);
}

/* Routine for named pipes */

static int fid;
static XtInputId iid;
static XtIntervalId tim;
static XtInputCallbackProc get_file_input(XtPointer cd, int *src, XtInputId * iid);
static XtTimerCallbackProc timercb(XtPointer cdp, XtIntervalId * id);

static void set_pipetimer(void)
{
    tim = XtAppAddTimeOut(app_con, timer_delay, (XtTimerCallbackProc) timercb, NULL);
}

static XtTimerCallbackProc timercb(XtPointer cdp, XtIntervalId * id)
{
    static int opflag = 0;
    opflag = opflag ? 0 : 1;
    if (named_pipe==1) {
	fid = open(pipe_name, O_NONBLOCK | O_RDONLY);
	if (fid < 0) {
	    perror("Can't open fifo");
	    named_pipe = -1;
	} else {
	    named_pipe =  2;
	}
    }
    if (named_pipe==2) {
	if (opflag) {
	    iid = XtAppAddInput(app_con, fid,
				(XtPointer) XtInputReadMask,
				(XtInputCallbackProc) get_file_input,
				NULL);
    } else {
	XtRemoveInput(iid);
    }
	tim = XtAppAddTimeOut(app_con, timer_delay, 
			     (XtTimerCallbackProc) timercb, NULL);
    }
    return (XtTimerCallbackProc) NULL;
}

/*
 * Read from named pipe
 */
static XtInputCallbackProc get_file_input(XtPointer cd, int *fid, XtInputId * id)
{
    char buf[BUFSIZ];
    int nb;
    char *s;

    s = buf;
    while ((nb = read(*fid, s, 1)) > 0) {
	if (*s == '\n') {
	    *(s + 1) = 0;
	    exec_cmd(buf);
	    s = buf;
	} else
	    s++;
    }
    if (nb == -1 && errno != EAGAIN) {
	perror("get_file_input");
    }
    return (XtInputCallbackProc) NULL;
}

#ifdef DRAGnDROP
/* HandleDropLabel() -- start the data transfer when data is dropped in
 * the filename status area.
 */
void HandleDropLabel(widget, client_data, call_data)
Widget widget;
XtPointer client_data;
XtPointer call_data;
{

    Display *dpy;
    Atom FILE_CONTENTS, FILE_NAME;
    XmDropProcCallback DropData;
    XmDropTransferEntryRec transferEntries[2];
    XmDropTransferEntry transferList;
    Arg args[10];
    int n, i;
    Widget dc;
    Cardinal numExportTargets;
    Atom *exportTargets;
    Boolean file_name = False;
    void TransferProc();

    /* intern the Atoms for data targets */
    dpy = XtDisplay(app_shell);
    FILE_CONTENTS = XmInternAtom(dpy, "FILE_CONTENTS", False);
    FILE_NAME = XmInternAtom(dpy, "FILE_NAME", False);

    DropData = (XmDropProcCallback) call_data;
    dc = DropData->dragContext;

    /* retrieve the data targets and search for FILE_NAME */
    n = 0;
    XtSetArg(args[n], XmNexportTargets, &exportTargets);
    n++;
    XtSetArg(args[n], XmNnumExportTargets, &numExportTargets);
    n++;
    XtGetValues(dc, args, n);

    for (i = 0; i < numExportTargets; i++) {
	printf("In drop label %d\n", exportTargets[i]);
	if (exportTargets[i] == FILE_CONTENTS) {
	    file_name = True;
	    break;
	}
    }

    /* make sure we have a drop that is a copy operation and one of
     * the targets is FILE_NAME.  if not, set the status to failure.
     */
    n = 0;
    printf("In drop label success\n");
    /* set up transfer requests for drop site */
    transferEntries[0].target = FILE_CONTENTS;
    transferEntries[0].client_data = (XtPointer) canvas;
    transferEntries[1].target = FILE_NAME;
    transferEntries[1].client_data = (XtPointer) canvas;
    transferList = transferEntries;
    XtSetArg(args[n], XmNdropTransfers, transferEntries);
    n++;
    XtSetArg(args[n], XmNnumDropTransfers,
	     XtNumber(transferEntries));
    n++;
    XtSetArg(args[n], XmNtransferProc, TransferProc);
    n++;
    XmDropTransferStart(dc, args, n);
    return;
    if ((!file_name) || (DropData->dropAction != XmDROP) ||
	(DropData->operation != XmDROP_COPY)) {
	XtSetArg(args[n], XmNtransferStatus, XmTRANSFER_FAILURE);
	n++;
	XtSetArg(args[n], XmNnumDropTransfers, 0);
	n++;
	printf("In drop label failure\n");
    } else {
	printf("In drop label success\n");
	/* set up transfer requests for drop site */
	transferEntries[0].target = FILE_CONTENTS;
	transferEntries[0].client_data = (XtPointer) canvas;
	transferEntries[1].target = FILE_NAME;
	transferEntries[1].client_data = (XtPointer) canvas;
	transferList = transferEntries;
	XtSetArg(args[n], XmNdropTransfers, transferEntries);
	n++;
	XtSetArg(args[n], XmNnumDropTransfers,
		 XtNumber(transferEntries));
	n++;
	XtSetArg(args[n], XmNtransferProc, TransferProc);
	n++;
    }
    XmDropTransferStart(dc, args, n);
}

/* TransferProc() -- handle data transfer of converted data from drag
 * source to drop site.
 */
void TransferProc(widget, client_data, seltype, type, value, length, format)
Widget widget;
XtPointer client_data;
Atom *seltype;
Atom *type;
XtPointer value;
unsigned long *length;
int format;
{
    Display *dpy;
    Atom FILE_CONTENTS, FILE_NAME;
    Widget w;
    XmString string;
    char *label[256];

    /* intern the Atoms for data targets */
    dpy = XtDisplay(app_shell);
    FILE_CONTENTS = XmInternAtom(dpy, "FILE_CONTENTS", False);
    FILE_NAME = XmInternAtom(dpy, "FILE_NAME", False);

    w = (Widget) client_data;

    printf("In transfer proc\n");
    printf("Contents = %s\n", value);
    if (*type == FILE_CONTENTS)
	printf("Contents = %s\n", value);
/*
        XmTextSetString (w, value);
*/
    else if (*type == FILE_NAME) {
	printf("Filename: %s", value);
/*
        sprintf (label, "Filename: %s", value);
        string = XmStringCreateLocalized (label);
        XtVaSetValues (w, XmNlabelString, string, NULL);
        XmStringFree (string);
*/
    }
}

void init_dragndrop(void)
{
    Arg args[10];
    int n;

    n = 0;
    importList[0] = FILE_CONTENTS;
    importList[1] = FILE_NAME;
    XtSetArg(args[n], XmNimportTargets, importList);
    n++;
    XtSetArg(args[n], XmNnumImportTargets, 2);
    n++;
    XtSetArg(args[n], XmNdropSiteOperations, XmDROP_COPY);
    n++;
    XtSetArg(args[n], XmNdropProc, HandleDropLabel);
    n++;
    XmDropSiteRegister(canvas, args, n);
}

#endif
