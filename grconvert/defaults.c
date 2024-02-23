#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "grconvert.h"

int pslwincr = 2;  /* factor for setting linewidth increment */
int pslwbegin = 1; /* offset for setting linewidth */

static defaults d_d = {1, 1, 1, 1.0, 4, 0, 1.0};

/* defaults layout
    int color;
    int lines;
    int linew;
    double charsize;
    int font;
    int fontsrc;
    double symsize;
*/

static world d_w = {0.0, 1.0, 0.0, 1.0};

static view d_v = {0.15, 0.85, 0.15, 0.85};

void set_program_defaults(void) {
    int i;
    grdefaults = d_d;
    g = (graph*)calloc(maxgraph, sizeof(graph));
    for (i = 0; i < maxgraph; i++) {
        g[i].p = (plotarr*)calloc(maxplot, sizeof(plotarr));
        if (g[i].p == NULL) {
            fprintf(stderr, "Couldn't allocate memory for sets in graph %d, maxplot = %d, fatal error", i, maxplot);
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
}

void set_region_defaults(int rno) {
    int j;
    rg[rno].active = OFF;
    rg[rno].type = 0;
    rg[rno].color = grdefaults.color;
    rg[rno].lines = grdefaults.lines;
    rg[rno].linew = grdefaults.linew;
    rg[rno].linkto = (int*)malloc(maxgraph * sizeof(int));
    for (j = 0; j < maxgraph; j++) {
        rg[rno].linkto[j] = -1;
    }
    rg[rno].n = 0;
    rg[rno].x = rg[rno].y = (double*)NULL;
    rg[rno].x1 = rg[rno].y1 = rg[rno].x2 = rg[rno].y2 = 0.0;
}

void set_default_framep(framep* f) {
    f->active = ON; /* frame on or off */
    f->type = 0;    /* frame type */
    f->lines = grdefaults.lines;
    f->linew = grdefaults.linew;
    f->color = grdefaults.color;
    f->fillbg = OFF; /* fill background */
    f->bgcolor = 0;  /* background color inside frame */
}

void set_default_world(world* w) { memcpy(w, &d_w, sizeof(world)); }

void set_default_view(view* v) { memcpy(v, &d_v, sizeof(view)); }

void set_default_string(plotstr* s) {
    s->active = OFF;
    s->loctype = VIEW;
    s->gno = -1;
    s->x = s->y = 0.0;
    s->lines = grdefaults.lines;
    s->linew = grdefaults.linew;
    s->color = grdefaults.color;
    s->rot = 0;
    s->font = grdefaults.font;
    s->just = 0;
    s->charsize = grdefaults.charsize;
    s->s = (char*)malloc(sizeof(char));
    s->s[0] = 0;
}

void set_default_line(linetype* l) {
    l->active = OFF;
    l->loctype = VIEW;
    l->gno = -1;
    l->x1 = l->y1 = l->x2 = l->y2 = 0.0;
    l->lines = grdefaults.lines;
    l->linew = grdefaults.linew;
    l->color = grdefaults.color;
    l->arrow = 0;
    l->atype = 0;
    l->asize = 1.0;
}

void set_default_box(boxtype* b) {
    b->active = OFF;
    b->loctype = VIEW;
    b->gno = -1;
    b->x1 = b->y1 = b->x2 = b->y2 = 0.0;
    b->lines = grdefaults.lines;
    b->linew = grdefaults.linew;
    b->color = grdefaults.color;
    b->fill = OFF;
    b->fillcolor = 1;
    b->fillpattern = 1;
}

void set_default_ellipse(ellipsetype* e) {
    e->active = OFF;
    e->loctype = VIEW;
    e->gno = -1;
    e->x1 = e->y1 = e->x2 = e->y2 = 0.0;
    e->lines = grdefaults.lines;
    e->linew = grdefaults.linew;
    e->color = grdefaults.color;
    e->fill = OFF;
    e->fillcolor = 1;
    e->fillpattern = 1;
}

void set_default_legend(int gno, legend* l) {
    l->active = OFF;
    l->loctype = VIEW;
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
    l->box = OFF;
    l->boxfill = OFF;
    l->boxfillusing = COLOR;
    l->boxfillcolor = 0;
    l->boxfillpat = 1;
    l->boxlcolor = grdefaults.color; /* color for symbol */
    l->boxlinew = grdefaults.linew;  /* set plot sym line width */
    l->boxlines = grdefaults.lines;  /* set plot sym line style */
}

void set_default_plotarr(plotarr* p) {
    int i;
    p->active = OFF;              /* active flag */
    p->type = XY;                 /* dataset type */
    p->deact = 0;                 /* deactivated set */
    p->hotlink = 0;               /* hot linked set */
    p->hotfile[0] = 0;            /* hot linked file name */
    p->len = 0;                   /* set length */
    p->missing = DATASET_MISSING; /* value for missing data */
    p->s = (char**)NULL;          /* pointer to strings */
    p->xmin = p->xmax = p->ymin = p->ymax = 0.0;

    p->sym = 0;                     /* set plot symbol */
    p->symchar = 0;                 /* character for symbol */
    p->symskip = 0;                 /* How many symbols to skip */
    p->symfill = 0;                 /* Symbol fill type */
    p->symdot = 0;                  /* Symbol dot in center */
    p->symlines = grdefaults.lines; /* set plot sym line style */
    p->symlinew = grdefaults.linew; /* set plot sym line width */
    p->symcolor = -1;               /* color for symbol; -1 means same color as line */
    p->symsize = 1.0;               /* size of symbols */

    p->font = grdefaults.font; /* font for strings */
    p->format = DECIMAL;       /* format for drawing values */
    p->prec = 1;               /* precision for drawing values */
    p->just = LEFT;            /* justification for drawing values */
    p->where = RIGHT;          /* where to draw values */
    p->valsize = 1.0;          /* char size for drawing values */

    p->lines = grdefaults.lines;
    p->linew = grdefaults.linew;
    p->color = grdefaults.color;
    p->lineskip = 0; /* How many points to skip when drawing lines */

    p->fill = 0;          /* fill type */
    p->fillusing = COLOR; /* fill using color or pattern */
    p->fillcolor = 1;     /* fill color */
    p->fillpattern = 0;   /* fill pattern */

    p->errbar = -1;                     /* if type is _DX, _DY, _DXDY and errbar =
                                         * TRUE */
    p->errbarxy = BOTH;                 /* type of error bar */
    p->errbar_lines = grdefaults.lines; /* error bar line width */
    p->errbar_linew = grdefaults.linew; /* error bar line style */
    p->errbar_riser = ON;               /* connecting line between error limits */
    p->errbar_riser_linew = 1;          /* connecting line between error limits line
                                         * width */
    p->errbar_riser_lines = 1;          /* connecting line between error limits line
                                         * style */

    p->errbarper = 1.0; /* length of error bar */
    p->hilowper = 1.0;  /* length of hi-low */

    p->density_plot = 0;     /* if type is XYZ then density_plot  = 1 */
    p->zmin = p->zmax = 0.0; /* min max for density plots */

    p->comments[0] = 0; /* how did this set originate */
    p->lstr[0] = 0;     /* legend */

    for (i = 0; i < MAX_SET_COLS; i++) {
        p->ex[i] = NULL;
        p->emin[i] = 0.0; /* min for each column */
        p->emax[i] = 0.0; /* max for each column */
        p->imin[i] = 0;   /* min loc for each column */
        p->imax[i] = 0;   /* max loc for each column */
    }
    p->ep = NULL; /* EditPoints pointer */
}

void set_default_velocityp(velocityp* vp) {
    vp->active = OFF;
    vp->type = 0;
    vp->loctype = VIEW;
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

void set_default_graph(int gno) {
    int i;

    g[gno].active = OFF;
    g[gno].hidden = FALSE;
    g[gno].label = OFF;
    g[gno].type = XY;
    g[gno].auto_type = AUTO;
    g[gno].autoscale = 0;
    g[gno].noautoscale = 0;
    g[gno].revx = FALSE;
    g[gno].revy = FALSE;
    g[gno].ws_top = 1;
    g[gno].ws[0].w.xg1 = g[gno].ws[0].w.xg2 = g[gno].ws[0].w.yg1 = g[gno].ws[0].w.yg2 = 0;
    g[gno].curw = 0;
    g[gno].maxplot = maxplot;
    g[gno].dsx = g[gno].dsy = 0.0; /* locator props */
    g[gno].pointset = FALSE;
    g[gno].pt_type = 0;
    g[gno].fx = GENERAL;
    g[gno].fy = GENERAL;
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

void realloc_plots(int maxplot) {
    int i, j;
    for (i = 0; i < maxgraph; i++) {
        g[i].p = (plotarr*)realloc(g[i].p, maxplot * sizeof(plotarr));
        for (j = g[i].maxplot; j < maxplot; j++) {
            g[i].p[j].len = 0;
            set_default_plotarr(&g[i].p[j]);
        }
        g[i].maxplot = maxplot;
    }
}

void realloc_graph_plots(int gno, int maxplot) {
    int j;
    g[gno].p = (plotarr*)realloc(g[gno].p, maxplot * sizeof(plotarr));
    for (j = g[gno].maxplot; j < maxplot; j++) {
        g[gno].p[j].len = 0;
        set_default_plotarr(&g[gno].p[j]);
    }
    g[gno].maxplot = maxplot;
}

void realloc_graphs(void) {
    int j;

    g = (graph*)realloc(g, maxgraph * sizeof(graph));
    for (j = MAXGRAPH; j < maxgraph; j++) {
        g[j].p = (plotarr*)calloc(maxplot, sizeof(plotarr));
        set_default_graph(j);
    }
}

void set_default_annotation(void) {
    int i;

    lines = (linetype*)malloc(maxlines * sizeof(linetype));
    boxes = (boxtype*)malloc(maxboxes * sizeof(boxtype));
    pstr = (plotstr*)malloc(maxstr * sizeof(plotstr));
    ellip = (ellipsetype*)malloc(maxellipses * sizeof(ellipsetype));

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

void set_default_ticks(tickmarks* t, int a) {
    int i;

    t->axis = a;
    switch (a) {
    case X_AXIS:
    case Y_AXIS:
        t->active = ON;
        t->alt = OFF;
        t->tl_flag = ON;
        t->t_flag = ON;
        break;
    case ZX_AXIS:
    case ZY_AXIS:
        t->active = ON;
        t->alt = OFF;
        t->tl_flag = OFF;
        t->t_flag = OFF;
        break;
    }
    set_default_string(&t->label);
    t->tmin = 0.0;
    t->tmax = 1.0;
    t->tmajor = 0.5;
    t->tminor = 0.25;
    t->offsx = 0.0;
    t->offsy = 0.0;
    t->label_layout = PARA;
    t->label_place = AUTO;
    t->tl_type = AUTO;
    t->tl_layout = HORIZONTAL;
    t->tl_loc = ON;
    t->tl_sign = NORMAL;
    t->tl_prec = 1;
    t->tl_format = DECIMAL;
    t->tl_angle = 0;
    t->tl_just = (a % 2) ? RIGHT : CENTER;
    t->tl_skip = 0;
    t->tl_staggered = 0;
    t->tl_starttype = AUTO;
    t->tl_stoptype = AUTO;
    t->tl_start = 0.0;
    t->tl_stop = 0.0;
    t->tl_op = (a % 2) ? LEFT : BOTTOM;
    t->tl_vgap = 1.0;
    t->tl_hgap = 1.0;
    t->tl_font = grdefaults.font;
    t->tl_charsize = 1.0;
    t->tl_linew = grdefaults.linew;
    t->tl_color = grdefaults.color;
    t->tl_appstr[0] = 0;
    t->tl_prestr[0] = 0;
    t->t_type = AUTO;
    t->t_mflag = ON;
    t->t_integer = OFF;
    t->t_num = 6;
    t->t_inout = IN;
    t->t_log = OFF;
    t->t_op = BOTH;
    t->t_size = 1.0;
    t->t_msize = 0.5;
    t->t_drawbar = OFF;
    t->t_drawbarcolor = grdefaults.color;
    t->t_drawbarlines = grdefaults.lines;
    t->t_drawbarlinew = grdefaults.linew;
    t->t_gridflag = OFF;
    t->t_mgridflag = OFF;
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

static int default_color[MAXPLOT] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                     1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

void setdefaultcolors(int gno) {
    int i;
    for (i = 0; i < g[gno].maxplot; i++) {
        g[gno].p[i].color = default_color[i % MAXPLOT];
    }
}

void kill_blockdata(void) {
    int j;
    if (blockdata != NULL) {
        for (j = 0; j < maxblock; j++) {
            cxfree(blockdata[j]);
        }
    }
}

void alloc_blockdata(int ncols) {
    int j;
    if (blockdata != NULL) {
        kill_blockdata();
    }
    if (ncols < MAXPLOT) {
        ncols = MAXPLOT;
    }
    blockdata = (double**)malloc(ncols * sizeof(double*));
    if (blockdata != NULL) {
        maxblock = ncols;
        for (j = 0; j < maxblock; j++) {
            blockdata[j] = NULL;
        }
    } else {
        errmsg("alloc_blockdata(): Error, unable to allocate memory for block data");
    }
}

int init_array(double** a, int n) {
    if (*a != NULL) {
        *a = (double*)realloc(*a, n * sizeof(double));
    } else {
        *a = (double*)calloc(n, sizeof(double));
    }
    return *a == NULL ? 1 : 0;
}

int init_scratch_arrays(int n) {
    if (!init_array(&ax, n)) {
        if (!init_array(&bx, n)) {
            if (!init_array(&cx, n)) {
                if (!init_array(&dx, n)) {
                    maxarr = n;
                    return 0;
                }
                free(cx);
            }
            free(bx);
        }
        free(ax);
    }
    return 1;
}
