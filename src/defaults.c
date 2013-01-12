/* $Id: defaults.c,v 1.6 1995/07/01 04:53:30 pturner Exp pturner $
 *
 * set defaults - changes to the types in defines.h
 * will require changes in here also
 *
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#ifdef VMS
#  include <string.h>
#endif

#include "protos.h"

static defaults d_d =
{1, 1, 1, 1.0, 4, 0, 1.0};

/* defaults layout
    int color;
    int lines;
    int linew;
    double charsize;
    int font;
    int fontsrc;
    double symsize;
*/

static world d_w =
{0.0, 1.0, 0.0, 1.0};

static view d_v =
{0.15, 0.85, 0.15, 0.85};

void set_program_defaults(void)
{
    int i;
    grdefaults = d_d;
    g = (graph *) calloc(maxgraph, sizeof(graph));
    for (i = 0; i < maxgraph; i++) {
	g[i].p = (plotarr *) calloc(maxplot, sizeof(plotarr));
	if (g[i].p == NULL) {
	    fprintf(stderr, 
		"Couldn't allocate memory for sets in graph %d, maxplot = %d, fatal error", i, maxplot);
	    exit(1);
	}
	set_default_graph(i);
	setdefaultcolors(i);
    }
    for (i = 0; i < MAXREGION; i++) {
	set_region_defaults(i);
    }
    set_default_annotation();
    set_default_string(&timestamp);
    alloc_blockdata(maxplot);
    timestamp.x = 0.03;
    timestamp.y = 0.03;
    if (init_scratch_arrays(maxarr)) {
	errmsg("Couldn't allocate memory for scratch arrays, don't use them");
    }
    
    target_set.gno = -1;
    target_set.setno = -1;
}

void set_region_defaults(int rno)
{
    int j;
    rg[rno].active = FALSE;
    rg[rno].type = 0;
    rg[rno].color = grdefaults.color;
    rg[rno].lines = grdefaults.lines;
    rg[rno].linew = grdefaults.linew;
    rg[rno].linkto = (int *) malloc(maxgraph * sizeof(int));
    for (j = 0; j < maxgraph; j++) {
	rg[rno].linkto[j] = -1;
    }
    rg[rno].n = 0;
    rg[rno].x = rg[rno].y = (double *) NULL;
    rg[rno].x1 = rg[rno].y1 = rg[rno].x2 = rg[rno].y2 = 0.0;
}

void set_default_framep(framep * f)
{
    f->active = TRUE;		/* frame on or off */
    f->type = 0;		/* frame type */
    f->lines = grdefaults.lines;
    f->linew = grdefaults.linew;
    f->color = grdefaults.color;
    f->fillbg = FALSE;		/* fill background */
    f->bgcolor = 0;		/* background color inside frame */
}

void set_default_world(world * w)
{
    memcpy(w, &d_w, sizeof(world));
}

void set_default_view(view * v)
{
    memcpy(v, &d_v, sizeof(view));
}

void set_default_string(plotstr * s)
{
    s->active = FALSE;
    s->loctype = COORD_VIEW;
    s->gno = -1;
    s->x = s->y = 0.0;
    s->lines = grdefaults.lines;
    s->linew = grdefaults.linew;
    s->color = grdefaults.color;
    s->rot = 0;
    s->font = grdefaults.font;
    s->just = 0;
    s->charsize = grdefaults.charsize;
    s->s = (char *) malloc(sizeof(char));
    s->s[0] = 0;
}

void set_default_line(linetype * l)
{
    l->active = FALSE;
    l->loctype = COORD_VIEW;
    l->gno = -1;
    l->x1 = l->y1 = l->x2 = l->y2 = 0.0;
    l->lines = grdefaults.lines;
    l->linew = grdefaults.linew;
    l->color = grdefaults.color;
    l->arrow = 0;
    l->atype = 0;
    l->asize = 1.0;
}

void set_default_box(boxtype * b)
{
    b->active = FALSE;
    b->loctype = COORD_VIEW;
    b->gno = -1;
    b->x1 = b->y1 = b->x2 = b->y2 = 0.0;
    b->lines = grdefaults.lines;
    b->linew = grdefaults.linew;
    b->color = grdefaults.color;
    b->fill = FALSE;
    b->fillcolor = 1;
    b->fillpattern = 1;
}

void set_default_ellipse(ellipsetype * e)
{
    e->active = FALSE;
    e->loctype = COORD_VIEW;
    e->gno = -1;
    e->x1 = e->y1 = e->x2 = e->y2 = 0.0;
    e->lines = grdefaults.lines;
    e->linew = grdefaults.linew;
    e->color = grdefaults.color;
    e->fill = FALSE;
    e->fillcolor = 1;
    e->fillpattern = 1;
}

void set_default_arc(arctype * a)
{
    a->active = FALSE;
    a->loctype = COORD_VIEW;
    a->gno = -1;
    a->xc = a->yc = 0.0;
    a->lines = grdefaults.lines;
    a->linew = grdefaults.linew;
    a->color = grdefaults.color;
    a->fill = FALSE;
    a->fillcolor = 1;
    a->fillpattern = 1;
}

void set_default_circle(circletype * c)
{
    c->active = FALSE;
    c->loctype = COORD_VIEW;
    c->gno = -1;
    c->xc = c->yc = 0.0;
    c->lines = grdefaults.lines;
    c->linew = grdefaults.linew;
    c->color = grdefaults.color;
    c->fill = FALSE;
    c->fillcolor = 1;
    c->fillpattern = 1;
}

void set_default_legend(int gno, legend * l)
{
    l->active = FALSE;
    l->loctype = COORD_VIEW;
    l->layout = 0;
    l->vgap = 2;
    l->hgap = 1;
    l->len = 4;
    l->legx = 0.8;
    l->legy = 0.8;
    l->font = grdefaults.font;
    l->charsize = 1.0;
    l->color = grdefaults.color;
    l->linew = grdefaults.linew;
    l->lines = grdefaults.lines;
    l->box = FALSE;
    l->boxfill = FALSE;
    l->boxfillusing = CLRFILLED;
    l->boxfillcolor = 0;
    l->boxfillpat = 1;
    l->boxlcolor = grdefaults.color;	/* color for symbol */
    l->boxlinew = grdefaults.linew;	/* set plot sym line width */
    l->boxlines = grdefaults.lines;	/* set plot sym line style */
}

void set_default_plotarr(plotarr * p)
{
    int i;
    p->active = FALSE;		/* active flag */
    p->type = SET_XY;		/* dataset type */
    p->deact = 0;		/* deactivated set */
    p->hotlink = 0;		/* hot linked set */
    p->hotfile[0] = 0;		/* hot linked file name */
    p->len = 0;			/* set length */
    p->missing = DATASET_MISSING;	/* value for missing data */
    p->s = (char **) NULL;	/* pointer to strings */
    p->xmin = p->xmax = p->ymin = p->ymax = 0.0;

    p->sym = 0;			/* set plot symbol */
    p->symchar = 0;		/* character for symbol */
    p->symskip = 0;		/* How many symbols to skip */
    p->symfill = 0;		/* Symbol fill type */
    p->symdot = 0;		/* Symbol dot in center */
    p->symlines = grdefaults.lines;	/* set plot sym line style */
    p->symlinew = grdefaults.linew;	/* set plot sym line width */
    p->symcolor = -1;		/* color for symbol; -1 means same color as line */
    p->symsize = 1.0;		/* size of symbols */

    p->font = grdefaults.font;	/* font for strings */
    p->format = FORMAT_DECIMAL;	/* format for drawing values */
    p->prec = 1;		/* precision for drawing values */
    p->just = JUST_LEFT;	/* justification for drawing values */
    p->where = PLACE_RIGHT;	/* where to draw values */
    p->valsize = 1.0;		/* char size for drawing values */

    p->lines = grdefaults.lines;
    p->linew = grdefaults.linew;
    p->color = grdefaults.color;
    p->lineskip = 0;		/* How many points to skip when drawing lines */

    p->fill = 0;		/* fill type */
    p->fillusing = CLRFILLED;	/* fill using color or pattern */
    p->fillcolor = 1;		/* fill color */
    p->fillpattern = 0;		/* fill pattern */

    p->errbar = -1;		/* if type is _DX, _DY, _DXDY and errbar =
				 * TRUE */
    p->errbarxy = PLACE_BOTH;	/* type of error bar */
    p->errbar_lines = grdefaults.lines;	/* error bar line width */
    p->errbar_linew = grdefaults.linew;	/* error bar line style */
    p->errbar_riser = TRUE;	/* connecting line between error limits */
    p->errbar_riser_linew = 1;	/* connecting line between error limits line
				 * width */
    p->errbar_riser_lines = 1;	/* connecting line between error limits line
				 * style */

    p->errbarper = 1.0;		/* length of error bar */
    p->hilowper = 1.0;		/* length of hi-low */

    p->density_plot = 0;	/* if type is XYZ then density_plot  = 1 */
    p->zmin = p->zmax = 0.0;	/* min max for density plots */

    p->comments[0] = 0;		/* how did this set originate */
    p->lstr[0] = 0;		/* legend */

    for (i = 0; i < MAX_SET_COLS; i++) {
	p->ex[i] = NULL;
	p->emin[i] = 0.0;	/* min for each column */
	p->emax[i] = 0.0;	/* max for each column */
	p->imin[i] = 0;		/* min loc for each column */
	p->imax[i] = 0;		/* max loc for each column */
    }
    p->ep = NULL;		/* EditPoints pointer */
}

void set_default_velocityp(velocityp * vp)
{
    vp->active = FALSE;
    vp->type = 0;
    vp->loctype = COORD_VIEW;
    vp->velx = 0.8;
    vp->vely = 0.7;
    vp->lines = grdefaults.lines;
    vp->linew = grdefaults.linew;
    vp->color = grdefaults.color;
    set_default_string(&(vp->vstr));
    vp->arrowtype = 0;
    vp->vscale = 1.0;
    vp->units = 0;
    vp->userlength = 1.0;
}

void set_default_graph(int gno)
{
    int i;

    g[gno].active = FALSE;
    g[gno].hidden = FALSE;
    g[gno].label = FALSE;
    g[gno].type = GRAPH_XY;
    g[gno].auto_type = TYPE_AUTO;
    g[gno].autoscale = 0;
    g[gno].noautoscale = 0;
    g[gno].revx = FALSE;
    g[gno].revy = FALSE;
    g[gno].ws_top = 1;
    g[gno].ws[0].w.xg1=g[gno].ws[0].w.xg2=g[gno].ws[0].w.yg1=g[gno].ws[0].w.yg2=0;
	g[gno].curw = 0;
    g[gno].maxplot = maxplot;
    g[gno].dsx = g[gno].dsy = 0.0;	/* locator props */
    g[gno].pointset = FALSE;
    g[gno].pt_type = 0;
    g[gno].fx = FORMAT_GENERAL;
    g[gno].fy = FORMAT_GENERAL;
    g[gno].px = 6;
    g[gno].py = 6;
    g[gno].barwid = 0.85;
    g[gno].sbarwid = 0.75;
    set_default_ticks(&g[gno].t[0], X_AXIS);
    set_default_ticks(&g[gno].t[1], Y_AXIS);
    set_default_ticks(&g[gno].t[2], ZX_AXIS);
    set_default_ticks(&g[gno].t[3], ZY_AXIS);
    set_default_framep(&g[gno].f);
    set_default_world(&g[gno].w);
    set_default_view(&g[gno].v);
    g[gno].rt.xg1 = 1.0;
    g[gno].rt.yg1 = 2.0 * M_PI;
    set_default_legend(gno, &g[gno].l);
    set_default_string(&g[gno].labs.title);
    g[gno].labs.title.charsize = 1.5;
    set_default_string(&g[gno].labs.stitle);
    g[gno].labs.stitle.charsize = 1.0;
    for (i = 0; i < maxplot; i++) {
	set_default_plotarr(&g[gno].p[i]);
    }
    set_default_velocityp(&g[gno].vp);
}

void realloc_plots(int maxplot)
{
    int i, j;
    for (i = 0; i < maxgraph; i++) {
	g[i].p = (plotarr *) realloc(g[i].p, maxplot * sizeof(plotarr));
	for (j = g[i].maxplot; j < maxplot; j++) {
	    g[i].p[j].len = 0;
	    set_default_plotarr(&g[i].p[j]);
	}
	g[i].maxplot = maxplot;
	setdefaultcolors(i);
    }
}

void realloc_graph_plots(int gno, int maxplot)
{
    int j;
    g[gno].p = (plotarr *) realloc(g[gno].p, maxplot * sizeof(plotarr));
    for (j = g[gno].maxplot; j < maxplot; j++) {
	g[gno].p[j].len = 0;
	set_default_plotarr(&g[gno].p[j]);
    }
    g[gno].maxplot = maxplot;
	setdefaultcolors(gno);
}

void realloc_graphs(void)
{
    int j;

    g = (graph *) realloc(g, maxgraph * sizeof(graph));
    for (j = MAXGRAPH; j < maxgraph; j++) {
		g[j].p = (plotarr *) calloc(maxplot, sizeof(plotarr));
		set_default_graph(j);
		setdefaultcolors(j);
    }
}

void set_default_annotation(void)
{
    int i;

    lines = (linetype *) malloc(maxlines * sizeof(linetype));
    boxes = (boxtype *) malloc(maxboxes * sizeof(boxtype));    
    pstr = (plotstr *) malloc(maxstr * sizeof(plotstr));
    ellip = (ellipsetype *) malloc(maxellipses * sizeof(ellipsetype));

    for (i = 0; i < maxboxes; i++) {
	set_default_box(&boxes[i]);
    }
    for (i = 0; i < maxlines; i++) {
	set_default_line(&lines[i]);
    }
    for (i = 0; i < maxellipses; i++) {
	set_default_ellipse(&ellip[i]);
    }
    for (i = 0; i < maxstr; i++) {
	set_default_string(&pstr[i]);
    }
}

void set_default_ticks(tickmarks * t, int a)
{
    int i;

    t->axis = a;
    switch (a) {
    case X_AXIS:
    case Y_AXIS:
	t->active = TRUE;
	t->alt = FALSE;
	t->tl_flag = TRUE;
	t->t_flag = TRUE;
	break;
    case ZX_AXIS:
    case ZY_AXIS:
	t->active = TRUE;
	t->alt = FALSE;
	t->tl_flag = FALSE;
	t->t_flag = FALSE;
	break;
    }
    set_default_string(&t->label);
    t->tmin = 0.0;
    t->tmax = 1.0;
    t->tmajor = 0.5;
    t->tminor = 0.25;
    t->offsx = 0.0;
    t->offsy = 0.0;
    t->label_layout = LAYOUT_PARALLEL;
    t->label_place = TYPE_AUTO;
    t->tl_type = TYPE_AUTO;
    t->tl_layout = TICKS_HORIZONTAL;
    t->tl_loc = LABEL_ONTICK;
    t->tl_sign = SIGN_NORMAL;
    t->tl_prec = 5;
    t->tl_format = FORMAT_GENERAL;
    t->tl_angle = 0;
    t->tl_just = (a % 2) ? JUST_RIGHT : JUST_CENTER;
    t->tl_skip = 0;
    t->tl_staggered = 0;
    t->tl_starttype = TYPE_AUTO;
    t->tl_stoptype = TYPE_AUTO;
    t->tl_start = 0.0;
    t->tl_stop = 0.0;
    t->tl_op = (a % 2) ? PLACE_LEFT : PLACE_BOTTOM;
    t->tl_vgap = 1.0;
    t->tl_hgap = 1.0;
    t->tl_font = grdefaults.font;
    t->tl_charsize = 1.0;
    t->tl_linew = grdefaults.linew;
    t->tl_color = grdefaults.color;
    t->tl_appstr[0] = 0;
    t->tl_prestr[0] = 0;
    t->t_type = TYPE_AUTO;
    t->t_mflag = TRUE;
    t->t_integer = FALSE;
    t->t_num = 6;
    t->t_inout = TICKS_IN;
    t->t_log = FALSE;
    t->t_op = PLACE_BOTH;
    t->t_size = 1.0;
    t->t_msize = 0.5;
    t->t_drawbar = FALSE;
    t->t_drawbarcolor = grdefaults.color;
    t->t_drawbarlines = grdefaults.lines;
    t->t_drawbarlinew = grdefaults.linew;
    t->t_gridflag = FALSE;
    t->t_mgridflag = FALSE;
    t->t_color = grdefaults.color;
    t->t_lines = grdefaults.lines;
    t->t_linew = grdefaults.linew;
    t->t_mcolor = grdefaults.color;
    t->t_mlines = grdefaults.lines;
    t->t_mlinew = grdefaults.linew;
    t->t_spec = 0;
    for (i = 0; i < MAX_TICK_LABELS; i++) {
	t->t_specloc[i] = 0.0;
	t->t_speclab[i].s = NULL;
    }
}
