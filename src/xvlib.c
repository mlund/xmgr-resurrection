/* $Id: xvlib.c,v 1.4 1995/06/02 03:23:59 pturner Exp pturner $
 *
 * driver for xlib for gr
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include "protos.h"
#include "externs.h"
#include "globals.h"
#include "patterns.h"
#include "rotated.h"

#include <Xm/Xm.h>

double xconv(double x);
double yconv(double y);

extern Display* disp;
extern GC gc;
extern GC gcxor;
extern GC gcclr;
extern Window xwin;
extern XGCValues gc_val;

extern int use_colors;
extern int use_defaultcmap;
extern int revflag; /* defined in main.c */
extern int use_xvertext;
extern int ncolors;                          /* in draw.c */
extern unsigned char red[], green[], blue[]; /* in draw.c */

static int save_images;
char saveimage_prstr[256] = "xmgr.xwd";

extern double devcharsize;
extern double charsize;

extern unsigned long colors[NUM_COLORS];
extern unsigned char red[NUM_COLORS], green[NUM_COLORS], blue[NUM_COLORS];
Colormap cmap, mycmap;

extern Widget canvas;
extern Widget app_shell;

#define MINCOLOR 0
#define MAXLINEW 15

extern int maxcolors;

static int xlibcolor = 1;
static int xliblinewidth = 0;
static int xlibdmode;
static int xlibfont = 0;
static int xliblinestyle = 1;
static int doublebuff = 0; /* no double buffering by default */
static Pixmap backbuff, displaybuff;
int win_h, win_w;

int xv_pathlength = 0;

extern int backingstore;
Pixmap backpix;

static void xlibinit(void);

/*
 * the following is a tunable parameter and may
 * need to be adjusted
 */
double xlibcharsize = 2.00;

/*
 * fix for dotted/dashed linestyles
 */
XPoint polypoints[MAXLINELEN];

static char solid[1] = {1};
static char dotted[2] = {3, 1};
static char shortdashed[2] = {3, 3};
static char longdashed[2] = {7, 7};
static char dot_dashed[4] = {1, 3, 7, 3};

static char* dash_list[] = {solid, dotted, shortdashed, longdashed, dot_dashed};

static int dash_list_length[] = {1, 2, 2, 2, 4};

void get_xlib_dims(int* w, int* h) {
    Arg args;
    Dimension ww, wh;

    XtSetArg(args, XmNwidth, &ww);
    XtGetValues(canvas, &args, 1);
    XtSetArg(args, XmNheight, &wh);
    XtGetValues(canvas, &args, 1);
    *w = ww;
    *h = wh;
}

static void xlibinit(void) {
    double wx1, wx2, wy1, wy2;
    static int inc = 1;
    extern int doclear, bgcolor;
    Arg args;
    Dimension ww, wh;

    disp = XtDisplay(canvas);
    xwin = XtWindow(canvas);
    XtSetArg(args, XmNwidth, &ww);
    XtGetValues(canvas, &args, 1);
    XtSetArg(args, XmNheight, &wh);
    XtGetValues(canvas, &args, 1);
    win_w = ww;
    win_h = wh;

    devwidth = win_w;
    devheight = win_h;
    wx1 = DisplayWidth(disp, DefaultScreen(disp));
    wx2 = DisplayWidthMM(disp, DefaultScreen(disp));
    wy1 = DisplayHeight(disp, DefaultScreen(disp));
    wy2 = DisplayHeightMM(disp, DefaultScreen(disp));
    devwidthmm = (int)(wx2 / wx1 * win_w);
    devheightmm = (int)(wy2 / wy1 * win_h);
    if (inc) {
        if (use_xvertext) {
            xlibcharsize = 1.1;
        } else {
            xlibcharsize = 0.6;
        }
        xlibinit_tiles();
        if (backingstore) {
            backpix =
                XCreatePixmap(disp, DefaultRootWindow(disp), win_w, win_h, DisplayPlanes(disp, DefaultScreen(disp)));
        }
        inc = 0;
    }
    if (doublebuff) {
        xlibdoublebuffer(doublebuff);
        displaybuff = backbuff;
    } else {
        displaybuff = xwin;
    }
    if (doclear) {
        xlibsetcolor(bgcolor);
        XFillRectangle(disp, displaybuff, gc, 0, 0, win_w, win_h);
        if (backingstore) {
            XFillRectangle(disp, backpix, gc, 0, 0, win_w, win_h);
        }
    }
}

void xlibdoublebuffer(int mode) {

    doublebuff = mode;
    if (!inwin) {
        return;
    }
    if (mode) {
        if (!backbuff) {
            backbuff =
                XCreatePixmap(disp, DefaultRootWindow(disp), win_w, win_h, DisplayPlanes(disp, DefaultScreen(disp)));
        }
        displaybuff = backbuff;
    } else {
        if (backbuff) {
            XFreePixmap(disp, backbuff);
            backbuff = (Pixmap)NULL;
            displaybuff = xwin;
        }
    }
}

void xlibfrontbuffer(int mode) {

    if (!inwin) {
        return;
    }
    if (mode) {
        displaybuff = xwin;
    } else {
        if (doublebuff && backbuff) {
            displaybuff = backbuff;
        }
    }
}

void xlibbackbuffer(int mode) {

    if (!inwin) {
        return;
    }
    if (mode && doublebuff && backbuff) {
        displaybuff = backbuff;
    } else {
        displaybuff = xwin;
    }
}

void xlibswapbuffer(void) {

    if (!inwin) {
        return;
    }
    if (doublebuff && backbuff) {
        XCopyArea(disp, displaybuff, xwin, gc, 0, 0, win_w, win_h, 0, 0);
    }
}

void refresh_from_backpix(void) {
    if (backpix) {
        XCopyArea(disp, backpix, xwin, gc, 0, 0, win_w, win_h, 0, 0);
    }
}

void resize_backpix(void) {
    XFreePixmap(disp, backpix);
    backpix = XCreatePixmap(disp, DefaultRootWindow(disp), win_w, win_h, DisplayPlanes(disp, DefaultScreen(disp)));
}

static int xlib_write_mode = 1;

void set_write_mode(int m) {
    flush_pending();
    xlib_write_mode = m;
}

void xlibsetmode(int mode) {
    switch (mode) {
    case 1:
        xlibinit();
        break;
    case 2:
        flush_pending();
        if (doublebuff && backbuff) {
            xlibswapbuffer();
        }
        if (save_images) {
            save_image_on_disk(disp, xwin, displaybuff, 0, 0, win_w, win_h, saveimage_prstr, (Colormap)NULL);
        }
        break;
    }
}

/* scrunch the array of XPoints */
void scrunch_xpoints(XPoint* p, int* n) {
    int i, cnt = 0;
    cnt = 0;
    for (i = 0; i < *n - 1; i++) {
        if (p[cnt].x == p[i + 1].x && p[cnt].y == p[i + 1].y) {
        } else {
            cnt++;
            p[cnt] = p[i + 1];
        }
    }
    cnt++;
    if (cnt <= 2) {
        cnt = 2;
        p[1] = p[*n - 1];
    }
    *n = cnt;
}

void flush_pending(void) {
    if (xv_pathlength > 1) {
        if (xv_pathlength > 3) {
            scrunch_xpoints(polypoints, &xv_pathlength);
        }
        if (xlib_write_mode) {
            XDrawLines(disp, displaybuff, gc, polypoints, xv_pathlength, CoordModeOrigin);
            if (backingstore) {
                XDrawLines(disp, backpix, gc, polypoints, xv_pathlength, CoordModeOrigin);
            }
        } else {
            XDrawLines(disp, displaybuff, gcclr, polypoints, xv_pathlength, CoordModeOrigin);
            if (backingstore) {
                XDrawLines(disp, backpix, gcclr, polypoints, xv_pathlength, CoordModeOrigin);
            }
        }
    }
    xv_pathlength = 0;
}

static int x1, y1;

void drawxlib(int x, int y, int mode) {
    if (mode) {
        polypoints[xv_pathlength].x = x;
        polypoints[xv_pathlength].y = win_h - y;
        xv_pathlength++;
        if (xv_pathlength == MAXLINELEN) {
            flush_pending();
            polypoints[xv_pathlength].x = x;
            polypoints[xv_pathlength].y = win_h - y;
            xv_pathlength = 1;
        }
    } else {
        if ((x == x1 && y == y1)) {
            return;
        } else {
            flush_pending();
            polypoints[xv_pathlength].x = x;
            polypoints[xv_pathlength].y = win_h - y;
            xv_pathlength = 1;
        }
    }
    x1 = x;
    y1 = y;
}

int xconvxlib(double x) { return ((int)(win_w * xconv(x))); }

int yconvxlib(double y) { return ((int)(win_h * yconv(y))); }

void write_image(char* fname) {
    set_wait_cursor();
    save_image_on_disk(disp, xwin, displaybuff, 0, 0, win_w, win_h, fname, mycmap);
    unset_wait_cursor();
}

static int valid_cmap = 0;

void xlibinitcmap(void) {
    int i;
    XColor xc[NUM_COLORS];
    ncolors = DisplayCells(disp, DefaultScreen(disp));
    if (ncolors > 256) {
        ncolors = 256;
    }
    if (ncolors > 16) {
        cmap = DefaultColormap(disp, DefaultScreen(disp));
        for (i = 0; i < ncolors; i++) {
            xc[i].pixel = i;
            xc[i].flags = DoRed | DoGreen | DoBlue;
        }
        if (!use_defaultcmap) {
            XQueryColors(disp, cmap, xc, ncolors);
            mycmap = XCreateColormap(disp, xwin, DefaultVisual(disp, DefaultScreen(disp)), AllocAll);
        } else {
            mycmap = cmap;
        }
        for (i = 2; i < maxcolors; i++) {
            xc[i].red = red[i] << 8;
            xc[i].green = green[i] << 8;
            xc[i].blue = blue[i] << 8;
            if (use_defaultcmap) {
                if (!XAllocColor(disp, cmap, &xc[i])) {
                    fprintf(stderr, " Can't allocate color\n");
                }
            }
            colors[i] = xc[i].pixel;
        }
        if (!use_defaultcmap) {
            XStoreColors(disp, mycmap, xc, ncolors);
        }
    }
    if (revflag) {
        colors[1] = WhitePixel(disp, DefaultScreen(disp));
        colors[0] = BlackPixel(disp, DefaultScreen(disp));
    } else {
        colors[0] = WhitePixel(disp, DefaultScreen(disp));
        colors[1] = BlackPixel(disp, DefaultScreen(disp));
    }
    valid_cmap = 1;
}

void UpdateAllColorChoice(unsigned);

void xlibsetcmap(int i, int r, int g, int b) {
    XColor xct;

    if (i >= maxcolors) {
        return;
    }
    if (!valid_cmap && (use_colors > 4) && (i >= 2)) {
        red[i] = r;
        green[i] = g;
        blue[i] = b;
    } else if (valid_cmap && (use_colors > 4) && (i >= 2)) {
        xct.green = g << 8;
        xct.blue = b << 8;
        xct.red = r << 8;
        xct.flags = DoRed | DoGreen | DoBlue;
        xct.pad = 0;
        xct.pixel = colors[i];
        if (use_defaultcmap) {
            /* Attempt to allocate the color */
            if (!XAllocColor(disp, cmap, &xct))
                fprintf(stderr, " Can't allocate color\n");
            else {
                extern int free_colors;
                if (free_colors) {
                    /* If successful, first free the pixel that we're replacing
                   if we're not using it anywhere else */
                    /* This may cause problems if we have coincidentally chosen
                       one of the motif widget colors.  Thus the free_colors
                   switch, which defaults to true.  There is a resource
                   and a command line switch for this.  */
                    int j;
                    for (j = 0; j < maxcolors; ++j)
                        if (i != j && colors[j] == colors[i])
                            break;
                    if (j == maxcolors)
                        XFreeColors(disp, cmap, colors + i, 1, 0);
                }
                /* Set values based on what XAllocColor put into xct */
                colors[i] = xct.pixel;
                red[i] = xct.red >> 8;
                green[i] = xct.green >> 8;
                blue[i] = xct.blue >> 8;
                UpdateAllColorChoice(i);
            }
        } else {
            XStoreColor(disp, mycmap, &xct);
            red[i] = xct.red >> 8;
            green[i] = xct.green >> 8;
            blue[i] = xct.blue >> 8;
            colors[i] = xct.pixel;
        }
        XFlush(disp);
    }
}

int xlibsetlinewidth(int c) {
    flush_pending();
    x1 = y1 = 99999;
    if (c) {
        c = c % MAXLINEW;
        if (c == 0) {
            c = 1;
        }
        if (xliblinestyle <= 1) {
            XSetLineAttributes(disp, gc, c - 1 == 0 ? 0 : c, LineSolid, CapButt, JoinRound);
        } else {
            XSetLineAttributes(disp, gc, c - 1 == 0 ? 0 : c, LineOnOffDash, CapButt, JoinRound);
            XSetDashes(disp, gc, 0, dash_list[xliblinestyle - 1], dash_list_length[xliblinestyle - 1]);
        }
    }
    return (xliblinewidth = c);
}

int xlibsetlinestyle(int style) {
    flush_pending();
    x1 = y1 = 99999;
    if (style > 1 && xliblinewidth) {
        XSetLineAttributes(disp, gc, xliblinewidth - 1 == 0 ? 0 : xliblinewidth, LineOnOffDash, CapButt, JoinRound);
        XSetDashes(disp, gc, 0, dash_list[style - 1], dash_list_length[style - 1]);
    } else if (style == 1 && xliblinewidth) {
        XSetLineAttributes(disp, gc, xliblinewidth - 1 == 0 ? 0 : xliblinewidth, LineSolid, CapButt, JoinRound);
    }
    return (xliblinestyle = style);
}

int xlibsetcolor(int c) {

    flush_pending();

    x1 = y1 = 99999;
    c = c % maxcolors;

    if (use_colors > 4) {
        XSetForeground(disp, gc, colors[c]);
    } else {
        XSetForeground(disp, gc, colors[c == 0 ? 0 : 1]);
    }
    xlibcolor = c;
    return c;
}

void xlibdrawtic(int x, int y, int dir, int updown) {
    switch (dir) {
    case 0:
        switch (updown) {
        case 0:
            drawxlib(x, y, 0);
            drawxlib(x, y + devxticl, 1);
            break;
        case 1:
            drawxlib(x, y, 0);
            drawxlib(x, y - devxticl, 1);
            break;
        }
        break;
    case 1:
        switch (updown) {
        case 0:
            drawxlib(x, y, 0);
            drawxlib(x + devyticl, y, 1);
            break;
        case 1:
            drawxlib(x, y, 0);
            drawxlib(x - devyticl, y, 1);
            break;
        }
        break;
    }
}

static XFontStruct* font;
static char* fontname[] = {
    /*
    "-adobe-times-medium-r-normal--14-100-100-100-p-74-iso8859-1",
    "-adobe-times-bold-r-normal--14-100-100-100-p-76-iso8859-1",
    "-adobe-times-medium-i-normal--14-100-100-100-p-73-iso8859-1",
    "-adobe-times-bold-i-normal--14-100-100-100-p-77-iso8859-1",
    "-adobe-helvetica-medium-r-normal--14-100-100-100-p-76-iso8859-1",
    "-adobe-helvetica-bold-r-normal--14-100-100-100-p-82-iso8859-1",
    "-adobe-helvetica-medium-o-normal--14-100-100-100-p-78-iso8859-1",
    "-adobe-helvetica-bold-o-normal--14-100-100-100-p-82-iso8859-1",
    "-adobe-symbol-medium-r-normal--14-100-100-100-p-85-adobe-fontspecific",
    "-adobe-symbol-medium-r-normal--14-100-100-100-p-85-adobe-fontspecific",
    "-adobe-courier-medium-r-normal--14-100-100-100-m-90-iso8859-1",
    "-adobe-courier-bold-r-normal--14-100-100-100-m-90-iso8859-1",
    "-adobe-courier-medium-o-normal--14-100-100-100-m-90-iso8859-1",
    "-adobe-courier-bold-o-normal--14-100-100-100-m-90-iso8859-1"
    */
    "-adobe-times-medium-r-normal--20-140-100-100-p-96-iso8859-1",
    "-adobe-times-bold-r-normal--20-140-100-100-p-100-iso8859-1",
    "-adobe-times-medium-i-normal--20-140-100-100-p-94-iso8859-1",
    "-adobe-times-bold-i-normal--20-140-100-100-p-98-iso8859-1",
    "-adobe-helvetica-medium-r-normal--20-140-100-100-p-100-iso8859-1",
    "-adobe-helvetica-bold-r-normal--20-140-100-100-p-105-iso8859-1",
    "-adobe-helvetica-medium-o-normal--20-140-100-100-p-98-iso8859-1",
    "-adobe-helvetica-bold-o-normal--20-140-100-100-p-103-iso8859-1",
    "-adobe-symbol-medium-r-normal--20-140-100-100-p-107-adobe-fontspecific",
    "-adobe-symbol-medium-r-normal--20-140-100-100-p-107-adobe-fontspecific"};

void xlibsetfont(int n) {
    flush_pending();
    x1 = y1 = 99999;
    if (use_xvertext) {
        if (font == (XFontStruct*)NULL) {
            font = XLoadQueryFont(disp, fontname[n]);
            if (font == NULL) {
                fprintf(stderr, "no font `%s'\n", *fontname);
                font = XLoadQueryFont(disp, "fixed");
            }
        } else {
            font = XLoadQueryFont(disp, fontname[n]);
            if (font == NULL) {
                fprintf(stderr, "no font `%s'\n", *fontname);
                font = XLoadQueryFont(disp, "fixed");
            }
        }
    } else {
        hselectfont(xlibfont = n);
    }
}

int GetXVText(double scale, char* s) {
    XPoint* xp;
    int retval;
    if (font == NULL) {
        xlibsetfont(xlibfont);
    }
    if (s == NULL || strlen(s) == 0) {
        return 0;
    }
    xp = XRotTextExtents(disp, font, 0.0, 0, 0, s, MLEFT);
    if (xp) {
        /*
            printf("In GetXVText %s %d %d %d\n", s, (int) xp[2].x, (int) xp[0].x, (int) xp[2].x - xp[0].x);
        */
        retval = (int)(xp[2].x - xp[0].x);
        free((char*)xp);
    } else {
        retval = 10 * scale * strlen(s);
    }
    return retval;
}

int GetYVText(double scale, char* s) {
    XPoint* xp;
    int retval;
    if (font == NULL) {
        xlibsetfont(xlibfont);
    }
    if (s == NULL || strlen(s) == 0) {
        return 0;
    }
    xp = XRotTextExtents(disp, font, 0.0, 0, 0, s, MLEFT);
    if (xp) {
        /*
            printf("In GetYVText %lf %s %d %d %d\n", scale, s, (int) xp[0].y, (int) xp[2].y, (int) xp[0].y - xp[2].y);
        */
        retval = (int)(xp[0].y - xp[2].y);
        free((char*)xp);
    } else {
        retval = 10 * scale;
    }
    return retval * 0.62;
}

void dispstrxlib(int x, int y, int rot, char* s, int just, int fudge) {
    int i, slen, curcnt;
    int underline;
    double scale;
    char curstr[256];
    char* buf;
    int upperset;
    int symfont;
    int xpos, ypos;

    scale = xlibcharsize;
    xpos = x, ypos = win_h - y;
    curcnt = 0;
    upperset = 0;
    symfont = 0;
    underline = 0;
    slen = strlen(s);

    buf = (char*)malloc((strlen(s) + 1) * sizeof(char));
    if (buf == NULL) {
        return;
    }
    strcpy(buf, s);
    if (use_xvertext) {
        float frot = rot;
        /* load the font if necessary */
        if (font == (XFontStruct*)NULL) {
            font = XLoadQueryFont(disp, fontname[6]);
            if (font == NULL) {
                fprintf(stderr, "no font `%s'\n", *fontname);
                font = XLoadQueryFont(disp, "fixed");
            }
        }
        switch (just) {
        case 0:
            just = MLEFT;
            break;
        case 1:
            just = MRIGHT;
            break;
        case 2:
            just = MCENTRE;
            break;
        }

        if (xlibfont == 9) {
            symfont = 1;
            upperset = 0x80;
        } else {
            symfont = 0;
            upperset = 0;
        }
        for (i = 0; i < slen; i++) {
            if (buf[i] == '\\' && isdigit(buf[i + 1])) {
                curstr[curcnt] = 0;
                if (curcnt >= 1) {}
                curcnt = 0;
                if (symfont) {
                    symfont = 0;
                    upperset = 0;
                }
                xlibsetfont(buf[i + 1] - '0');
                if (xlibfont == 9) {
                    symfont = 1;
                    upperset = 0x80;
                }
                i++;
                continue;
            } else if (buf[i] == '\\' && isoneof(buf[i + 1], "cCbxsSNuU+-")) {
                switch (buf[i + 1]) {
                case 'x':
                    curstr[curcnt] = 0;
                    if (curcnt >= 1) {
                        if (xlib_write_mode) {
                            XRotDrawAlignedString(disp, font, frot, xwin, gc, xpos, ypos, curstr, just);
                        } else {
                            XRotDrawAlignedString(disp, font, frot, xwin, gcclr, xpos, ypos, curstr, just);
                        }
                    }
                    curcnt = 0;
                    if (symfont == 0) {
                        symfont = 1;
                        upperset = 0x80;
                    }
                    xlibsetfont(10);
                    i++;
                    break;
                case 's':
                    curstr[curcnt] = 0;
                    if (curcnt >= 1) {
                        if (xlib_write_mode) {
                            XRotDrawAlignedString(disp, font, frot, xwin, gc, xpos, ypos, curstr, just);
                        } else {
                            XRotDrawAlignedString(disp, font, frot, xwin, gcclr, xpos, ypos, curstr, just);
                        }
                    }
                    curcnt = 0;
                    scale = 0.6 * xlibcharsize;
                    XRotSetMagnification(scale);
                    i++;
                    break;
                case 'S':
                    curstr[curcnt] = 0;
                    if (curcnt >= 1) {
                        if (xlib_write_mode) {
                            XRotDrawAlignedString(disp, font, frot, xwin, gc, xpos, ypos, curstr, just);
                        } else {
                            XRotDrawAlignedString(disp, font, frot, xwin, gcclr, xpos, ypos, curstr, just);
                        }
                    }
                    curcnt = 0;
                    scale = 0.6 * xlibcharsize;
                    XRotSetMagnification(scale);
                    i++;
                    break;
                case 'N':
                    curstr[curcnt] = 0;
                    if (curcnt >= 1) {
                        if (xlib_write_mode) {
                            XRotDrawAlignedString(disp, font, frot, xwin, gc, xpos, ypos, curstr, just);
                        } else {
                            XRotDrawAlignedString(disp, font, frot, xwin, gcclr, xpos, ypos, curstr, just);
                        }
                    }
                    curcnt = 0;
                    scale = xlibcharsize;
                    XRotSetMagnification(scale);
                    i++;
                    break;
                case 'b':
                    i++;
                    break;
                case 'c':
                    upperset = 0x80;
                    i++;
                    break;
                case 'C':
                    upperset = 0;
                    i++;
                    break;
                case 'u':
                    underline = 1;
                    i++;
                    break;
                case 'U':
                    underline = 0;
                    i++;
                    break;
                case '-':
                    curstr[curcnt] = 0;
                    if (curcnt >= 1) {
                        if (xlib_write_mode) {
                            XRotDrawAlignedString(disp, font, frot, xwin, gc, xpos, ypos, curstr, just);
                        } else {
                            XRotDrawAlignedString(disp, font, frot, xwin, gcclr, xpos, ypos, curstr, just);
                        }
                    }
                    curcnt = 0;
                    scale -= 0.2;
                    if (scale < 0.2) {
                        scale = 0.2;
                    }
                    XRotSetMagnification(scale);
                    i++;
                    break;
                case '+':
                    curstr[curcnt] = 0;
                    if (curcnt >= 1) {
                        if (xlib_write_mode) {
                            XRotDrawAlignedString(disp, font, frot, xwin, gc, xpos, ypos, curstr, just);
                        } else {
                            XRotDrawAlignedString(disp, font, frot, xwin, gcclr, xpos, ypos, curstr, just);
                        }
                    }
                    curcnt = 0;
                    scale += 0.2;
                    XRotSetMagnification(scale);
                    i++;
                    break;
                }
                continue;
            } else if (buf[i] == '\\' && buf[i + 1] == '\\') {
                if (upperset) {
                    curstr[curcnt++] = 'U';
                } else {
                    curstr[curcnt++] = '\\';
                    curstr[curcnt++] = buf[i];
                }
                i++;
                continue;
            }
            curstr[curcnt++] = buf[i] + upperset;
        }
        curstr[curcnt] = 0;
        XRotSetMagnification(charsize * xlibcharsize);
        if (xlib_write_mode) {
            XRotDrawAlignedString(disp, font, frot, xwin, gc, xpos, ypos, curstr, just);
        } else {
            XRotDrawAlignedString(disp, font, frot, xwin, gcclr, xpos, ypos, curstr, just);
        }
    } else {

        flush_pending();
        x1 = y1 = 99999;
        puthersh(x, y, xlibcharsize * charsize, rot, just, xlibcolor, drawxlib, buf);
        flush_pending();
        x1 = y1 = 99999;
    }

    free(buf);
}

#define MAXPATTERNS 16

static int patno = 0;

static Pixmap tiles[30];
static Pixmap curtile;

void xlibinit_tiles(void) {
    int i;
    Pixmap ptmp;

    for (i = 0; i < MAXPATTERNS; i++) {
        tiles[i] = XCreatePixmap(disp, xwin, 16, 16, DisplayPlanes(disp, DefaultScreen(disp)));
    }
    for (i = 0; i < MAXPATTERNS; i++) {
        if (tiles[i] == (Pixmap)NULL) {
            printf("bad tile %d\n", i);
        } else {
            XFillRectangle(disp, tiles[i], gcclr, 0, 0, 16, 16);
        }
    }
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat0_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[0], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat1_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[1], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat2_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[2], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat3_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[3], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat4_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[4], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat5_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[5], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat6_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[6], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat7_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[7], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat8_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[8], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat9_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[9], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat10_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[10], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat11_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[11], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat12_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[12], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat13_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[13], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat14_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[14], gc, 0, 0, 16, 16, 0, 0, 1);
    ptmp = XCreateBitmapFromData(disp, xwin, (char*)pat15_bits, 16, 16);
    XCopyPlane(disp, ptmp, tiles[15], gc, 0, 0, 16, 16, 0, 0, 1);
    curtile = tiles[0];
}

int xlibsetpat(int k) {
    patno = k;
    if (k > MAXPATTERNS) {
        k = 1;
    }
    if (patno != 0) {
        curtile = tiles[k - 1];
    }
    return 0;
}

void xlibfill(int n, int* px, int* py) {
    int i;
    XPoint* p;
    GC* cgc;

    p = (XPoint*)calloc(n, sizeof(XPoint));
    if (p == NULL) {
        return;
    }
    if (patno == 0) {
        return;
    }

    if (xlib_write_mode) {
        cgc = &gc;
        XSetFillStyle(disp, *cgc, FillTiled);
        XSetTile(disp, *cgc, curtile);
    } else
        cgc = &gcclr;

    for (i = 0; i < n; i++) {
        p[i].x = px[i];
        p[i].y = win_h - py[i];
    }

    XFillPolygon(disp, displaybuff, *cgc, p, n, Nonconvex, CoordModeOrigin);
    if (backingstore) {
        XFillPolygon(disp, backpix, *cgc, p, n, Nonconvex, CoordModeOrigin);
    }
    XSetFillStyle(disp, *cgc, FillSolid);
    free(p);
}

void xlibfillcolor(int n, int* px, int* py) {
    int i;
    XPoint* p;
    GC cgc;

    p = (XPoint*)calloc(n, sizeof(XPoint));
    if (p == NULL) {
        return;
    }
    for (i = 0; i < n; i++) {
        p[i].x = px[i];
        p[i].y = win_h - py[i];
    }
    if (xlib_write_mode)
        cgc = gc;
    else
        cgc = gcclr;

    XFillPolygon(disp, displaybuff, cgc, p, n, Nonconvex, CoordModeOrigin);
    if (backingstore) {
        XFillPolygon(disp, backpix, cgc, p, n, Nonconvex, CoordModeOrigin);
    }
    free(p);
}

void xlibdrawarc(int x, int y, int r) {
    if (r > 0) {
        XDrawArc(disp, displaybuff, gc, x - r, win_h - (y + r), 2 * r, 2 * r, 0, 360 * 64);
        if (backingstore)
            XDrawArc(disp, backpix, gc, x - r, win_h - (y + r), 2 * r, 2 * r, 0, 360 * 64);
    } else {
        XDrawPoint(disp, displaybuff, gc, x, win_h - y);
        if (backingstore)
            XDrawPoint(disp, backpix, gc, x, win_h - y);
    }
}

void xlibfillarc(int x, int y, int r) {
    if (r > 0) {
        XFillArc(disp, displaybuff, gc, x - r, win_h - (y + r), 2 * r, 2 * r, 0, 360 * 64);
        if (backingstore)
            XFillArc(disp, backpix, gc, x - r, win_h - (y + r), 2 * r, 2 * r, 0, 360 * 64);
    } else {
        XDrawPoint(disp, displaybuff, gc, x, win_h - y);
        if (backingstore)
            XDrawPoint(disp, backpix, gc, x, win_h - y);
    }
}

void xlibdrawellipse(int x, int y, int xm, int ym) {
    GC cgc;

    if (xm == 0) {
        xm = 1;
    }
    if (ym == 0) {
        ym = 1;
    }
    if (xlib_write_mode)
        cgc = gc;
    else
        cgc = gcclr;

    XDrawArc(disp, displaybuff, cgc, x - xm, win_h - (y + ym), 2 * xm, 2 * ym, 0, 360 * 64);
    if (backingstore) {
        XDrawArc(disp, backpix, cgc, x - xm, win_h - (y + ym), 2 * xm, 2 * ym, 0, 360 * 64);
    }
}

void xlibfillellipse(int x, int y, int xm, int ym) {
    GC* cgc;

    if (xm == 0) {
        xm = 1;
    }
    if (ym == 0) {
        ym = 1;
    }

    if (xlib_write_mode) {
        cgc = &gc;
        XSetFillStyle(disp, *cgc, FillTiled);
        XSetTile(disp, *cgc, curtile);
    } else
        cgc = &gcclr;

    XFillArc(disp, displaybuff, *cgc, x - xm, win_h - (y + ym), 2 * xm, 2 * ym, 0, 360 * 64);
    if (backingstore) {
        XFillArc(disp, backpix, *cgc, x - xm, win_h - (y + ym), 2 * xm, 2 * ym, 0, 360 * 64);
    }
    XSetFillStyle(disp, *cgc, FillSolid);
}

void xlibfillellipsecolour(int x, int y, int xm, int ym) {
    GC cgc;

    if (xm == 0) {
        xm = 1;
    }
    if (ym == 0) {
        ym = 1;
    }
    if (xlib_write_mode)
        cgc = gc;
    else
        cgc = gcclr;

    XFillArc(disp, displaybuff, cgc, x - xm, win_h - (y + ym), 2 * xm, 2 * ym, 0, 360 * 64);
    if (backingstore) {
        XFillArc(disp, backpix, cgc, x - xm, win_h - (y + ym), 2 * xm, 2 * ym, 0, 360 * 64);
    }
}

void xlibleavegraphics(void) {
    flush_pending();
    x1 = y1 = 99999;
    xlibsetmode(2);
    save_images = 0;
    XFlush(disp);
}

int xlibinitgraphics(int dmode) {
    xv_pathlength = 0;
    x1 = 99999;
    y1 = 99999;
    if (dmode > 1) {
        save_images = 1;
        dmode = 0;
    }
    xlibdmode = dmode;
    xlibsetmode(1);
    devorient = 1;
    devconvx = xconvxlib;
    devconvy = yconvxlib;
    vector = drawxlib;
    devwritestr = dispstrxlib;
    devsetcolor = xlibsetcolor;
    devsetfont = xlibsetfont;
    devsetline = xlibsetlinestyle;
    devsetlinew = xlibsetlinewidth;
    devdrawtic = xlibdrawtic;
    devsetpat = xlibsetpat;
    devdrawarc = xlibdrawarc;
    devfillarc = xlibfillarc;
    devdrawellipse = xlibdrawellipse;
    devfillellipse = xlibfillellipse;
    devfillellipsecolour = xlibfillellipsecolour;
    devfill = xlibfill;
    devfillcolor = xlibfillcolor;
    devleavegraphics = xlibleavegraphics;
    devxticl = 12;
    devyticl = 12;
    devarrowlength = 12;
    devsymsize = 6;
    devcharsize = xlibcharsize;
    (*devsetcolor)(1);
    xlibsetlinestyle(1);
    return 0;
}

/*
 * cursors
 */

#include <X11/cursorfont.h>

static Cursor wait_cursor;
static Cursor line_cursor;
static Cursor find_cursor;
static Cursor move_cursor;
static Cursor text_cursor;
static Cursor kill_cursor;
static int cur_cursor = -1;
static int waitcursoron = FALSE;

void DefineDialogCursor(Cursor c);
void UndefineDialogCursor();

void set_wait_cursor() {
    XDefineCursor(disp, xwin, wait_cursor);
    DefineDialogCursor(wait_cursor);
    waitcursoron = TRUE;
}

void unset_wait_cursor() {
    UndefineDialogCursor();
    if (cur_cursor == -1) {
        XUndefineCursor(disp, xwin);
    } else {
        set_cursor(cur_cursor);
    }
    waitcursoron = FALSE;
}

void set_cursor(int c) {
    XUndefineCursor(disp, xwin);
    cur_cursor = -1;
    switch (c) {
    case 0:
        XDefineCursor(disp, xwin, line_cursor);
        cur_cursor = 0;
        break;
    case 1:
        XDefineCursor(disp, xwin, find_cursor);
        cur_cursor = 1;
        break;
    case 2:
        XDefineCursor(disp, xwin, text_cursor);
        cur_cursor = 2;
        break;
    case 3:
        XDefineCursor(disp, xwin, kill_cursor);
        cur_cursor = 3;
        break;
    case 4:
        XDefineCursor(disp, xwin, move_cursor);
        cur_cursor = 4;
        break;
    }
    XFlush(disp);
}

void set_window_cursor(Window xwin, int c) {
    XUndefineCursor(disp, xwin);
    switch (c) {
    case 0:
        XDefineCursor(disp, xwin, line_cursor);
        break;
    case 1:
        XDefineCursor(disp, xwin, find_cursor);
        break;
    case 2:
        XDefineCursor(disp, xwin, text_cursor);
        break;
    case 3:
        XDefineCursor(disp, xwin, kill_cursor);
        break;
    case 4:
        XDefineCursor(disp, xwin, move_cursor);
        break;
    case 5:
        XDefineCursor(disp, xwin, wait_cursor);
        break;
    }
    XFlush(disp);
}

void init_cursors(void) {
    wait_cursor = XCreateFontCursor(disp, XC_watch);
    line_cursor = XCreateFontCursor(disp, XC_crosshair);
    find_cursor = XCreateFontCursor(disp, XC_hand2);
    text_cursor = XCreateFontCursor(disp, XC_xterm);
    kill_cursor = XCreateFontCursor(disp, XC_pirate);
    move_cursor = XCreateFontCursor(disp, XC_fleur);
    cur_cursor = -1;
}

void my_doublebuffer(int mode) { xlibdoublebuffer(mode); }

void my_frontbuffer(int mode) { xlibfrontbuffer(mode); }

void my_backbuffer(int mode) { xlibbackbuffer(mode); }

void my_swapbuffer(void) { xlibswapbuffer(); }
