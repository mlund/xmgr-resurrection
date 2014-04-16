/*
 * featext.c - routines to perform feature extraction on a set of curves
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/List.h>

#include "globals.h"
#include "protos.h"
#include "motifinc.h"
#include "protos.h"

typedef struct _Featext_ui {
    Widget top;
    Widget *tograph;
    Widget *feature_item;
    Widget *xval_item;
    Widget *absic_graph;
    SetChoiceItem absic_set;
    Widget legload_rc;
} Featext_ui;

static Featext_ui feui;
static Widget but2[3];
 
void do_fext_proc( Widget, XtPointer, XtPointer );
double linear_interp( double, double, double, double, double );
int dbl_comp( const void *, const void * );

int getmedian( int grno, int setno, int sorton, double *median );
int get_zero_crossing( int setl, double *xv, double *yv, double *crossing );
int get_rise_time( int setl, double *xv, double *yv, double min, double max,
		double *width );
int get_fall_time( int setl, double *xv, double *yv, double min, double max,
		double *width );
int mute_linear_regression(int n, double *x, double *y, double *slope, 
			double *intercept);
int get_half_max_width( int setl, double *xv, double *yv, double min, double max,
			double *width  );
int get_barycenter( int n, double *x, double *y, double *barycenter );
void fext_routine( int gto, int feature, int abs_src, int abs_set, int abs_graph );
void get_max_pos( double *a, double *b, int n, double max, double *d );

void do_fext_toggle (Widget w, XtPointer client_data, XtPointer call_data)
{
    int value = (int) client_data;
    if (value == 2 || value == 3) {
    	XtSetSensitive(feui.legload_rc, True);
    } else {
    	XtSetSensitive(feui.legload_rc, False);
    }
}

void do_gto_setsel_update(Widget w, XtPointer client_data, XtPointer call_data)
{
	feui.absic_set.gno = (int)client_data;
	update_save_set_list( feui.absic_set, (int)client_data );
}


void create_featext_frame(Widget w, XtPointer client_data, XtPointer call_data)
{
    int x, y, i;
    Widget dialog;
    set_wait_cursor();
    if (feui.top == NULL) {
	char *label2[3];
	label2[0] = "Accept";
	label2[1] = "Close";
	XmGetPos(app_shell, 0, &x, &y);
	feui.top = XmCreateDialogShell(app_shell, "Feature Extraction", NULL, 0);
	handle_close(feui.top);
	XtVaSetValues(feui.top, XmNx, x, XmNy, y, NULL);
	dialog = XmCreateRowColumn(feui.top, "dialog_rc", NULL, 0);

	feui.tograph = CreateGraphChoice(dialog, "Results to graph: ", maxgraph, 1);
	feui.feature_item = CreatePanelChoice0(dialog,
					  "Feature:", 3, 24,
					  "Y minimum", "Y maximum", "Y average", "Y std. dev.",
					  "Y median",
					  "X minimum", "X maximum", "X average", "X std. dev.",
					  "X median",
					  "Frequency", "Period", "Zero crossing", 
					  "Rise time", "Fall time", "Slope", "Y intercept", 
					  "Set length", "Half maximal width", 
					  "Barycenter X", "Barycenter Y", 
					  "X(Y max)", "Y(X max)",
					  NULL, 0);
	feui.xval_item = CreatePanelChoice(dialog,
						"X values from:", 5,
						"Index", "Legends", "X from Set", "Y from set",
						NULL, 0 );
	
	for (i = 0; i < 4; i++) {
	    XtAddCallback(feui.xval_item[2 + i], XmNactivateCallback,
			(XtCallbackProc) do_fext_toggle, (XtPointer) (intptr_t) i);
	}
	
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	feui.legload_rc= XmCreateRowColumn(dialog, "fext_legload_rc", NULL, 0);
	
	feui.absic_graph = CreateGraphChoice(feui.legload_rc,
								"Abscissa from graph: ",maxgraph,0);
	for(i=0; i<maxgraph; i++ )
	    XtAddCallback(feui.absic_graph[2 + i], XmNactivateCallback,
			(XtCallbackProc) do_gto_setsel_update, (XtPointer) (intptr_t) i);
	
	feui.absic_set = CreateSetSelector(feui.legload_rc, "set:",
					SET_SELECT_ACTIVE,
					FILTER_SELECT_NONE,
					0,
					SELECTION_TYPE_SINGLE);
	update_save_set_list( feui.absic_set, 0 );
	
	XtManageChild(feui.legload_rc);
	XtSetSensitive(feui.legload_rc, False);
	XtVaCreateManagedWidget("sep", xmSeparatorWidgetClass, dialog, NULL);

	CreateCommandButtons(dialog, 2, but2, label2);
	XtAddCallback(but2[0], XmNactivateCallback, (XtCallbackProc)
	do_fext_proc,(XtPointer) & feui);
	XtAddCallback(but2[1], XmNactivateCallback,
	(XtCallbackProc)destroy_dialog,(XtPointer)feui.top);

	XtManageChild(dialog);
    }
    XtRaise(feui.top);
    unset_wait_cursor();
}


void do_fext_proc( Widget w, XtPointer client_data, XtPointer call_data )
{
    int gto, feature, abs_graph = -1, abs_set = -1, abs_src;

    Featext_ui *ui = (Featext_ui *) client_data;

    feature = (int) GetChoice(ui->feature_item);
    gto = (int) GetChoice(ui->tograph )-1;
    if( gto == -1 )
            gto = cg;

    abs_src = (int) GetChoice(ui->xval_item);
    if( abs_src ==2 || abs_src==3 ) {
        abs_set = GetSelectedSet(ui->absic_set);
        abs_graph = (int) GetChoice(ui->absic_graph);
    }
    fext_routine( gto, feature, abs_src, abs_set, abs_graph ); 
}

void fext_routine( int gto, int feature, int abs_src, int abs_set, int abs_graph )
{
	int i, cs, ns, fts, ncurves, extract_err;
	double datum, dummy, *absy;
	double y1, y2;
	int iy1, iy2;
	char tbuf[1024];
	float *abscissa;

	abscissa = (float *)malloc( maxplot*sizeof(float) );
	
	if( !isactive_graph( gto )	){
		errwin("Graph for results must be active");
	    return;
	}
	if( (ns=nextset( gto ) )== -1 ) {
		errwin("Choose a new graph or kill sets!");
	    return;
	}
	ncurves = nactive(cg);
	switch( abs_src ) {
		case 0:		/* use index */
			for( i=0; i<ncurves; i++ )
				abscissa[i] = i+1;
			break;	
		case 1:		/* use legend label */
			cs = 0;
			for( i=0; i<ncurves; i++ ){
				while( !isactive_set( cg, cs ) )
					cs++;
				if(!sscanf( g[cg].p[cs].lstr, "%f", &abscissa[i]))
					break;
				cs++;
			}
			if( i != ncurves ) {
				errwin("Bad legend label");
				return;
			}
			break;
		case 2:		/* use X from set */
			if( !isactive_set( abs_graph, abs_set ) ){
	    		errwin("Abscissa set not active");
	    		return;
			}
			if( getsetlength( abs_graph, abs_set ) < ncurves ) {
				errwin("Not enough points in set");
				return;
			}
			absy = getx( abs_graph, abs_set );
			for( i=0; i<ncurves; i++ )
				abscissa[i] = absy[i];
			break;			
		case 3:										/* use Y from set */
			if( !isactive_set( abs_graph, abs_set ) ){
	    		errwin("Abscissa set not active");
	    		return;
			}
			if( getsetlength( abs_graph, abs_set ) < ncurves ) {
				errwin("Not enough points in set");
				return;
			}
			absy = gety( abs_graph, abs_set );
			for( i=0; i<ncurves; i++ )
				abscissa[i] = absy[i];
			break;
	}

	cs = 0;
	tbuf[0] = '\0';
	for( i=0; i<ncurves; i++ ) {
		while( !isactive_set( cg, cs ) )
			cs++;
		extract_err = 0;
			
		switch( feature ) {
			case 0:			/* Y minimum */
				datum = g[cg].p[cs].ymin;		
				break;
			case 1: 		/* Y maximum */
				datum = g[cg].p[cs].ymax;		
				break;
			case 2: 		/* Y mean    */
				stasum(gety(cg, cs), getsetlength(cg, cs), &datum, &dummy, 0);
				break;
			case 3:			/* Y std dev */
				stasum(gety(cg, cs), getsetlength(cg, cs), &dummy, &datum, 0);
				break;
			case 4: 		/* Y median  */
				getmedian( cg, cs, DATA_Y, &datum );
				break;
			case 5:			/* X minimum */
				datum = g[cg].p[cs].xmin;		
				break;
			case 6: 		/* X maximum */
				datum = g[cg].p[cs].xmax;		
				break;
			case 7: 		/* X mean    */
				stasum(getx(cg, cs), getsetlength(cg, cs), &datum, &dummy, 0);
				break;
			case 8:			/* X std dev */
				stasum(getx(cg, cs), getsetlength(cg, cs), &dummy, &datum, 0);
				break;
			case 9:			/* X median  */
				getmedian( cg, cs, DATA_X, &datum );
				break;
			case 10: 		/* frequency and period */
			case 11:
				if ( ilog2(getsetlength(cg, cs)) <= 0)    /* only DFT */
					do_fourier(0, cs, 0, 1, 0, 0, 0);
				else							/* FFT      */
					do_fourier(1, cs, 0, 1, 0, 0, 0);

				sprintf( tbuf, "FT of set %d", cs );
				fts = 0;
				while( strcmp( tbuf, g[cg].p[fts].comments+1 ) )
					fts++;
					
				minmax(gety(cg, fts), getsetlength(cg, fts),&y1,&y2,&iy1,&iy2);
				if( feature == 8 )
					datum = g[cg].p[fts].ex[0][iy2-1];
				else
					datum = 1./g[cg].p[fts].ex[0][iy2-1];
				killset( cg, fts );				/* get rid of Fourier set */
				break;
			case 12:		/* first zero crossing */
				if( get_zero_crossing( getsetlength( cg, cs ), 
									getx( cg, cs ),gety( cg, cs ), &datum ) ){
					sprintf( tbuf+strlen(tbuf), 
								"Unable to find zero crossing of set %d\n", cs );
					errwin( tbuf );
					extract_err = 1;
				}
				break;
			case 13:		/* rise time   */
				if( get_rise_time( getsetlength(cg,cs), getx(cg,cs), 
					gety(cg,cs), g[cg].p[cs].ymin, g[cg].p[cs].ymax, &datum ) ){
					sprintf( tbuf+strlen(tbuf), 
							"Unable to find rise time of set %d\n", cs );
					errwin( tbuf );
					extract_err = 1;
				}
				break;
			case 14: 		/* fall time   */
				if( get_fall_time( getsetlength(cg,cs), getx(cg,cs), 
					gety(cg,cs), g[cg].p[cs].ymin, g[cg].p[cs].ymax, &datum ) ){
					sprintf( tbuf+strlen(tbuf), 
									"Unable to find fall time of set %d\n", cs );
					extract_err = 1;
					errwin( tbuf );
				}
				break;
			case 15:		/* slope       */
				if( mute_linear_regression( getsetlength( cg, cs ), 
					getx( cg, cs ),gety( cg, cs ), &datum, &dummy ) ) {
					sprintf( tbuf+strlen(tbuf), 
										"Unable to find slope of set %d\n", cs );
					errwin( tbuf );
					extract_err = 1;
				}
				break;
			case 16:		/* Y intercept */
				if( mute_linear_regression( getsetlength( cg, cs ), 
						getx( cg, cs ), gety( cg, cs ), &dummy, &datum ) ) {
					sprintf( tbuf+strlen(tbuf), 
						"Unable to find y-intercept of set %d\n", cs );
					errwin( tbuf );
					extract_err = 1;
				}
				break;
			case 17:		/* set length  */
				datum = getsetlength( cg, cs );
				break;
			case 18:		/* half maximal widths */
				if( get_half_max_width(getsetlength( cg, cs ), getx(cg,cs), 
					   gety(cg,cs), g[cg].p[cs].ymin, g[cg].p[cs].ymax,&datum) ) {
					sprintf( tbuf+strlen(tbuf), 
						"Unable to find half maximal width of set %d\n", cs );
					extract_err = 1;
					errwin( tbuf );
				}
				break;
			case 19:		/* Barycenter X */
				get_barycenter( getsetlength( cg, cs ), gety(cg,cs), 
									getx(cg,cs), &datum );
				break;
			case 20:		/* Barycenter Y */
				get_barycenter( getsetlength( cg, cs ), getx(cg,cs), 
									gety(cg,cs), &datum );
				break;
			case 21:		/* X of Maximum Y */
				get_max_pos( gety(cg, cs), getx( cg, cs ),
							getsetlength( cg, cs ), g[cg].p[cs].ymax, &datum ); 
				break;
			case 22:		/* Y of Maximum X */
				get_max_pos( getx(cg, cs), gety( cg, cs ),
							getsetlength( cg, cs ), g[cg].p[cs].xmax, &datum ); 
				break;
		}
		if( !extract_err )
			add_point( gto, ns, abscissa[i], datum, 0, 0, SET_XY );
		cs++;
	}

	/* set comment */	
	switch( feature ) {
		case 0:			/* Y minimum */
			sprintf(tbuf,"Y minima of graph %d",cg); 
			break;
		case 1: 		/* Y maximum */
			sprintf(tbuf,"Y maxima of graph %d",cg);
			break;
		case 2: 		/* Y mean    */
			sprintf(tbuf,"Y means of graph %d",cg);
			break;
		case 3:			/* Y std dev */
			sprintf(tbuf,"Y std. dev.'s of graph %d",cg);
			break;
		case 4:			/* Y median  */
			sprintf(tbuf,"Y medians of graph %d",cg);
			break;
		case 5:			/* X minimum */
			sprintf(tbuf,"X minima of graph %d",cg); 
			break;
		case 6: 		/* X maximum */
			sprintf(tbuf,"X maxima of graph %d",cg);
			break;
		case 7: 		/* X mean    */
			sprintf(tbuf,"X means of graph %d",cg);
			break;
		case 8:			/* X std dev */
			sprintf(tbuf,"X std. dev.'s of graph %d",cg);
			break;
		case 9:			/* X median  */
			sprintf(tbuf,"X medians of graph %d",cg);
			break;
		case 10: 		/* frequency and period */
			sprintf(tbuf,"frequencies of graph %d",cg);
			break;
		case 11:
			sprintf(tbuf,"periods of graph %d",cg);
			break;
		case 12:		/* first zero crossing */
			sprintf(tbuf,"zero crossings of graph %d",cg);
			break;
		case 13:		/* rise time */
			sprintf(tbuf,"rise times of graph %d",cg);
			break;
		case 14: 		/* fall time */
			sprintf(tbuf,"fall times of graph %d",cg);
			break;
		case 15: 		/* slopes     */
			sprintf(tbuf,"slopes of graph %d",cg);
			break;
		case 16: 		/* Y intercepts */
			sprintf(tbuf,"Y intercepts of graph %d",cg);
			break;
		case 17: 		/* set lengths */
			sprintf(tbuf,"set lengths of graph %d",cg);
			break;
		case 18: 		/* 1/2 maximal widths */
			sprintf(tbuf,"half maximal widths of graph %d",cg);
			break;
		case 19: 		/* barycenter X */
			sprintf(tbuf,"X barycenters of graph %d",cg);
			break;
		case 20: 		/* barycenter Y */
			sprintf(tbuf,"Y barycenters of graph %d",cg);
			break;
		case 21:		/* X of maximum Y */
			sprintf(tbuf,"X positions of maximum Y's of graph %d",cg);
			break;
		case 22:		/* Y of maximum X */
			sprintf(tbuf,"Y positions of maximum X's of graph %d",cg);
			break;
	}
	setcomment( gto, ns, tbuf );
	free( abscissa );
	plotone( gto );
}


/* linear regression without posting results to log */
int mute_linear_regression(int n, double *x, double *y, double *slope, 
			double *intercept)
{
    double xbar, ybar;		/* sample means */
    double sdx, sdy;		/* sample standard deviations */
    double SXX, SYY, SXY;	/* sums of squares */
    int i;

    if (n <= 3) {
		return 1;
    }
    xbar = ybar = 0.0;
    SXX = SYY = SXY = 0.0;
    for (i = 0; i < n; i++) {
		xbar = xbar + x[i];
		ybar = ybar + y[i];
    }
    xbar = xbar / n;
    ybar = ybar / n;
    for (i = 0; i < n; i++) {
		SXX = SXX + (x[i] - xbar) * (x[i] - xbar);
		SYY = SYY + (y[i] - ybar) * (y[i] - ybar);
		SXY = SXY + (x[i] - xbar) * (y[i] - ybar);
    }
    sdx = sqrt(SXX / (n - 1));
    sdy = sqrt(SYY / (n - 1));
    if (sdx == 0.0) {
		return 2;
    }
    if (sdy == 0.0) {
		return 2;
    }
    *slope = SXY / SXX;
    *intercept = ybar - *slope * xbar;
    return 0;
}

/*
 * assume graph starts off at ymin and rises to ymax 
 * Determine time to go from 10% to 90% of rise
 */
int get_rise_time( int setl, double *xv, double *yv, double min, double max,
		double *width )
{
	int x10=0, x90;
	double amp10, amp90;
	
	amp10 = min + (max-min)*0.1;
	amp90 = min + (max-min)*0.9;
	while( x10<setl && yv[x10]<amp10  )
		x10++;
	
	if( x10==setl || x10==0)
		return 1;
	
	x90 = x10+1;
	
	while( x90<setl && yv[x90]<amp90 )
		x90++;
	
	*width = linear_interp( yv[x90-1], xv[x90-1], yv[x90], xv[x90], amp90 ) -
			 linear_interp( yv[x10-1], xv[x10-1], yv[x10], xv[x10], amp10 );
	return 0;
}

/* assume graph starts off at ymax and drops to ymin 
   Determine time to go from 90% to 10% of drop			*/
int get_fall_time( int setl, double *xv, double *yv, double min, double max,
		double *width )
{
	int x10, x90=0;
	double amp10, amp90;
	
	amp10 = min + (max-min)*0.1;
	amp90 = min + (max-min)*0.9;
	while( x90<setl && yv[x90]>amp90 )
		x90++;
	
	if( x90==setl || x90==0)
		return 1;
	
	x10= x90+1;
	
	while( x10<setl && yv[x10]>amp10  )
		x10++;

	if( x10==setl )
		return 1;
	
	
	*width = linear_interp( yv[x10-1], xv[x10-1], yv[x10], xv[x10], amp10 )-
	         linear_interp( yv[x90-1], xv[x90-1], yv[x90], xv[x90], amp90 );
	return 0;
}


int get_zero_crossing( int setl, double *xv, double *yv, double *crossing )
{
	int i=0;
	
	while( i<setl && yv[i] != 0. && yv[i]*yv[i+1]>0. )
		i++;
	
	if( i==setl )
		return 1;
	
	if( yv[i] == 0 )
		*crossing = xv[i];
	else
		*crossing = linear_interp( yv[i], xv[i], yv[i+1], xv[i+1], 0 );

	return 0;
}


/*
 * assume curve starts at min, rises to max and then drops towards min
 */
int get_half_max_width( int setl, double *xv, double *yv, double min, double max,
			double *width  )
{
	int xu=0, xd=0;
	double amp;
	
	amp = (min + max)*0.5;
	while( xu<setl && yv[xu]<amp )
		xu++;
	
	if( xu==setl )
		return 1;
	
	xd= xu+1;
	while( xd<setl && yv[xd]>amp  )
		xd++;

	if( xd==setl )
		return 1;
	
	*width =  linear_interp( yv[xd-1], xv[xd-1], yv[xd], xv[xd], amp ) -
			  linear_interp( yv[xu-1], xv[xu-1], yv[xu], xv[xu], amp );
	return 0;
}

/* linear interpolate between two points, return a y value given an x */
double linear_interp( double x1, double y1, double x2, double y2, double x )
{
	return y1 + ( x-x1 )*(y2-y1)/(x2-x1);
}

/* get the median of the X or Y portion of a set */
int getmedian( int grno, int setno, int sorton, double *median )
{
	int setlen;
	double *setdata;
	
	setlen = getsetlength( cg, setno );
	setdata = (double *)malloc( setlen*sizeof(double) );
	if( sorton == DATA_X )
		memcpy( setdata, getx( grno, setno ), setlen*sizeof(double) );
	else
		memcpy( setdata, gety( grno, setno ), setlen*sizeof(double) );
	
	qsort( setdata, setlen, sizeof(double), dbl_comp );
	
	if( setlen%2 )		/* odd set */
		*median = setdata[(setlen+1)/2-1];
	else
		*median = ( setdata[setlen/2-1] + setdata[setlen/2] )/2.;

	free( setdata );
	return 0;
}

int dbl_comp( const void *a, const void *b )
{
	if( *(double *)a < *(double *)b )
		return -1;
	else if( *(double *)a > *(double *)b )
		return 1;
	else
		return 0;
}

int get_barycenter( int n, double *x, double *y, double *barycenter )
{
	double wsum=0;
	
	*barycenter = 0;
	for( n--; n>=0; n-- ) {
		wsum += x[n]*y[n];
		*barycenter += x[n];
	}
	*barycenter = wsum/(*barycenter);
	return 0;
}

/* given maximum of set a, find the corresponding entry in set b */
void get_max_pos( double *a, double *b, int n, double max, double *d )
{
	int i=-1;
	
	while( ++i<n && a[i] != max  );
	
	if( i==n )
		return;
	else
		*d = b[i];
}
