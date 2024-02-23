/* $Id: defines.h,v 1.9 1995/07/01 04:53:30 pturner Exp pturner $
 *
 * constants and typedefs
 *
 */
#ifndef __DEFINES_H_
#define __DEFINES_H_

#include <config.h>
#include <stdint.h>

/*
 * Have a report of a system that doesn't have this
 * in anything included
 */
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#define MAX_BUF_LEN 128

#if !defined(MAXHOSTNAMELEN)
#define MAXHOSTNAMELEN 256
#endif

/*
 * some constants
 *
 */
#define MAX_SET_COLS 6        /* max number of data columns for a set */
#define MAXPLOT 30            /* max number of sets in a graph */
#define MAXGRAPH 10           /* max number of graphs */
#define MAX_ZOOM_STACK 20     /* max stack depth for world stack */
#define MAXREGION 5           /* max number of regions */
#define MAX_TICK_LABELS 40    /* max number of user defined ticks/labels */
#define MAX_STRING_LENGTH 512 /* max length for strings */
#define MAXBOXES 50           /* max number of boxes */
#define MAXLINES 50           /* max number of lines */
#define MAXELLIPSES 50        /* max number of ellipses */
#define MAXSTR 100            /* max number of strings */
#define MAXSYM 47             /* max number of symbols */
#define MAX_LINESTYLE 5       /* max number of linestyles */
#define MAX_LINEWIDTH 10      /* max number of line widths */
#define MAXCOLORS 256         /* max number of colors */
#define MAXAXES 4             /* max number of axes per graph */

#define MAXPARM 10 /* max number of parameters for non-lin fit */

/*
 *  location types for objects
 */
#define LOCWORLD 0
#define LOCVIEW 1

#ifndef MAXARR
#define MAXARR 20000 /* max elements in an array */
#endif

#define MAXFIT                                                                                                         \
    12 /* max degree of polynomial+1 that can be                                                                       \
        * fitted */

#ifndef MAXPICKDIST
#define MAXPICKDIST 0.1 /* the maximum distance away from an object */
#endif                  /* you may be when picking it (in viewport  */
/* coordinates)				    */

#define DATASET_MISSING (1.23456789e+30)

/*
 * types of tick displays
 */
#define X_AXIS 0
#define Y_AXIS 1
#define ZX_AXIS 2
#define ZY_AXIS 3
#define POLAR_RADIUS 6
#define POLAR_ANGLE 7

/*
 * grid types
 */
#define GRID_Z 0
#define GRID_UV 1
#define GRID_ZUV 2
#define GRID_ZUVW 3
#define GRID_ZW 4

/* type of splines */
#define SPLINE_NONE 0
#define SPLINE_CUBIC 1
#define SPLINE_AKIMA 2

/* graphics output to the following */
#define GR_FILE                                                                                                        \
    -1 /* print to file; TODO: no need to define it                                                                    \
               here */
#define GR_X 0
#define GR_PS_L 1      /* PostScript landscape */
#define GR_PS_P 2      /* PostScript portrait */
#define GR_MIF_L 3     /* mif landscape */
#define GR_MIF_P 4     /* mif portrait */
#define GR_HPGL_L 5    /* hpgl landscape */
#define GR_HPGL_P 6    /* hpgl portrait */
#define GR_LEAF_L 7    /* InterLeaf landscape */
#define GR_LEAF_P 8    /* InterLeaf portrait */
#define GR_PSTEX_INC 9 /* PSTEX to be included */
#define GR_PSTEX_L 10  /* PSTEX stand alone potrait*/
#define GR_PSTEX_P 11  /* PSTEX stand alone landscape*/

/* Orientation and page size */
/* TODO: LANDSCAPE and PORTRAIT are just special cases of FIXED */
#define PAGE_FREE 0
#define PAGE_FIXED 1
#define PAGE_LANDSCAPE 2
#define PAGE_PORTRAIT 3

/* Graph type */
#define GRAPH_XY 0
#define GRAPH_LOGX 1  /* TODO: must be in axis defines */
#define GRAPH_LOGY 2  /* TODO: must be in axis defines */
#define GRAPH_LOGXY 3 /* TODO: must be in axis defines */
#define GRAPH_BAR 4
#define GRAPH_HBAR 5
#define GRAPH_STACKEDBAR 6
#define GRAPH_STACKEDHBAR 7
#define GRAPH_STACKEDLINE 8
#define GRAPH_POLAR 9
#define GRAPH_SMITH 10

/* Set type */
#define SET_XY 0
#define SET_XYBOX 2
#define SET_XYBOXPLOT 3
#define SET_XYHILO 4
#define SET_XYRT 5
#define SET_XYSTRING 7
#define SET_XYDX 8
#define SET_XYDY 9
#define SET_XYDXDX 10
#define SET_XYDYDY 11
#define SET_XYDXDY 12
#define SET_XYXX 14
#define SET_XYYY 15
#define SET_XYZ 16
#define SET_XYZW 17
#define SET_XYUV 18
#define SET_NXY 20
#define SET_BIN 21
#define SET_POLY 22
#define SET_RAWSPICE 23
#define SET_IHL 24
#define SET_BLOCK 25

/* Data column names; */
#define DATA_INDEX 0 /* reserved */
#define DATA_X 1
#define DATA_Y 2
#define DATA_Y1 3
#define DATA_Y2 4
#define DATA_Y3 5
#define DATA_Y4 6

/* Toolbars */
#define BAR_TOOLBAR 0
#define BAR_LOCATORBAR 1
#define BAR_STATUSBAR 2

/* Strings and things */
#define OBJECT_LINE 0
#define OBJECT_BOX 1
#define OBJECT_ELLIPSE 2
#define OBJECT_STRING 3

/* Region definitions */
#define REGION_ABOVE 0
#define REGION_BELOW 1
#define REGION_TOLEFT 2
#define REGION_TORIGHT 3
#define REGION_POLYI 4
#define REGION_POLYO 5

/* Axis label layout */
#define LAYOUT_PARALLEL 0
#define LAYOUT_PERPENDICULAR 1

/* Axis & tick placement */
#define PLACE_LEFT 0
#define PLACE_RIGHT 1
#define PLACE_TOP 2
#define PLACE_BOTTOM 3
#define PLACE_BOTH 4

/* Tick label placement */
#define LABEL_ONTICK 0
#define LABEL_BETWEEN 1

/* Objects to display in status window */
#define STATUS_SETS 0
#define STATUS_GRAPHS 1
#define STATUS_REGIONS 2

/* Coordinates */
#define COORD_VIEW 0
#define COORD_WORLD 1

/* Tick sign type */
#define SIGN_NORMAL 0
#define SIGN_ABSOLUTE 1
#define SIGN_NEGATE 2

/* Tick label/display formats */
#define FORMAT_INVALID -1
#define FORMAT_DECIMAL 0
#define FORMAT_EXPONENTIAL 1
#define FORMAT_POWER 2
#define FORMAT_GENERAL 3
#define FORMAT_DDMMYY 4
#define FORMAT_MMDDYY 5
#define FORMAT_YYMMDD 6
#define FORMAT_MMYY 7
#define FORMAT_MMDD 8
#define FORMAT_MONTHDAY 9
#define FORMAT_DAYMONTH 10
#define FORMAT_MONTHS 11
#define FORMAT_MONTHSY 12
#define FORMAT_MONTHL 13
#define FORMAT_DAYOFWEEKS 14
#define FORMAT_DAYOFWEEKL 15
#define FORMAT_DAYOFYEAR 16
#define FORMAT_HMS 17
#define FORMAT_MMDDHMS 18
#define FORMAT_MMDDYYHMS 19
#define FORMAT_YYMMDDHMS 20
#define FORMAT_DEGREESLON 21
#define FORMAT_DEGREESMMLON 22
#define FORMAT_DEGREESMMSSLON 23
#define FORMAT_MMSSLON 24
#define FORMAT_DEGREESLAT 25
#define FORMAT_DEGREESMMLAT 26
#define FORMAT_DEGREESMMSSLAT 27
#define FORMAT_MMSSLAT 28

/* Focus policy */
#define FOCUS_CLICK 0
#define FOCUS_SET 1
#define FOCUS_FOLLOWS 2

/* Autoscale, tick mark etc. type */
#define TYPE_AUTO 0
#define TYPE_SPEC 10 /* TODO: used in a lot of places; rename! */

/* Tick label layout; TODO: obsolete, the angle is enough! */
#define TICKS_HORIZONTAL 0
#define TICKS_VERTICAL 1

/* Tick direction */
#define TICKS_IN 0
#define TICKS_OUT 1
#define TICKS_BOTH 2

/* Data source type */
#define SOURCE_DISK 0
#define SOURCE_PIPE 1
#define SOURCE_STDIN 2

/* Axis label justification */
#define JUST_LEFT 0
#define JUST_RIGHT 1
#define JUST_CENTER 2

/* Types of running command */
#define RUN_AVG 0
#define RUN_MED 1
#define RUN_MIN 2
#define RUN_MAX 3
#define RUN_STD 4

/* Types of Fourier transforms */
#define FFT_FFT 0
#define FFT_INVFFT 1
#define FFT_DFT 2
#define FFT_INVDFT 3

/* set HDEV to the default hardcopy device */
#ifndef HDEV
#define HDEV GR_PS_L
#endif

/* TDEV is always X */
#define TDEV GR_X

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*
 * for set selector gadgets
 */
#define SET_SELECT_ERROR -99
#define SET_SELECT_ACTIVE 0
#define SET_SELECT_ALL -1
#define SET_SELECT_NEXT -2
#define SET_SELECT_NEAREST -3
#define GRAPH_SELECT_CURRENT -1
#define GRAPH_SELECT_ALL -2
#define FILTER_SELECT_NONE 0
#define FILTER_SELECT_ACTIVE 1
#define FILTER_SELECT_ALL 2
#define FILTER_SELECT_INACT 3
#define FILTER_SELECT_DEACT 4
#define FILTER_SELECT_SORT 5
#define SELECTION_TYPE_SINGLE 0
#define SELECTION_TYPE_MULTIPLE 1

/* for canvas event proc */
#define ZOOM_1ST 1
#define ZOOM_2ND 2
#define VIEW_1ST 3
#define VIEW_2ND 4
#define STR_LOC 5
#define LEG_LOC 6
#define FIND_POINT 7
#define DEL_POINT 8
#define MOVE_POINT1ST 9
#define MOVE_POINT2ND 10
#define ADD_POINT 11
#define DEL_OBJECT 12
#define MOVE_OBJECT_1ST 13
#define MOVE_OBJECT_2ND 14
#define MAKE_BOX_1ST 15
#define MAKE_BOX_2ND 16
#define MAKE_LINE_1ST 17
#define MAKE_LINE_2ND 18
#define MAKE_CIRC_1ST 19
#define MAKE_CIRC_2ND 20
#define MAKE_ARC_1ST 21
#define MAKE_ARC_2ND 22
#define MAKE_ELLIP_1ST 23
#define MAKE_ELLIP_2ND 24
#define SEL_POINT 25
#define STR_EDIT 26
#define COMP_AREA 27
#define COMP_PERIMETER 28
#define STR_LOC1ST 29
#define STR_LOC2ND 30
#define GRAPH_FOCUS 31
#define TRACKER 32
#define INTERP_GRID 33
#define FIND_GRID 34
#define FIND_GRIDPT 35
#define FIND_GRIDRECT 36
#define SLICE_GRIDDIST 37
#define SLICE_GRIDX 38
#define SLICE_GRIDY 39
#define SLICE_GRIDLINE_1ST 40
#define SLICE_GRIDLINE_2ND 41
#define SLICE_GRIDPATH 42
#define DEF_REGION 43
#define DEF_REGION1ST 44
#define DEF_REGION2ND 45
#define PAINT_POINTS 46
#define KILL_NEAREST 47
#define COPY_NEAREST1ST 48
#define COPY_NEAREST2ND 49
#define MOVE_NEAREST1ST 50
#define MOVE_NEAREST2ND 51
#define REVERSE_NEAREST 52
#define JOIN_NEAREST1ST 53
#define JOIN_NEAREST2ND 54
#define DEACTIVATE_NEAREST 55
#define EXTRACT_NEAREST1ST 56
#define EXTRACT_NEAREST2ND 57
#define DELETE_NEAREST1ST 58
#define DELETE_NEAREST2ND 59
#define INSERT_POINTS 60
#define INSERT_SET 61
#define EDIT_OBJECT 62
#define PLACE_TIMESTAMP 63
#define COPY_OBJECT1ST 64
#define COPY_OBJECT2ND 65
#define CUT_OBJECT 66
#define PASTE_OBJECT 67
#define AUTO_NEAREST 68
#define ZOOMX_1ST 69
#define ZOOMX_2ND 70
#define ZOOMY_1ST 71
#define ZOOMY_2ND 72
#define PICK_SET 73
#define PICK_SET1 74
#define PICK_SET2 75
#define PICK_EXPR 76
#define PICK_HISTO 77
#define PICK_FOURIER 78
#define PICK_RUNAVG 79
#define PICK_RUNSTD 80
#define PICK_RUNMIN 81
#define PICK_RUNMAX 82
#define PICK_DIFF 83
#define PICK_INT 84
#define PICK_REG 85
#define PICK_XCOR 86
#define PICK_SAMP 87
#define PICK_PRUNE 88
#define PICK_FILTER 89
#define PICK_EXPR2 90
#define PICK_SPLINE 91
#define PICK_INTERP 92
#define PICK_SAMPLE 93
#define PICK_SEASONAL 94
#define PICK_BREAK 95
#define ADD_POINT1ST 96
#define ADD_POINT2ND 97
#define ADD_POINT3RD 98
#define ADD_POINT_INTERIOR 99
#define DISLINE1ST 100
#define DISLINE2ND 101

/* for stufftext() in monwin.c used here and there */
#define STUFF_TEXT 0
#define STUFF_START 1
#define STUFF_STOP 2

#define NONAME "Untitled"

/* for data pruning */
#define PRUNE_INTERPOLATION 0
#define PRUNE_CIRCLE 1
#define PRUNE_ELLIPSE 2
#define PRUNE_RECTANGLE 3
#define PRUNE_LIN 0
#define PRUNE_LOG 1
#define PRUNE_VIEWPORT 0
#define PRUNE_WORLD 1

/*
 * symbol table entry type
 */
typedef struct {
    char* s;
    int type;
    double (*fnc)();
} symtab_entry;

/*
 * defaults
 */
typedef struct {
    int color;
    int lines;
    int linew;
    double charsize;
    int font;
    int fontsrc;
    double symsize;
} defaults;

/*
 * typedefs for objects
 */
typedef struct {
    int active;
    int loctype;
    int gno;
    double x1;
    double y1;
    double x2;
    double y2;
    int lines;
    int linew;
    int color;
    int fill;
    int fillcolor;
    int fillpattern;
} boxtype;

typedef struct {
    int active;
    int loctype;
    int gno;
    double x1;
    double y1;
    double x2;
    double y2;
    int lines;
    int linew;
    int color;
    int arrow;
    int atype;
    double asize;
} linetype;

typedef struct {
    int active;
    int loctype;
    int gno;
    double xc;
    double yc;
    double r;
    double th1;
    double th2;
    int lines;
    int linew;
    int color;
    int fill;
    int fillcolor;
    int fillpattern;
    int arrow;
    int atype;
    double asize;
} arctype;

typedef struct {
    int active;
    int loctype;
    int gno;
    double xc;
    double yc;
    double r;
    int lines;
    int linew;
    int color;
    int fill;
    int fillcolor;
    int fillpattern;
} circletype;

typedef struct {
    int active;
    int loctype;
    int gno;
    double x1;
    double y1;
    double x2;
    double y2;
    int lines;
    int linew;
    int color;
    int fill;
    int fillcolor;
    int fillpattern;
} ellipsetype;

typedef struct {
    int active;
    int loctype;
    int gno;
    double x;
    double y;
    int lines;
    int linew;
    int color;
    int rot;
    int font;
    int just;
    double charsize;
    char* s;
} plotstr;

typedef struct {
    int active;
    int loctype;
    int gno;
    int type;
    int n;
    double* x;
    double* y;
    int lines;
    int linew;
    int color;
    int fill;
    int fillcolor;
    int fillpattern;
    int arrow;
    int atype;
    double asize;
} polytype;

typedef struct {
    int active;        /* velocity legend on or off */
    int type;          /* velocity type */
    int color;         /* velocity color */
    int lines;         /* velocity linestyle */
    int linew;         /* velocity line width */
    int arrowtype;     /* velocity arrow type, fixed or variable head */
    int loctype;       /* world or viewport coords for legend */
    double velx, vely; /* location of velocity legend */
    double vscale;     /* velocity scale */
    int units;         /* units of flow field */
    double userlength; /* length of the legend vector in user units */
    plotstr vstr;      /* legend string for velocity legend */
} velocityp;

typedef struct {
    double xg1, xg2, yg1, yg2; /* window into world coords */
} world;

typedef struct {
    double xv1, xv2, yv1, yv2; /* device viewport */
} view;

/*
 * world stack
 */
typedef struct {
    world w;           /* current world */
    world t[3];        /* current tick spacing */
    int prec[MAXAXES]; /* precision of labels */
} world_stack;

typedef struct {
    plotstr title;  /* graph title */
    plotstr stitle; /* graph subtitle */
} labels;

typedef struct {
    int active; /* active flag */
    int type;   /* regression type */
    double xmin;
    double xmax;
    double coef[15];
} Regression;

typedef struct {
    int active; /* active flag */
    int type;   /* regression type */
    int npts;   /* number of points */
    double xmin;
    double xmax;
    double* a;
    double* b;
    double* c;
    double* d;
} Spline;

typedef struct {
    int active;         /* active flag */
    int type;           /* dataset type */
    int deact;          /* deactivated set */
    int len;            /* set length */
    int nx, ny;         /* number of pts in X and Y for grids */
    int setno;          /* set number */
    int gno;            /* graph number */
    char comments[256]; /* how did this set originate */
    char lstr[256];     /* legend for this set */

    double missing;           /* value for missing data */
    double* ex[MAX_SET_COLS]; /* x, y, dx, z, r, hi depending on dataset type */
    char** s;                 /* pointer to strings */
    int nel;                  /* # of ? */
    int** con;                /* con */

    double xmin, xmax; /* min max for x */
    double ymin, ymax; /* min max for y */

    int sym;        /* set plot symbol */
    char symchar;   /* character for symbol */
    int symskip;    /* How many symbols to skip */
    int symfill;    /* Symbol fill type */
    int symdot;     /* Symbol dot in center */
    int symlines;   /* Symbol linestyle */
    int symlinew;   /* Symbol linewidth */
    int symcolor;   /* color for symbol line */
    double symsize; /* size of symbols */

    int avgflag;       /* average */
    int avgstdflag;    /* average+- std */
    int avg2stdflag;   /* average+- 2std */
    int avg3stdflag;   /* average+- 3std */
    int avgallflag;    /* average+- 3std */
    int avgvalflag;    /* average+- val */
    int harmonicflag;  /* harmonic mean */
    int geometricflag; /* geometric */

    int font;       /* font for strings */
    int format;     /* format for drawing values */
    int prec;       /* precision for drawing values */
    int just;       /* justification for drawing values */
    int where;      /* where to draw values */
    double valsize; /* char size for drawing values */

    int lines;    /* set line style */
    int linew;    /* line width */
    int color;    /* color for linestyle */
    int lineskip; /* How many points to skip when drawing lines */
    int clipflag; /* turn clipping on or off for this set */

    int fill;        /* fill type */
    int fillusing;   /* fill using color or pattern */
    int fillcolor;   /* fill color */
    int fillpattern; /* fill pattern */

    int errbar;             /* if type is _DX, _DY, _DXDY and errbar = TRUE */
    int errbarxy;           /* type of error bar */
    int errbar_linew;       /* error bar line width */
    int errbar_lines;       /* error bar line style */
    int errbar_riser;       /* connecting line between error limits */
    int errbar_riser_linew; /* connecting line between error limits line width */
    int errbar_riser_lines; /* connecting line between error limits line style */

    double errbarper; /* length of error bar */
    double hilowper;  /* length of hi-low */

    int density_plot;  /* if type is XYZ then density_plot  = 1 */
    double zmin, zmax; /* min max for density plots */

    int hotlink;       /* hot linked set */
    int hotsrc;        /* source for hot linked file (DISK|PIPE) */
    char hotfile[256]; /* hot linked filename */

    double emin[MAX_SET_COLS]; /* min for each column */
    double emax[MAX_SET_COLS]; /* max for each column */
    int imin[MAX_SET_COLS];    /* min loc for each column */
    int imax[MAX_SET_COLS];    /* max loc for each column */

    Regression* r; /* coefs from any regression performed on this set */
    Spline* spl;   /* coefs from any spline performed on this set */

    void* ep; /* pointer to EditPoints structure */

} plotarr;

typedef struct {
    int axis;              /* which axis */
    int active;            /* active or not */
    int alt;               /* alternate map if TRUE */
    double tmin, tmax;     /* mapping for alternate tickmarks */
    double tmajor, tminor; /* major, minor tick divisions */
    double offsx, offsy;   /* offset of axes in viewport coords */
    plotstr label;         /* graph axis label */
    int label_layout;      /* axis label orientation (h or v) */
    int label_place;       /* axis label placement (specfied or auto) */
    int tl_flag;           /* toggle ticmark labels on or off */
    int tl_type;           /* either auto or specified (below) */
    int tl_loc;            /* Tick label location, at tick, between ticks */
    int tl_layout;         /* horizontal, vertical, or specified */
    int tl_angle;          /* angle to draw labels if layout is specified */
    int tl_sign;           /* tick labels normal, absolute value, or negate */
    int tl_just;           /* justification of ticklabel and type of anchor point */
    int tl_prec;           /* places to right of decimal point */
    int tl_format;         /* decimal or exponential ticmark labels .. */
    int tl_skip;           /* tick labels to skip */
    int tl_staggered;      /* tick labels staggered */
    int tl_starttype;      /* start at graphmin or use tl_start/stop */
    int tl_stoptype;       /* start at graphmax or use tl_start/stop */
    double tl_start;       /* value of x to begin tick labels and major ticks */
    double tl_stop;        /* value of x to begin tick labels and major ticks */
    int tl_op;             /* tick labels on opposite side or both */
    double tl_vgap;        /* tick label to tickmark distance vertically */
    double tl_hgap;        /* tick label to tickmark distance horizontally */
    int tl_font;           /* font to use for labels */
    double tl_charsize;    /* character size for labels */
    int tl_color;          /* color */
    int tl_linew;          /* line width for labels */
    char tl_appstr[256];   /* append string to tick label */
    char tl_prestr[256];   /* prepend string to tick label */
    int t_type;            /* type of tickmarks, usual, xticstart, or specified */
    int t_flag;            /* toggle tickmark display */
    int t_mflag;           /* toggle minor tickmark display */
    int t_integer;         /* major tic marks on integer divisions */
    int t_num;             /* approximate default number of X-axis ticks */
    int t_inout;           /* ticks inward, outward or both */
    int t_log;             /* logarithmic ticmarks */
    int t_op;              /* ticks on opposite side */
    int t_color;           /* colors and linestyles */
    int t_lines;
    int t_linew;
    int t_mcolor;
    int t_mlines;
    int t_mlinew;       /* minor grid colors and linestyles */
    double t_size;      /* length of tickmarks */
    double t_msize;     /* length of minor tickmarks */
    int t_drawbar;      /* draw a bar connecting tick marks */
    int t_drawbarcolor; /* color of bar */
    int t_drawbarlines; /* linestyle of bar */
    int t_drawbarlinew; /* line width of bar */
    int t_gridflag;     /* grid lines at major tick marks */
    int t_mgridflag;    /* grid lines at minor tick marks */
    int t_spec;         /* number of ticks at specified locations */
    double t_specloc[MAX_TICK_LABELS];
    plotstr t_speclab[MAX_TICK_LABELS];
    int spec_font;
    double spec_charsize;
    int spec_color;
    int spec_linew;
} tickmarks;

typedef struct {
    int active;  /* legend on or off */
    int loctype; /* locate in world or viewport coords */
    int layout;  /* verticle or horizontal */
    int vgap;    /* verticle gap between entries */
    int hgap;    /* horizontal gap between entries */
    int len;     /* length of line to draw */
    int box;     /* box around legend on or off */
    double legx; /* location on graph */
    double legy;
    int font;
    double charsize;
    int color;
    int linew;
    int lines;
    int boxfill;      /* legend frame fill toggle */
    int boxfillusing; /* legend frame fill type */
    int boxfillcolor; /* legend frame fill color */
    int boxfillpat;   /* legend frame fill pattern */
    int boxlcolor;    /* legend frame line color */
    int boxlinew;     /* legend frame line width */
    int boxlines;     /* legend frame line style */
} legend;

typedef struct {
    int active;            /* region on or off */
    int type;              /* region type */
    int color;             /* region color */
    int lines;             /* region linestyle */
    int linew;             /* region line width */
    int* linkto;           /* associated with graphs in linkto */
    int n;                 /* number of points if type is POLY */
    double *x, *y;         /* coordinates if type is POLY */
    double x1, y1, x2, y2; /* starting and ending points if type is not POLY */
} region;

typedef struct {
    int active;  /* frame on or off */
    int type;    /* frame type */
    int color;   /* frame color */
    int lines;   /* frame linestyle */
    int linew;   /* frame line width */
    int fillbg;  /* fill background */
    int bgcolor; /* background color inside frame */
} framep;

/*
If you decide to incorporate these into xmgr (I and others here
would be eternally grateful if you did), then I would suggest that
the following should be user-defined quantities:
1) The inner limits (the box, 25th and 75th percentiles in the example),
2) The outer limits (the whiskers, 10th and 90th percentiles in the example),
3) The number of points below which the box plot is unacceptable, and
therefore the points are plotted instead (e.g. 10 in the examples), and
4) Whether to plot outlying points or not.
*/

typedef struct _BoxPlot {
    double il;    /* inner lower limit */
    double iu;    /* inner upper limit */
    double ol;    /* outer lower limit */
    double ou;    /* outer uppper limit */
    int nthresh;  /* threshhold for number of points for
                   * boxplot */
    int outliers; /* plot outliers */
    int wtype;    /* 1 = width by std dev or 0 = symsize */
    double boxwid;
} BoxPlot;

/*
 * a graph
 */
typedef struct {
    int active;                     /* alive or dead */
    int hidden;                     /* display or not */
    int label;                      /* label graph */
    int type;                       /* type of graph */
    int clipflag;                   /* turn clipping on or off */
    int autoscale;                  /* */
    int noautoscale;                /* */
    int noauto_world;               /* only time this is used is at startup */
    int noauto_tics;                /* only time this is used is at startup */
    int auto_type;                  /* */
    int parmsread;                  /* was a paramter file read for this graph */
    int revx, revy;                 /* reverse mapping for x and y if true */
    int maxplot;                    /* max number of sets for this graph */
    plotarr* p;                     /* sets go here */
    legend l;                       /* legends */
    world w;                        /* world */
    view v;                         /* world/view */
    world rt;                       /* world for polar plots */
    labels labs;                    /* title, subtitle, axes labels */
    tickmarks t[MAXAXES];           /* flags etc. for tickmarks for all axes */
    framep f;                       /* type of box around plot */
    int pointset;                   /* if (dsx, dsy) have been set */
    int pt_type;                    /* type of locator display */
    double dsx, dsy;                /* locator fixed point */
    int fx, fy;                     /* locator format type */
    int px, py;                     /* locator precision */
    double barwid;                  /* bar width for bar charts */
    double sbarwid;                 /* bar width for stacked bar charts */
    world_stack ws[MAX_ZOOM_STACK]; /* zoom stack */
    int ws_top;                     /* stack pointer */
    int curw;                       /* for cycling through the stack */
    velocityp vp;
    BoxPlot bp;
} graph;

/* parameters for non-linear fit */
typedef struct {
    double value; /* parameter itself */
    int constr;   /* whether or not to use constraints */
    double min;   /* low bound constraint */
    double max;   /* upper bound constraint */
} nonlparms;

/* options for non-linear fit */
typedef struct {
    char title[256];   /* fit title */
    char formula[256]; /* fit function */
    int parnum;        /* # of fit parameters */
    double tolerance;  /* tolerance */
} nonlopts;

/* prefs for non-linear fit */
typedef struct {
    int autoload; /* do autoload */
    int load;     /* load to... */
    int npoints;  /* # of points to evaluate function at */
    double start; /* start... */
    double stop;  /* stop ... */
} nonlprefs;

/* nonlprefs.load possible values */
#define LOAD_VALUES 0
#define LOAD_RESIDUALS 1
#define LOAD_FUNCTION 2

/* target graph & set*/
typedef struct {
    int gno;   /* graph # */
    int setno; /* set # */
} target;

#define copyx(gno, setfrom, setto) copycol(gno, setfrom, setto, 0)
#define copyy(gno, setfrom, setto) copycol(gno, setfrom, setto, 1)
#define getsetlength(gno, set) (g[gno].p[set].len)
#define getx(gno, set) ((double*)g[gno].p[set].ex[0])
#define gety(gno, set) ((double*)g[gno].p[set].ex[1])
#define getcol(gno, set, col) ((double*)g[gno].p[set].ex[col])
#define getcomment(gno, set) ((char*)g[gno].p[set].comments)
#define getsetlines(gno, set) (g[gno].p[set].lines)
#define getsetlinew(gno, set) (g[gno].p[set].linew)
#define getsetcolor(gno, set) (g[gno].p[set].color)
#define getsetplotsym(gno, set) (g[gno].p[set].sym)
#define getsetplotsymcolor(gno, set) (g[gno].p[set].symcolor)
#define getsetplotsymsize(gno, set) (g[gno].p[set].symsize)
#define getsetplotsymchar(gno, set) (g[gno].p[set].symchar)
#define getsetplotsymskip(gno, set) (g[gno].p[set].symskip)
#define dataset_type(gno, set) (g[gno].p[set].type)
#define graph_type(gno) (g[gno].type)
#define setcomment(gno, set, s) (strcpy(g[gno].p[set].comments, s))
#define settype(gno, i, it) (g[gno].p[i].type = it)
#define setplotsym(gno, i, hsym) (g[gno].p[i].sym = hsym)
#define setplotsymcolor(gno, i, col) (g[gno].p[i].symcolor = col)
#define setplotsymsize(gno, i, hsym) (g[gno].p[i].symsize = hsym)
#define setplotsymskip(gno, i, hsym) (g[gno].p[i].symskip = hsym)
#define setplotsymchar(gno, i, hsym) (g[gno].p[i].symchar = hsym)
#define setplotlines(gno, i, line) (g[gno].p[i].lines = line)
#define setplotlinew(gno, i, wid) (g[gno].p[i].linew = wid)
#define setplotcolor(gno, i, pen) (g[gno].p[i].color = pen)
#define ishidden_graph(gno) (g[gno].hidden)
#define yes_or_no(x) ((x) ? "yes" : "no")
#if defined(GRCONVERT)
#define isactive_set(gno, set) (g[gno].p[set].active == ON)
#define isactive_graph(gno) (g[gno].active == ON)
#define on_or_off(x) ((x == ON) ? "on" : "off")
#define w_or_v(x) ((x == WORLD) ? "world" : "view")
#else
#define isactive_set(gno, set) (g[gno].p[set].active)
#define isactive_graph(gno) (g[gno].active)
#define on_or_off(x) ((x) ? "on" : "off")
#define w_or_v(x) ((x == COORD_WORLD) ? "world" : "view")
#endif

/* replacements for some functions */

#ifndef HAVE_MEMMOVE
#define memmove(a, b, c) bcopy((b), (a), (c))
#endif

#ifndef HAVE_MEMCPY
#define memcpy(a, b, c) bcopy((b), (a), (c))
#endif

#ifndef HAVE_GETHOSTNAME
#define gethostname(a, n) (strncpy((a), "localhost", n) ? 0 : 1)
#endif

#ifndef HAVE_DRAND48
#define srand48 srand
#define drand48 (1.0 / RAND_MAX) * rand
#define lrand48 rand
#endif

#ifndef HAVE_GETCWD
#ifdef OS2
#define getcwd _getcwd2
#define chdir _chdir2
#endif
#endif

#endif /* __DEFINES_H_ */
