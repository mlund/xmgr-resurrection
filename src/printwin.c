/* $Id: printwin.c,v 1.2 1995/05/20 04:06:05 pturner Exp pturner $
 *
 * Printer initialization
 */

#include <config.h>

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"
#include "ps.h"

#ifdef VMS
#  include "vms_unix.h"
#endif

static Widget *printto_item;	/* for printer select popup */
static Widget printstring_item;
static Widget psetup_frame;
static Widget psetup_rc;
static Widget pdev_rc;
static Widget pslw_rc;
static Widget ps_rc;
static Widget *devices_item;
static Widget *pslwincr_item;
static Widget *pslwbegin_item;
static Widget eps_item;
static Widget ps2_item;
static Widget buts[5];

static void do_pr_toggle(Widget w, XtPointer client_data, XtPointer call_data);
static void set_printer_proc(Widget w, XtPointer client_data, XtPointer call_data);
static void do_print(Widget w, XtPointer client_data, XtPointer call_data);
void create_printfiles_popup(Widget, XtPointer, XtPointer call_data);
void do_eps_toggle(Widget, XtPointer, XtPointer call_data);

void create_printer_setup(Widget w, XtPointer client_data, XtPointer call_data)
{
    int i, x, y;
    set_wait_cursor();
    if (psetup_frame == NULL) {
	char *label1[5];
	label1[0] = "Accept";
	label1[1] = "Print";
	label1[2] = "File...";
	label1[3] = "Cancel";
	label1[4] = "Help";
	XmGetPos(app_shell, 0, &x, &y);
	psetup_frame = XmCreateDialogShell(app_shell, "Printer setup", NULL, 0);
	handle_close(psetup_frame);
	XtVaSetValues(psetup_frame, XmNx, x, XmNy, y, NULL);
	psetup_rc = XmCreateRowColumn(psetup_frame, "psetup_rc", NULL, 0);
	pdev_rc = XmCreateRowColumn(psetup_rc, "pdev_rc", NULL, 0);
        XtVaSetValues(pdev_rc,
                      XmNorientation, XmHORIZONTAL,
                      NULL);

	devices_item = CreatePanelChoice(pdev_rc, "Device: ",
					 12,
					 "PostScript landscape",
					 "PostScript portrait",
					 "FrameMaker landscape",
					 "FrameMaker portrait",
					 "HPGL landscape",
					 "HPGL portrait",
					 "Interleaf landscape",
					 "Interleaf portrait",
					 "PSTeX include",
					 "PSTeX landscape",
					 "PSTeX portrait",
					 0, 0);
	for (i = 0; i < 11; i++) {
	    XtAddCallback(devices_item[2 + i], XmNactivateCallback,
			(XtCallbackProc) do_prstr_toggle, (XtPointer) (intptr_t) i);
	}
	printto_item = CreatePanelChoice(pdev_rc, "Print to: ",
					 3,
					 "Printer",
					 "File", 0, 0);
	for (i = 0; i < 2; i++) {
	    XtAddCallback(printto_item[2 + i], XmNactivateCallback,
			  (XtCallbackProc) do_pr_toggle, (XtPointer) (intptr_t) i);
	}
	printstring_item = CreateTextItem2(psetup_rc, 30, 
                                           "Print control string:");
	XtVaSetValues( printstring_item, XmNscrollHorizontal, True, NULL );

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, psetup_rc, NULL);

        pslw_rc = XtVaCreateWidget("pslw_rc",
                                   xmRowColumnWidgetClass, psetup_rc,
                                   XmNorientation, XmHORIZONTAL,
                                   NULL);

	pslwbegin_item = CreatePanelChoice(pslw_rc, "PS line width start: ",
					10,
					"1", "2", "3", "4", "5", "6", "7",
					"8", "9", NULL, NULL);
	pslwincr_item = CreatePanelChoice(pslw_rc, "PS line width increment: ",
					10,
					"1", "2", "3", "4", "5", "6", "7",
					"8", "9", NULL, NULL);

	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, psetup_rc, NULL);

        ps_rc = XtVaCreateWidget("ps_rc",
                                   xmRowColumnWidgetClass, psetup_rc,
                                   XmNorientation, XmHORIZONTAL,
                                   XmNspacing, 80,
                                   NULL);

	eps_item = XtVaCreateManagedWidget("Generate EPS ", 
			xmToggleButtonWidgetClass, ps_rc, 
                        NULL);
	XtAddCallback(eps_item, XmNvalueChangedCallback, 
 				(XtCallbackProc) do_eps_toggle, NULL );
	ps2_item = XtVaCreateManagedWidget("Level 2 PS ", 
			xmToggleButtonWidgetClass, ps_rc,
                        NULL);
	
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, psetup_rc, NULL);

	CreateCommandButtons(psetup_rc, 5, buts, label1);
	XtAddCallback(buts[0], XmNactivateCallback,
		    (XtCallbackProc) set_printer_proc, (XtPointer) NULL);
	XtAddCallback(buts[1], XmNactivateCallback,
		      (XtCallbackProc) do_print, (XtPointer) NULL);
	XtAddCallback(buts[2], XmNactivateCallback,
		    (XtCallbackProc) create_printfiles_popup, (XtPointer) NULL);
	XtAddCallback(buts[3], XmNactivateCallback,
	      (XtCallbackProc) destroy_dialog, (XtPointer) psetup_frame);
	XtAddCallback(buts[4], XmNactivateCallback,
	      (XtCallbackProc) HelpCB, (XtPointer) "file.html#printersetup");
	XtManageChild(pdev_rc);
	XtManageChild(pslw_rc);
	XtManageChild(ps_rc);
	XtManageChild(psetup_rc);

	if( hdevice >=9 && hdevice <=11 ) {
		SetChoice( printto_item, ptofile );
		do_pr_toggle( NULL, NULL, (XtPointer) (intptr_t) ptofile );
		update_printer_setup();
    	XtSetSensitive( printto_item[1], False );
    }		
    }
    SetChoice (pslwbegin_item, pslwbegin - 1);
    SetChoice (pslwincr_item, pslwincr - 1);
    XtRaise(psetup_frame);
    do_pr_toggle(NULL, (XtPointer) (intptr_t) ptofile, NULL );
    update_printer_setup();
    unset_wait_cursor();
}

void update_printer_setup(void)
{
    if (psetup_frame) {
	SetChoice(devices_item, hdevice - 1);
	SetChoice(printto_item, ptofile);

	XmToggleButtonSetState(eps_item, epsflag, False);
	XmToggleButtonSetState(ps2_item, ps2flag, False);
	if (hdevice != 1 && hdevice != 2 && hdevice<9 ) {
	    XtSetSensitive(eps_item, False);
	    XtSetSensitive(ps2_item, False);
	} else if (hdevice == 1 || hdevice == 2 ){
	    XtSetSensitive(eps_item, True);
	    XtSetSensitive(ps2_item, True);
	} else {
	    XtSetSensitive(eps_item, False);
	    XtSetSensitive(ps2_item, True);
	}
	if (ptofile) {
	    xv_setstr(printstring_item, printstr);
	} else {
	    xv_setstr(printstring_item, curprint);
	}
    }
}

static void set_printer_proc(Widget w, XtPointer client_data, XtPointer call_data)
{
    char tmpstr[128];
    hdevice = (int) GetChoice(devices_item) + 1;
    ptofile = (int) GetChoice(printto_item);
    pslwbegin = (int) GetChoice(pslwbegin_item) + 1;
    pslwincr = (int) GetChoice(pslwincr_item) + 1;
    epsflag = XmToggleButtonGetState(eps_item);
    ps2flag = XmToggleButtonGetState(ps2_item);
    strcpy(tmpstr, (char *) xv_getstr(printstring_item));
    if (ptofile) {
	strcpy(printstr, tmpstr);
    } else {
	strcpy(curprint, tmpstr);
    }
    XtUnmanageChild(psetup_frame);
}


void do_eps_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    epsflag = XmToggleButtonGetState(eps_item);
	strcpy(buf, (char *) xv_getstr(printstring_item));
	if( epsflag ){
		cursuffix = eps_suffix;
		change_suffix( buf, eps_suffix );
	} else {
		cursuffix = ps_suffix;
		change_suffix( buf, ps_suffix );
	}
	xv_setstr(printstring_item, buf);
    XtVaSetValues(printstring_item , XmNcursorPosition, strlen(buf), NULL );
}

/*
 * Print button
 */
static void do_print(Widget w, XtPointer client_data, XtPointer call_data)
{
    set_wait_cursor();
    set_printer_proc(NULL, NULL, NULL);
    do_hardcopy();
    unset_wait_cursor();
}

/*
 * set the print options
 */
void do_prstr_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int value = (int) client_data;
 
    set_printer(value + 1, NULL);
    if( hdevice ==9 || hdevice == 10 || hdevice == 11 ) {
    	SetChoice(printto_item, ptofile=1);
    	do_pr_toggle( NULL, (XtPointer) (intptr_t) ptofile, NULL );
    	XtSetSensitive( printto_item[1], False );
    }else {
    	XtSetSensitive( printto_item[1], True );
    }
    if ((int) GetChoice(printto_item) == 0) {
		xv_setstr(printstring_item, curprint);
    }else {
		strcpy(buf, (char *) xv_getstr(printstring_item));
    	change_suffix( buf, cursuffix );
		xv_setstr(printstring_item, buf);
    	XtVaSetValues(printstring_item , XmNcursorPosition, strlen(buf), NULL );
	}
	if (hdevice != 1 && hdevice != 2){
		epsflag = False;
		XmToggleButtonSetState(eps_item, epsflag, False);
	}
}

static void do_pr_toggle(Widget w, XtPointer client_data, XtPointer call_data)
{
    int value = (int) client_data;
    int i;
    char *base;
    
    ptofile = (int) GetChoice(printto_item);
    if (value) {
   	/* strip suffix from docname and use that as default file name */
   	base = strdup( mybasename( docname ) ); 
   	i = strlen( base );
   	do{ 
   		i--;
   	}while( i>=0 && base[i] != '.' );
    	strcpy( printstr, base );
   	if( i>0 )
    		printstr[i]='\0';
    	strcat( printstr, cursuffix );
	xv_setstr(printstring_item, printstr);
	XtSetSensitive(buts[2], True);
    } else {
	xv_setstr(printstring_item, curprint);
	XtSetSensitive(buts[2], False);
    }
}

static void do_prfilesel_proc(Widget w, XtPointer client_data, 
														XtPointer call_data)
{
    Widget dialog = (Widget) client_data;
    char *s;
    XmFileSelectionBoxCallbackStruct *cbs =
        (XmFileSelectionBoxCallbackStruct *) call_data;

	if (!XmStringGetLtoR(cbs->value, charset, &s)) {
        errwin("do_prfilesel_proc(): Error converting XmString to char string");
        return;
    }
    xv_setstr(printstring_item, s);
    strcpy( printstr, s );
    XtVaSetValues(printstring_item , XmNcursorPosition, strlen(s), NULL );
    XtFree(s);
    XtUnmanageChild(dialog);
}


void create_printfiles_popup(Widget w, XtPointer client_data, XtPointer call_data)
{
    static Widget top;

    if( !GetChoice(printto_item) ) { 
	return;     	/* return if print to printer */
    } else {
    	set_wait_cursor();
    	if (top == NULL) {
	    top = XmCreateFileSelectionDialog(app_shell, "prfilsel", NULL, 0);
	    XtVaSetValues(XtParent(top), XmNtitle, "Select print file", NULL);
	    XtAddCallback(top, XmNokCallback, (XtCallbackProc) do_prfilesel_proc,
        							(XtPointer) top);
     	    XtAddCallback(top, XmNcancelCallback, (XtCallbackProc) destroy_dialog,
        							(XtPointer) top);
    	}
	    XtVaSetValues(top, XmNdirMask,  XmStringConcat(
				    XmStringCreate( "*", charset),
				    XmStringCreate( cursuffix, charset) ), NULL );
    	XtRaise(top);
    	unset_wait_cursor();
    }
}
