
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*
 * types of tick displays
 */
#define X_AXIS 0
#define Y_AXIS 1
#define ZX_AXIS 2
#define ZY_AXIS 3

#ifndef MAXARR
#define MAXARR 20000 /* max elements in an array */
#endif

#define MAXGRAPH 10        /* max number of graphs */
#define MAXPLOT 30         /* max number of sets in a graph */
#define MAXAXES 4          /* max number of axes per graph */
#define MAX_SET_COLS 6     /* max number of data columns for a set */
#define MAX_TICK_LABELS 40 /* max number of user defined ticks/labels */
#define MAX_ZOOM_STACK 20  /* max stack depth for world stack */
#define MAXREGION 5        /* max number of regions */
#define MAXBOXES 50        /* max number of boxes */
#define MAXLINES 50        /* max number of lines */
#define MAXELLIPSES 50     /* max number of ellipses */
#define MAXSTR 100         /* max number of strings */

#define PAGE_FREE 0

#define GR_PS_L 1 /* PostScript landscape */

/* set HDEV to the default hardcopy device */
#ifndef HDEV
#define HDEV GR_PS_L
#endif

#define DATASET_MISSING (1.23456789e+30)

#define NONAME "Untitled"

#define ABOVE 589
#define ABSOLUTE 309
#define AUTO 324
#define BAR 332
#define BELOW 590
#define BOTH 336
#define BOTTOM 337
#define CENTER 342
#define COLOR 348
#define DAYMONTH 600
#define DAYOFWEEKL 605
#define DAYOFWEEKS 604
#define DAYOFYEAR 606
#define DDMMYY 594
#define DECIMAL 352
#define DEGREESLAT 615
#define DEGREESLON 611
#define DEGREESMMLAT 616
#define DEGREESMMLON 612
#define DEGREESMMSSLAT 617
#define DEGREESMMSSLON 613
#define DISK 360
#define EXPONENTIAL 373
#define FIXED 634
#define FREE 633
#define GENERAL 593
#define HBAR 395
#define HMS 607
#define HORIZONTAL 399
#define IN 407
#define LANDSCAPE 631
#define LEFT 419
#define LOGX 432
#define LOGXY 434
#define LOGY 433
#define MMDD 598
#define MMDDHMS 608
#define MMDDYY 596
#define MMDDYYHMS 609
#define MMSSLAT 618
#define MMSSLON 614
#define MMYY 597
#define MONTHDAY 599
#define MONTHL 603
#define MONTHS 601
#define MONTHSY 602
#define NEGATE 443
#define NONE 445
#define NORMAL 446
#define OFF 448
#define ON 451
#define OUT 454
#define PARA 456
#define PATTERN 460
#define PERP 462
#define PIE 464
#define POLAR 468
#define POLY 469
#define POLYI 591
#define POLYO 592
#define PORTRAIT 632
#define POWER 470
#define RIGHT 490
#define SPEC 513
#define STACKEDBAR 517
#define STACKEDHBAR 518
#define STACKEDLINE 519
#define TOP 535
#define VERTICAL 540
#define VIEW 542
#define WORLD 544
#define XY 554
#define XYBOX 556
#define XYBOXPLOT 557
#define XYDX 563
#define XYDXDX 565
#define XYDXDY 567
#define XYDY 564
#define XYDYDY 566
#define XYFIXED 558
#define XYHILO 559
#define XYRT 560
#define XYSTRING 562
#define XYUV 573
#define XYZ 571
#define YYMMDD 595

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
    int fill;
    int fillcolor;
    int fillpattern;
} ellipsetype;

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

#if defined(MAIN)
graph* g;
int cg = 0; /* the current graph */
int maxgraph = MAXGRAPH;
char sformat[128] = "%16lg %16lg"; /* format for saving (ascii) projects */
defaults grdefaults;               /* default properties */
int maxplot = MAXPLOT;
int maxarr = MAXARR;
plotstr timestamp; /* timestamp */
region rg[MAXREGION];
linetype* lines; /* lines */
int maxlines = MAXLINES;
int maxboxes = MAXBOXES;
boxtype* boxes; /* boxes */
plotstr* pstr;  /* strings */
int maxstr = MAXSTR;
int maxellipses = MAXELLIPSES;
ellipsetype* ellip; /* ellipses */
double** blockdata;
int maxblock = MAXPLOT;
double *ax, *bx, *cx, *dx; /* scratch arrays used in scanner */
char docname[512] = NONAME;
int blocklen;
int blockncols;
char description[2048];
int page_layout = PAGE_FREE;
int hdevice = HDEV;
double scrollper = 0.05;    /* scroll fraction */
double shexper = 0.05;      /* expand/shrink fraction */
int scrolling_islinked = 0; /* linked scroll */
char buf[1024];             /* a string used here and there */

#else
extern graph* g;
extern int cg;
extern int maxarr;
extern char sformat[];
extern defaults grdefaults; /* default properties */
extern int maxgraph;
extern int maxplot;
extern plotstr timestamp; /* timestamp */
extern region rg[];
extern linetype* lines; /* lines */
extern int maxlines;
extern int maxboxes;
extern boxtype* boxes; /* boxes */
extern plotstr* pstr;  /* strings */
extern int maxstr;
extern int maxellipses;
extern ellipsetype* ellip; /* ellipses */
extern double** blockdata;
extern int maxblock;
extern double *ax, *bx, *cx, *dx;
extern char docname[];
extern int blocklen;
extern int blockncols;
extern char description[];
extern int page_layout;
extern int hdevice;
extern double scrollper;       /* scroll fraction */
extern double shexper;         /* expand/shrink fraction */
extern int scrolling_islinked; /* linked scroll */
extern char buf[];

#endif

void set_program_defaults(void);
void set_region_defaults(int rno);
void set_default_framep(framep* f);
void set_default_world(world* w);
void set_default_view(view* v);
void set_default_string(plotstr* s);
void set_default_line(linetype* l);
void set_default_box(boxtype* b);
void set_default_ellipse(ellipsetype* e);
void set_default_legend(int gno, legend* l);
void set_default_plotarr(plotarr* p);
void set_default_velocityp(velocityp* vp);
void set_default_graph(int gno);
void realloc_plots(int maxplot);
void realloc_graph_plots(int gno, int maxplot);
void realloc_graphs(void);
void set_default_annotation(void);
void set_default_ticks(tickmarks* t, int a);
void setdefaultcolors(int gno);
void kill_blockdata(void);
void alloc_blockdata(int ncols);
int init_array(double** a, int n);
int init_scratch_arrays(int n);
int read_boxtype(boxtype* d, FILE* fin);
int read_ellipsetype(ellipsetype* d, FILE* fin);
int read_linetype(linetype* d, FILE* fin);
int read_plotstr(plotstr* d, FILE* fin);
int read_graph(graph* d, FILE* fin);
void close_xdr(void);
int read_double(double* d, int n, FILE* fp);
int read_int(int* d, int n, FILE* fp);
int read_charstr(char* d, FILE* fp);
int read_char(char* d, int n, FILE* fp);
int read_short(short* d, int n, FILE* fp);
int read_float(float* d, int n, FILE* fp);
int replace_xdr_int(int* i);
int replace_xdr_short(short* i);
int is_state_save(char* fname);
int getbinary(int gno, char* fname, int imbed);
int do_writesets(int gno, int setno, int imbed, char* fn, char* format);
void putparms(int gno, FILE* pp, int imbed);
char* graph_types(int it, int which);
char* get_format_types(int f);
void get_graph_box(int i, boxtype* b);
void get_graph_ellipse(int i, ellipsetype* b);
void get_graph_line(int i, linetype* l);
void get_graph_string(int i, plotstr* s);
void get_graph_framep(int gno, framep* f);
void get_graph_world(int gno, world* w);
void get_graph_view(int gno, view* v);
void get_graph_labels(int gno, labels* labs);
void get_graph_plotarr(int gno, int i, plotarr* p);
void get_graph_tickmarks(int gno, tickmarks* t, int a);
void get_graph_legend(int gno, legend* leg);
void set_graph_tickmarks(int gno, tickmarks* t, int a);
void default_ticks(int gno, int axis, double* gmin, double* gmax);
void errmsg(char* buf);
char* escapequotes(char* s);
char* set_types(int it);
int is_hotlinked(int gno, int setno);
void cxfree(void* ptr);

#define isactive_set(gno, set) (g[gno].p[set].active == ON)
#define isactive_graph(gno) (g[gno].active == ON)
#define on_or_off(x) ((x == ON) ? "on" : "off")
#define w_or_v(x) ((x == WORLD) ? "world" : "view")
#define dataset_type(gno, set) (g[gno].p[set].type)
#define getx(gno, set) ((double*)g[gno].p[set].ex[0])
#define gety(gno, set) ((double*)g[gno].p[set].ex[1])
#define getsetlength(gno, set) (g[gno].p[set].len)
#define getcol(gno, set, col) ((double*)g[gno].p[set].ex[col])
