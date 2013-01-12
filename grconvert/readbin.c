#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * for XDR
 */
#ifndef VMS
#  include <rpc/rpc.h>
#else
#  ifndef __ALPHA
#    define _XOPEN_SOURCE_EXTENDED 1
#  endif
#  ifdef MULTINET
#    include <types.h>
#    define DONT_DECLARE_MALLOC
#    include "multinet_root:[multinet.include.rpc]rpc.h"
#  else
#    include <ucx$rpcxdr.h>
#  endif
#endif

#include "grconvert.h"

static XDR xdrs;

#define BADMAGIC 999999
#define BADVERSION 999998

/*
 * Function prototypes
 */
int read_char(char *d, int n, FILE * fout);
int read_int(int *d, int n, FILE * fout);
int read_double(double *d, int n, FILE * fin);

int read_charstr(char *d, FILE * fout);
int read_float(float *d, int n, FILE * fout);
int read_short(short *d, int n, FILE * fout);
static int read_velocityp(velocityp * d, FILE * fin);
static int read_world(world * d, FILE * fin);
static int read_view(view * d, FILE * fin);
static int read_world_stack(world_stack * d, FILE * fin);
static int read_labels(labels * d, FILE * fin);
static int read_plotarr(plotarr * d, FILE * fin);
static int read_tickmarks(tickmarks * d, FILE * fin);
static int read_legend(legend * d, FILE * fin);
static int read_framep(framep * d, FILE * fin);
static int read_BoxPlot(BoxPlot * d, FILE * fin);

int replace_xdr_short( short *i );


/*
 * Read type boxtype
 */
int read_boxtype(boxtype * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 20, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return err;
    if ((err = read_int(&(d->loctype), 1, fin)))
	return err;
    if ((err = read_int(&(d->gno), 1, fin)))
	return err;
    if ((err = read_double(&(d->x1), 1, fin)))
	return err;
    if ((err = read_double(&(d->y1), 1, fin)))
	return err;
    if ((err = read_double(&(d->x2), 1, fin)))
	return err;
    if ((err = read_double(&(d->y2), 1, fin)))
	return err;
    if ((err = read_int(&(d->lines), 1, fin)))
	return err;
    if ((err = read_int(&(d->linew), 1, fin)))
	return err;
    if ((err = read_int(&(d->color), 1, fin)))
	return err;
    if ((err = read_int(&(d->fill), 1, fin)))
	return err;
    if ((err = read_int(&(d->fillcolor), 1, fin)))
	return err;
    if ((err = read_int(&(d->fillpattern), 1, fin)))
	return err;
    return err;
}

/*
 * Read type ellipsetype
 */
int read_ellipsetype(ellipsetype * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 41, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic) {
    	/* try to recover so that older saves will be compatible */
		replace_xdr_short( &magic );
		return BADMAGIC;
	}
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return err;
    if ((err = read_int(&(d->loctype), 1, fin)))
	return err;
    if ((err = read_int(&(d->gno), 1, fin)))
	return err;
    if ((err = read_double(&(d->x1), 1, fin)))
	return err;
    if ((err = read_double(&(d->y1), 1, fin)))
	return err;
    if ((err = read_double(&(d->x2), 1, fin)))
	return err;
    if ((err = read_double(&(d->y2), 1, fin)))
	return err;
    if ((err = read_int(&(d->lines), 1, fin)))
	return err;
    if ((err = read_int(&(d->linew), 1, fin)))
	return err;
    if ((err = read_int(&(d->color), 1, fin)))
	return err;
    if ((err = read_int(&(d->fill), 1, fin)))
	return err;
    if ((err = read_int(&(d->fillcolor), 1, fin)))
	return err;
    if ((err = read_int(&(d->fillpattern), 1, fin)))
	return err;
    return err;
}

/*
 * Read type linetype
 */
int read_linetype(linetype * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 21, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return err;
    if ((err = read_int(&(d->loctype), 1, fin)))
	return err;
    if ((err = read_int(&(d->gno), 1, fin)))
	return err;
    if ((err = read_double(&(d->x1), 1, fin)))
	return err;
    if ((err = read_double(&(d->y1), 1, fin)))
	return err;
    if ((err = read_double(&(d->x2), 1, fin)))
	return err;
    if ((err = read_double(&(d->y2), 1, fin)))
	return err;
    if ((err = read_int(&(d->lines), 1, fin)))
	return err;
    if ((err = read_int(&(d->linew), 1, fin)))
	return err;
    if ((err = read_int(&(d->color), 1, fin)))
	return err;
    if ((err = read_int(&(d->arrow), 1, fin)))
	return err;
    if ((err = read_int(&(d->atype), 1, fin)))
	return err;
    if ((err = read_double(&(d->asize), 1, fin)))
	return err;
    return err;
}

/*
 * Read type plotstr
 */
int read_plotstr(plotstr * d, FILE * fin)
{
    int i, err = 0;
    short magic, version, testmagic = 22, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return 1;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return 2;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return 3;
    if ((err = read_int(&(d->loctype), 1, fin)))
	return 4;
    if ((err = read_int(&(d->gno), 1, fin)))
	return 5;
    if ((err = read_double(&(d->x), 1, fin)))
	return 6;
    if ((err = read_double(&(d->y), 1, fin)))
	return 7;
    if ((err = read_int(&(d->lines), 1, fin)))
	return 8;
    if ((err = read_int(&(d->linew), 1, fin)))
	return 9;
    if ((err = read_int(&(d->color), 1, fin)))
	return 10;
    if ((err = read_int(&(d->rot), 1, fin)))
	return 11;
    if ((err = read_int(&(d->font), 1, fin)))
	return 12;
    if ((err = read_int(&(d->just), 1, fin)))
	return 13;
    if ((err = read_double(&(d->charsize), 1, fin)))
	return 14;
    if ((err = read_int(&i, 1, fin)))
	return 15;
    if (i <= 0) {
	err = -1;
	return 0;
    }
/*
    if (d->s != NULL) {
	free(d->s);
    } else {
	d->s = (char *) malloc(i * sizeof(char));
    }
*/
    d->s = (char *) malloc(i * sizeof(char));
    if ((err = read_char(d->s, i, fin)))
	return 16;
    return err;
}

/*
 * Read type velocityp
 */
static int read_velocityp(velocityp * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 25, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return err;
    if ((err = read_int(&(d->type), 1, fin)))
	return err;
    if ((err = read_int(&(d->color), 1, fin)))
	return err;
    if ((err = read_int(&(d->lines), 1, fin)))
	return err;
    if ((err = read_int(&(d->linew), 1, fin)))
	return err;
    if ((err = read_int(&(d->arrowtype), 1, fin)))
	return err;
    if ((err = read_int(&(d->loctype), 1, fin)))
	return err;
    if ((err = read_double(&(d->velx), 1, fin)))
	return err;
    if ((err = read_double(&(d->vely), 1, fin)))
	return err;
    if ((err = read_double(&(d->vscale), 1, fin)))
	return err;
    if ((err = read_int(&(d->units), 1, fin)))
	return err;
    if ((err = read_double(&(d->userlength), 1, fin)))
	return err;
    if ((err = read_plotstr(&(d->vstr), fin)))
	return err;
    return err;
}

/*
 * Read type world
 */
static int read_world(world * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 26, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_double(&(d->xg1), 1, fin)))
	return err;
    if ((err = read_double(&(d->xg2), 1, fin)))
	return err;
    if ((err = read_double(&(d->yg1), 1, fin)))
	return err;
    if ((err = read_double(&(d->yg2), 1, fin)))
	return err;
    return err;
}

/*
 * Read type view
 */
static int read_view(view * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 27, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_double(&(d->xv1), 1, fin)))
	return err;
    if ((err = read_double(&(d->xv2), 1, fin)))
	return err;
    if ((err = read_double(&(d->yv1), 1, fin)))
	return err;
    if ((err = read_double(&(d->yv2), 1, fin)))
	return err;
    return err;
}

/*
 * Read type world_stack
 */
static int read_world_stack(world_stack * d, FILE * fin)
{
    int i, err = 0;
    short magic, version, testmagic = 28, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_world(&(d->w), fin)))
	return err;
    for (i = 0; i < 3; i++) {
	if ((err = read_world(&(d->t[i]), fin)))
	    return err;
    }
    return err;
}

/*
 * Read type labels
 */
static int read_labels(labels * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 29, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_plotstr(&(d->title), fin)))
	return err;
    if ((err = read_plotstr(&(d->stitle), fin)))
	return err;
    return err;
}

/*
 * Read type plotarr
 */
static int read_plotarr(plotarr * d, FILE * fin)
{
    char buf[512];
    int i, cnt, ind, err = 0;
    short magic, version, testmagic = 33, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return 1;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return 2;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return 3;
    if ((err = read_int(&(d->type), 1, fin)))
	return 4;
    if ((err = read_int(&(d->deact), 1, fin)))
	return 5;
    if ((err = read_int(&(d->len), 1, fin)))
	return 6;
    if ((err = read_double(&(d->missing), 1, fin)))
	return 7;
    if ((err = read_int(&cnt, 1, fin)))
	return 8;
    for (i = 0; i < cnt; i++) {
	if ((err = read_int(&ind, 1, fin)))
	    return 9;
	d->ex[ind] = (double *) malloc(d->len * sizeof(double));
	if ((err = read_double(d->ex[ind], d->len, fin)))
	    return 10;
    }
    if (d->type == XYSTRING) {
	d->s = (char **) malloc(d->len * sizeof(char *));
	if (d->s == NULL) {
	    return 101;
	}
	for (i=0;i<d->len;i++) {
	    if ((err = read_charstr(buf, fin)))
		return 102;
	    d->s[i] = (char *) malloc((strlen(buf) + 1) * sizeof(char));
	    if (d->s[i] == NULL) {
		return 101;
	    }
	    strcpy(d->s[i], buf);
	}
    }
    if ((err = read_double(&(d->xmin), 1, fin)))
	return 11;
    if ((err = read_double(&(d->xmax), 1, fin)))
	return 12;
    if ((err = read_double(&(d->ymin), 1, fin)))
	return 13;
    if ((err = read_double(&(d->ymax), 1, fin)))
	return 14;
    if ((err = read_int(&(d->sym), 1, fin)))
	return 15;
    if ((err = read_char(&(d->symchar), 1, fin)))
	return 16;
    if ((err = read_int(&(d->symskip), 1, fin)))
	return 17;
    if ((err = read_int(&(d->symfill), 1, fin)))
	return 18;
    if ((err = read_int(&(d->symdot), 1, fin)))
	return 19;
    if ((err = read_int(&(d->symlines), 1, fin)))
	return 20;
    if ((err = read_int(&(d->symlinew), 1, fin)))
	return 21;
    if ((err = read_int(&(d->symcolor), 1, fin)))
	return 22;
    if ((err = read_double(&(d->symsize), 1, fin)))
	return 23;
    if ((err = read_int(&(d->avgflag), 1, fin)))
	return 24;
    if ((err = read_int(&(d->avgstdflag), 1, fin)))
	return 25;
    if ((err = read_int(&(d->avg2stdflag), 1, fin)))
	return 26;
    if ((err = read_int(&(d->avg3stdflag), 1, fin)))
	return 27;
    if ((err = read_int(&(d->avgallflag), 1, fin)))
	return 28;
    if ((err = read_int(&(d->avgvalflag), 1, fin)))
	return 29;
    if ((err = read_int(&(d->harmonicflag), 1, fin)))
	return 30;
    if ((err = read_int(&(d->geometricflag), 1, fin)))
	return 31;
    if ((err = read_int(&(d->font), 1, fin)))
	return 32;
    if ((err = read_int(&(d->format), 1, fin)))
	return 33;
    if ((err = read_int(&(d->prec), 1, fin)))
	return 34;
    if ((err = read_int(&(d->just), 1, fin)))
	return 35;
    if ((err = read_int(&(d->where), 1, fin)))
	return 36;
    if ((err = read_double(&(d->valsize), 1, fin)))
	return 37;
    if ((err = read_int(&(d->lines), 1, fin)))
	return 38;
    if ((err = read_int(&(d->linew), 1, fin)))
	return 39;
    if ((err = read_int(&(d->color), 1, fin)))
	return 40;
    if ((err = read_int(&(d->lineskip), 1, fin)))
	return 41;
    if ((err = read_int(&(d->fill), 1, fin)))
	return 42;
    if ((err = read_int(&(d->fillusing), 1, fin)))
	return 43;
    if ((err = read_int(&(d->fillcolor), 1, fin)))
	return 44;
    if ((err = read_int(&(d->fillpattern), 1, fin)))
	return 45;
    if ((err = read_int(&(d->errbar), 1, fin)))
	return 46;
    if ((err = read_int(&(d->errbarxy), 1, fin)))
	return 47;
    if ((err = read_int(&(d->errbar_linew), 1, fin)))
	return 48;
    if ((err = read_int(&(d->errbar_lines), 1, fin)))
	return 49;
    if ((err = read_int(&(d->errbar_riser), 1, fin)))
	return 50;
    if ((err = read_int(&(d->errbar_riser_linew), 1, fin)))
	return 51;
    if ((err = read_int(&(d->errbar_riser_lines), 1, fin)))
	return 52;
    if ((err = read_double(&(d->errbarper), 1, fin)))
	return 53;
    if ((err = read_double(&(d->hilowper), 1, fin)))
	return 54;
    if ((err = read_int(&(d->density_plot), 1, fin)))
	return 55;
    if ((err = read_double(&(d->zmin), 1, fin)))
	return 56;
    if ((err = read_double(&(d->zmax), 1, fin)))
	return 57;
    if ((err = read_charstr(d->comments, fin)))
	return 58;
    if ((err = read_charstr(d->lstr, fin)))
	return 58;
    if ((err = read_int(&(d->hotlink), 1, fin)))
	return 59;
    if ((err = read_int(&(d->hotsrc), 1, fin)))
	return 60;
    if ((err = read_charstr(d->hotfile, fin)))
	return 61;
    if ((err = read_double(d->emin, 6, fin)))
	return 62;
    if ((err = read_double(d->emax, 6, fin)))
	return 63;
    if ((err = read_int(d->imin, 6, fin)))
	return 64;
    if ((err = read_int(d->imax, 6, fin)))
	return 65;
    /* TODO for (i = 0; i < 0; i++) {
	if ((err = read_Regression(d->r, fin))) return err;
    }
     TODO for (i = 0; i < 0; i++) {
	if ((err = read_Spline(d->spl, fin))) return err;
    } */
    return err;
}

/*
 * Read type tickmarks
 */
static int read_tickmarks(tickmarks * d, FILE * fin)
{
    int i, cnt, err = 0;
    short magic, version, testmagic = 34, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->axis), 1, fin)))
	return err;
    if ((err = read_int(&(d->active), 1, fin)))
	return err;
    if ((err = read_int(&(d->alt), 1, fin)))
	return err;
    if ((err = read_double(&(d->tmin), 1, fin)))
	return err;
    if ((err = read_double(&(d->tmax), 1, fin)))
	return err;
    if ((err = read_double(&(d->tmajor), 1, fin)))
	return err;
    if ((err = read_double(&(d->tminor), 1, fin)))
	return err;
    if ((err = read_double(&(d->offsx), 1, fin)))
	return err;
    if ((err = read_double(&(d->offsy), 1, fin)))
	return err;
    if ((err = read_plotstr(&(d->label), fin)))
	return err;
    if ((err = read_int(&(d->label_layout), 1, fin)))
	return err;
    if ((err = read_int(&(d->label_place), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_flag), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_type), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_layout), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_angle), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_sign), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_just), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_prec), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_format), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_skip), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_staggered), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_starttype), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_stoptype), 1, fin)))
	return err;
    if ((err = read_double(&(d->tl_start), 1, fin)))
	return err;
    if ((err = read_double(&(d->tl_stop), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_op), 1, fin)))
	return err;
    if ((err = read_double(&(d->tl_vgap), 1, fin)))
	return err;
    if ((err = read_double(&(d->tl_hgap), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_font), 1, fin)))
	return err;
    if ((err = read_double(&(d->tl_charsize), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_color), 1, fin)))
	return err;
    if ((err = read_int(&(d->tl_linew), 1, fin)))
	return err;
    if ((err = read_charstr(d->tl_appstr, fin)))
	return err;
    if ((err = read_charstr(d->tl_prestr, fin)))
	return err;
    if ((err = read_int(&(d->t_type), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_flag), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_mflag), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_integer), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_num), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_inout), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_log), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_op), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_color), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_lines), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_linew), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_mcolor), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_mlines), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_mlinew), 1, fin)))
	return err;
    if ((err = read_double(&(d->t_size), 1, fin)))
	return err;
    if ((err = read_double(&(d->t_msize), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_drawbar), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_drawbarcolor), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_drawbarlines), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_drawbarlinew), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_gridflag), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_mgridflag), 1, fin)))
	return err;
    if ((err = read_int(&(d->t_spec), 1, fin)))
	return err;
    if ((err = read_int(&cnt, 1, fin)))
	return err;
    if ((err = read_double(d->t_specloc, cnt, fin)))
	return err;
    for (i = 0; i < cnt; i++) {
	if ((err = read_plotstr(&(d->t_speclab[i]), fin)))
	    return err;
    }
    if ((err = read_int(&(d->spec_font), 1, fin)))
	return err;
    if ((err = read_double(&(d->spec_charsize), 1, fin)))
	return err;
    if ((err = read_int(&(d->spec_color), 1, fin)))
	return err;
    if ((err = read_int(&(d->spec_linew), 1, fin)))
	return err;
    return err;
}

/*
 * Read type legend
 */
static int read_legend(legend * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 36, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return 1;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return 2;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return 3;
    if ((err = read_int(&(d->loctype), 1, fin)))
	return 4;
    if ((err = read_int(&(d->layout), 1, fin)))
	return 5;
    if ((err = read_int(&(d->vgap), 1, fin)))
	return 6;
    if ((err = read_int(&(d->hgap), 1, fin)))
	return err;
    if ((err = read_int(&(d->len), 1, fin)))
	return 7;
    if ((err = read_int(&(d->box), 1, fin)))
	return 8;
    if ((err = read_double(&(d->legx), 1, fin)))
	return 9;
    if ((err = read_double(&(d->legy), 1, fin)))
	return 10;
    if ((err = read_int(&(d->font), 1, fin)))
	return 11;
    if ((err = read_double(&(d->charsize), 1, fin)))
	return 12;
    if ((err = read_int(&(d->color), 1, fin)))
	return 13;
    if ((err = read_int(&(d->linew), 1, fin)))
	return 14;
    if ((err = read_int(&(d->lines), 1, fin)))
	return 15;
    if ((err = read_int(&(d->boxfill), 1, fin)))
	return 16;
    if ((err = read_int(&(d->boxfillusing), 1, fin)))
	return 17;
    if ((err = read_int(&(d->boxfillcolor), 1, fin)))
	return 18;
    if ((err = read_int(&(d->boxfillpat), 1, fin)))
	return 19;
    if ((err = read_int(&(d->boxlcolor), 1, fin)))
	return 20;
    if ((err = read_int(&(d->boxlinew), 1, fin)))
	return 21;
    if ((err = read_int(&(d->boxlines), 1, fin)))
	return 22;
    return err;
}

/*
 * Read type framep
 */
static int read_framep(framep * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 38, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return err;
    if ((err = read_int(&(d->type), 1, fin)))
	return err;
    if ((err = read_int(&(d->color), 1, fin)))
	return err;
    if ((err = read_int(&(d->lines), 1, fin)))
	return err;
    if ((err = read_int(&(d->linew), 1, fin)))
	return err;
    if ((err = read_int(&(d->fillbg), 1, fin)))
	return err;
    if ((err = read_int(&(d->bgcolor), 1, fin)))
	return err;
    return err;
}

/*
 * Read type BoxPlot
 */
static int read_BoxPlot(BoxPlot * d, FILE * fin)
{
    int err = 0;
    short magic, version, testmagic = 39, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_double(&(d->il), 1, fin)))
	return err;
    if ((err = read_double(&(d->iu), 1, fin)))
	return err;
    if ((err = read_double(&(d->ol), 1, fin)))
	return err;
    if ((err = read_double(&(d->ou), 1, fin)))
	return err;
    if ((err = read_int(&(d->nthresh), 1, fin)))
	return err;
    if ((err = read_int(&(d->outliers), 1, fin)))
	return err;
    if ((err = read_int(&(d->wtype), 1, fin)))
	return err;
    if ((err = read_double(&(d->boxwid), 1, fin)))
	return err;
    return err;
}

/*
 * Read type graph
 */
int read_graph(graph * d, FILE * fin)
{
    int i, cnt, ind, err = 0;
    short magic, version, testmagic = 40, testversion = 0;
    if ((err = read_short(&magic, 1, fin)))
	return err;
    if (magic != testmagic)
	return BADMAGIC;
    if ((err = read_short(&version, 1, fin)))
	return err;
    if (version != testversion)
	return BADVERSION;
    if ((err = read_int(&(d->active), 1, fin)))
	return err;
    if ((err = read_int(&(d->hidden), 1, fin)))
	return err;
    if ((err = read_int(&(d->label), 1, fin)))
	return err;
    if ((err = read_int(&(d->type), 1, fin)))
	return err;
    if ((err = read_int(&(d->noauto_world), 1, fin)))
	return err;
    if ((err = read_int(&(d->noauto_tics), 1, fin)))
	return err;
    if ((err = read_int(&(d->auto_type), 1, fin)))
	return err;
    if ((err = read_int(&(d->parmsread), 1, fin)))
	return err;
    
    /* parmsread must be assumed to be true when read from a binary file */
    /* (we don't erase the above 2 lines for the sake of compatibility)  */
    d->parmsread = TRUE;
    
    if ((err = read_int(&(d->revx), 1, fin)))
	return err;
    if ((err = read_int(&(d->revy), 1, fin)))
	return err;
    if ((err = read_int(&(d->maxplot), 1, fin)))
	return err;

    if ((err = read_int(&cnt, 1, fin)))
	return err;
    for (i = 0; i < cnt; i++) {
	if ((err = read_int(&ind, 1, fin)))
	    return err;
	if ((err = read_plotarr(&(d->p[ind]), fin))) {
	    return err;
	}
    }

    if ((err = read_legend(&(d->l), fin))) {
	return err;
    }
    if ((err = read_world(&(d->w), fin)))
	return err;
    if ((err = read_view(&(d->v), fin)))
	return err;
    if ((err = read_world(&(d->rt), fin)))
	return err;
    if ((err = read_labels(&(d->labs), fin)))
	return err;
    for (i = 0; i < 4; i++) {
	if ((err = read_tickmarks(&(d->t[i]), fin)))
	    return err;
    }
    if ((err = read_framep(&(d->f), fin)))
	return err;
    if ((err = read_int(&(d->pointset), 1, fin)))
	return err;
    if ((err = read_int(&(d->pt_type), 1, fin)))
	return err;
    if ((err = read_double(&(d->dsx), 1, fin)))
	return err;
    if ((err = read_double(&(d->dsy), 1, fin)))
	return err;
    if ((err = read_int(&(d->fx), 1, fin)))
	return err;
    if ((err = read_int(&(d->fy), 1, fin)))
	return err;
    if ((err = read_int(&(d->px), 1, fin)))
	return err;
    if ((err = read_int(&(d->py), 1, fin)))
	return err;
    if ((err = read_int(&(d->ws_top), 1, fin)))
	return err;
    for (i = 0; i < d->ws_top; i++) {
	if ((err = read_world_stack(&(d->ws[i]), fin)))
	    return err;
    }
    
    /* add this for compatability issues */
    if( d->ws_top == 0 )
    	d->ws_top = 1;
    	
    if ((err = read_int(&(d->curw), 1, fin)))
	return err;
    if ((err = read_velocityp(&(d->vp), fin)))
	return err;
    if ((err = read_BoxPlot(&(d->bp), fin)))
	return err;
    return err;
}

void open_xdr(FILE *fp, int rw) /* rw write = 0, read = 1 */
{
    if (rw == 0) {
	xdrstdio_create(&xdrs, fp,  XDR_ENCODE);
    } else {
	xdrstdio_create(&xdrs, fp,  XDR_DECODE);
    }
}

void close_xdr(void)
{
/*
    xdr_destroy(&xdrs);
*/
}

int read_double(double *d, int n, FILE * fp)
{
    int err;
    if (n <= 0) return 0;
    err = xdr_vector(&xdrs, (char *) d, n, sizeof(double), (xdrproc_t) xdr_double);
    return err ? 0 : err;
}


int read_int(int *d, int n, FILE * fp)
{
    int err;
    if (n <= 0) return 0;
    err = xdr_vector(&xdrs, (char *) d, n, sizeof(int), (xdrproc_t) xdr_int);
    return err ? 0 : err;
}

int read_charstr(char *d, FILE * fp)
{
    int err, n;
    n = strlen(d) + 1;
    xdr_int(&xdrs, &n);
    err = xdr_vector(&xdrs, (char *) d, n, sizeof(char), (xdrproc_t) xdr_char);
    return err ? 0 : err;
}

int read_char(char *d, int n, FILE * fp)
{
    int err;
    if (n <= 0) return 0;
    err = xdr_vector(&xdrs, (char *) d, n, sizeof(char), (xdrproc_t) xdr_char);
    return err ? 0 : err;
}

int read_short(short *d, int n, FILE * fp)
{
    int err;
    if (n <= 0) return 0;
    err = xdr_vector(&xdrs, (char *) d, n, sizeof(short), (xdrproc_t) xdr_short);
    return err ? 0 : err;
}

int read_float(float *d, int n, FILE * fp)
{
    int err;
    if (n <= 0) return 0;
    err = xdr_vector(&xdrs, (char *) d, n, sizeof(float), (xdrproc_t) xdr_float);
    return err ? 0 : err;
}

int replace_xdr_int( int *i )
{
	int err;
	xdr_setpos( &xdrs, xdr_getpos( &xdrs )-4 );
	err = xdr_vector(&xdrs, (char *) i, 1, sizeof(int), (xdrproc_t) xdr_int);
	xdr_setpos( &xdrs, xdr_getpos( &xdrs )-4 );
	return err ? 0: err;
}

int replace_xdr_short( short *i )
{
	int err;
	xdr_setpos( &xdrs, xdr_getpos( &xdrs )-4 );
	err = xdr_vector(&xdrs, (char *) i, 1, sizeof(short), (xdrproc_t) xdr_short);
	xdr_setpos( &xdrs, xdr_getpos( &xdrs )-4 );
	return err ? 0: err;
}



/*
 * write state information
 */
#define PARMS_MAGIC 1002003
#define HEADERLENGTH 64


int is_state_save(char *fname)
{
    int magic, clen, slen, ilen, flen, dlen;
    char buf[256];
    char ver[64], ord[64], math[64], name[64];
    FILE *fp;
    if ((fp = fopen(fname, "r")) == NULL) {
	return 0;
    }
    open_xdr(fp, 1); /* open XDR stream */
    read_char(buf, HEADERLENGTH, fp);
    buf[HEADERLENGTH - 1] = 0;
    sscanf(buf, "%d %s %s %s %s %d %d %d %d %d",
	&magic, ver, ord, math, name, &clen, &slen, &ilen, &flen, &dlen);
    close_xdr();
    fclose(fp);
    return (magic == PARMS_MAGIC) ? 1 : 0;
}

/*
 * getbinary - read in binary project file
 *
 * return: 0 - o.k.
 *         1 - nothing read in
 *         2 - partial read
 */
int getbinary(int gno, char *fname, int imbed)
{
    int ind, i, k, cnt, ng, magic;
    int clen, slen, ilen, flen, dlen, nblocks;
    char buf[256];
    char ver[64], ord[64], math[64], name[64];
    FILE *pp;
    if ((pp = fopen(fname, "r")) == NULL) {
	sprintf(buf, "Can't open project file %s", fname);
	errmsg(buf);
	return 1;
    }
    strcpy( docname, fname );
    open_xdr(pp, 1); /* open XDR stream */
    read_char(buf, HEADERLENGTH, pp);
    buf[HEADERLENGTH - 1] = 0;
    sscanf(buf, "%d %s %s %s %s %d %d %d %d %d",
	&magic, ver, ord, math, name, &clen, &slen, &ilen, &flen, &dlen);
    if (magic != PARMS_MAGIC) {
	errmsg("Bad magic in project file");
	fclose(pp);
	return 1;
    }
    read_int(&cnt, 1, pp);
    for (k = 0; k < cnt; k++) {
	read_int(&ng, 1, pp);
	if (read_graph(&g[ng], pp)) {
	    errmsg("Error reading project file (graphs), cancelled");
	    fclose(pp);
	    return 2;
	}
    }
    read_int(&cnt, 1, pp);
    for (k = 0; k < cnt; k++) {
	read_int(&ind, 1, pp);
	if (read_linetype(&lines[ind], pp)) {
	    errmsg("Error reading project file (lines), cancelled");
	    fclose(pp);
	    return 2;
	}
	lines[ind].active = ON;
    }
    read_int(&cnt, 1, pp);
    for (k = 0; k < cnt; k++) {
	read_int(&ind, 1, pp);
	if (read_boxtype(&boxes[ind], pp)) {
	    errmsg("Error reading project file (boxes), cancelled");
	    fclose(pp);
	    return 2;
	}
	boxes[ind].active = ON;
    }
    read_int(&cnt, 1, pp);
    for (k = 0; k < cnt; k++) {
		read_int(&ind, 1, pp);
		switch(read_ellipsetype(&ellip[ind], pp)) {
			case 0:
				ellip[ind].active = ON;
				break;
			case BADMAGIC:
				/* assume error from reading pre-ellipse file so put things
		    		back onto stream										*/
				if( replace_xdr_int( &ind ) || replace_xdr_int( &cnt ) ) {
	    			errmsg("Error reading project file (ellipses), cancelled");
	    			fclose(pp);
	    			return 2;
	    		} else
	    			k = cnt;	/* end loop */
	    	break;
	    default:
	    	errmsg("Error reading project file (plotstr), cancelled");
	    	fclose(pp);
	    	return 2;
	}
    }
   	read_int(&cnt, 1, pp);
    for (k = 0; k < cnt; k++) {
	read_int(&ind, 1, pp);
	if (read_plotstr(&pstr[ind], pp)) {
	    errmsg("Error reading project file (plotstr), cancelled");
	    fclose(pp);
	    return 2;
	}
    }
/* read block data */
    read_int(&nblocks, 1, pp);
    if (nblocks != 0) {
	read_int(&blocklen, 1, pp);
	read_int(&blockncols, 1, pp);
	for (i = 0; i < blockncols; i++) {
	    read_int(&ind, 1, pp); /* TODO  need to fix this malloc business */
	    blockdata[ind] = (double *) malloc(sizeof(double) * blocklen);
	    read_double(blockdata[ind], blocklen, pp);
	}
    }
	/* read description */
	if( read_charstr( description, pp ) )
		strcpy( description, "Just a typical project I assume" );
        if( read_int(&page_layout, 1, pp))
                page_layout=FREE;
    close_xdr();
    fclose(pp);
    return 0;
}
