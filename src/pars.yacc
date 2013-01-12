%{

/*  $Id: pars.yacc,v 1.4 1995/06/30 22:32:11 pturner Exp pturner $
 * 
 * evaluate expressions, commands, parameter files
 * 
 */

#define PARS			

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#ifndef VMS
#  include <sys/param.h>
#endif

/* bison not always handles it well itself */
#if defined(HAVE_ALLOCA_H)
#  include <alloca.h>
#endif

#include "globals.h"
#include "defines.h"
#include "ps.h"
#include "protos.h"
#include "cephes/protos.h"
#include "dlmodule.h"

double result;		/* return value if expression */

static int interr;

static double *freelist[100]; 	/* temporary vectors */
static int fcnt;		/* number allocated */

int naxis = 0;	/* current axis */
static int curline, curbox, curellipse, curstring;

static int gotbatch = 0, gotparams = 0, gotread = 0; /* these guys attempt to avoid reentrancy problems */
int readtype, readsrc, readxformat;
static int gotnlfit, nlfit_gno, nlfit_setno, nlfit_nsteps;

char batchfile[MAXPATHLEN] = "", paramfile[MAXPATHLEN] = "", readfile[MAXPATHLEN] = "";

static char f_string[MAX_STRING_LENGTH]; /* buffer for string to parse */
static int pos = 0;
static double *aa, *bb, *cc, *dd, *xx, *yy;
static int setindex, lxy, ls;
static int setsetno;
static int whichgraph;
static int whichset;

static int alias_force = FALSE; /* controls whether aliases can override
                                                       existing keywords */

extern int change_gno;
extern int change_type;

static int index_shift = 0; /* 1 for F77 index notation */

extern int signgam; /* sign of lgamma and lbeta */

int check_err;

double rnorm(double mean, double sdev);
double fx(double x);
double vmin(double *x, int n);
double vmax(double *x, int n);
void set_prop(int gno,...);
void set_axis_prop(int whichgraph, int naxis, int prop, double val);
int checkon(int prop, int old_val, int new_val);
int getcharstr(void);
void ungetchstr(void);
int follow(int expect, int ifyes, int ifno);

double ai_wrap(double x);
double bi_wrap(double x);
double ci_wrap(double x);
double si_wrap(double x);
double chi_wrap(double x);
double shi_wrap(double x);
double fresnlc_wrap(double x);
double fresnls_wrap(double x);
double iv_wrap(double v, double x);
double jv_wrap(double v, double x);
double kn_wrap(int n, double x);
double yv_wrap(double v, double x);
double sqr_wrap(double x);

int yylex(void);
int yyparse(void);
void yyerror(char *s);

int findf(symtab_entry *keytable, char *s);

/* Total (intrinsic + user-defined) list of functions and keywords */
symtab_entry *key;

%}

%union {
    double val;
    long ival;
    double *ptr;
    long func;
    long pset;
    char *str;
}

%token <func> DEG
%token <func> DX
%token <func> DY
%token <func> INDEX
%token <func> IRAND
%token <func> JDAY
%token <func> JDAY0
%token <func> MAXP
%token <func> MINP
%token <func> PI_TOK 
%token <func> RAD
%token <func> RAND

%token <func> FUNC_I	 /* a function of 1 int variable                            */
%token <func> FUNC_D	 /* a function of 1 double variable                         */
%token <func> FUNC_NN    /* a function of 2 int parameters                          */
%token <func> FUNC_ND    /* a function of 1 int parameter and 1 double variable     */
%token <func> FUNC_DD    /* a function of 2 double variables                        */
%token <func> FUNC_NND   /* a function of 2 int parameters and 1 double variable    */
%token <func> FUNC_PPD   /* a function of 2 double parameters and 1 double variable */
%token <func> FUNC_PPPV  /* a function of 3 double parameters and 1 double variable */
%token <pset> PROC_FUNC_I
%token <pset> PROC_FUNC_D
%token <pset> PROC_FUNC_NN
%token <pset> PROC_FUNC_ND
%token <pset> PROC_FUNC_DD
%token <pset> PROC_FUNC_NND
%token <pset> PROC_FUNC_PPD
%token <pset> PROC_FUNC_PPPV

%token <pset> ABOVE
%token <pset> ABSOLUTE
%token <pset> ACTIVATE
%token <pset> ACTIVE
%token <pset> ALIAS
%token <pset> ALT
%token <pset> ALTERNATE
%token <pset> ALTXAXIS
%token <pset> ALTYAXIS
%token <pset> ANGLE
%token <pset> APPEND
%token <pset> AREA
%token <pset> ARRANGE
%token <pset> ARROW
%token <pset> ASCENDING
%token <pset> ASPLINE
%token <pset> AUTO
%token <pset> AUTOSCALE
%token <pset> AUTOTICKS
%token <pset> AVG
%token <pset> AXES
%token <pset> AXIS
%token <pset> BACKBUFFER
%token <pset> BACKGROUND
%token <pset> BAR
%token <pset> BATCH
%token <pset> BEGIN
%token <pset> BELOW
%token <pset> BETWEEN
%token <pset> BIN
%token <pset> BLACKMAN
%token <pset> BLOCK
%token <pset> BOTH
%token <pset> BOTTOM
%token <pset> BOX
%token <pset> CD
%token <pset> CENTER
%token <pset> CHAR
%token <pset> CHRSTR
%token <pset> CIRCLE
%token <pset> CLEAR
%token <pset> CLICK
%token <pset> CMAP
%token <pset> COEFFICIENTS
%token <pset> COLOR
%token <pset> COMMENT
%token <pset> COMPLEX
%token <pset> CONSTRAINTS
%token <pset> COPY
%token <pset> CROSS
%token <pset> CYCLE
%token <pset> DAYMONTH
%token <pset> DAYOFWEEKL
%token <pset> DAYOFWEEKS
%token <pset> DAYOFYEAR
%token <pset> DDMMYY
%token <pset> DEACTIVATE
%token <pset> DECIMAL
%token <pset> DEF
%token <pset> DEFAULT
%token <pset> DEGREESLAT
%token <pset> DEGREESLON
%token <pset> DEGREESMMLAT
%token <pset> DEGREESMMLON
%token <pset> DEGREESMMSSLAT
%token <pset> DEGREESMMSSLON
%token <pset> DELETE
%token <pset> DESCENDING
%token <pset> DESCRIPTION
%token <pset> DEVICE
%token <pset> DFT
%token <pset> DIAMOND
%token <pset> DIFFERENCE
%token <pset> DISK
%token <pset> DOT
%token <pset> DOUBLEBUFFER
%token <pset> DOWN
%token <pset> DRAW2
%token <pset> DROP
%token <pset> DXDX
%token <pset> DXP
%token <pset> DYDY
%token <pset> DYP
%token <pset> ECHO
%token <pset> EDIT
%token <pset> ELLIPSE
%token <pset> ERRORBAR
%token <pset> EXIT
%token <pset> EXPONENTIAL
%token <pset> FALSEP
%token <pset> FFT
%token <pset> FILEP
%token <pset> FILL
%token <pset> FIND
%token <pset> FIT
%token <pset> FIXED
%token <pset> FIXEDPOINT
%token <pset> FLUSH
%token <pset> FOCUS
%token <pset> FOLLOWS
%token <pset> FONTP
%token <pset> FORCE
%token <pset> FOREGROUND
%token <pset> FORMAT
%token <pset> FORMULA
%token <pset> FRAMEP
%token <pset> FREE
%token <pset> FREQUENCY
%token <pset> FROM
%token <pset> FRONTBUFFER
%token <pset> GENERAL
%token <pset> GETP
%token <pset> GRAPH
%token <pset> GRAPHNO
%token <pset> GRAPHS
%token <pset> GRAPHTYPE
%token <pset> GRID
%token <pset> HAMMING
%token <pset> HANNING
%token <pset> HARDCOPY
%token <pset> HBAR
%token <pset> HGAP
%token <pset> HIDDEN
%token <pset> HISTO
%token <pset> HMS
%token <pset> HORIZONTAL
%token <pset> HPGLL
%token <pset> HPGLP
%token <pset> IGNORE
%token <pset> IMAGE
%token <pset> IN
%token <pset> INCREMENT
%token <pset> INIT
%token <pset> INOUT
%token <pset> INTEGRATE
%token <pset> INTERP
%token <pset> INVDFT
%token <pset> INVFFT
%token <pset> JUST
%token <pset> KILL
%token <pset> LABEL
%token <pset> LANDSCAPE
%token <pset> LAYOUT
%token <pset> LEAVE
%token <pset> LEFT
%token <pset> LEGEND
%token <pset> LENGTH
%token <pset> LEVEL
%token <pset> LEVELS
%token <pset> LINE
%token <pset> LINESTYLE
%token <pset> LINETO
%token <pset> LINEWIDTH
%token <pset> LINK
%token <pset> LOAD
%token <pset> LOCATOR
%token <pset> LOCATORBAR
%token <pset> LOCTYPE
%token <pset> LOG
%token <pset> LOGX
%token <pset> LOGXY
%token <pset> LOGY
%token <pset> MAGNITUDE
%token <pset> MAJOR
%token <pset> MIFL
%token <pset> MIFP
%token <pset> MINOR
%token <pset> MMDD
%token <pset> MMDDHMS
%token <pset> MMDDYY
%token <pset> MMDDYYHMS
%token <pset> MMSSLAT
%token <pset> MMSSLON
%token <pset> MMYY
%token <pset> MONTHDAY
%token <pset> MONTHL
%token <pset> MONTHS
%token <pset> MONTHSY
%token <pset> MOVE
%token <pset> MOVE2
%token <pset> NEGATE
%token <pset> NONE
%token <pset> NONLFIT
%token <pset> NORMAL
%token <pset> NXY
%token <pset> OFF
%token <pset> OFFSETX
%token <pset> OFFSETY
%token <pset> ON
%token <pset> OP
%token <pset> OUT
%token <pset> PAGE
%token <pset> PARA
%token <pset> PARAMETERS
%token <pset> PARAMS
%token <pset> PARZEN
%token <pset> PATTERN
%token <pset> PERIMETER
%token <pset> PERIOD
%token <pset> PERP
%token <pset> PHASE
%token <pset> PIPE
%token <pset> PLACE
%token <pset> PLUS
%token <pset> POINT
%token <pset> POLAR
%token <pset> POLY
%token <pset> POLYI
%token <pset> POLYO
%token <pset> POP
%token <pset> PORTRAIT
%token <pset> POWER
%token <pset> PREC
%token <pset> PREPEND
%token <pset> PRINT
%token <pset> PS
%token <pset> PSMONOL
%token <pset> PSMONOP
%token <pset> PUSH
%token <pset> PUTP
%token <pset> RAWSPICE
%token <pset> REACTIVATE
%token <pset> READ
%token <pset> REAL
%token <pset> REDRAW
%token <pset> REGNUM
%token <pset> REGRESS
%token <pset> RENDER
%token <pset> RIGHT
%token <pset> RISER
%token <pset> ROT
%token <pset> RUNAVG
%token <pset> RUNMAX
%token <pset> RUNMED
%token <pset> RUNMIN
%token <pset> RUNSTD
%token <pset> SAVE
%token <pset> SAVEALL
%token <pset> SCALE
%token <pset> SD
%token <pset> SET
%token <pset> SETNUM
%token <pset> SETS
%token <pset> SFORMAT
%token <pset> SIGN
%token <pset> SIZE
%token <pset> SKIP
%token <pset> SLEEP
%token <pset> SLICE
%token <pset> SMITH 
%token <pset> SORT
%token <pset> SOURCE
%token <pset> SPEC
%token <pset> SPECIFIED
%token <pset> SPLINE
%token <pset> SQUARE
%token <pset> STACK
%token <pset> STACKEDBAR
%token <pset> STACKEDHBAR
%token <pset> STAGGER
%token <pset> STAR
%token <pset> START
%token <pset> STARTTYPE
%token <pset> STATUS
%token <pset> STATUSBAR
%token <pset> STOP
%token <pset> STRING
%token <pset> SUBTITLE
%token <pset> SWAPBUFFER
%token <pset> SYMBOL
%token <pset> TARGET
%token <pset> TICKLABEL
%token <pset> TICKMARKS
%token <pset> TICKP
%token <pset> TICKS
%token <pset> TIMESTAMP
%token <pset> TITLE
%token <pset> TO
%token <pset> TOOLBAR
%token <pset> TOP
%token <pset> TRIANGLE1
%token <pset> TRIANGLE2
%token <pset> TRIANGLE3
%token <pset> TRIANGLE4
%token <pset> TRIANGULAR
%token <pset> TRUEP
%token <pset> TYPE
%token <pset> UP
%token <pset> USE
%token <pset> VELOCITY
%token <pset> VERSION
%token <pset> VERTICAL
%token <pset> VGAP
%token <pset> VIEW
%token <pset> VX1
%token <pset> VX2
%token <pset> VY1
%token <pset> VY2
%token <pset> WELCH
%token <pset> WITH
%token <pset> WORLD
%token <pset> WRITE
%token <pset> WX1
%token <pset> WX2
%token <pset> WY1
%token <pset> WY2
%token <pset> X_TOK
%token <pset> X0
%token <pset> X1
%token <pset> XAXES
%token <pset> XAXIS
%token <pset> XCOR
%token <pset> XMAX
%token <pset> XMIN
%token <pset> XY
%token <pset> XYBOX
%token <pset> XYBOXPLOT
%token <pset> XYDX
%token <pset> XYDXDX
%token <pset> XYDXDY
%token <pset> XYDY
%token <pset> XYDYDY
%token <pset> XYHILO
%token <pset> XYRT
%token <pset> XYSTRING
%token <pset> XYUV
%token <pset> XYXX
%token <pset> XYYY
%token <pset> XYZ
%token <pset> XYZW
%token <pset> Y_TOK
%token <pset> Y0
%token <pset> Y1
%token <pset> Y2
%token <pset> Y3
%token <pset> Y4
%token <pset> YAXES
%token <pset> YAXIS
%token <pset> YMAX
%token <pset> YMIN
%token <pset> YYMMDD
%token <pset> YYMMDDHMS
%token <pset> ZEROXAXIS
%token <pset> ZEROYAXIS

%token <ptr> VAR

%token <val> FITPARM
%token <val> FITPMAX
%token <val> FITPMIN
%token <val> NUMBER

%type <pset> barsd
%type <pset> colpat
%type <pset> direction
%type <pset> extremetype
%type <pset> ffttype
%type <pset> filltype
%type <pset> formatchoice
%type <pset> fourierdata
%type <pset> fourierloadx
%type <pset> fourierloady
%type <pset> proctype
%type <pset> graphtype
%type <pset> inoutchoice
%type <pset> justchoice
%type <pset> nonlfitopts
%type <pset> onoff
%type <pset> opchoice
%type <pset> pagelayout
%type <pset> parmset
%type <pset> printer
%type <pset> prop
%type <pset> regionset
%type <pset> regiontype
%type <pset> runtype
%type <pset> selectsets
%type <pset> setvelocity
%type <pset> signchoice
%type <pset> sortdir
%type <pset> sorton
%type <pset> sourcetype
%type <pset> torf
%type <pset> vector
%type <pset> windowtype
%type <pset> worldview
%type <pset> xytype

%type <ptr> asgn
%type <ptr> vasgn
%type <ptr> vexpr

%type <val> expr

/* Precedence */
%right '='
%left OR
%left AND
%nonassoc GT LT LE GE EQ NE
%left '+' '-'
%left '*' '/' '%'
%left UMINUS NOT	/* negation--unary minus */
%right '^'		/* exponentiation        */


%%

list:
	| asgn '\n' {}
	| vasgn '\n' {}
	| expr '\n' {
	    result = $1;
	}
	| vexpr '\n' {
	    result = *$1;
	}
	| parmset '\n' {}
	| regionset '\n' {}
	| setaxis '\n' {}
	| setvelocity '\n' {}
	| set_setprop '\n' {}
	| setprint '\n' {}
	| error '\n' {
	    return 1;
	}
	;

setprint:
	PRINT printer CHRSTR {
	    set_printer($2, (char *) $3);
	    free((char *) $3);
	}
	| PRINT TO printer CHRSTR {
	    set_printer($3, (char *) $4);
	    free((char *) $4);
	}
	| PRINT TO printer {
	    set_printer($3, NULL);
	}
	| DEVICE NUMBER {
	    tdevice = (int) $2;
	}
	| HARDCOPY DEVICE NUMBER {
	    hdevice = (int) $3;
	}
	| HARDCOPY {
	    do_hardcopy();
	}
	| PS LINEWIDTH BEGIN NUMBER { 
            pslwbegin = (int) $4; 
	}
	| PS LINEWIDTH INCREMENT NUMBER { 
            pslwincr = (int) $4;
	}
	| PS LINEWIDTH NUMBER { 
	    pslwincr = (int) $3; 
	}
	;

printer:
	PSMONOP { $$ = GR_PS_P; }
	| PSMONOL { $$ = GR_PS_L; }
	| MIFP { $$ = GR_MIF_P; }
	| MIFL { $$ = GR_MIF_L; }
	| HPGLP { $$ = GR_HPGL_P; }
	| HPGLL { $$ = GR_HPGL_L; }
	| HARDCOPY { $$ = hdevice; }
	| FILEP { $$ = GR_FILE; }
	;

regionset:
	REGNUM onoff {
	    rg[$1].active = $2;
	}
	| REGNUM TYPE regiontype {
	    rg[$1].type = $3;
	}
	| REGNUM COLOR NUMBER {
	    rg[$1].color = checkon(COLOR, rg[$1].color, (int) $3);
	}
	| REGNUM LINESTYLE NUMBER {
	    rg[$1].lines = checkon(LINESTYLE, rg[$1].lines, (int) $3);
	}
	| REGNUM LINEWIDTH NUMBER {
	    rg[$1].linew = checkon(LINEWIDTH, rg[$1].linew, (int) $3);
	}
	| REGNUM LINE expr ',' expr ',' expr ',' expr
	{
	    rg[$1].x1 = $3;
	    rg[$1].y1 = $5;
	    rg[$1].x2 = $7;
	    rg[$1].y2 = $9;
	}
	| REGNUM XY expr ',' expr
	{
	    if (rg[$1].x == NULL || rg[$1].n == 0) {
		rg[$1].n = 0;
		rg[$1].x = (double *) calloc(1, sizeof(double));
		rg[$1].y = (double *) calloc(1, sizeof(double));
	    } else {
		rg[$1].x = (double *) realloc(rg[$1].x, (rg[$1].n + 1) * sizeof(double));
		rg[$1].y = (double *) realloc(rg[$1].y, (rg[$1].n + 1) * sizeof(double));
	    }
	    rg[$1].x[rg[$1].n] = $3;
	    rg[$1].y[rg[$1].n] = $5;
	    rg[$1].n++;
	}
	| LINK REGNUM TO GRAPHNO {
	    rg[$2].linkto[$4] = TRUE;
	}
	;

parmset:
        VERSION NUMBER {
        }
	| REDRAW {
#ifndef NONE_GUI
	    drawgraph();
#endif
	}
	| AUTO REDRAW onoff {
	    auto_redraw = $3;
#ifndef NONE_GUI
	    update_draw();
#endif
	}
	| STATUS AUTO REDRAW onoff {
	    status_auto_redraw = $4;
#ifndef NONE_GUI
	    update_status_auto_redraw();
#endif
	}
	| CD CHRSTR 
	{
	    set_workingdir((char *) $2);
	    free((char *) $2);
	}
	| ECHO CHRSTR {
	    echomsg((char *) $2);
	    free((char *) $2);
	}
        | ECHO expr {
            char buf[MAX_STRING_LENGTH];
            sprintf(buf, "%g", (double) $2);
            echomsg(buf);
        }
	| STATUS {
#ifndef NONE_GUI
	    update_stuff_status();
#endif
	}
	| BACKGROUND COLOR NUMBER {
	    setbgcolor((int) $3);
	}
	| CMAP NUMBER ',' NUMBER ',' NUMBER ',' NUMBER {
#ifndef NONE_GUI
	    xlibsetcmap((int) $2, (int) $4, (int) $6, (int) $8);
#endif
	}
	| EXIT {
	    exit(0);
	}
        | PAGE LAYOUT pagelayout
        {
#ifndef NONE_GUI
            set_pagelayout($3);
#endif
        }
        | FIT nonlfitopts
        {
            ;
        }
        | STATUSBAR onoff
        {
#ifndef NONE_GUI
            set_toolbars($1, $2);
#endif
        }
        | TOOLBAR onoff
        {
#ifndef NONE_GUI
            set_toolbars($1, $2);
#endif
        }
        | LOCATORBAR onoff
        {
#ifndef NONE_GUI
            set_toolbars($1, $2);
#endif
        }

	| DRAW2 expr ',' expr {
	    if (inwin) {
		my_draw2((double) $2, (double) $4);
#ifndef NONE_GUI
		flush_pending();
#endif
	    }
	}
	| MOVE2 expr ',' expr {
	    if (inwin) {
		my_move2((double) $2, (double) $4);
	    }
	}
	| SYMBOL expr ',' expr ',' NUMBER {
	    if (inwin) {
		double x = (double) $2;
		double y = (double) $4;
		drawpolysym(&x, &y, 1, (int) $6, 0, 0, 1.0);
#ifndef NONE_GUI
		flush_pending();
#endif
	    }
	}
	| COLOR NUMBER {
	    if (inwin) {
		setcolor((int) $2);
	    }
	}
	| LINEWIDTH NUMBER {
	    if (inwin) {
		setlinewidth((int) $2);
	    }
	}
	| LINESTYLE NUMBER {
	    if (inwin) {
		setlinestyle((int) $2);
	    }
	}
	| PAGE direction
	{
	    switch ($2) {
	    case UP:
		gwindup_proc();
		break;
	    case DOWN:
		gwinddown_proc();
		break;
	    case RIGHT:
		gwindright_proc();
		break;
	    case LEFT:
		gwindleft_proc();
		break;
	    case IN:
		gwindshrink_proc();
		break;
	    case OUT:
		gwindexpand_proc();
		break;
	    }
	}
	| PAGE NUMBER {
	    scroll_proc((int) $2);
	}
	| PAGE INOUT NUMBER {
	    scrollinout_proc((int) $3);
	}
	| LINK PAGE onoff {
	    scrolling_islinked = $3;
	}
	| DOUBLEBUFFER torf {
#ifndef NONE_GUI
	    my_doublebuffer($2);
#endif
	}
	| FRONTBUFFER torf {
#ifndef NONE_GUI
	    my_frontbuffer($2);
#endif
	}
	| BACKBUFFER torf {
#ifndef NONE_GUI
	    my_backbuffer($2);
#endif
	}
	| SWAPBUFFER {
#ifndef NONE_GUI
	    my_swapbuffer();
#endif
	}
	| SLEEP NUMBER {
	    sleep((int) $2);
	}
	| GETP CHRSTR
	{
	    gotparams = TRUE;
	    strcpy(paramfile, (char *) $2);
	    free((char *) $2);
	}
	| PUTP CHRSTR
	{
	    if (!fexists((char *) $2)) {
		FILE *pp = fopen((char *) $2, "w");
		if (pp != NULL) {
		    putparms(cg, pp, 0);
		    fclose(pp);
		} else {
		    errmsg("Unable to write parameter file");
		}
	    }
	    free((char *) $2);
	}
	| TARGET SETNUM {
	    target_set.gno = cg;
	    target_set.setno = (int) $2;
	}
	| TARGET GRAPHNO '.' SETNUM {
	    target_set.gno = (int) $2;
	    target_set.setno = (int) $4;
	}
	| WITH GRAPHNO {
	    cg = (int) $2;
	    g[cg].parmsread = TRUE;
	    change_gno = cg;
	}
	| WITH SETNUM {
	    curset = (int) $2;
	}

/* Hot links */
	| SETNUM LINK sourcetype CHRSTR {
	    set_hotlink(cg, $1, 1, (char *) $4, $3);
	    free((char *) $4);
	}
	| GRAPHNO '.' SETNUM LINK sourcetype CHRSTR {
	    set_hotlink($1, $3, 1, (char *) $6, $5);
	    free((char *) $6);
	}
	| SETNUM LINK onoff {
	    set_hotlink(cg, $1, $3, NULL, 0);
	}
	| GRAPHNO '.' SETNUM LINK onoff {
	    set_hotlink($1, $3, $5, NULL, 0);
	}
	| ACTIVATE SETNUM NUMBER {
	    do_activateset(cg, $2, (int) $3);
	}
	| ACTIVATE GRAPHNO '.' SETNUM NUMBER {
	    do_activateset($2, $4, (int) $5);
	}
	| DEACTIVATE SETNUM {
	    if (isactive_set(cg, $2)) {
		do_deactivate(cg, $2);
	    } else {
		yyerror("Set is not active");
	    }
	}
	| DEACTIVATE GRAPHNO '.' SETNUM {
	    if (isactive_set($2, $4)) {
		do_deactivate($2, $4);
	    } else {
		yyerror("Set is not active");
	    }
	}
	| REACTIVATE SETNUM {
	    do_reactivate(cg, $2);
	}
	| REACTIVATE GRAPHNO '.' SETNUM {
	    do_reactivate($2, $4);
	}
	| SETNUM LENGTH NUMBER {
	    setlength(cg, $1, (int) $3);
	    updatesetminmax(cg, $1);
#ifndef NONE_GUI
	    update_set_status(cg, $1);
#endif
	}
	| GRAPHNO '.' SETNUM LENGTH NUMBER {
	    setlength($1, $3, (int) $5);
	    updatesetminmax($1, $3);
#ifndef NONE_GUI
	    update_set_status($1, $3);
#endif
	}
	| SETNUM POINT expr ',' expr {
	    add_point(cg, $1, $3, $5, 0.0, 0.0, SET_XY);
	}
	| GRAPHNO '.' SETNUM POINT expr ',' expr {
	    add_point($1, $3, $5, $7, 0.0, 0.0, SET_XY);
	}

	| SETNUM DROP NUMBER ',' NUMBER {
	    int start = (int) $3 - 1;
	    int stop = (int) $5 - 1;
	    int dist = stop - start + 1;
	    if (dist > 0 && start >= 0) {
	        droppoints(cg, $1, start, stop, dist);
	    }
	}
	| GRAPHNO '.' SETNUM DROP NUMBER ',' NUMBER {
	    int start = (int) $5 - 1;
	    int stop = (int) $7 - 1;
	    int dist = stop - start + 1;
	    if (dist > 0 && start >= 0) {
	        droppoints($1, $3, start, stop, dist);
	    }
	}
	| SORT SETNUM sorton sortdir {
	    if (isactive_set(cg, $2)) {
	        sortset(cg, $2, $3, $4 == ASCENDING ? 0 : 1);
	    } else {
		errmsg("Set not active!");
	    }
	}
	| COPY SETNUM TO SETNUM {
	    do_copyset(cg, $2, cg, $4);
	}
	| COPY GRAPHNO '.' SETNUM TO GRAPHNO '.' SETNUM {
	    do_copyset($2, $4, $6, $8);
	}
	| MOVE SETNUM TO SETNUM {
	    do_moveset(cg, $2, cg, $4);
	}
	| MOVE GRAPHNO '.' SETNUM TO GRAPHNO '.' SETNUM {
	    do_moveset($2, $4, $6, $8);
	}
	| KILL SETNUM
	{
	    killset(cg, $2);
	}
	| KILL SETS
	{
	    int i;
	    for (i = 0; i < g[cg].maxplot; i++) {
		killset(cg, i);
	    }
	}
	| KILL SETNUM SAVEALL { softkillset(cg, $2); }
	| KILL SETS SAVEALL
	{
	    int i;
	    for (i = 0; i < g[cg].maxplot; i++) {
		softkillset(cg, i);
	    }
	}
	| KILL GRAPHNO { kill_graph($2); }
	| KILL GRAPHS { kill_graph(maxgraph); }
	| FLUSH { wipeout(0); }
	| ARRANGE NUMBER ',' NUMBER { arrange_graphs((int) $2, (int) $4); }
	| LOAD VAR NUMBER ',' expr ',' expr
	{
	    int i, ilen = (int) $3;
	    if (ilen < 0) {
		yyerror("Length of array < 0");
		return 1;
	    } else if (ilen > maxarr) {
		maxarr = ilen;
		init_scratch_arrays(ilen);
	    }
	    for (i = 0; i < ilen; i++) {
		$2[i] = $5 + $7 * i;
	    }
	}
	| NONLFIT '(' SETNUM ',' NUMBER ')'
	{
	    gotnlfit = TRUE;
	    nlfit_gno = cg;
	    nlfit_setno = $3;
	    nlfit_nsteps = (int) $5;
	}
	| NONLFIT '(' GRAPHNO '.' SETNUM ',' NUMBER ')'
	{
	    gotnlfit = TRUE;
	    nlfit_gno = $3;
	    nlfit_setno = $5;
	    nlfit_nsteps = (int) $7;
	}
	| REGRESS '(' SETNUM ',' NUMBER ')'
	{
	    int setno = $3, ideg = (int) $5;
	    do_regress(setno, ideg, 0, -1, 0);
	}
	| runtype '(' SETNUM ',' NUMBER ')'
	{
	    do_running_command($1, $3, (int) $5);
	}
	| ffttype '(' SETNUM ',' NUMBER ')'
	{
	    do_fourier_command($1, $3, (int) $5);
	}
        | ffttype '(' SETNUM ',' fourierdata ',' windowtype ',' 
          fourierloadx ','  fourierloady ')'
        {
	    switch ($1) {
	    case DFT:
                do_fourier(0, $3, $11, $9, 0, $5, $7);
	        break;
	    case INVDFT:
                do_fourier(0, $3, $11, $9, 1, $5, $7);
	        break;
	    case FFT:
                do_fourier(1, $3, $11, $9, 0, $5, $7);
	        break;
	    case INVFFT:
                do_fourier(1, $3, $11, $9, 1, $5, $7);
	        break;
	    }
        }
	| SPLINE '(' SETNUM ',' expr ',' expr ',' NUMBER ')'
	{
	    do_spline($3, $5, $7, (int) $9, SPLINE_CUBIC);
	}
	| ASPLINE '(' SETNUM ',' expr ',' expr ',' NUMBER ')'
	{
	    do_spline($3, $5, $7, (int) $9, SPLINE_AKIMA);
	}
	| INTERP '(' SETNUM ',' SETNUM ',' NUMBER ')'
	{
	    do_interp($3, $5, (int) $7);
	}
	| HISTO '(' SETNUM ',' expr ',' expr ',' NUMBER ')'
	{
	    do_histo_command($3, SET_SELECT_NEXT, -1, $5, $7, (int) $9);
	}
	| DIFFERENCE '(' SETNUM ',' NUMBER ')'
	{
	    do_differ($3, (int) $5);
	}
	| INTEGRATE '(' SETNUM ')'
	{
	    do_int($3, 0);
	}
 	| XCOR '(' SETNUM ',' SETNUM ',' NUMBER ')'
	{
	    do_xcor($3, $5, (int) $7);
	}
	| AUTOSCALE
	{
	    if (activeset(cg)) {
		defaultgraph(cg);
		default_axis(cg, g[cg].auto_type, X_AXIS);
		default_axis(cg, g[cg].auto_type, ZX_AXIS);
		default_axis(cg, g[cg].auto_type, Y_AXIS);
		default_axis(cg, g[cg].auto_type, ZY_AXIS);
#ifndef NONE_GUI
		update_world(cg);
		drawgraph();
#endif
	    } else {
		errmsg("No active sets!");
	    }
	}
	| AUTOSCALE XAXES
	{
	    if (activeset(cg)) {
		defaultx(cg, -1);
		default_axis(cg, g[cg].auto_type, X_AXIS);
		default_axis(cg, g[cg].auto_type, ZX_AXIS);
#ifndef NONE_GUI
		update_world(cg);
#endif
#ifndef NONE_GUI
		drawgraph();
#endif
	    } else {
		errmsg("No active sets!");
	    }
	}
	| AUTOSCALE YAXES
	{
	    if (activeset(cg)) {
		defaulty(cg, -1);
		default_axis(cg, g[cg].auto_type, Y_AXIS);
		default_axis(cg, g[cg].auto_type, ZY_AXIS);
#ifndef NONE_GUI
		update_world(cg);
		drawgraph();
#endif
	    } else {
		errmsg("No active sets!");
	    }
	}
	| AUTOSCALE SETNUM
	{
	    if (isactive_set(cg, $2)) {
		defaultsetgraph(cg, $2);
		default_axis(cg, g[cg].auto_type, X_AXIS);
		default_axis(cg, g[cg].auto_type, ZX_AXIS);
		default_axis(cg, g[cg].auto_type, Y_AXIS);
		default_axis(cg, g[cg].auto_type, ZY_AXIS);
#ifndef NONE_GUI
		update_world(cg);
		drawgraph();
#endif
	    } else {
		errmsg("Set not active");
	    }
	}
        | AUTOTICKS
        {
            default_axis(cg, g[cg].auto_type, X_AXIS);
            default_axis(cg, g[cg].auto_type, ZX_AXIS);
            default_axis(cg, g[cg].auto_type, Y_AXIS);
            default_axis(cg, g[cg].auto_type, ZY_AXIS);
#ifndef NONE_GUI
            update_all(cg);
            drawgraph();
#endif
        }
	| LOCATOR onoff
	{
#ifndef NONE_GUI
	    extern int go_locateflag;
	    go_locateflag = $2;
#endif
	}
	| FOCUS GRAPHNO
	{
#ifndef NONE_GUI
	    if (inwin) {
	        draw_focus(cg);
	    }
#endif
	    cg = (int) $2;
	    defineworld(g[cg].w.xg1, g[cg].w.yg1, g[cg].w.xg2, g[cg].w.yg2, 
			islogx(cg), islogy(cg));
	    viewport(g[cg].v.xv1, g[cg].v.yv1, g[cg].v.xv2, g[cg].v.yv2);
#ifndef NONE_GUI
	    if (inwin) {
	        draw_focus(cg);
	    }
#endif
	    update_all(cg);
	}
	| FOCUS onoff {
#ifndef NONE_GUI
	    draw_focus_flag = $2;
#endif
	}
	| FOCUS SET {
	    focus_policy = FOCUS_SET;
	}
	| FOCUS FOLLOWS {
	    focus_policy = FOCUS_FOLLOWS;
	}
	| FOCUS CLICK {
	    focus_policy = FOCUS_CLICK;
	}
	| SOURCE sourcetype {
	    cursource = $2;
	}
	| TYPE xytype {
	    curtype = $2;
	    change_type = curtype;
	}
	| FORMAT formatchoice
	{
	    readxformat = $2;
	}
	| READ CHRSTR
	{
	    gotread = TRUE;
	    readtype = curtype;
	    readsrc = cursource;
	    strcpy(readfile, (char *) $2);
	    free((char *) $2);
	}
	| READ BATCH CHRSTR
	{
	    gotbatch = TRUE;
	    strcpy(batchfile, (char *) $3);
	    free((char *) $3);
	}
	| READ BLOCK CHRSTR
	{
	    getdata(cg, (char *) $3, SOURCE_DISK, SET_BLOCK);
	    free((char *) $3);
	}
	| READ BLOCK sourcetype CHRSTR
	{
	    getdata(cg, (char *) $4, $3, SET_BLOCK);
	    free((char *) $4);
	}
	| BLOCK xytype CHRSTR
	{
	    create_set_fromblock(cg, $2, (char *) $3);
	    free((char *) $3);
	}
	| READ xytype CHRSTR
	{
	    gotread = TRUE;
	    readtype = $2;
	    readsrc = cursource;
	    strcpy(readfile, (char *) $3);
	    free((char *) $3);
	}
	| READ xytype sourcetype CHRSTR
	{
	    gotread = TRUE;
	    strcpy(readfile, (char *) $4);
	    readtype = $2;
	    readsrc = $3;
	    free((char *) $4);
	}
	| READ IMAGE CHRSTR
	{
#ifndef NONE_GUI
	    read_image((char *) $3);
#endif
	    free((char *) $3);
	}
	| WRITE IMAGE CHRSTR
	{
#ifndef NONE_GUI
	    write_image((char *) $3);
#endif
	    free((char *) $3);
	}
	| IMAGE XY NUMBER ',' NUMBER
	{
	    imagex = (int) $3;
	    imagey = (int) $5;
	}
	| WRITE SETNUM
	{
	    outputset(cg, $2, (char *) NULL, (char *) NULL);
	}
	| WRITE SETNUM FORMAT CHRSTR
	{
	    outputset(cg, $2, (char *) NULL, (char *) $4);
	    free((char *) $4);
	}
	| WRITE SETNUM FILEP CHRSTR
	{
	    outputset(cg, $2, (char *) $4, (char *) NULL);
	    free((char *) $4);
	}
	| WRITE SETNUM FILEP CHRSTR FORMAT CHRSTR
	{
	    outputset(cg, $2, (char *) $4, (char *) $6);
	    free((char *) $4);
	    free((char *) $6);
	}
        | SAVEALL CHRSTR
        {
            do_writesets(maxgraph, -1, 1, (char *) $2, sformat);
            free((char *) $2);
        }
	| PUSH {
	    push_world();
	}
	| POP {
	    pop_world();
	}
	| CYCLE {
	    cycle_world_stack();
	}
	| STACK NUMBER {
	    if ((int) $2 > 0)
		show_world_stack((int) $2 - 1);
	}
	| STACK WORLD expr ',' expr ',' expr ',' expr TICKP expr ',' expr ',' expr ',' expr
	{
	    add_world(cg, $3, $5, $7, $9, $11, $13, $15, $17);
	}
	| CLEAR STACK {
	    clear_world_stack();
	}
	| CLEAR BOX {
	    do_clear_boxes();
	}
	| WITH BOX {
	    curbox = next_box();
	}
	| WITH BOX NUMBER {
	    curbox = (int) $3;
	}
	| BOX onoff {
	    boxes[curbox].active = $2;
	}
	| BOX GRAPHNO {
	    boxes[curbox].gno = $2;
	}
	| BOX expr ',' expr ',' expr ',' expr
	{
	    if (curbox >= 0 && curbox < maxboxes) {
		boxes[curbox].x1 = $2;
		boxes[curbox].y1 = $4;
		boxes[curbox].x2 = $6;
		boxes[curbox].y2 = $8;
	    }
	}
	| BOX LOCTYPE worldview {
	    box_loctype = $3;
	}
	| BOX LINESTYLE NUMBER {
	    box_lines = checkon(LINESTYLE, box_lines, (int) $3);
	}
	| BOX LINEWIDTH NUMBER {
	    box_linew = checkon(LINEWIDTH, box_linew, (int) $3);
	}
	| BOX COLOR NUMBER {
	    box_color = checkon(COLOR, box_color, (int) $3);
	}
	| BOX FILL filltype {
	    box_fill = $3;
	}
	| BOX FILL COLOR NUMBER {
	    box_fillcolor = checkon(COLOR, box_fillcolor, (int) $4);
	}
	| BOX FILL PATTERN NUMBER {
	    box_fillpat = checkon(PATTERN, box_fillpat, (int) $4);
	}
	| BOX DEF
	{
	    if (curbox >= 0 && curbox < maxboxes) {
		boxes[curbox].lines = box_lines;
		boxes[curbox].linew = box_linew;
		boxes[curbox].color = box_color;
		boxes[curbox].fill = box_fill;
		boxes[curbox].fillcolor = box_fillcolor;
		boxes[curbox].fillpattern = box_fillpat;
		boxes[curbox].loctype = box_loctype;
	    }
	}
	| CLEAR ELLIPSE {
	    do_clear_ellipses();
	}
	| WITH ELLIPSE {
		curellipse = next_ellipse();
	}
	| WITH ELLIPSE NUMBER {
	    curellipse = (int) $3;
	}
	| ELLIPSE onoff {
	    ellip[curellipse].active = $2;
	}
	| ELLIPSE GRAPHNO {
	    ellip[curellipse].gno = $2;
	}
	| ELLIPSE expr ',' expr ',' expr ',' expr
	{
	    if (curellipse >= 0 && curellipse < maxellipses) {
		ellip[curellipse].x1 = $2;
		ellip[curellipse].y1 = $4;
		ellip[curellipse].x2 = $6;
		ellip[curellipse].y2 = $8;
	    }
	}
	| ELLIPSE LOCTYPE worldview {
	    ellipse_loctype = $3;
	}
	| ELLIPSE LINESTYLE NUMBER {
	    ellipse_lines = checkon(LINESTYLE, ellipse_lines, (int) $3);
	}
	| ELLIPSE LINEWIDTH NUMBER {
	    ellipse_linew = checkon(LINEWIDTH, ellipse_linew, (int) $3);
	}
	| ELLIPSE COLOR NUMBER {
	    ellipse_color = checkon(COLOR, ellipse_color, (int) $3);
	}
	| ELLIPSE FILL filltype {
	    ellipse_fill = $3;
	}
	| ELLIPSE FILL COLOR NUMBER {
	    ellipse_fillcolor = checkon(COLOR, ellipse_fillcolor, (int) $4);
	}
	| ELLIPSE FILL PATTERN NUMBER {
	    ellipse_fillpat = checkon(PATTERN, ellipse_fillpat, (int) $4);
	}
	| ELLIPSE DEF
	{
	    if (curellipse >= 0 && curellipse < maxellipses) {
		ellip[curellipse].lines = ellipse_lines;
		ellip[curellipse].linew = ellipse_linew;
		ellip[curellipse].color = ellipse_color;
		ellip[curellipse].fill = ellipse_fill;
		ellip[curellipse].fillcolor = ellipse_fillcolor;
		ellip[curellipse].fillpattern = ellipse_fillpat;
		ellip[curellipse].loctype = ellipse_loctype;
	    }
	}
	| WITH LINE {
	    curline = next_line();
	}
	| WITH LINE NUMBER {
	    curline = (int) $3;
	}
	| CLEAR LINE {
	    do_clear_lines();
	}
	| LINE onoff {
	    lines[curline].active = $2;
	}
	| LINE GRAPHNO {
	    lines[curline].gno = $2;
	}
	| LINE expr ',' expr ',' expr ',' expr
	{
	    lines[curline].x1 = $2;
	    lines[curline].y1 = $4;
	    lines[curline].x2 = $6;
	    lines[curline].y2 = $8;
	}
	| LINE LOCTYPE worldview {
	    line_loctype = $3;
	}
	| LINE LINEWIDTH NUMBER {
	    line_linew = checkon(LINEWIDTH, line_linew, (int) $3);
	}
	| LINE LINESTYLE NUMBER {
	    line_lines = checkon(LINESTYLE, line_lines, (int) $3);
	}
	| LINE COLOR NUMBER {
	    line_color = checkon(COLOR, line_color, (int) $3);
	}
	| LINE ARROW NUMBER {
	    line_arrow = checkon(ARROW, line_arrow, (int) $3);
	}
	| LINE ARROW SIZE NUMBER {
	    line_asize = $4;
	}
	| LINE ARROW TYPE NUMBER {
	    line_atype = (int) $4;
	}
	| LINE DEF
	{
	    if (curline >= 0 && curline < maxlines) {
		lines[curline].lines = line_lines;
		lines[curline].linew = line_linew;
		lines[curline].color = line_color;
		lines[curline].arrow = line_arrow;
		lines[curline].asize = line_asize;
		lines[curline].atype = line_atype;
		lines[curline].loctype = line_loctype;
	    }
	}
	| CLEAR STRING {
	    do_clear_text();
	}
	| WITH STRING { curstring = next_string(); }
	| WITH STRING NUMBER { curstring = (int) $3; }
	| STRING onoff { pstr[curstring].active = $2; }
	| STRING GRAPHNO { pstr[curstring].gno = $2; }
	| STRING expr ',' expr
	{
	    pstr[curstring].x = $2;
	    pstr[curstring].y = $4;
	}
	| STRING LOCTYPE worldview { string_loctype = $3; }
	| STRING LINEWIDTH NUMBER { string_linew = checkon(LINEWIDTH, string_linew, (int) $3); }
	| STRING COLOR NUMBER { string_color = checkon(COLOR, string_color, (int) $3); }
	| STRING ROT NUMBER { string_rot = (int) $3; }
	| STRING FONTP NUMBER { string_font = checkon(FONTP, string_font, (int) $3); }
	| STRING JUST NUMBER { string_just = checkon(JUST, string_just, (int) $3); }
	| STRING CHAR SIZE NUMBER { string_size = $4; }
	| STRING DEF CHRSTR
	{
	    set_plotstr_string(&pstr[curstring], (char *) $3);
	    pstr[curstring].linew = string_linew;
	    pstr[curstring].color = string_color;
	    pstr[curstring].font = string_font;
	    pstr[curstring].just = string_just;
	    pstr[curstring].loctype = string_loctype;
	    pstr[curstring].rot = string_rot;
	    pstr[curstring].charsize = string_size;
	    free((char *) $3);
	}
	| TIMESTAMP onoff { timestamp.active = $2; }
	| TIMESTAMP FONTP NUMBER { timestamp.font = checkon(FONTP, timestamp.font, (int) $3); }
	| TIMESTAMP CHAR SIZE NUMBER { timestamp.charsize = $4; }
	| TIMESTAMP ROT NUMBER { timestamp.rot = (int) $3; }
	| TIMESTAMP COLOR NUMBER { timestamp.color = checkon(COLOR, timestamp.color, (int) $3); }
	| TIMESTAMP LINEWIDTH NUMBER { timestamp.linew = checkon(LINEWIDTH, timestamp.linew, (int) $3); }
	| TIMESTAMP NUMBER ',' NUMBER
	{
	    timestamp.x = $2;
	    timestamp.y = $4;
	}
	| TIMESTAMP DEF CHRSTR
	{
	  set_plotstr_string(&timestamp, (char *) $3);
	  free((char *) $3);
	}
	| DEFAULT LINESTYLE NUMBER {
	    grdefaults.lines = (int) $3;
	}
	| DEFAULT LINEWIDTH NUMBER {
	    grdefaults.linew = (int) $3;
	}
	| DEFAULT COLOR NUMBER {
	    grdefaults.color = (int) $3;
	}
	| DEFAULT CHAR SIZE NUMBER {
	    grdefaults.charsize = $4;
	}
	| DEFAULT FONTP NUMBER {
	    grdefaults.font = (int) $3;
	}
	| DEFAULT FONTP SOURCE NUMBER {
	    grdefaults.fontsrc = (int) $4;
	}
	| DEFAULT SYMBOL SIZE NUMBER {
	    grdefaults.symsize = $4;
	}
	| DEFAULT SFORMAT CHRSTR {
	    strcpy(sformat, (char *) $3);
	    free((char *) $3);
	}

	| WORLD expr ',' expr ',' expr ',' expr
	{
	    g[cg].w.xg1 = $2;
	    g[cg].w.yg1 = $4;
	    g[cg].w.xg2 = $6;
	    g[cg].w.yg2 = $8;
	}
	| WORLD XMIN expr {
	    g[cg].w.xg1 = $3;
	}
	| WORLD XMAX expr {
	    g[cg].w.xg2 = $3;
	}
	| WORLD YMIN expr {
	    g[cg].w.yg1 = $3;
	}
	| WORLD YMAX expr {
	    g[cg].w.yg2 = $3;
	}
        | POLAR WORLD expr ',' expr
        {
            g[cg].rt.xg1 = $3;
            g[cg].rt.yg1 = $5;
        }
	| VIEW expr ',' expr ',' expr ',' expr
	{
	    g[cg].v.xv1 = $2;
	    g[cg].v.yv1 = $4;
	    g[cg].v.xv2 = $6;
	    g[cg].v.yv2 = $8;
	}
	| VIEW XMIN NUMBER {
	    g[cg].v.xv1 = $3;
	}
	| VIEW XMAX NUMBER {
	    g[cg].v.xv2 = $3;
	}
	| VIEW YMIN NUMBER {
	    g[cg].v.yv1 = $3;
	}
	| VIEW YMAX NUMBER {
	    g[cg].v.yv2 = $3;
	}
	| TITLE CHRSTR {
	    set_plotstr_string(&g[cg].labs.title, (char *) $2);
	    free((char *) $2);
	}
	| TITLE FONTP NUMBER {
	    g[cg].labs.title.font = checkon(FONTP, g[cg].labs.title.font, (int) $3);
	}
	| TITLE SIZE NUMBER {
	    g[cg].labs.title.charsize = $3;
	}
	| TITLE COLOR NUMBER {
	    g[cg].labs.title.color = checkon(COLOR, g[cg].labs.title.color, (int) $3);
	}
	| TITLE LINEWIDTH NUMBER
	{
	    g[cg].labs.title.linew = checkon(LINEWIDTH, g[cg].labs.title.linew, (int) $3);
	}
	| SUBTITLE CHRSTR {
	    set_plotstr_string(&g[cg].labs.stitle, (char *) $2);
	    free((char *) $2);
	}
	| SUBTITLE FONTP NUMBER
	{
	    g[cg].labs.stitle.font = checkon(FONTP, g[cg].labs.stitle.font, (int) $3);
	}
	| SUBTITLE SIZE NUMBER {
	    g[cg].labs.stitle.charsize = $3;
	}
	| SUBTITLE COLOR NUMBER
	{
	    g[cg].labs.stitle.color = checkon(COLOR, g[cg].labs.stitle.color, (int) $3);
	}
	| SUBTITLE LINEWIDTH NUMBER
	{
	    g[cg].labs.stitle.linew = checkon(LINEWIDTH, g[cg].labs.stitle.color, (int) $3);
	}
	| DESCRIPTION CHRSTR {
            strcat (description, (char *) $2);
	    free((char *) $2);
            strcat (description, "\n");
#ifndef NONE_GUI
            update_describe_popup ();
#endif
	}
        | CLEAR DESCRIPTION {
            *description = 0;
#ifndef NONE_GUI
            update_describe_popup ();
#endif
        }
	| GRAPHS MAXP SETS NUMBER {
	    realloc_plots((int) $4);
	}
	| LEGEND onoff {
	    g[cg].l.active = $2;
	}
	| LEGEND LOCTYPE worldview {
	    g[cg].l.loctype = $3;
	}
	| LEGEND LAYOUT NUMBER {
	    g[cg].l.layout = (int) $3;
	}
	| LEGEND VGAP NUMBER {
	    g[cg].l.vgap = (int) $3;
	}
	| LEGEND HGAP NUMBER {
	    g[cg].l.hgap = (int) $3;
	}
	| LEGEND LENGTH NUMBER {
	    g[cg].l.len = (int) $3;
	}
	| LEGEND BOX onoff {
	    g[cg].l.box = $3;
	}
	| LEGEND BOX FILL onoff {
	    g[cg].l.boxfill = $4;
	}
	| LEGEND BOX FILL WITH colpat {
	    g[cg].l.boxfillusing = $5;
	}
	| LEGEND BOX FILL colpat NUMBER
	{
	    if ($4 == CLRFILLED) {
		g[cg].l.boxfillcolor = (int) $5;
	    } else {
		g[cg].l.boxfillpat = (int) $5;
	    }
	}
	| LEGEND BOX COLOR NUMBER {
	    g[cg].l.boxlcolor = checkon(COLOR, g[cg].l.boxlcolor, (int) $4);
	}
	| LEGEND BOX LINESTYLE NUMBER {
	    g[cg].l.boxlines = checkon(LINESTYLE, g[cg].l.boxlines, (int) $4);
	}
	| LEGEND BOX LINEWIDTH NUMBER {
	    g[cg].l.boxlinew = checkon(LINEWIDTH, g[cg].l.boxlinew, (int) $4);
	}
	| LEGEND expr ',' expr {
	    g[cg].l.legx = $2;
	    g[cg].l.legy = $4;
	}
	| LEGEND X1 expr {
	    g[cg].l.legx = $3;
	}
	| LEGEND Y1 expr {
	    g[cg].l.legy = $3;
	}
	| LEGEND CHAR SIZE NUMBER {
	    g[cg].l.charsize = $4;
	}
	| LEGEND FONTP NUMBER {
	    g[cg].l.font = checkon(FONTP, g[cg].l.font, (int) $3);
	}
	| LEGEND LINESTYLE NUMBER {
	    g[cg].l.lines = checkon(LINESTYLE, g[cg].l.lines, (int) $3);
	}
	| LEGEND LINEWIDTH NUMBER {
	    g[cg].l.linew = checkon(LINEWIDTH, g[cg].l.linew, (int) $3);
	}
	| LEGEND COLOR NUMBER {
	    g[cg].l.color = checkon(COLOR, g[cg].l.color, (int) $3);
	}
	| LEGEND STRING NUMBER CHRSTR {
	    strcpy(g[cg].p[(int) $3].lstr, (char *) $4);
	    free((char *) $4);
	}
	| FRAMEP onoff {
	    g[cg].f.active = $2;
	}
	| FRAMEP TYPE NUMBER {
	    g[cg].f.type = (int) $3;
	}
	| FRAMEP LINESTYLE NUMBER {
	    g[cg].f.lines = checkon(LINESTYLE, g[cg].f.lines, (int) $3);
	}
	| FRAMEP LINEWIDTH NUMBER {
	    g[cg].f.linew = checkon(LINEWIDTH, g[cg].f.linew, (int) $3);
	}
	| FRAMEP COLOR NUMBER {
	    g[cg].f.color = checkon(COLOR, g[cg].f.color, (int) $3);
	}
	| FRAMEP FILL onoff { g[cg].f.fillbg = $3; }
	| FRAMEP BACKGROUND COLOR NUMBER { g[cg].f.bgcolor = (int) $4; }
	| GRAPHNO onoff { g[$1].active = $2; }
	| GRAPHNO LABEL onoff { g[$1].label = $3; }
	| GRAPHNO AUTOSCALE TYPE AUTO { g[$1].auto_type = TYPE_AUTO; }
	| GRAPHNO AUTOSCALE TYPE SPEC { g[$1].auto_type = TYPE_SPEC; }
	| GRAPHNO AUTOSCALE torf { g[$1].parmsread = !$3; }
	| GRAPHNO HIDDEN torf { g[$1].hidden = $3; }
	| GRAPHNO TYPE graphtype { g[$1].type = $3; }
        | GRAPHNO BAR SIZE NUMBER { g[$1].barwid = $4; }
        | GRAPHNO STACKEDBAR SIZE NUMBER { g[$1].sbarwid = $4; }
	| GRAPHNO FIXEDPOINT onoff { g[$1].pointset = $3; }
	| GRAPHNO FIXEDPOINT FORMAT formatchoice formatchoice
	{
	    g[$1].fx = $4;
	    g[$1].fy = $5;
	}
	| GRAPHNO FIXEDPOINT PREC NUMBER ',' NUMBER
	{
	    g[$1].px = $4;
	    g[$1].py = $6;
	}
	| GRAPHNO FIXEDPOINT XY expr ',' expr
	{
	    g[$1].dsx = $4;
	    g[$1].dsy = $6;
	}
	| GRAPHNO FIXEDPOINT TYPE NUMBER { g[$1].pt_type = (int) $4; }
	| GRAPHNO MAXP SETS NUMBER { realloc_graph_plots($1, (int) $4); }
	| ALIAS CHRSTR CHRSTR
	{
	    int position;
	    lowtoupper((char *) $2);
	    lowtoupper((char *) $3);
	    if ((position = findf(key, (char *) $3)) >= 0) {
	        symtab_entry tmpkey;
		tmpkey.s = malloc(strlen((char *) $2) + 1);
		strcpy(tmpkey.s, (char *) $2);
		tmpkey.type = key[position].type;
		tmpkey.fnc = key[position].fnc;
		if (addto_symtab(tmpkey) != 0) {
		    yyerror("Keyword already exists");
		}
		free (tmpkey.s);
	    } else {
	        yyerror("Aliased keyword not found");
	    }
	    free((char *) $2);
	    free((char *) $3);
	}
	| ALIAS FORCE onoff
	{
	    alias_force = (int) $3;
	}
	| USE CHRSTR TYPE proctype FROM CHRSTR
	{
	    if (load_module((char *) $6, (char *) $2, (char *) $2, $4) != 0) {
	        yyerror("DL module load failed");
	    }
	    free((char *) $2);
	    free((char *) $6);
	}
	| USE CHRSTR TYPE proctype FROM CHRSTR ALIAS CHRSTR
	{
	    if (load_module((char *) $6, (char *) $2, (char *) $8, $4) != 0) {
	        yyerror("DL module load failed");
	    }
	    free((char *) $2);
	    free((char *) $6);
	    free((char *) $8);
	}
	;


proctype:
        PROC_FUNC_I         { $$ = FUNC_I; }
	| PROC_FUNC_D       { $$ = FUNC_D; }
	| PROC_FUNC_ND     { $$ = FUNC_ND; }
	| PROC_FUNC_NN     { $$ = FUNC_NN; }
	| PROC_FUNC_DD     { $$ = FUNC_DD; }
	| PROC_FUNC_NND   { $$ = FUNC_NND; }
	| PROC_FUNC_PPD   { $$ = FUNC_PPD; }
	| PROC_FUNC_PPPV { $$ = FUNC_PPPV; }
	;


setvelocity:
        VELOCITY SCALE NUMBER { g[cg].vp.vscale = $3; }
        | VELOCITY LEGEND onoff { g[cg].vp.active = $3; }
        | VELOCITY LINEWIDTH NUMBER { g[cg].vp.linew = (int) $3; }
        | VELOCITY LINESTYLE NUMBER { g[cg].vp.lines = (int) $3; }
        | VELOCITY COLOR NUMBER { g[cg].vp.color = (int) $3; }
        | VELOCITY LOCTYPE worldview { g[cg].vp.loctype = $3; }
        | VELOCITY ARROW TYPE NUMBER { g[cg].vp.arrowtype = (int) $4; }
        ;

xytype:
	XY { $$ = SET_XY; }
	| XYBOX { $$ = SET_XYBOX; }
	| XYBOXPLOT { $$ = SET_XYBOXPLOT; }
	| XYHILO { $$ = SET_XYHILO; }
	| XYRT { $$ = SET_XYRT; }
	| XYSTRING { $$ = SET_XYSTRING; }
	| XYDX { $$ = SET_XYDX; }
	| XYDY { $$ = SET_XYDY; }
	| XYDXDX { $$ = SET_XYDXDX; }
	| XYDYDY { $$ = SET_XYDYDY; }
	| XYDXDY { $$ = SET_XYDXDY; }
	| XYXX { $$ = SET_XYXX; }
	| XYYY { $$ = SET_XYYY; }
	| XYZ { $$ = SET_XYZ; }
	| XYZW { $$ = SET_XYZW; }
	| XYUV { $$ = SET_XYUV; }
	| NXY { $$ = SET_NXY; }
	| BIN { $$ = SET_BIN; }
	| POLY { $$ = SET_POLY; }
	| RAWSPICE { $$ = SET_RAWSPICE; }
	;

graphtype:
	XY { $$ = GRAPH_XY; }
	| LOGX { $$ = GRAPH_LOGX; }
	| LOGY { $$ = GRAPH_LOGY; }
	| LOGXY { $$ = GRAPH_LOGXY; }
	| BAR { $$ = GRAPH_BAR; }
	| HBAR { $$ = GRAPH_HBAR; }
	| STACKEDBAR { $$ = GRAPH_STACKEDBAR; }
	| STACKEDHBAR { $$ = GRAPH_STACKEDHBAR; }
	| POLAR { $$ = GRAPH_POLAR; }
	| SMITH { $$ = GRAPH_SMITH; }
	;

	
nonlfitopts:
        TITLE CHRSTR { 
          strcpy(nonl_opts.title, (char *) $2);
	  free((char *) $2);
        }
        | FORMULA CHRSTR { 
          strcpy(nonl_opts.formula, (char *) $2);
	  free((char *) $2);
        }
        | WITH NUMBER PARAMETERS { 
            nonl_opts.parnum = (int) $2; 
        }
        | PREC NUMBER { 
            nonl_opts.tolerance = $2; 
        }
        ;
        
pagelayout:
        FREE { $$ = PAGE_FREE; }
        | LANDSCAPE { $$ = PAGE_LANDSCAPE; }
        | PORTRAIT { $$ = PAGE_PORTRAIT; }
        | FIXED { $$ = PAGE_FIXED; }
        ;

regiontype:
	ABOVE { $$ = REGION_ABOVE; }
	|  BELOW { $$ = REGION_BELOW; }
	|  LEFT { $$ = REGION_TOLEFT; }
	|  RIGHT { $$ = REGION_TORIGHT; }
	|  POLYI { $$ = REGION_POLYI; }
	|  POLYO { $$ = REGION_POLYO; }
	;

set_setprop:
	selectsets setprop {}
/* for ranges - yet to be implemented
	|  SETNUM '-' SETNUM {
	    printf("%d-%d\n", $1, $3);
	}
	| GRAPHNO SETNUM '-' SETNUM {
	    printf("Graph %d %d-%d\n", $1, $2, $4);
	}
	|  GRAPHS SETNUM '-' SETNUM {
	    printf("ALL graphs %d-%d\n", $2, $4);
	}
*/
	;

setprop:
	onoff {
	    set_prop(whichgraph, SET, SETNUM, whichset, ACTIVE, $1, 0);
	}
	| IGNORE {
	    set_prop(whichgraph, SET, SETNUM, whichset, ACTIVE, $1, 0);
	}
	| TYPE xytype {
	    set_prop(whichgraph, SET, SETNUM, whichset, TYPE, $2, 0);
	}
	| PREC NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, PREC, (int) $2, 0);
	}
	| FORMAT formatchoice {
	    set_prop(whichgraph, SET, SETNUM, whichset, FORMAT, $2, 0);
	}
	| SYMBOL expr {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, TYPE, (int) $2, 0);
	}
	| SYMBOL FILL NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, FILL, (int) $3, 0);
	}
	| SYMBOL CENTER torf {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, CENTER, $3, 0);
	}
	| SYMBOL SIZE expr {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, SIZE, $3, 0);
	}
	| SYMBOL CHAR NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, CHAR, (int) $3, 0);
	}
	| SYMBOL SKIP NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, SKIP, (int) $3, 0);
	}
	| SYMBOL COLOR expr {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, COLOR, (int) $3, 0);
	}
	| SYMBOL LINEWIDTH NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, LINEWIDTH, (int) $3, 0);
	}
	| SYMBOL LINESTYLE NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SYMBOL, LINESTYLE, (int) $3, 0);
	}
	| prop NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, $1, (int) $2, 0);
	}
	| FILL NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, FILL, TYPE, (int) $2, 0);
	}
	| FILL WITH colpat {
	    set_prop(whichgraph, SET, SETNUM, whichset, FILL, WITH, $3, 0);
	}
	| FILL colpat NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, FILL, $2, (int) $3, 0);
	}
	| SKIP NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, SKIP, (int) $2, 0);
	}
	| ERRORBAR LENGTH NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, LENGTH, $3, 0);
	}
	| ERRORBAR TYPE opchoice {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, TYPE, $3, 0);
	}
	| ERRORBAR LINEWIDTH NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, LINEWIDTH, (int) $3, 0);
	}
	| ERRORBAR LINESTYLE NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, LINESTYLE, (int) $3, 0);
	}
	| ERRORBAR RISER onoff {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, RISER, ACTIVE, $3, 0);
	}
	| ERRORBAR RISER LINEWIDTH NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, RISER, LINEWIDTH, (int) $4, 0);
	}
	| ERRORBAR RISER LINESTYLE NUMBER {
	    set_prop(whichgraph, SET, SETNUM, whichset, ERRORBAR, RISER, LINESTYLE, (int) $4, 0);
	}
	| XYZ expr ',' expr {
	    set_prop(whichgraph, SET, SETNUM, whichset, XYZ, $2, $4, 0);
	}
	| COMMENT CHRSTR {
	    set_prop(whichgraph, SET, SETNUM, whichset, COMMENT, (char *) $2, 0);
	    free((char *) $2);
	}
	;

setaxis:
	axis {}
	|  axis axisfeature {}
	|  allaxes {}
	|  GRAPHS axis {}
	|  GRAPHS axis axisfeature {}
	|  GRAPHS allaxes {}
	;

axis:
	XAXIS {}
	|  YAXIS {}
	|  ALTXAXIS {}
	|  ALTYAXIS {}
	|  ZEROXAXIS {}
	|  ZEROYAXIS {}
	;

allaxes:
	AXES axesprops {}
	|  XAXES axesprops {}
	|  YAXES axesprops {}
	;

axesprops:
	onoff {
	    set_axis_prop(whichgraph, naxis, $1, 0.0);
	}
	| COLOR NUMBER {
	    set_axis_prop(whichgraph, naxis, $1, $2);
	}
	| LINEWIDTH NUMBER {
	    set_axis_prop(whichgraph, naxis, $1, $2);
	}
	| LINESTYLE NUMBER {
	    set_axis_prop(whichgraph, naxis, $1, $2);
	}
	| FONTP NUMBER {
	    set_axis_prop(whichgraph, naxis, $1, $2);
	}
	| CHAR SIZE NUMBER {
	    set_axis_prop(whichgraph, naxis, $1, $3);
	}
	| GRID onoff {
	    set_axis_prop(whichgraph, naxis, $1, $2);
	}
	;

axisfeature:
	TICKP tickdesc {}
	|  TICKLABEL ticklabeldesc {}
	|  LABEL axislabeldesc {}
	|  BAR axisbardesc {}
	|  onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].active = $1;
	    }
	}
	;

tickdesc:
	tickattr {}
	|  tickdesc tickattr {}
	;

tickattr:
	onoff
	{
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_flag = $1;
	        g[cg].t[naxis].t_mflag = $1;
	    }
	}
	| MAJOR onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_flag = $2;
	    }
	}
	| MINOR onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_mflag = $2;
	    }
	}
	| MAJOR expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tmajor = $2;
	    }
	}
	| MINOR expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tminor = $2;
	    }
	}
	| OFFSETX expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].offsx = $2;
	    }
	}
	| OFFSETY expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].offsy = $2;
	    }
	}
	| ALT onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].alt = $2;
	    }
	}
	| MINP expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tmin = $2;
	    }
	}
	| MAXP expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tmax = $2;
	    }
	}
	| DEFAULT NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_num = (int) $2;
	    }
	}
	| inoutchoice {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_inout = $1;
	    }
	}
	| LOG onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_log = $2;
	    }
	}
	| SIZE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_size = $2;
	    }
	}
	| MAJOR SIZE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_size = $3;
	    }
	}
	| MINOR SIZE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_msize = $3;
	    }
	}
	| COLOR NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_color = g[cg].t[naxis].t_mcolor = (int) $2;
	    }
	}
	| LINEWIDTH NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_linew = g[cg].t[naxis].t_mlinew = (int) $2;
	    }
	}
	| MAJOR COLOR NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_color = (int) $3;
	    }
	}
	| MINOR COLOR NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_mcolor = (int) $3;
	    }
	}
	| MAJOR LINEWIDTH NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_linew = (int) $3;
	    }
	}
	| MINOR LINEWIDTH NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_mlinew = (int) $3;
	    }
	}
	| MAJOR LINESTYLE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_lines = (int) $3;
	    }
	}
	| MINOR LINESTYLE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_mlines = (int) $3;
	    }
	}
	| MAJOR GRID onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_gridflag = $3;
	    }
	}
	| MINOR GRID onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_mgridflag = $3;
	    }
	}
	| OP opchoice {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_op = $2;
	    }
	}
	| TYPE AUTO {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_type = TYPE_AUTO;
	    }
	}
	| TYPE SPEC {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_type = TYPE_SPEC;
	    }
	}
	| SPEC NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_spec = (int) $2;
	    }
	}
	| NUMBER ',' expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_specloc[(int) $1] = $3;
	    }
	}
	;

ticklabeldesc:
	ticklabelattr
	|  ticklabeldesc ticklabelattr
	;

ticklabelattr:
	onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_flag = $1;
	    }
	}
	| TYPE AUTO {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_type = TYPE_AUTO;
	    }
	}
	| TYPE SPEC {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_type = TYPE_SPEC;
	    }
	}
	| PREC NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_prec = (int) $2;
	    }
	}
	| FORMAT formatchoice {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_format = $2;
	    }
	}
	| FORMAT NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_format = $2;
	    }
	}
	| APPEND CHRSTR {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        strcpy(g[cg].t[naxis].tl_appstr, (char *) $2);
	    }
	    free((char *) $2);
	}
	| PREPEND CHRSTR {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        strcpy(g[cg].t[naxis].tl_prestr, (char *) $2);
	    }
	    free((char *) $2);
	}
	| LAYOUT HORIZONTAL {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_layout = TICKS_HORIZONTAL;
	    }
	}
	| LAYOUT VERTICAL {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_layout = TICKS_VERTICAL;
	    }
	}
	| LAYOUT SPEC {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_layout = TYPE_SPEC;
	    }
	}
	| PLACE ON TICKS {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_loc = LABEL_ONTICK;
	    }
	}
	| PLACE BETWEEN TICKS {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_loc = LABEL_BETWEEN;
	    }
	}
	| LOCTYPE ON {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_layout = ON;
	    }
	}
	| LOCTYPE CENTER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_layout = CENTER;
	    }
	}
	| LOCTYPE LEFT {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_layout = LEFT;
	    }
	}
	| LOCTYPE RIGHT {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_layout = RIGHT;
	    }
	}
	| ANGLE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_angle = (int) $2;
	    }
	}
	| JUST justchoice {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_just = (int) $2;
	    }
	}
	| SKIP NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_skip = (int) $2;
	    }
	}
	| STAGGER NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_staggered = (int) $2;
	    }
	}
	| OP opchoice {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_op = $2;
	    }
	}
	| SIGN signchoice {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_sign = $2;
	    }
	}
	| START expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_start = $2;
	    }
	}
	| STOP expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_stop = $2;
	    }
	}
	| START TYPE SPEC {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_starttype = TYPE_SPEC;
	    }
	}
	| START TYPE AUTO {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_starttype = TYPE_AUTO;
	    }
	}
	| STOP TYPE SPEC {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_stoptype = TYPE_SPEC;
	    }
	}
	| STOP TYPE AUTO {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_stoptype = TYPE_AUTO;
	    }
	}
	| VGAP NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_vgap = $2;
	    }
	}
	| HGAP NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_hgap = $2;
	    }
	}
	| CHAR SIZE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_charsize = $3;
	    }
	}
	| FONTP NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_font = (int) $2;
	    }
	}
	| COLOR NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_color = (int) $2;
	    }
	}
	| LINEWIDTH NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].tl_linew = (int) $2;
	    }
	}
	| NUMBER ',' CHRSTR {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        set_plotstr_string(&g[cg].t[naxis].t_speclab[(int) $1], (char *) $3);
	    }
	    free((char *) $3);
	}
	;

axislabeldesc:
	CHRSTR {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        set_plotstr_string(&g[cg].t[naxis].label, (char *) $1);
	    }
	    free((char *) $1);
	}
	| LAYOUT PERP {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label_layout = LAYOUT_PERPENDICULAR;
	    }
	}
	| LAYOUT PARA {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label_layout = LAYOUT_PARALLEL;
	    }
	}
	| PLACE AUTO {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label_place = TYPE_AUTO;
	    }
	}
	| PLACE SPEC {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label_place = TYPE_SPEC;
	    }
	}
	| PLACE expr ',' expr {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label.x = $2;
	        g[cg].t[naxis].label.y = $4;
	    }
	}
	| JUST justchoice {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label.just = (int) $2;
	    }
	}
	| CHAR SIZE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label.charsize = $3;
	    }
	}
	| FONTP NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label.font = (int) $2;
	    }
	}
	| COLOR NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label.color = (int) $2;
	    }
	}
	| LINEWIDTH NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].label.linew = (int) $2;
	    }
	}
	;

axisbardesc:
	onoff {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_drawbar = $1;
	    }
	}
	| COLOR NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_drawbarcolor = (int) $2;
	    }
	}
	| LINESTYLE NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_drawbarlines = (int) $2;
	    }
	}
	| LINEWIDTH NUMBER {
	    if (naxis >= 0 && naxis < MAXAXES) {
	        g[cg].t[naxis].t_drawbarlinew = (int) $2;
	    }
	}
	;

selectsets:
	GRAPHNO '.' SETNUM
	{
	    whichgraph = $1;
	    whichset = $3;
	}
	| SETNUM
	{
	    whichgraph = cg;
	    whichset = $1;
	}
	|  SETS
	{
	    whichgraph = cg;
	    whichset = $1;
	}
	| GRAPHNO SETS
	{
	    whichgraph = $1;
	    whichset = $2;
	}
	|  GRAPHS SETS
	{
	    whichgraph = $1;
	    whichset = $2;
	}
	|  GRAPHS SETNUM
	{
	    whichgraph = $1;
	    whichset = $2;
	}
	;

prop: LINESTYLE { $$ = $1; }
        | LINEWIDTH { $$ = $1; }
        | FONTP { $$ = $1; }
        | COLOR { $$ = $1; }
        | SIZE { $$ = $1; }
        ;

onoff: ON { $$ = TRUE; }
	| OFF { $$ = FALSE; }
	;

colpat: NONE { $$ = UNFILLED; }
	| COLOR { $$ = CLRFILLED; }
	| PATTERN { $$ = PTNFILLED; }
	;

runtype: RUNAVG { $$ = RUN_AVG; }
	| RUNSTD { $$ = RUN_STD; }
	| RUNMED { $$ = RUN_MED; }
	| RUNMAX { $$ = RUN_MAX; }
	| RUNMIN { $$ = RUN_MIN; }
	;

ffttype: DFT { $$ = FFT_DFT; }
	| FFT { $$ = FFT_FFT; }
	| INVDFT { $$ = FFT_INVDFT; }
	| INVFFT { $$ = FFT_INVFFT; }
	;

sourcetype: DISK { $$ = SOURCE_DISK; }
	| PIPE { $$ = SOURCE_PIPE; }
	;

filltype: PATTERN { $$ = PTNFILLED; }
	| COLOR { $$ = CLRFILLED; }
	| NONE { $$ = UNFILLED; }
	;

opchoice: TOP { $$ = PLACE_TOP; }
	| BOTTOM { $$ = PLACE_BOTTOM; }
	| LEFT { $$ = PLACE_LEFT; }
	| RIGHT { $$ = PLACE_RIGHT; }
	| BOTH { $$ = PLACE_BOTH; }
	;

justchoice: RIGHT { $$ = JUST_RIGHT; }
	| LEFT { $$ = JUST_LEFT; }
	| CENTER { $$ = JUST_CENTER; }
	;

extremetype: MINP { $$ = MINP; }
	| MAXP { $$ = MAXP; }
	;

barsd: AVG { $$ = AVG; }
	| SD { $$ = SD; }
	;

torf: TRUEP { $$ = TRUE; }
	| FALSEP { $$ = FALSE; }
	;

inoutchoice: IN { $$ = TICKS_IN; }
	| OUT { $$ = TICKS_OUT; }
	| BOTH { $$ = TICKS_BOTH; }
	;

formatchoice: DECIMAL { $$ = FORMAT_DECIMAL; }
	| EXPONENTIAL { $$ = FORMAT_EXPONENTIAL; }
	| POWER { $$ = FORMAT_POWER; }
	| GENERAL { $$ = FORMAT_GENERAL; }
	| DDMMYY { $$ = FORMAT_DDMMYY; }
	| MMDDYY { $$ = FORMAT_MMDDYY; }
	| YYMMDD { $$ = FORMAT_YYMMDD; }
	| MMYY { $$ = FORMAT_MMYY; }
	| MMDD { $$ = FORMAT_MMDD; }
	| MONTHDAY { $$ = FORMAT_MONTHDAY; }
	| DAYMONTH { $$ = FORMAT_DAYMONTH; }
	| MONTHS { $$ = FORMAT_MONTHS; }
	| MONTHSY { $$ = FORMAT_MONTHSY; }
	| MONTHL { $$ = FORMAT_MONTHL; }
	| DAYOFWEEKS { $$ = FORMAT_DAYOFWEEKS; }
	| DAYOFWEEKL { $$ = FORMAT_DAYOFWEEKL; }
	| DAYOFYEAR { $$ = FORMAT_DAYOFYEAR; }
	| HMS { $$ = FORMAT_HMS; }
	| MMDDHMS { $$ = FORMAT_MMDDHMS; }
	| MMDDYYHMS { $$ = FORMAT_MMDDYYHMS; }
	| YYMMDDHMS { $$ = FORMAT_YYMMDDHMS; }
	| DEGREESLON { $$ = FORMAT_DEGREESLON; }
	| DEGREESMMLON { $$ = FORMAT_DEGREESMMLON; }
	| DEGREESMMSSLON { $$ = FORMAT_DEGREESMMSSLON; }
	| MMSSLON { $$ = FORMAT_MMSSLON; }
	| DEGREESLAT { $$ = FORMAT_DEGREESLAT; }
	| DEGREESMMLAT { $$ = FORMAT_DEGREESMMLAT; }
	| DEGREESMMSSLAT { $$ = FORMAT_DEGREESMMSSLAT; }
	| MMSSLAT { $$ = FORMAT_MMSSLAT; }
	;

signchoice: NORMAL { $$ = SIGN_NORMAL; }
	| ABSOLUTE { $$ = SIGN_ABSOLUTE; }
	| NEGATE { $$ = SIGN_NEGATE; }
	;

direction: UP { $$ = UP; }
	| DOWN { $$ = DOWN; }
	| RIGHT { $$ = RIGHT; }
	| LEFT { $$ = LEFT; }
	| IN { $$ = IN; }
	| OUT { $$ = OUT; }
	;

worldview: WORLD { $$ = COORD_WORLD; }
	| VIEW { $$ = COORD_VIEW; }
	;

sortdir: ASCENDING { $$ = ASCENDING; }
	| DESCENDING { $$ = DESCENDING; }
	;

sorton: X_TOK { $$ = DATA_X; }
	| Y_TOK { $$ = DATA_Y; }
	;

vector: X_TOK { $$ = DATA_X; }
	| Y_TOK { $$ = DATA_Y; }
	| X0 { $$ = DATA_X; }
	| Y0 { $$ = DATA_Y; }
	| Y1 { $$ = DATA_Y1; }
	| Y2 { $$ = DATA_Y2; }
	| Y3 { $$ = DATA_Y3; }
	| Y4 { $$ = DATA_Y4; }
	;

asgn:
	VAR '[' expr ']' '=' expr
	{
	    int itmp = (int) $3 - index_shift;
	    if (itmp >= ls) {
		yyerror("Subscript out of range");
		return 1;
	    } else {
		$1[itmp] = $6;
		result = $6;
	    }
	}
	| FITPARM '=' expr
	{
	    nonl_parms[(int) $1].value = $3;
	    result = $3;
	}
	| FITPARM CONSTRAINTS onoff
	{
	    nonl_parms[(int) $1].constr = $3;
	    result = $3;
	}
	| FITPMAX '=' expr
	{
	    nonl_parms[(int) $1].max = $3;
	    result = $3;
	}
	| FITPMIN '=' expr
	{
	    nonl_parms[(int) $1].min = $3;
	    result = $3;
	}
	| vector '[' expr ']' '=' expr
	{
	    int itmp = (int) $3 - index_shift;
	    double *ptr = getvptr(cg, curset, $1);
	    if (ptr != NULL) {
	        ptr[itmp] = $6;
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    result = $6;
	    updatesetminmax(cg, curset);
#ifndef NONE_GUI
	    update_set_status(cg, curset);
#endif
	}
	| SETNUM '.' vector '[' expr ']' '=' expr
	{
	    int itmp = (int) $5 - index_shift;
	    double *ptr = getvptr(cg, $1, $3);
	    if (ptr != NULL) {
	        ptr[itmp] = $8;
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    result = $8;
	    updatesetminmax(cg, $1);
#ifndef NONE_GUI
	    update_set_status(cg, $1);
#endif
	}
	| SETNUM '.' SYMBOL '=' expr
	{
	    set_prop(cg, SET, SETNUM, $1, SYMBOL, TYPE, (int) $5, 0);
	    result = 0;
	}
	| GRAPHNO '.' SETNUM '.' vector '[' expr ']' '=' expr
	{
	    int itmp = (int) $7 - index_shift;
	    double *ptr = getvptr($1, $3, $5);
	    if (ptr != NULL) {
	        ptr[itmp] = $10;
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    result = $10;
	    updatesetminmax($1, $3);
#ifndef NONE_GUI
	    update_set_status($1, $3);
#endif
	}
	;

vasgn:
	VAR '=' vexpr
	{
	    int i;
	    if (ls < 0) {
		yyerror("Length of array < 0");
		return 1;
	    } else if (ls > maxarr) {
		maxarr = ls;
		init_scratch_arrays(ls);
	    }
	    for (i = 0; i < lxy; i++) {
		$1[i] = $3[i];
	    }
	    result = $3[0];
	}
	| vector '=' vexpr
	{
	    int i;
	    double *ptr;
	    if (!isactive_set(cg, curset)) {
		activateset(cg, curset);
		setlength(cg, curset, lxy);
		setcomment(cg, curset, "Created");
	    }
	    ptr = getvptr(cg, curset, $1);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $3[i];
	        }
	        result = $3[0];
	        updatesetminmax(cg, curset);
#ifndef NONE_GUI
	        update_set_status(cg, curset);
#endif
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| SETNUM '=' SETNUM
	{
	    if (isactive_set(cg, $3)) {
	        do_copyset(cg, $3, cg, $1);
	    } else {
		yyerror("Right value set not active");
	    }
	    result = 0;
	}
	| GRAPHNO '.' SETNUM '=' SETNUM
	{
	    if (isactive_set(cg, $5)) {
	        do_copyset(cg, $5, $1, $3);
	    } else {
		yyerror("Right value set not active");
	    }
	    result = 0;
	}
	| SETNUM '=' GRAPHNO '.' SETNUM
	{
	    if (isactive_set($3, $5)) {
	        do_copyset($3, $5, cg, $1);
	    } else {
		yyerror("Right value set not active");
	    }
	    result = 0;
	}
	| GRAPHNO '.' SETNUM '=' GRAPHNO '.' SETNUM
	{
	    if (isactive_set($5, $7)) {
	        do_copyset($5, $7, $1, $3);
	    } else {
		yyerror("Right value set not active");
	    }
	    result = 0;
	}
	| SETNUM '.' vector '=' vexpr {
	    int i;
	    double *ptr;
	    if (!isactive_set(cg, $1)) {
		activateset(cg, $1);
		setlength(cg, $1, lxy);
		setcomment(cg, $1, "Created");
	    }
	    ptr = getvptr(cg, $1, $3);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $5[i];
	        }
	        result = $5[0];
	        updatesetminmax(cg, $1);
#ifndef NONE_GUI
	        update_set_status(cg, $1);
#endif
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector '=' vexpr
	{
	    int i;
	    double *ptr;
	    
	    if (!isactive_set($1, $3)) {
		activateset($1, $3);
		setlength($1, $3, lxy);
		setcomment($1, $3, "Created");
	    }
	    ptr = getvptr($1, $3, $5);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $7[i];
	        }
	        result = $7[0];
	        updatesetminmax($1, $3);
#ifndef NONE_GUI
	        update_set_status($1, $3);
#endif
	    } else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| VAR '=' expr
	{
	    int i;
	    if (lxy < 0) {
		yyerror("Length of array < 0");
		return 1;
	    } else if (lxy > maxarr) {
		maxarr = lxy;
		init_scratch_arrays(lxy);
	    }
	    for (i = 0; i < lxy; i++) {
		$1[i] = $3;
	    }
	    result = $3;
	}
	| vector '=' expr
	{
	    int i;
	    double *ptr;
	    if (!isactive_set(cg, curset)) {
		activateset(cg, curset);
		setlength(cg, curset, lxy);
		setcomment(cg, curset, "Created");
	    }
	    ptr = getvptr(cg, curset, $1);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $3;
	        }
	        result = $3;
	        updatesetminmax(cg, curset);
#ifndef NONE_GUI
	        update_set_status(cg, curset);
#endif
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| SETNUM '.' vector '=' expr
	{
	    int i;
	    double *ptr;
	    if (!isactive_set(cg, $1)) {
		activateset(cg, $1);
		setlength(cg, $1, lxy);
		setcomment(cg, $1, "Created");
	    }
	    ptr = getvptr(cg, $1, $3);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $5;
	        }
	        result = $5;
	        updatesetminmax(cg, $1);
#ifndef NONE_GUI
	        update_set_status(cg, $1);
#endif
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector '=' expr
	{
	    int i;
	    double *ptr;
	    
	    if (!isactive_set($1, $3)) {
		activateset($1, $3);
		setlength($1, $3, lxy);
		setcomment($1, $3, "Created");
	    }
	    ptr = getvptr($1, $3, $5);
	    if (ptr != NULL) {
	        for (i = 0; i < lxy; i++) {
		    ptr[i] = $7;
	        }
	        result = $7;
	        updatesetminmax($1, $3);
#ifndef NONE_GUI
	        update_set_status($1, $3);
#endif
	    } else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	;

vexpr:
	VAR
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    if (lxy < 0) {
		yyerror("Length of array < 0");
		return 1;
	    } else if (lxy > maxarr) {
		maxarr = lxy;
		init_scratch_arrays(lxy);
	    }
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i];
	    }
	}
	| vector
	{
	    int i;
	    double *ptr = getvptr(cg, curset, $1);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = ptr[i];
	    }
	}
	| SETNUM '.' vector
	{
	    int i;
	    double *ptr = getvptr(cg, $1, $3);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = ptr[i];
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector
	{
	    int i;
	    double *ptr = getvptr($1, $3, $5);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = ptr[i];
	    }
	}
	| expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1;
	    }
	}
	| expr '+' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 + $3;
	    }
	}
	| vexpr '+' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] + $3[i];
	    }
	}
	| expr '+' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 + $3[i];
	    }
	}
	| vexpr '+' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] + $3;
	    }
	}
	| expr '-' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 - $3;
	    }
	}
	| vexpr '-' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] - $3[i];
	    }
	}
	| expr '-' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 - $3[i];
	    }
	}
	| vexpr '-' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] - $3;
	    }
	}
	| expr '*' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 * $3;
	    }
	}
	| vexpr '*' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] * $3[i];
	    }
	}
	| expr '*' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 * $3[i];
	    }
	}
	| vexpr '*' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] * $3;
	    }
	}
	| expr '/' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    if ($3 == 0.0) {
		yyerror("Divide by Zero");
		return 1;
	    }
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 / $3;
	    }
	}
	| vexpr '/' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		if ($3[i] == 0.0) {
		    yyerror("Divide by Zero");
		    return 1;
		}
	    }
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] / $3[i];
	    }
	}
	| expr '/' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		if ($3[i] == 0.0) {
		    yyerror("Divide by Zero");
		    return 1;
		}
	    }
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1 / $3[i];
	    }
	}
	| vexpr '/' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    if ($3 == 0.0) {
		yyerror("Divide by Zero");
		return 1;
	    }
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] / $3;
	    }
	}
	| expr '^' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = pow($1, $3);
	    }
	}
	| expr '^' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = pow($1, $3[i]);
	    }
	}
	| vexpr '^' expr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = pow($1[i], $3);
	    }
	}
	| vexpr '^' vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = pow($1[i], $3[i]);
	    }
	}
	| FUNC_I '(' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc((int) $3[i]);
	    }
	}
	| FUNC_D '(' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3[i]);
	    }
	}
	| FUNC_DD '(' vexpr ',' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3[i], $5[i]);
	    }
	}
	| FUNC_DD '(' expr ',' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3, $5[i]);
	    }
	}
	| FUNC_DD '(' vexpr ',' expr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3[i], $5);
	    }
	}
	| FUNC_ND '(' expr ',' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc((int) $3, $5[i]);
	    }
	}
	| FUNC_NND '(' expr ',' expr ',' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc((int) $3, (int) $5, $7[i]);
	    }
	}
	| FUNC_PPD '(' expr ',' expr ',' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3, $5, $7[i]);
	    }
	}
	| FUNC_PPPV '(' expr ',' expr ',' expr ',' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = key[$1].fnc($3, $5, $7, $9[i]);
	    }
	}
	| DEG
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] *= M_PI / 180.0;
	    }
	}
	| INDEX
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = i + index_shift;
	    }
	}
	| PI_TOK
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = M_PI;
	    }
	}
	| RAD
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = M_PI / 180.0;
	    }
	}
	| IRAND '(' expr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = lrand48() % (long) ($3);
	    }
	}
	| MAXP '(' vexpr ',' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $3[i] >= $5[i] ? $3[i] : $5[i];
	    }
	}
	| MINP '(' vexpr ',' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $3[i] <= $5[i] ? $3[i] : $5[i];
	    }
	}
	| RAND
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = (double) drand48();
	    }
	}
	| vexpr '?' vexpr ':' vexpr {
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
	        if ((int) $1[i]) {
		    $$[i] = $3[i];
	        } else {
		    $$[i] = $5[i];
	        }
	    }
	}
	| vexpr GT vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] > $3[i];
	    }
	}
	| vexpr LT vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] < $3[i];
	    }
	}
	| vexpr LE vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] <= $3[i];
	    }
	}
	| vexpr GE vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] >= $3[i];
	    }
	}
	| vexpr EQ vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] == $3[i];
	    }
	}
	| vexpr NE vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] != $3[i];
	    }
	}
	| vexpr AND vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] && $3[i];
	    }
	}
	| vexpr OR vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $1[i] || $3[i];
	    }
	}
	| NOT vexpr
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = !($2[i]);
	    }
	}
	| '(' vexpr ')'
	{
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = $2[i];
	    }
	}
	| '-' vexpr %prec UMINUS {
	    int i;
	    $$ = (double *) calloc(lxy, sizeof(double));
	    freelist[fcnt++] = $$;
	    for (i = 0; i < lxy; i++) {
		$$[i] = -$2[i];
	    }
	}
	;

expr:	NUMBER {
	    $$ = $1;
	}
	|  FITPARM {
	    $$ = nonl_parms[(int) $1].value;
	}
	|  FITPMAX {
	    $$ = nonl_parms[(int) $1].max;
	}
	|  FITPMIN {
	    $$ = nonl_parms[(int) $1].min;
	}
	|  VAR '[' expr ']' {
	    $$ = $1[(int) $3 - index_shift];
	}
	| REGNUM '.' vector {
	    $$ = ($3 == X_TOK) ? *xx : *yy;
	}
	| vector '[' expr ']' {
	    double *ptr = getvptr(cg, curset, $1);
	    if (ptr != NULL) {
		$$ = ptr[(int) $3 - index_shift];
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| SETNUM '.' vector '[' expr ']' {
	    double *ptr = getvptr(cg, $1, $3);
	    if (ptr != NULL) {
		$$ = ptr[(int) $5 - index_shift];
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector '[' expr ']' {
	    double *ptr = getvptr($1, $3, $5);
	    if (ptr != NULL) {
		$$ = ptr[(int) $7 - index_shift];
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| SETNUM '.' vector '.' extremetype {
	    double *ptr = getvptr(cg, $1, $3);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    switch ($5) {
	    case MINP:
		$$ = vmin(ptr, g[cg].p[$1].len);
		break;
	    case MAXP:
		$$ = vmax(ptr, g[cg].p[$1].len);
		break;
	    }
	}
	| GRAPHNO '.' SETNUM '.' vector '.' extremetype {
	    double *ptr = getvptr($1, $3, $5);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    switch ($7) {
	    case MINP:
		$$ = vmin(ptr, g[$1].p[$3].len);
		break;
	    case MAXP:
		$$ = vmax(ptr, g[$1].p[$3].len);
		break;
	    }
	}
	| vector '.' extremetype {
	    double *ptr = getvptr(cg, curset, $1);
	    if (ptr == NULL) {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	    switch ($3) {
	    case MINP:
		$$ = vmin(ptr, g[cg].p[curset].len);
		break;
	    case MAXP:
		$$ = vmax(ptr, g[cg].p[curset].len);
		break;
	    }
	}
	| GRAPHNO '.' SETNUM '.' LENGTH {
	    $$ = g[$1].p[$3].len;
	}
	| SETNUM '.' LENGTH {
	    $$ = g[cg].p[$1].len;
	}
	| LENGTH {
	    $$ = g[cg].p[curset].len;
	}
	| GRAPHNO '.' SETNUM '.' vector '.' barsd {
	    double bar, sd;
	    double *ptr = getvptr($1, $3, $5);
	    if (ptr != NULL) {
		stasum(ptr, getsetlength($1, $3), &bar, &sd, 0);
                switch ($7) {
                case AVG:;
	            $$ = bar;
                    break;
                case SD:
                    $$ = sd;
                    break;
                }
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
        | SETNUM '.' vector '.' barsd {
	    double bar, sd;
	    double *ptr = getvptr(cg, $1, $3);
	    if (ptr != NULL) {
		stasum(ptr, getsetlength(cg, $1), &bar, &sd, 0);
                switch ($5) {
                case AVG:;
	            $$ = bar;
                    break;
                case SD:
                    $$ = sd;
                    break;
                }
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
        | vector '.' barsd {
	    double bar, sd;
	    double *ptr = getvptr(cg, curset, $1);
	    if (ptr != NULL) {
		stasum(ptr, getsetlength(cg, curset), &bar, &sd, 0);
                switch ($3) {
                case AVG:;
	            $$ = bar;
                    break;
                case SD:
                    $$ = sd;
                    break;
                }
	    }
	    else {
		yyerror("NULL variable, check set type");
		return 1;
	    }
	}
	| expr '+' expr {
	    $$ = $1 + $3;
	}
	| expr '-' expr {
	    $$ = $1 - $3;
	}
	| expr '*' expr {
	    $$ = $1 * $3;
	}
	| expr '/' expr
	{
	    if ($3 != 0.0) {
		$$ = $1 / $3;
	    } else {
		yyerror("Divide by Zero");
		return 1;
	    }
	}
	| expr '%' expr {
	    $$ = fmod($1, $3);
	}
	| expr '^' expr {
	    $$ = pow($1, $3);
	}
	| FUNC_I '(' expr ')'
	{
	    $$ = key[$1].fnc((int) $3);
	}
	| FUNC_D '(' expr ')'
	{
	    $$ = key[$1].fnc($3);
	}
	| FUNC_ND '(' expr ',' expr ')'
	{
	    $$ = key[$1].fnc((int) $3, $5);
	}
	| FUNC_NN '(' expr ',' expr ')'
	{
	    $$ = key[$1].fnc((int) $3, (int) $5);
	}
	| FUNC_DD '(' expr ',' expr ')'
	{
	    $$ = key[$1].fnc($3, $5);
	}
	| FUNC_NND '(' expr ',' expr ',' expr')'
	{
	    $$ = key[$1].fnc((int) $3, (int) $5, $7);
	}
	| FUNC_PPD '(' expr ',' expr ',' expr')'
	{
	    $$ = key[$1].fnc($3, $5, $7);
	}
	| DEG {
	    $$ = 180.0 / M_PI;
	}
	| DX {
	    $$ = g[cg].w.xg2 - g[cg].w.xg1;
	}
	| DY {
	    $$ = g[cg].w.yg2 - g[cg].w.yg1;
	}
	| GRAPHNO '.' VX1 {
	    $$ = g[$1].v.xv1;
	}
	| GRAPHNO '.' VX2 {
	    $$ = g[$1].v.xv2;
	}
	| GRAPHNO '.' VY1 {
	    $$ = g[$1].v.yv1;
	}
	| GRAPHNO '.' VY2 {
	    $$ = g[$1].v.yv2;
	}
	| GRAPHNO '.' WX1 {
	    $$ = g[$1].w.xg1;
	}
	| GRAPHNO '.' WX2 {
	    $$ = g[$1].w.xg2;
	}
	| GRAPHNO '.' WY1 {
	    $$ = g[$1].w.yg1;
	}
	| GRAPHNO '.' WY2 {
	    $$ = g[$1].w.yg2;
	}
	| INDEX {
	    $$ = setindex;
	}
	| IRAND '(' expr ')' {
	    $$ = lrand48() % (long) ($3);
	}
	| JDAY '(' expr ',' expr ',' expr ')' { /* yr, mo, day */
	    $$ = julday((int) $5, (int) $7, (int) $3, 12, 0, 0.0);
	}
	| JDAY0 '(' expr ',' expr ',' expr ',' expr ',' expr ',' expr ')' 
	{ /* yr, mo, day, hr, min, sec */
	    $$ = julday((int) $5, (int) $7, (int) $3, (int) $9, (int) $11, (double) $13);
	}
	| MAXP '(' expr ',' expr ')' {
	    $$ = $3 >= $5 ? $3 : $5;
	}
	| MINP '(' expr ',' expr ')' {
	    $$ = $3 <= $5 ? $3 : $5;
	}
	| PI_TOK {
	    $$ = M_PI;
	}
	| RAD {
	    $$ = M_PI / 180.0;
	}
	| RAND {
	    $$ = (double) drand48();
	}
	| VX1 {
	    $$ = g[cg].v.xv1;
	}
	| VX2 {
	    $$ = g[cg].v.xv2;
	}
	| VY1 {
	    $$ = g[cg].v.yv1;
	}
	| VY2 {
	    $$ = g[cg].v.yv2;
	}
	| WX1 {
	    $$ = g[cg].w.xg1;
	}
	| WX2 {
	    $$ = g[cg].w.xg2;
	}
	| WY1 {
	    $$ = g[cg].w.yg1;
	}
	| WY2 {
	    $$ = g[cg].w.yg2;
	}
	| expr '?' expr ':' expr {
	    if ((int) $1) {
		$$ = $3;
	    } else {
		$$ = $5;
	    }
	}
	| expr GT expr {
	    $$ = $1 > $3;
	}
	| expr LT expr {
	    $$ = $1 < $3;
	}
	| expr LE expr {
	    $$ = $1 <= $3;
	}
	| expr GE expr {
	    $$ = $1 >= $3;
	}
	| expr EQ expr {
	    $$ = $1 == $3;
	}
	| expr NE expr {
	    $$ = $1 != $3;
	}
	| expr AND expr {
	    $$ = $1 && $3;
	}
	| expr OR expr {
	    $$ = $1 || $3;
	}
	| NOT expr {
	    $$ = !($2);
	}
	| '(' expr ')' {
	    $$ = $2;
	}
	| '-' expr %prec UMINUS {
	    $$ = -$2;
	}
	;

windowtype:
	NONE {$$=0;}
	| TRIANGULAR {$$=1;}
	| HANNING {$$=2;}
	| WELCH {$$=3;}
	| HAMMING {$$=4;}
	| BLACKMAN {$$=5;}
	| PARZEN {$$=6;}
	;
	
fourierloadx:
	INDEX {$$=0;}
	| FREQUENCY {$$=1;}
	| PERIOD {$$=2;}
	;

fourierloady:
	MAGNITUDE {$$=0;}
	| PHASE {$$=1;}
	| COEFFICIENTS {$$=2;}
	;

fourierdata:
	REAL {$$=0;}
	| COMPLEX {$$=1;}
	;

%%

/* list of intrinsic functions and keywords */
symtab_entry ikey[] = {
	{"A", VAR, NULL},
	{"A0", FITPARM, NULL},
	{"A0MAX", FITPMAX, NULL},
	{"A0MIN", FITPMIN, NULL},
	{"A1", FITPARM, NULL},
	{"A1MAX", FITPMAX, NULL},
	{"A1MIN", FITPMIN, NULL},
	{"A2", FITPARM, NULL},
	{"A2MAX", FITPMAX, NULL},
	{"A2MIN", FITPMIN, NULL},
	{"A3", FITPARM, NULL},
	{"A3MAX", FITPMAX, NULL},
	{"A3MIN", FITPMIN, NULL},
	{"A4", FITPARM, NULL},
	{"A4MAX", FITPMAX, NULL},
	{"A4MIN", FITPMIN, NULL},
	{"A5", FITPARM, NULL},
	{"A5MAX", FITPMAX, NULL},
	{"A5MIN", FITPMIN, NULL},
	{"A6", FITPARM, NULL},
	{"A6MAX", FITPMAX, NULL},
	{"A6MIN", FITPMIN, NULL},
	{"A7", FITPARM, NULL},
	{"A7MAX", FITPMAX, NULL},
	{"A7MIN", FITPMIN, NULL},
	{"A8", FITPARM, NULL},
	{"A8MAX", FITPMAX, NULL},
	{"A8MIN", FITPMIN, NULL},
	{"A9", FITPARM, NULL},
	{"A9MAX", FITPMAX, NULL},
	{"A9MIN", FITPMIN, NULL},
	{"ABOVE", ABOVE, NULL},
	{"ABS", FUNC_D, fabs},
	{"ABSOLUTE", ABSOLUTE, NULL},
	{"ACOS", FUNC_D, acos},
	{"ACOSH", FUNC_D, acosh},
	{"ACTIVATE", ACTIVATE, NULL},
	{"ACTIVE", ACTIVE, NULL},
	{"AI", FUNC_D, ai_wrap},
	{"ALIAS", ALIAS, NULL},
	{"ALT", ALT, NULL},
	{"ALTERNATE", ALTERNATE, NULL},
	{"ALTXAXIS", ALTXAXIS, NULL},
	{"ALTYAXIS", ALTYAXIS, NULL},
	{"AND", AND, NULL},
	{"ANGLE", ANGLE, NULL},
	{"APPEND", APPEND, NULL},
	{"AREA", AREA, NULL},
	{"ARRANGE", ARRANGE, NULL},
	{"ARROW", ARROW, NULL},
	{"ASCENDING", ASCENDING, NULL},
	{"ASIN", FUNC_D, asin},
	{"ASINH", FUNC_D, asinh},
	{"ASPLINE", ASPLINE, NULL},
	{"ATAN", FUNC_D, atan},
	{"ATAN2", FUNC_DD, atan2},
	{"ATANH", FUNC_D, atanh},
	{"AUTO", AUTO, NULL},
	{"AUTOSCALE", AUTOSCALE, NULL},
	{"AUTOTICKS", AUTOTICKS, NULL},
	{"AVG", AVG, NULL},
	{"AXES", AXES, NULL},
	{"AXIS", AXIS, NULL},
	{"B", VAR, NULL},
	{"BACKBUFFER", BACKBUFFER, NULL},
	{"BACKGROUND", BACKGROUND, NULL},
	{"BAR", BAR, NULL},
	{"BATCH", BATCH, NULL},
        {"BEGIN", BEGIN, NULL},
	{"BELOW", BELOW, NULL},
	{"BETA", FUNC_DD, beta},
	{"BETWEEN", BETWEEN, NULL},
	{"BI", FUNC_D, bi_wrap},
	{"BIN", BIN, NULL},
	{"BLACKMAN", BLACKMAN, NULL},
	{"BLOCK", BLOCK, NULL},
	{"BOTH", BOTH, NULL},
	{"BOTTOM", BOTTOM, NULL},
	{"BOX", BOX, NULL},
	{"C", VAR, NULL},
	{"CD", CD, NULL},
	{"CEIL", FUNC_D, ceil},
	{"CENTER", CENTER, NULL},
	{"CHAR", CHAR, NULL},
	{"CHDTR", FUNC_DD, chdtr},
	{"CHDTRC", FUNC_DD, chdtrc},
	{"CHDTRI", FUNC_DD, chdtri},
	{"CHI", FUNC_D, chi_wrap},
	{"CHRSTR", CHRSTR, NULL},
	{"CI", FUNC_D, ci_wrap},
	{"CLEAR", CLEAR, NULL},
	{"CLICK", CLICK, NULL},
	{"CMAP", CMAP, NULL},
	{"COEFFICIENTS", COEFFICIENTS, NULL},
	{"COLOR", COLOR, NULL},
	{"COMMENT", COMMENT, NULL},
	{"COMPLEX", COMPLEX, NULL},
	{"CONSTRAINTS", CONSTRAINTS, NULL},
	{"COPY", COPY, NULL},
	{"COS", FUNC_D, cos},
	{"COSH", FUNC_D, cosh},
	{"CYCLE", CYCLE, NULL},
	{"D", VAR, NULL},
	{"DAWSN", FUNC_D, dawsn},
	{"DAYMONTH", DAYMONTH, NULL},
	{"DAYOFWEEKL", DAYOFWEEKL, NULL},
	{"DAYOFWEEKS", DAYOFWEEKS, NULL},
	{"DAYOFYEAR", DAYOFYEAR, NULL},
	{"DDMMYY", DDMMYY, NULL},
	{"DEACTIVATE", DEACTIVATE, NULL},
	{"DECIMAL", DECIMAL, NULL},
	{"DEF", DEF, NULL},
	{"DEFAULT", DEFAULT, NULL},
	{"DEG", DEG, NULL},
	{"DEGREESLAT", DEGREESLAT, NULL},
	{"DEGREESLON", DEGREESLON, NULL},
	{"DEGREESMMLAT", DEGREESMMLAT, NULL},
	{"DEGREESMMLON", DEGREESMMLON, NULL},
	{"DEGREESMMSSLAT", DEGREESMMSSLAT, NULL},
	{"DEGREESMMSSLON", DEGREESMMSSLON, NULL},
	{"DELETE", DELETE, NULL},
	{"DESCENDING", DESCENDING, NULL},
	{"DESCRIPTION", DESCRIPTION, NULL},
	{"DEVICE", DEVICE, NULL},
	{"DFT", DFT, NULL},
	{"DIFF", DIFFERENCE, NULL},
	{"DIFFERENCE", DIFFERENCE, NULL},
	{"DISK", DISK, NULL},
	{"DOUBLEBUFFER", DOUBLEBUFFER, NULL},
	{"DOWN", DOWN, NULL},
	{"DRAW2", DRAW2, NULL},
	{"DROP", DROP, NULL},
	{"DX", DX, NULL},
	{"DXDX", DXDX, NULL},
	{"DY", DY, NULL},
	{"DYDY", DYDY, NULL},
	{"ECHO", ECHO, NULL},
	{"EDIT", EDIT, NULL},
	{"ELLIE", FUNC_DD, ellie},
	{"ELLIK", FUNC_DD, ellik},
	{"ELLIPSE", ELLIPSE, NULL},
	{"ELLPE", FUNC_D, ellpe},
	{"ELLPK", FUNC_D, ellpk},
	{"EQ", EQ, NULL},
	{"ER", ERRORBAR, NULL},
	{"ERF", FUNC_D, erf},
	{"ERFC", FUNC_D, erfc},
	{"ERRORBAR", ERRORBAR, NULL},
	{"EXIT", EXIT, NULL},
	{"EXP", FUNC_D, exp},
	{"EXPN", FUNC_ND, expn},
	{"EXPONENTIAL", EXPONENTIAL, NULL},
	{"FAC", FUNC_I, fac},
	{"FALSE", FALSEP, NULL},
	{"FDTR", FUNC_NND, fdtr},
	{"FDTRC", FUNC_NND, fdtrc},
	{"FDTRI", FUNC_NND, fdtri},
	{"FFT", FFT, NULL},
	{"FILE", FILEP, NULL},
	{"FILL", FILL, NULL},
	{"FIND", FIND, NULL},
	{"FIT", FIT, NULL},
	{"FIXED", FIXED, NULL},
	{"FIXEDPOINT", FIXEDPOINT, NULL},
	{"FLOOR", FUNC_D, floor},
	{"FLUSH", FLUSH, NULL},
	{"FOCUS", FOCUS, NULL},
	{"FOLLOWS", FOLLOWS, NULL},
	{"FONT", FONTP, NULL},
	{"FORCE", FORCE, NULL},
	{"FOREGROUND", FOREGROUND, NULL},
	{"FORMAT", FORMAT, NULL},
	{"FORMULA", FORMULA, NULL},
	{"FRAME", FRAMEP, NULL},
	{"FREE", FREE, NULL},
	{"FREQUENCY", FREQUENCY, NULL},
	{"FRESNLC", FUNC_D, fresnlc_wrap},
	{"FRESNLS", FUNC_D, fresnls_wrap},
	{"FROM", FROM, NULL},
	{"FRONTBUFFER", FRONTBUFFER, NULL},
	{"F_OF_D", PROC_FUNC_D, NULL},
	{"F_OF_DD", PROC_FUNC_DD, NULL},
        {"F_OF_I", PROC_FUNC_I, NULL},
	{"F_OF_ND", PROC_FUNC_ND, NULL},
	{"F_OF_NN", PROC_FUNC_NN, NULL},
	{"F_OF_NND", PROC_FUNC_NND, NULL},
	{"F_OF_PPD", PROC_FUNC_PPD, NULL},
	{"F_OF_PPPD", PROC_FUNC_PPPV, NULL},
	{"GAMMA", FUNC_D, true_gamma},
	{"GDTR", FUNC_PPD, gdtr},
	{"GDTRC", FUNC_PPD, gdtrc},
	{"GE", GE, NULL},
	{"GENERAL", GENERAL, NULL},
	{"GETP", GETP, NULL},
	{"GRAPH", GRAPH, NULL},
	{"GRAPHS", GRAPHS, NULL},
	{"GRAPHTYPE", GRAPHTYPE, NULL},
	{"GRID", GRID, NULL},
	{"GT", GT, NULL},
	{"HAMMING", HAMMING, NULL},
	{"HANNING", HANNING, NULL},
	{"HARDCOPY", HARDCOPY, NULL},
	{"HBAR", HBAR, NULL},
	{"HGAP", HGAP, NULL},
	{"HIDDEN", HIDDEN, NULL},
	{"HISTO", HISTO, NULL},
	{"HMS", HMS, NULL},
	{"HORIZONTAL", HORIZONTAL, NULL},
	{"HPGLL", HPGLL, NULL},
	{"HPGLP", HPGLP, NULL},
	{"HYP2F1", FUNC_PPPV, hyp2f1},
	{"HYPERG", FUNC_PPD, hyperg},
	{"HYPOT", FUNC_DD, hypot},
	{"I0E", FUNC_D, i0e},
	{"I1E", FUNC_D, i1e},
	{"IGAM", FUNC_DD, igam},
	{"IGAMC", FUNC_DD, igamc},
	{"IGAMI", FUNC_DD, igami},
	{"IGNORE", IGNORE, NULL},
	{"IMAGE", IMAGE, NULL},
	{"IN", IN, NULL},
	{"INCBET", FUNC_PPD, incbet},
	{"INCBI", FUNC_PPD, incbi},
        {"INCREMENT", INCREMENT, NULL},
	{"INDEX", INDEX, NULL},
	{"INIT", INIT, NULL},
	{"INOUT", INOUT, NULL},
	{"INTEGRATE", INTEGRATE, NULL},
	{"INTERP", INTERP, NULL},
	{"INVDFT", INVDFT, NULL},
	{"INVFFT", INVFFT, NULL},
	{"IRAND", IRAND, NULL},
	{"IV", FUNC_DD, iv_wrap},
	{"JDAY", JDAY, NULL},
	{"JDAY0", JDAY0, NULL},
	{"JUST", JUST, NULL},
	{"JV", FUNC_DD, jv_wrap},
	{"K0E", FUNC_D, k0e},
	{"K1E", FUNC_D, k1e},
	{"KILL", KILL, NULL},
	{"KN", FUNC_ND, kn_wrap},
	{"LABEL", LABEL, NULL},
	{"LANDSCAPE", LANDSCAPE, NULL},
	{"LAYOUT", LAYOUT, NULL},
	{"LBETA", FUNC_DD, lbeta},
	{"LE", LE, NULL},
	{"LEAVE", LEAVE, NULL},
	{"LEFT", LEFT, NULL},
	{"LEGEND", LEGEND, NULL},
	{"LENGTH", LENGTH, NULL},
	{"LEVEL", LEVEL, NULL},
	{"LEVELS", LEVELS, NULL},
	{"LGAMMA", FUNC_D, lgamma},
	{"LINE", LINE, NULL},
	{"LINESTYLE", LINESTYLE, NULL},
	{"LINETO", LINETO, NULL},
	{"LINEWIDTH", LINEWIDTH, NULL},
	{"LINK", LINK, NULL},
	{"LN", FUNC_D, log},
	{"LOAD", LOAD, NULL},
	{"LOCATOR", LOCATOR, NULL},
	{"LOCATORBAR", LOCATORBAR, NULL},
	{"LOCTYPE", LOCTYPE, NULL},
	{"LOG", LOG, NULL},
	{"LOG10", FUNC_D, log10},
	{"LOG2", FUNC_D, log2},
	{"LOGX", LOGX, NULL},
	{"LOGXY", LOGXY, NULL},
	{"LOGY", LOGY, NULL},
	{"LT", LT, NULL},
	{"MAGNITUDE", MAGNITUDE, NULL},
	{"MAJOR", MAJOR, NULL},
	{"MAX", MAXP, NULL},
	{"MIFL", MIFL, NULL},
	{"MIFP", MIFP, NULL},
	{"MIN", MINP, NULL},
	{"MINOR", MINOR, NULL},
	{"MMDD", MMDD, NULL},
	{"MMDDHMS", MMDDHMS, NULL},
	{"MMDDYY", MMDDYY, NULL},
	{"MMDDYYHMS", MMDDYYHMS, NULL},
	{"MMSSLAT", MMSSLAT, NULL},
	{"MMSSLON", MMSSLON, NULL},
	{"MMYY", MMYY, NULL},
	{"MOD", FUNC_DD, fmod},
	{"MONTHDAY", MONTHDAY, NULL},
	{"MONTHL", MONTHL, NULL},
	{"MONTHS", MONTHS, NULL},
	{"MONTHSY", MONTHSY, NULL},
	{"MOVE", MOVE, NULL},
	{"MOVE2", MOVE2, NULL},
	{"NDTR", FUNC_D, ndtr},
	{"NDTRI", FUNC_D, ndtri},
	{"NE", NE, NULL},
	{"NEGATE", NEGATE, NULL},
	{"NONE", NONE, NULL},
	{"NONLFIT", NONLFIT, NULL},
	{"NORM", FUNC_D, fx},
	{"NORMAL", NORMAL, NULL},
	{"NOT", NOT, NULL},
	{"NUMBER", NUMBER, NULL},
	{"NXY", NXY, NULL},
	{"OFF", OFF, NULL},
	{"OFFSETX", OFFSETX, NULL},
	{"OFFSETY", OFFSETY, NULL},
	{"ON", ON, NULL},
	{"OP", OP, NULL},
	{"OR", OR, NULL},
	{"OUT", OUT, NULL},
	{"PAGE", PAGE, NULL},
	{"PARA", PARA, NULL},
	{"PARAMETERS", PARAMETERS, NULL},
	{"PARAMS", PARAMS, NULL},
	{"PARZEN", PARZEN, NULL},
	{"PATTERN", PATTERN, NULL},
	{"PDTR", FUNC_ND, pdtr},
	{"PDTRC", FUNC_ND, pdtrc},
	{"PDTRI", FUNC_ND, pdtri},
	{"PERIMETER", PERIMETER, NULL},
	{"PERIOD", PERIOD, NULL},
	{"PERP", PERP, NULL},
	{"PHASE", PHASE, NULL},
	{"PI", PI_TOK, NULL},
	{"PIPE", PIPE, NULL},
	{"PLACE", PLACE, NULL},
	{"POINT", POINT, NULL},
	{"POLAR", POLAR, NULL},
	{"POLY", POLY, NULL},
	{"POLYI", POLYI, NULL},
	{"POLYO", POLYO, NULL},
	{"POP", POP, NULL},
	{"PORTRAIT", PORTRAIT, NULL},
	{"POWER", POWER, NULL},
	{"PREC", PREC, NULL},
	{"PREPEND", PREPEND, NULL},
	{"PRINT", PRINT, NULL},
	{"PS", PS, NULL},
	{"PSI", FUNC_D, psi},
	{"PSMONOL", PSMONOL, NULL},
	{"PSMONOP", PSMONOP, NULL},
	{"PUSH", PUSH, NULL},
	{"PUTP", PUTP, NULL},
	{"RAD", RAD, NULL},
	{"RAND", RAND, NULL},
	{"RAWSPICE", RAWSPICE, NULL},
	{"REACTIVATE", REACTIVATE, NULL},
	{"READ", READ, NULL},
	{"REAL", REAL, NULL},
	{"REDRAW", REDRAW, NULL},
	{"REGRESS", REGRESS, NULL},
	{"RENDER", RENDER, NULL},
	{"RGAMMA", FUNC_D, rgamma},
	{"RIGHT", RIGHT, NULL},
	{"RINT", FUNC_D, rint},
	{"RISER", RISER, NULL},
	{"RNORM", FUNC_DD, rnorm},
	{"ROT", ROT, NULL},
	{"RUNAVG", RUNAVG, NULL},
	{"RUNMAX", RUNMAX, NULL},
	{"RUNMED", RUNMED, NULL},
	{"RUNMIN", RUNMIN, NULL},
	{"RUNSTD", RUNSTD, NULL},
	{"SAVE", SAVE, NULL},
	{"SAVEALL", SAVEALL, NULL},
	{"SCALE", SCALE, NULL},
	{"SD", SD, NULL},
	{"SET", SET, NULL},
	{"SETS", SETS, NULL},
	{"SFORMAT", SFORMAT, NULL},
	{"SHI", FUNC_D, shi_wrap},
	{"SI", FUNC_D, si_wrap},
	{"SIGN", SIGN, NULL},
	{"SIN", FUNC_D, sin},
	{"SINH", FUNC_D, sinh},
	{"SIZE", SIZE, NULL},
	{"SKIP", SKIP, NULL},
	{"SLEEP", SLEEP, NULL},
	{"SLICE", SLICE, NULL},
	{"SMITH", SMITH, NULL},
	{"SORT", SORT, NULL},
	{"SOURCE", SOURCE, NULL},
	{"SPEC", SPEC, NULL},
	{"SPECIFIED", SPECIFIED, NULL},
	{"SPENCE", FUNC_D, spence},
	{"SPLINE", SPLINE, NULL},
	{"SQR", FUNC_D, sqr_wrap},
	{"SQRT", FUNC_D, sqrt},
	{"STACK", STACK, NULL},
	{"STACKEDBAR", STACKEDBAR, NULL},
	{"STACKEDHBAR", STACKEDHBAR, NULL},
	{"STAGGER", STAGGER, NULL},
	{"START", START, NULL},
	{"STARTTYPE", STARTTYPE, NULL},
	{"STATUS", STATUS, NULL},
	{"STATUSBAR", STATUSBAR, NULL},
	{"STDTR", FUNC_ND, stdtr},
	{"STDTRI", FUNC_ND, stdtri},
	{"STOP", STOP, NULL},
	{"STRING", STRING, NULL},
	{"STRUVE", FUNC_DD, struve},
	{"SUBTITLE", SUBTITLE, NULL},
	{"SWAPBUFFER", SWAPBUFFER, NULL},
	{"SYMBOL", SYMBOL, NULL},
	{"TAN", FUNC_D, tan},
	{"TANH", FUNC_D, tanh},
	{"TARGET", TARGET, NULL},
	{"TICK", TICKP, NULL},
	{"TICKLABEL", TICKLABEL, NULL},
	{"TICKMARKS", TICKMARKS, NULL},
	{"TICKS", TICKS, NULL},
	{"TIMESTAMP", TIMESTAMP, NULL},
	{"TITLE", TITLE, NULL},
	{"TO", TO, NULL},
	{"TOOLBAR", TOOLBAR, NULL},
	{"TOP", TOP, NULL},
	{"TRIANGULAR", TRIANGULAR, NULL},
	{"TRUE", TRUEP, NULL},
	{"TYPE", TYPE, NULL},
	{"UP", UP, NULL},
	{"USE", USE, NULL},
	{"VAR", VAR, NULL},
	{"VELOCITY", VELOCITY, NULL},
	{"VERSION", VERSION, NULL},
	{"VERTICAL", VERTICAL, NULL},
	{"VGAP", VGAP, NULL},
	{"VIEW", VIEW, NULL},
	{"VX1", VX1, NULL},
	{"VX2", VX2, NULL},
	{"VY1", VY1, NULL},
	{"VY2", VY2, NULL},
	{"WELCH", WELCH, NULL},
	{"WITH", WITH, NULL},
	{"WORLD", WORLD, NULL},
	{"WRITE", WRITE, NULL},
	{"WX1", WX1, NULL},
	{"WX2", WX2, NULL},
	{"WY1", WY1, NULL},
	{"WY2", WY2, NULL},
	{"X", X_TOK, NULL},
	{"X0", X0, NULL},
	{"X1", X1, NULL},
	{"XAXES", XAXES, NULL},
	{"XAXIS", XAXIS, NULL},
	{"XCOR", XCOR, NULL},
	{"XMAX", XMAX, NULL},
	{"XMIN", XMIN, NULL},
	{"XY", XY, NULL},
	{"XYBOX", XYBOX, NULL},
	{"XYBOXPLOT", XYBOXPLOT, NULL},
	{"XYDX", XYDX, NULL},
	{"XYDXDX", XYDXDX, NULL},
	{"XYDXDY", XYDXDY, NULL},
	{"XYDY", XYDY, NULL},
	{"XYDYDY", XYDYDY, NULL},
	{"XYHILO", XYHILO, NULL},
	{"XYRT", XYRT, NULL},
	{"XYSTRING", XYSTRING, NULL},
	{"XYUV", XYUV, NULL},
	{"XYXX", XYXX, NULL},
	{"XYYY", XYYY, NULL},
	{"XYZ", XYZ, NULL},
	{"XYZW", XYZW, NULL},
	{"Y", Y_TOK, NULL},
	{"Y0", Y0, NULL},
	{"Y1", Y1, NULL},
	{"Y2", Y2, NULL},
	{"Y3", Y3, NULL},
	{"Y4", Y4, NULL},
	{"YAXES", YAXES, NULL},
	{"YAXIS", YAXIS, NULL},
	{"YMAX", YMAX, NULL},
	{"YMIN", YMIN, NULL},
	{"YV", FUNC_DD, yv_wrap},
	{"YYMMDD", YYMMDD, NULL},
	{"YYMMDDHMS", YYMMDDHMS, NULL},
	{"ZEROXAXIS", ZEROXAXIS, NULL},
	{"ZEROYAXIS", ZEROYAXIS, NULL},
	{"ZETA", FUNC_DD, zeta},
	{"ZETAC", FUNC_D, zetac}
};

static int maxfunc = sizeof(ikey) / sizeof(symtab_entry);


void scanner(char *s, double *x, double *y, int len, double *a, double *b, double *c, double *d, int lenscr, int i, int setno, int *errpos)
{
    char *seekpos;
    int vl;
    
    strcpy(f_string, s);
    
    seekpos=f_string;
    while ((seekpos - f_string < MAX_STRING_LENGTH - 1) && (*seekpos == ' ' || *seekpos == '\t')) {
        seekpos++;
    }
    if (*seekpos == '\n' || *seekpos == '#') {
        return;
    }
    
    lowtoupper(f_string);
    
    vl = strlen(f_string);
    if (f_string[vl - 1] != '\n') {
        f_string[vl] = '\n';
        f_string[vl + 1] = 0;
    } else {
        f_string[vl] = 0;
    }
    
    interr = 0;
    whichgraph = cg;
    whichset = setno;
    pos = 0;
    aa = a;
    bb = b;
    cc = c;
    dd = d;
    xx = x;
    yy = y;
    lxy = len;
    ls = lenscr;
    setindex = i + 1;
    curset = setsetno = setno;

    fcnt = 0;
    log_results(s);
    yyparse();
    *errpos = interr;
    for (i = 0; i < fcnt; i++) {
	free(freelist[i]);
	freelist[i] = NULL;
    }
    
    if (gotparams) {
	gotparams = FALSE;
        getparms(paramfile);
    }
    
    if (gotread) {
	gotread = FALSE;
        getdata(cg, readfile, readsrc, readtype);
    }
    
    if (gotnlfit) {
	gotnlfit = FALSE;
        do_nonlfit(nlfit_gno, nlfit_setno, nlfit_nsteps);
    }
}


int findf(symtab_entry *keytable, char *s)
{

    int low, high, mid;

    low = 0;
    high = maxfunc - 1;
    while (low <= high) {
	mid = (low + high) / 2;
	if (strcmp(s, keytable[mid].s) < 0) {
	    high = mid - 1;
	} else {
	    if (strcmp(s, keytable[mid].s) > 0) {
		low = mid + 1;
	    } else {
		return (mid);
	    }
	}
    }
    return (-1);
}

int compare_keys (const void *a, const void *b)
{
  return (int) strcmp (((symtab_entry*)a)->s, ((symtab_entry*)b)->s);
}

/* add new entry to the symbol table */
int addto_symtab(symtab_entry newkey)
{
    int position;
    if ((position = findf(key, newkey.s)) < 0) {
        if ((key = (symtab_entry *) realloc(key, (maxfunc + 1)*sizeof(symtab_entry))) != NULL) {
	    maxfunc++;
	    key[maxfunc - 1].type = newkey.type;
	    key[maxfunc - 1].fnc = newkey.fnc;
	    key[maxfunc - 1].s = malloc(strlen(newkey.s) + 1);
	    strcpy(key[maxfunc - 1].s, newkey.s);
	    qsort(key, maxfunc, sizeof(symtab_entry), compare_keys);
	    return 0;
	} else {
	    errmsg ("Memory allocation failed in addto_symtab()!");
	    return -2;
	}
    } else if (alias_force == TRUE) { /* already exists but alias_force enabled */
        key[position].type = newkey.type;
	key[position].fnc = newkey.fnc;
	return 0;
    } else {
        return -1;
    }
}

/* initialize symbol table */
void init_symtab(void)
{
    int i;
    
    if ((key = (symtab_entry *) malloc(maxfunc*sizeof(symtab_entry))) != NULL) {
    	memcpy (key, ikey, maxfunc*sizeof(symtab_entry));
	for (i = 0; i < maxfunc; i++) {
	    key[i].s = malloc(strlen(ikey[i].s) + 1);
	    strcpy(key[i].s, ikey[i].s);
	}
	qsort(key, maxfunc, sizeof(symtab_entry), compare_keys);
	return;
    } else {
        errmsg ("Memory allocation failed in init_symtab()!");
	key = ikey;
	return;
    }
}

int getcharstr(void)
{
    if (pos >= strlen(f_string))
	 return EOF;
    return (f_string[pos++]);
}

void ungetchstr(void)
{
    if (pos > 0)
	pos--;
}

int yylex(void)
{
    int c, i;
    int found;
    static char s[MAX_STRING_LENGTH];
    char sbuf[MAX_STRING_LENGTH];
    char *str;

    while ((c = getcharstr()) == ' ' || c == '\t');
    if (c == EOF) {
	return (0);
    }
    if (c == '"') {
	i = 0;
	while ((c = getcharstr()) != '"' && c != EOF) {
	    if (c == '\\') {
		int ctmp;
		ctmp = getcharstr();
		if (ctmp != '"') {
		    ungetchstr();
		}
		else {
		    c = ctmp;
		}
	    }
	    s[i] = c;
	    i++;
	}
	if (c == EOF) {
	    yyerror("Nonterminating string");
	    return 0;
	}
	s[i] = '\0';
	str = (char *) malloc(strlen(s) + 1);
	strcpy(str, s);
	yylval.str = str;
	return CHRSTR;
    }
    if (c == '.' || isdigit(c)) {
	char stmp[80];
	double d;
	int i, gotdot = 0;

	i = 0;
	while (c == '.' || isdigit(c)) {
	    if (c == '.') {
		if (gotdot) {
		    yyerror("Reading number, too many dots");
	    	    return 0;
		} else {
		    gotdot = 1;
		}
	    }
	    stmp[i++] = c;
	    c = getcharstr();
	}
	if (c == 'E' || c == 'e') {
	    stmp[i++] = c;
	    c = getcharstr();
	    if (c == '+' || c == '-') {
		stmp[i++] = c;
		c = getcharstr();
	    }
	    while (isdigit(c)) {
		stmp[i++] = c;
		c = getcharstr();
	    }
	}
	if (gotdot && i == 1) {
	    ungetchstr();
	    return '.';
	}
	stmp[i] = '\0';
	ungetchstr();
	sscanf(stmp, "%lf", &d);
	yylval.val = d;
	return NUMBER;
    }
/* graphs, sets, regions resp. */
    if (c == 'G' || c == 'S' || c == 'R') {
	char stmp[80];
	int i = 0, ctmp = c, gn, sn, rn;
	c = getcharstr();
	while (isdigit(c)) {
	    stmp[i++] = c;
	    c = getcharstr();
	}
	if (i == 0) {
	    c = ctmp;
	    ungetchstr();
	} else {
	    ungetchstr();
	    if (ctmp == 'G') {
	        stmp[i] = '\0';
		gn = atoi(stmp);
		if (gn >= 0 && gn < maxgraph) {
		    yylval.ival = gn;
		    whichgraph = gn;
		    return GRAPHNO;
		}
	    } else if (ctmp == 'S') {
	        stmp[i] = '\0';
		sn = atoi(stmp);
		if (sn >= 0 && sn < g[whichgraph].maxplot) {
		    lxy = getsetlength(whichgraph, sn);
		    yylval.ival = sn;
		    whichset = sn;
		    return SETNUM;
		}
	    } else if (ctmp == 'R') {
	        stmp[i] = '\0';
		rn = atoi(stmp);
		if (rn >= 0 && rn < MAXREGION) {
		    yylval.ival = rn;
		    return REGNUM;
		}
	    }
	}
    }
    if (isalpha(c)) {
	char *p = sbuf;

	do {
	    *p++ = c;
	} while ((c = getcharstr()) != EOF && (isalpha(c) || isdigit(c) || c == '_'));
	ungetchstr();
	*p = '\0';
        if (debuglevel == 2) {
	    printf("->%s<-\n", sbuf);
	}
	found = -1;
	if ((found = findf(key, sbuf)) >= 0) {
	    if (key[found].type == VAR) {
		switch (sbuf[0]) {
		case 'A':
		    yylval.ptr = aa;
		    return VAR;
		case 'B':
		    yylval.ptr = bb;
		    return VAR;
		case 'C':
		    yylval.ptr = cc;
		    return VAR;
		case 'D':
		    yylval.ptr = dd;
		    return VAR;
		}
	    }
	    else if (key[found].type == FITPARM) {
		int index = sbuf[1] - '0';
		yylval.val = index;
		return FITPARM;
	    }
	    else if (key[found].type == FITPMAX) {
		int index = sbuf[1] - '0';
		yylval.val = index;
		return FITPMAX;
	    }
	    else if (key[found].type == FITPMIN) {
		int index = sbuf[1] - '0';
		yylval.val = index;
		return FITPMIN;
	    }
	    else if (key[found].type == FUNC_I) {
		yylval.func = found;
		return FUNC_I;
	    }
	    else if (key[found].type == FUNC_D) {
		yylval.func = found;
		return FUNC_D;
	    }
	    else if (key[found].type == FUNC_ND) {
		yylval.func = found;
		return FUNC_ND;
	    }
	    else if (key[found].type == FUNC_DD) {
		yylval.func = found;
		return FUNC_DD;
	    }
	    else if (key[found].type == FUNC_NND) {
		yylval.func = found;
		return FUNC_NND;
	    }
	    else if (key[found].type == FUNC_PPD) {
		yylval.func = found;
		return FUNC_PPD;
	    }
	    else if (key[found].type == FUNC_PPPV) {
		yylval.func = found;
		return FUNC_PPPV;
	    }
	    else { /* set up special cases */
		switch (key[found].type) {
		case XAXIS:
		    naxis = 0;
		    break;
		case YAXIS:
		    naxis = 1;
		    break;
		case ZEROXAXIS:
		    naxis = 2;
		    break;
		case ZEROYAXIS:
		    naxis = 3;
		    break;
		case ALTXAXIS:
		    naxis = 4;
		    break;
		case ALTYAXIS:
		    naxis = 5;
		    break;
		case AXES:
		    naxis = 6;
		    break;
		case XAXES:
		    naxis = 7;
		    break;
		case YAXES:
		    naxis = 8;
		    break;
		case GRAPHS:
		    yylval.ival = -1;
		    whichgraph = -1;
		    return GRAPHS;
		    break;
		case SETS:
		    yylval.ival = -1;
		    whichset = -1;
		    return SETS;
		    break;
		default:
		    break;
		}
	    }
	    yylval.func = key[found].type;
	    return key[found].type;
	} else {
	    strcat(sbuf, ": No such function or variable");
	    yyerror(sbuf);
	    return 0;
	}
    }
    switch (c) {
    case '>':
	return follow('=', GE, GT);
    case '<':
	return follow('=', LE, LT);
    case '=':
	return follow('=', EQ, '=');
    case '!':
	return follow('=', NE, NOT);
    case '|':
	return follow('|', OR, '|');
    case '&':
	return follow('&', AND, '&');
    case '\n':
	return '\n';
    default:
	return c;
    }
}

int follow(int expect, int ifyes, int ifno)
{
    int c = getcharstr();

    if (c == expect) {
	return ifyes;
    }
    ungetchstr();
    return ifno;
}

void yyerror(char *s)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%s: %s", s, f_string);
    i = strlen(buf);
    buf[i - 1] = 0;
    errmsg(buf);
    interr = 1;
}

#define C1 0.1978977093962766
#define C2 0.1352915131768107

double rnorm(double mean, double sdev)
{
    double u = drand48();

    return mean + sdev * (pow(u, C2) - pow(1.0 - u, C2)) / C1;
}

double fx(double x)
{
    return 1.0 / sqrt(2.0 * M_PI) * exp(-x * x * 0.5);
}


/* TODO: the whole set_prop stuff to be removed! */
#include <stdarg.h>

void set_prop(int gno,...)
{
    va_list var;
    int prop, allsets = 0;
    int i, j, startg, endg, starts = 0, ends = 0;
    double dprop;
    double dprop1, dprop2;
    char *cprop;
    char buf[256];

    if (gno == -1) {
	startg = 0;
	endg = maxgraph - 1;
    } else {
	startg = endg = gno;
    }

    va_start(var, gno);
    while ((prop = va_arg(var, int)) != 0) {
	switch (prop) {
	case SETS:
	    allsets = 1;
	    starts = 0;
	    ends = maxplot - 1;
	    break;
	case SET:
	    switch (prop = va_arg(var, int)) {
	    case SETNUM:
		prop = va_arg(var, int);
		if (prop == -1) {
		    allsets = 1;
		    starts = 0;
		    ends = maxplot - 1;
		} else {
		    allsets = 0;
		    starts = ends = prop;
		}
		break;
	    }
	    break;
	case ACTIVE:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    if (prop == ON) {	/* could have been ignored */
			if (g[i].p[j].deact && (g[i].p[j].ex[0] != NULL)) {
			    g[i].p[j].deact = FALSE;
			    g[i].p[j].active = TRUE;
			}
		    } else if (prop == FALSE) {
			g[i].p[j].active = FALSE;
		    } else if (prop == IGNORE) {
			g[i].p[j].active = FALSE;
			g[i].p[j].deact = TRUE;
		    }
		}
	    }
	    break;
	case TYPE:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].type = prop;
		}
	    }
	    break;
	case FONTP:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].font = prop;
		}
	    }
	    break;
	case PREC:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].prec = prop;
		}
	    }
	    break;
	case FORMAT:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].format = prop;
		}
	    }
	    break;
	case LINEWIDTH:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].linew = checkon(LINEWIDTH, g[i].p[j].linew, prop);
		    if (check_err) {
			return;
		    }
		}
	    }
	    break;
	case LINESTYLE:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].lines = checkon(LINESTYLE, g[i].p[j].lines, prop);
		    if (check_err) {
			return;
		    }
		}
	    }
	    break;
	case COLOR:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].color = prop;
		}
	    }
	    break;
	case XYZ:
	    dprop1 = va_arg(var, double);
	    dprop2 = va_arg(var, double);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].zmin = dprop1;
		    g[i].p[j].zmax = dprop2;
		}
	    }
	    break;
	case COMMENT:
	    cprop = va_arg(var, char *);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    strcpy(g[i].p[j].comments, cprop);
		}
	    }
	    break;
	case FILL:
	    switch (prop = va_arg(var, int)) {
	    case TYPE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].fill = prop;
		    }
		}
		break;
	    case WITH:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].fillusing = prop;
		    }
		}
		break;
	    case CLRFILLED:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].fillcolor = prop;
		    }
		}
		break;
	    case PTNFILLED:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].fillpattern = prop;
		    }
		}
		break;
	    default:
		sprintf(buf, "Attribute not found in setprops()-FILL, # = %d", prop);
		errmsg(buf);
		break;
	    }
	    break;
	case SKIP:
	    prop = va_arg(var, int);
	    for (i = startg; i <= endg; i++) {
		if (allsets) {
		    ends = g[i].maxplot - 1;
		}
		for (j = starts; j <= ends; j++) {
		    g[i].p[j].symskip = prop;
		}
	    }
	    break;
	case SYMBOL:
	    switch (prop = va_arg(var, int)) {
	    case TYPE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].sym = prop;
		    }
		}
		break;
	    case FILL:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symfill = prop;
		    }
		}
		break;
	    case CENTER:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symdot = prop;
		    }
		}
		break;
	    case SIZE:
		dprop = va_arg(var, double);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symsize = dprop;
		    }
		}
		break;
	    case SKIP:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symskip = prop;
		    }
		}
		break;
	    case CHAR:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symchar = prop;
		    }
		}
		break;
	    case COLOR:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symcolor = prop;
		    }
		}
		break;
	    case LINEWIDTH:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symlinew = prop;
		    }
		}
		break;
	    case LINESTYLE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].symlines = prop;
		    }
		}
		break;
	    default:
		sprintf(buf, "Attribute not found in setprops()-SYMBOL, # = %d", prop);
		errmsg(buf);
		break;
	    }
	    break;
	case ERRORBAR:
	    switch (prop = va_arg(var, int)) {
	    case LENGTH:
		dprop = va_arg(var, double);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].errbarper = dprop;
		    }
		}
		break;
	    case TYPE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].errbarxy = prop;
		    }
		}
		break;
	    case LINEWIDTH:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].errbar_linew = prop;
		    }
		}
		break;
	    case LINESTYLE:
		prop = va_arg(var, int);
		for (i = startg; i <= endg; i++) {
		    if (allsets) {
			ends = g[i].maxplot - 1;
		    }
		    for (j = starts; j <= ends; j++) {
			g[i].p[j].errbar_lines = prop;
		    }
		}
		break;
	    case RISER:
		prop = va_arg(var, int);
		switch (prop) {
		case ACTIVE:
		    prop = va_arg(var, int);
		    for (i = startg; i <= endg; i++) {
			if (allsets) {
			    ends = g[i].maxplot - 1;
			}
			for (j = starts; j <= ends; j++) {
			    g[i].p[j].errbar_riser = prop;
			}
		    }
		    break;
		case LINEWIDTH:
		    prop = va_arg(var, int);
		    for (i = startg; i <= endg; i++) {
			if (allsets) {
			    ends = g[i].maxplot - 1;
			}
			for (j = starts; j <= ends; j++) {
			    g[i].p[j].errbar_riser_linew = prop;
			}
		    }
		    break;
		case LINESTYLE:
		    prop = va_arg(var, int);
		    for (i = startg; i <= endg; i++) {
			if (allsets) {
			    ends = g[i].maxplot - 1;
			}
			for (j = starts; j <= ends; j++) {
			    g[i].p[j].errbar_riser_lines = prop;
			}
		    }
		    break;
		default:
		    sprintf(buf, "Attribute not found in setprops()-RISER, # = %d", prop);
		    errmsg(buf);
		    break;
		}
		break;
	    default:
		sprintf(buf, "Attribute not found in setprops()-ERRORBAR, # = %d", prop);
		errmsg(buf);
		break;
	    }
	    break;
	default:
	    sprintf(buf, "Attribute not found in setprops()-top, # = %d", prop);
	    errmsg(buf);
	    break;
	}
    }
    va_end(var);
    set_dirtystate();
}

void set_axis_prop(int whichgraph, int naxis, int prop, double val)
{
    int i, j, startg, stopg;

    if (whichgraph == -1) {
	startg = 0;
	stopg = maxgraph - 1;
    } else {
	startg = whichgraph;
	stopg = whichgraph;
    }
    for (j = startg; j <= stopg; j++) {
	switch (prop) {
	case ON:
	case OFF:
	    switch (naxis) {
	    case 6:
		for (i = 0; i < MAXAXES; i++) {
		    g[j].t[i].active = (int) val;
		}
		break;
	    case 7:
		for (i = 0; i < MAXAXES; i += 2) {
		    g[j].t[i].active = (int) val;
		}
		break;
	    case 8:
		for (i = 1; i < MAXAXES; i += 2) {
		    g[j].t[i].active = (int) val;
		}
		break;
	    }
	    break;
	case COLOR:
	    switch (naxis) {
	    case 6:
		for (i = 0; i < MAXAXES; i++) {
		    g[j].t[i].tl_color = (int) val;
		    g[j].t[i].t_drawbarcolor = (int) val;
		    g[j].t[i].t_color = (int) val;
		    g[j].t[i].t_mcolor = (int) val;
		    g[j].t[i].label.color = (int) val;
		}
		break;
	    case 7:
		for (i = 0; i < MAXAXES; i += 2) {
		    g[j].t[i].tl_color = (int) val;
		    g[j].t[i].t_drawbarcolor = (int) val;
		    g[j].t[i].t_color = (int) val;
		    g[j].t[i].t_mcolor = (int) val;
		    g[j].t[i].label.color = (int) val;
		}
		break;
	    case 8:
		for (i = 1; i < MAXAXES; i += 2) {
		    g[j].t[i].tl_color = (int) val;
		    g[j].t[i].t_drawbarcolor = (int) val;
		    g[j].t[i].t_color = (int) val;
		    g[j].t[i].t_mcolor = (int) val;
		    g[j].t[i].label.color = (int) val;
		}
		break;
	    }
	    break;
	case LINEWIDTH:
	    switch (naxis) {
	    case 6:
		for (i = 0; i < MAXAXES; i++) {
		    g[j].t[i].tl_linew = (int) val;
		    g[j].t[i].t_linew = (int) val;
		    g[j].t[i].t_mlinew = (int) val;
		    g[j].t[i].t_drawbarlinew = (int) val;
		}
		break;
	    case 7:
		for (i = 0; i < MAXAXES; i += 2) {
		    g[j].t[i].tl_linew = (int) val;
		    g[j].t[i].t_linew = (int) val;
		    g[j].t[i].t_mlinew = (int) val;
		    g[j].t[i].t_drawbarlinew = (int) val;
		}
		break;
	    case 8:
		for (i = 1; i < MAXAXES; i += 2) {
		    g[j].t[i].tl_linew = (int) val;
		    g[j].t[i].t_linew = (int) val;
		    g[j].t[i].t_mlinew = (int) val;
		    g[j].t[i].t_drawbarlinew = (int) val;
		}
		break;
	    }
	    break;
	case FONTP:
	    switch (naxis) {
	    case 6:
		for (i = 0; i < MAXAXES; i++) {
		    g[j].t[i].tl_font = (int) val;
		    g[j].t[i].label.font = (int) val;
		}
		break;
	    case 7:
		for (i = 0; i < MAXAXES; i += 2) {
		    g[j].t[i].tl_font = (int) val;
		    g[j].t[i].label.font = (int) val;
		}
		break;
	    case 8:
		for (i = 1; i < MAXAXES; i += 2) {
		    g[j].t[i].tl_font = (int) val;
		    g[j].t[i].label.font = (int) val;
		}
		break;
	    }
	    break;
	case CHAR:
	    switch (naxis) {
	    case 6:
		for (i = 0; i < MAXAXES; i++) {
		    g[j].t[i].tl_charsize = val;
		    g[j].t[i].label.charsize = val;
		}
		break;
	    case 7:
		for (i = 0; i < MAXAXES; i += 2) {
		    g[j].t[i].tl_charsize = val;
		    g[j].t[i].label.charsize = val;
		}
		break;
	    case 8:
		for (i = 1; i < MAXAXES; i += 2) {
		    g[j].t[i].tl_charsize = val;
		    g[j].t[i].label.charsize = val;
		}
		break;
	    }
	    break;
	}
    }
}

#define MAX_FONT 10
#define MAX_JUST 2
#define MAX_ARROW 3
#define MAX_PATTERN 30
#define MAX_PREC 10

int checkon(int prop, int old_val, int new_val)
{
    char buf[256];
    int retval = old_val;
    check_err = 0;
    switch (prop) {
    case LINEWIDTH:
	if (new_val >= 0 && new_val <= MAX_LINEWIDTH) {
	    retval = new_val;
	} else {
	    sprintf(buf, "LINEWIDTH out of bounds, should be from 0 to %d", MAX_LINEWIDTH);
	    check_err = 1;
	}
	break;
    case LINESTYLE:
	if (new_val >= 0 && new_val <= MAX_LINESTYLE) {
	    retval = new_val;
	} else {
	    sprintf(buf, "LINESTYLE out of bounds, should be from 0 to %d", MAX_LINESTYLE);
	    check_err = 1;
	}
	break;
    case COLOR:		/* TODO use MAX_COLOR */
	if (new_val >= 0 && new_val < 16) {
	    retval = new_val;
	} else {
	    sprintf(buf, "COLOR out of bounds, should be from 0 to %d", 16 - 1);
	    check_err = 1;
	}
	break;
    case JUST:
	if (new_val >= 0 && new_val <= MAX_JUST) {
	    retval = new_val;
	} else {
	    sprintf(buf, "JUST out of bounds, should be from 0 to %d", MAX_JUST);
	    check_err = 1;
	}
	break;
    case FONTP:
	if (new_val >= 0 && new_val < MAX_FONT) {
	    retval = new_val;
	} else {
	    sprintf(buf, "FONT out of bounds, should be from 0 to %d", MAX_FONT - 1);
	    check_err = 1;
	}
	break;
    case ARROW:
	if (new_val >= 0 && new_val <= MAX_ARROW) {
	    retval = new_val;
	} else {
	    sprintf(buf, "ARROW out of bounds, should be from 0 to %d", MAX_ARROW);
	    check_err = 1;
	}
	break;
    case PATTERN:
	if (new_val >= 0 && new_val < MAX_PATTERN) {
	    retval = new_val;
	} else {
	    sprintf(buf, "PATTERN out of bounds, should be from 0 to %d", MAX_PATTERN - 1);
	    check_err = 1;
	}
	break;
    case SYMBOL:
	if (new_val >= 0 && new_val < MAXSYM) {
	    retval = new_val;
	} else {
	    sprintf(buf, "SYMBOL out of bounds, should be from 0 to %d", MAXSYM - 1);
	    check_err = 1;
	}
	break;
    case PREC:
	if (new_val >= 0 && new_val < MAX_PREC) {
	    retval = new_val;
	} else {
	    sprintf(buf, "PREC out of bounds, should be from 0 to %d", MAX_PREC - 1);
	    check_err = 1;
	}
	break;
    }
    if (check_err) {
	errmsg(buf);
    }
    return retval;
}

/* Wrappers for some functions*/

double ai_wrap(double x)
{
    double retval, dummy1, dummy2, dummy3;
    (void) airy(x, &retval, &dummy1, &dummy2, &dummy3);
    return retval;
}

double bi_wrap(double x)
{
    double retval, dummy1, dummy2, dummy3;
    (void) airy(x, &dummy1, &dummy2, &retval, &dummy3);
    return retval;
}

double ci_wrap(double x)
{
    double retval, dummy1;
    (void) sici(x, &dummy1, &retval);
    return retval;
}

double si_wrap(double x)
{
    double retval, dummy1;
    (void) sici(x, &retval, &dummy1);
    return retval;
}

double chi_wrap(double x)
{
    double retval, dummy1;
    (void) shichi(x, &dummy1, &retval);
    return retval;
}

double shi_wrap(double x)
{
    double retval, dummy1;
    (void) shichi(x, &retval, &dummy1);
    return retval;
}

double fresnlc_wrap(double x)
{
    double retval, dummy1;
    (void) fresnl(x, &dummy1, &retval);
    return retval;
}

double fresnls_wrap(double x)
{
    double retval, dummy1;
    (void) fresnl(x, &retval, &dummy1);
    return retval;
}

double iv_wrap(double v, double x)
{
    double retval;
    if (v == 0) {
	retval = i0(x);
    } else if (v == 1) {
	retval = i1(x);
    } else {
	retval = iv(v, x);
    }
    return retval;
}

double jv_wrap(double v, double x)
{
    double retval;
    if (v == rint(v)) {
	retval = jn((int) v, x);
    } else {
	retval = jv(v, x);
    }
    return retval;
}

double kn_wrap(int n, double x)
{
    double retval;
    if (n == 0) {
	retval = k0(x);
    } else if (n == 1) {
	retval = k1(x);
    } else {
	retval = kn(n, x);
    }
    return retval;
}

double yv_wrap(double v, double x)
{
    double retval;
    if (v == rint(v)) {
	retval = yn((int) v, x);
    } else {
	retval = yv(v, x);
    }
    return retval;
}

double sqr_wrap(double x)
{
    return x*x;
}

