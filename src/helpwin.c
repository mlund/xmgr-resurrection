#include <config.h>

#include <stdlib.h>
#include <string.h>

#include <X11/cursorfont.h>
#include <Xm/ToggleB.h>

#include "globals.h"
#include "protos.h"

#define NO_HELP "nohelp.html"

#ifdef WITH_LIBHELP
#include <help.h>
#endif

extern Display* disp;
extern Widget app_shell;

void HelpCB(Widget w, XtPointer client_data, XtPointer call_data) {
    char URL[256];
    char* ha;
#ifndef WITH_LIBHELP
    int i = 0, j = 0;
    char command[1024];
    int len;
#endif /* WITH_LIBHELP */

    ha = (char*)client_data;
    if (ha == NULL) {
        ha = NO_HELP;
    }

    set_wait_cursor();

#ifdef WITH_LIBHELP
    if (strstr(ha, "http:")) {
        char buf[256];
        strcpy(URL, ha);
        sprintf(buf, "The remote URL, %s, can't be accessed with xmhelp", URL);
        errmsg(buf);
    } else {
        /* xmhelp doesn't like "file://localhost/" prefix */
        sprintf(URL, "file:%s/doc/%s", xmgrdir, ha);
        get_help(w, (XtPointer)URL, ha);
    }
#else /* usual HTML browser */

    if (strstr(ha, "http:")) {
        strcpy(URL, ha);
    } else {
        sprintf(URL, "file://%s/doc/%s", xmgrdir, ha);
    }

    len = strlen(help_viewer);
    for (i = 0; i < len - 1; i++) {
        if ((help_viewer[i] == '%') && (help_viewer[i + 1] == 's')) {
            strcpy(&command[j], URL);
            j += strlen(URL);
            i++;
        } else {
            command[j++] = help_viewer[i];
        }
    }
#ifdef VMS
    system_spawn(command);
#else
    strcat(command, "&");
    system(command);
#endif

#endif /* WITH_LIBHELP */

    unset_wait_cursor();
}

void ContextHelpCB(Widget w, XtPointer client_data, XtPointer call_data) {
    Widget whelp;
    XmToggleButtonCallbackStruct* cb_struct = call_data;
    Cursor cursor = XCreateFontCursor(disp, XC_question_arrow);
    if ((whelp = XmTrackingLocate(app_shell, cursor, False))) {
        cb_struct->reason = XmCR_HELP;
        XtCallCallbacks(whelp, XmNhelpCallback, cb_struct);
    }
    XFreeCursor(disp, cursor);
}
