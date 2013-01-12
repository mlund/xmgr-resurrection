/* $Id: globals.h,v 1.7 1995/06/02 03:36:57 pturner Exp pturner $
 *
 * Global variables for gr
 *
 */

#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include <stdlib.h>
#include <stdio.h>

#include "defines.h"
#include "draw.h"

#ifndef PARS
#  ifdef GRCONVERT
#    include "oldpars.h"
#  endif
#endif

/*
 * following for hardcopy device drivers
 */
extern char hp_prstr[], ps_prstr[], mif_prstr[], leaf_prstr[], noprint[];
extern char hp_suffix[], ps_suffix[], eps_suffix[], mif_suffix[], leaf_suffix[];
extern char pstex_suffix[];

#ifdef MAIN

char docname[512] = NONAME;
char description[2048];
char workingdir[MAXPATHLEN];

int debuglevel = 0;
int sigcatch = 1;		/* we handle signals ourselves */

int maxplot = MAXPLOT;
int maxarr = MAXARR;
int maxgraph = MAXGRAPH;
int maxcolors = MAXCOLORS;

int maxboxes = MAXBOXES;
int maxlines = MAXLINES;
int maxstr = MAXSTR;
int maxellipses = MAXELLIPSES;

int ptofile = 0;                /* flag to indicate destination of hardcopy
                                 * output, ptofile = 0 means print to printer
                                 * non-zero print to file */
char printstr[128] = "pout.dat";/* hardcopy to this file */

char sformat[128] = "%16lg %16lg"; /* format for saving (ascii) projects */

double *ax, *bx, *cx, *dx;	/* scratch arrays used in scanner */

int hardcopyflag = FALSE;	/* TRUE if printing out a hardcopy */

int inplotter = FALSE;		/* TRUE if plotting the current graph */
int logwindow = FALSE;		/* TRUE if results are displayed in the log window */

int showdefault = TRUE;		/* display the default graph */
int labeldefault = FALSE;	/* label the default graph */

char currentdir[1024];		/* current directory */
char xmgrdir[1024];		/* location of xmgr home directory */
char help_viewer[256];		/* HTML helper command*/

int domodal;                    /* Make dialogs close on accept */
int canvasw = 800;		/* drawing area width & height */
int canvash = 700;

/*
 * named pipes
 */
int timer_delay = 1000;         /* timer */
int named_pipe;			/* true if named pipe is active */
char pipe_name[512];		/* path to named pipe */

/*
 * scroll amount
 */
int scrolling_islinked = 0;	/* linked scroll */
double scrollper = 0.05;	/* scroll fraction */
double shexper = 0.05;		/* expand/shrink fraction */

int linked_zoom = 0;		/* when zooming, apply to all graphs */

char plfile[MAXPATHLEN];	/* load parameters file name */
char fname[MAXPATHLEN];		/* last data file read */

int device;			/* graphics device */
int tdevice = TDEV;		/* default devices */
int hdevice = HDEV;

int use_colors;			/* number of bitplanes */
int monomode = 0;		/* set mono mode */
int invert = 0;			/* use GXxor or GXinvert for xor'ing */
int backingstore = 0;		/* do backing store if requested */
int redraw_now = 0;		/* hack for no refresh on startup */
int autoscale_onread = 0;	/* autoscale after reading data from fileswin.c */
int allow_refresh = 1;		/* if no backing store, then redraw to refresh */
int free_colors=1;              /* Attempt to free unneeded colors when 
                                   changing colors.  I suspect 
				   that this will free Motif button colors
				   and the such if you happen to make a
				   bad choice of colors.  Thus, you can
				   turn it off.  See xvlib.c.  There's
				   also a resource for this.  */
int use_xvertext = 0;		/* Use xvertext routines to draw text in xvlib.c */

int erronread = FALSE;		/* in startup code was there an error on
				 * reading a data file, assume not  */

int noask = FALSE;              /* if TRUE, assume yes for everything (dangerous) */

int readimage = FALSE;          /* If true read an image */
int imagex;              	/* x in pixels of the location of the image */
int imagey;              	/* y in pixels of the location of the image */
char image_filename[MAXPATHLEN];   	/* image file name */

char progname[MAX_STRING_LENGTH];	/* our name */

int gflag = FALSE;		/* hacks for generic graphics drivers */
char gfile[MAXPATHLEN];		/* used if gflag set to true on command line */
char resfile[MAXPATHLEN];	/* results to file resfile */
FILE *resfp;			/* file for results */

int inwin = FALSE;		/* true if running X */

int auto_redraw = TRUE;		/* if true, redraw graph each time action is
				 * performed */
int status_auto_redraw = TRUE;	/* if true, redraw graph each time action is
				 * performed in the status window */
int force_redraw = 0;		/* if no auto draw and re-draw pressed */

char buf[1024];			/* a string used here and there */

char *curprint;			/* the default printer */
int epsflag = 0;		/* force eps to be written */
int ps2flag = 1;		/* allow use of Level 2 PS (for patterns) */
char *cursuffix;		/* the default printer file suffix*/

int verify_action = 0;		/* request verification of actions if TRUE */
int allow_dc = 1;		/* allow double click ops */

defaults grdefaults;		/* default properties */

/* graph definition */
graph *g;
int cg = 0;			/* the current graph */

/* region definition */
region rg[MAXREGION];
int nr = 0;			/* the current region */

plotstr *pstr;       /* strings */
boxtype *boxes;    /* boxes */
linetype *lines;   /* lines */
arctype *arcs;   /* arcs */
circletype *circs;   /* circles */
ellipsetype *ellip;   /* ellipses */
polytype *polys;   /* polylines */

plotstr defpstr;
linetype defline;
boxtype defbox;
arctype defarc;
circletype defcirc;
ellipsetype defellip={TRUE,COORD_WORLD,0,0,0,0,0,1,1,1,UNFILLED,1,0};

/* lines and ellipses and boxes flags */
int box_color = 1;
int box_lines = 1;
int box_linew = 1;
int box_fill = UNFILLED;
int box_fillpat = 1;
int box_fillcolor = 1;
int box_loctype = COORD_VIEW;

int ellipse_color = 1;
int ellipse_lines = 1;
int ellipse_linew = 1;
int ellipse_fill = UNFILLED;
int ellipse_fillpat = 1;
int ellipse_fillcolor = 1;
int ellipse_loctype = COORD_VIEW;

int line_color = 1;
int line_arrow = 0;
int line_lines = 1;
int line_linew = 1;
int line_loctype = COORD_VIEW;
double line_asize = 1.0;
int line_atype = 0;

/* default string parameters */
int string_color = 1;
int string_linew = 1;
int string_font = 4;
int string_rot = 0;
int string_just = 0;
int string_fill = UNFILLED;
int string_fillpat = 1;
int string_fillcolor = 1;
int string_loctype = COORD_VIEW;
double string_size = 1.0;

int curset, curaxis;
int focus_policy = FOCUS_CLICK;

int use_defaultcmap = 1;
int revflag = 0;

int draw_focus_flag = TRUE;

int graph_focus = 0;

int slice_grid = 0;
int slice_graph = 0;
int slice_setno = 0;
int slicepath_setno = 0;
int slice_load = 0;
int slice_npts = 0;
int slice_first = TRUE;

int page_layout = PAGE_FREE;

plotstr timestamp;       /* timestamp */

/*
 * for the status popup
 */
int cur_statusitem = STATUS_SETS;

/*
 * used in the parser
 */
int cursource = SOURCE_DISK, curtype = SET_XY;

int format_types[] = { FORMAT_DECIMAL, FORMAT_EXPONENTIAL, FORMAT_POWER, FORMAT_GENERAL,
			FORMAT_DDMMYY, FORMAT_MMDDYY, FORMAT_YYMMDD, FORMAT_MMYY, FORMAT_MMDD,
        		FORMAT_MONTHDAY, FORMAT_DAYMONTH, FORMAT_MONTHS, FORMAT_MONTHSY, FORMAT_MONTHL, FORMAT_DAYOFWEEKS,
        		FORMAT_DAYOFWEEKL, FORMAT_DAYOFYEAR, FORMAT_HMS, FORMAT_MMDDHMS, FORMAT_MMDDYYHMS, FORMAT_YYMMDDHMS,
        		FORMAT_DEGREESLON, FORMAT_DEGREESMMLON, FORMAT_DEGREESMMSSLON, FORMAT_MMSSLON,
        		FORMAT_DEGREESLAT, FORMAT_DEGREESMMLAT, FORMAT_DEGREESMMSSLAT, FORMAT_MMSSLAT, FORMAT_INVALID};


/* block data globals */
double **blockdata;
int maxblock = MAXPLOT;
int blocklen;
int blockncols;

#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
int readcdf = 0;
char netcdf_name[512], xvar_name[128], yvar_name[128];
#endif

/* parameters for non-linear fit */
nonlparms nonl_parms[MAXPARM];
nonlopts nonl_opts;
nonlprefs nonl_prefs;

int symcolorbug = 0;

target target_set; /* target */

int dl_load_fast = TRUE; /* controls type of DL module load */
/* TODO: make it tunable through a command */

#endif

#ifndef MAIN

extern char *open_err_msg;

extern char version[];
extern char docname[];
extern char description[];
extern char workingdir[];

extern int debuglevel;
extern int sigcatch;		/* true if we handle signals ourselves */

extern int inwin;		/* true if running X */
extern int ispipe;		/* true if reading from stdin */

extern int maxarr, maxplot, maxgraph, maxcolors;

extern int maxboxes;
extern int maxlines;
extern int maxstr;
extern int maxellipses;

extern int ptofile;		/* flag to indicate destination of hardcopy
                                 * output, ptofile = 0 means print to printer
                                 * non-zero print to file */

extern char printstr[];         /* print to this file */

extern char sformat[];

extern int logwindow;		/* TRUE if results are displayed in the log window */

extern char plfile[], psfile[];
extern char resfile[];
extern FILE *resfp;

extern int device, tdevice, hdevice;
extern int hardcopyflag;

extern int use_colors;		/* number of bitplanes */
extern int monomode;		/* set mono mode */
extern int invert;		/* use GXxor or GXinvert for xor'ing */
extern int backingstore;	/* do backing store if requested */
extern int redraw_now;		/* hack for no refresh on startup */
extern int autoscale_onread;	/* autoscale after reading data from fileswin.c */
extern int allow_refresh;	/* if no backingstore, then redraw to refresh */
extern int free_colors;         /* Attempt to free unneeded colors when 
                                   changing colors.  I suspect 
				   that this will free Motif button colors
				   and the such if you happen to make a
				   bad choice of colors.  Thus, you can
				   turn it off.  See xvlib.c.  There's
				   also a resource for this.  */
extern int use_xvertext;	/* Use xvertext routines to draw text in xvlib.c */
extern int noask;		/* if TRUE, assume yes for everything (dangerous) */
extern int readimage;		/* If true read an image */
extern int readimage;		/* If true read an image */
extern int imagex;		/* x in pixels of the location of the image */
extern int imagey;		/* y in pixels of the location of the image */
extern char image_filename[];	/* image file name */

extern int inplotter;

extern int showdefault;
extern int labeldefault;

extern char currentdir[];	/* current directory */
extern char xmgrdir[];		/* location of ACE/gr home directory */
extern char help_viewer[];	/* help viewer command*/

extern int domodal;             /* Make dialogs close on accept */
extern int canvasw;		/* canvas dimensions */
extern int canvash;

extern int timer_delay;		/* timer */
extern int named_pipe;		/* true if named pipe is active */
extern char pipe_name[];	/* path to named pipe */

extern int scrolling_islinked;	/* linked scroll */
extern double scrollper;	/* scroll fraction */
extern double shexper;		/* expand/shrink fraction */
extern int linked_zoom;		/* when zooming, apply to all graphs */

extern double errbarper;

extern char fname[];
extern int nsets;
extern char buf[];

extern char *curprint;
extern int epsflag;		/* force eps to be written */
extern int ps2flag ;		/* allow use of Level 2 PS (for patterns) */
extern char *cursuffix;		/* the default printer file suffix*/

extern int verify_action;	/* request verification of actions if TRUE */
extern int allow_dc;		/* allow double click ops */

extern double *ax, *bx, *cx, *dx;

extern defaults grdefaults;	/* default properties */

/* graph definition */
extern graph *g;
extern int cg;

/* region definition */
extern region rg[];
extern int nr;

extern plotstr *pstr;       /* strings */
extern boxtype *boxes;    /* boxes */
extern linetype *lines;   /* lines */
extern arctype *arcs;   /* arcs */
extern circletype *circs;   /* circles */
extern ellipsetype *ellip;   /* ellipses */

extern plotstr defpstr;
extern linetype defline;
extern boxtype defbox;
extern arctype defarc;
extern circletype defcirc;
extern ellipsetype defellip;

extern int box_color;
extern int box_lines;
extern int box_linew;
extern int box_fill;
extern int box_fillpat;
extern int box_fillcolor;
extern int box_loctype;

extern int ellipse_color;
extern int ellipse_lines;
extern int ellipse_linew;
extern int ellipse_fill;
extern int ellipse_fillpat;
extern int ellipse_fillcolor;
extern int ellipse_loctype;

extern int line_color;
extern int line_arrow;
extern int line_lines;
extern int line_linew;
extern int line_loctype;
extern double line_asize;
extern int line_atype;

extern int string_color;
extern int string_linew;
extern int string_font;
extern int string_rot;
extern int string_just;
extern int string_fill;
extern int string_fillpat;
extern int string_fillcolor;
extern int string_loctype;
extern double string_size;

extern int auto_redraw;
extern int status_auto_redraw;
extern int force_redraw;

extern double charsize, xlibcharsize;	/* declared in draw.c and xlib.c resp. */

extern int curset, curaxis;
extern int focus_policy;
extern int draw_focus_flag;
extern int graph_focus;
extern int use_defaultcmap;
extern int revflag;

extern plotstr timestamp;       /* timestamp */
extern int page_layout;

extern int slice_grid;
extern int slice_graph;
extern int slice_setno;
extern int slicepath_setno;
extern int slice_load;
extern int slice_npts;
extern int slice_first;

extern int cur_statusitem;

extern int cursource, curtype;
extern int format_types[];

extern double **blockdata;
extern int maxblock;
extern int blocklen;
extern int blockncols;

#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
extern int readcdf;
extern char netcdf_name[];
extern char xvar_name[];
extern char yvar_name[];
#endif

/* parameters for non-linear fit */
extern nonlparms nonl_parms[];
extern nonlopts nonl_opts;
extern nonlprefs nonl_prefs;

extern int symcolorbug;

extern target target_set; /* target */

extern int dl_load_fast; /* controls type of DL module load */

#endif

#endif /* __GLOBALS_H_ */
