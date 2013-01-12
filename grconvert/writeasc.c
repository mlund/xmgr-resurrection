#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "grconvert.h"

#ifdef VMS
#  include "vms_unix.h"
#endif

static void put_annotation(int gno, FILE * pp, int embed);
static void put_region(FILE * pp, int embed);

extern  int pslwincr;
extern  int pslwbegin;

int realtime = 0;
int change_gno;			/* if the graph number changes on read in */
int change_type;		/* current set type */


/*
 * write out a set
 */
int do_writesets(int gno, int setno, int embed, char *fn, char *format)
{
    int i, j, k, n, which_graph = gno, save_cg = cg, start, stop, set_start, set_stop;
    FILE *cp;
    double *x, *y, *dx, *dy, *dz, *dw;

    if (!fn[0]) {
	errmsg("Define file name first");
	return 1;
    }

    if (strcmp(fn, "-") == 0) {
        cp = stdout;
    } else {
        cp = fopen(fn, "w");
    }

    if (cp  == NULL) {
	char s[192];

	sprintf(s, "Unable to open file %s", fn);
	errmsg(s);
	return 1;
    }

    if (which_graph == maxgraph) {
	start = 0;
	stop = maxgraph - 1;
    } else if (which_graph == -1) {
	start = cg;
	stop = cg;
    } else {
	start = which_graph;
	stop = which_graph;
    }

    if (embed) {
	if (start != stop) {
	    putparms(-1, cp, embed);
	} else {
	    putparms(start, cp, embed);
	}
    }

    for (k = start; k <= stop; k++) {
	if (isactive_graph(k)) {
	    if (start != stop) {
		fprintf(cp, "@WITH G%1d\n", k);
		fprintf(cp, "@G%1d ON\n", k);
	    }
	    if (setno == -1) {
		set_start = 0;
		set_stop = g[cg].maxplot - 1;
	    } else {
		set_start = setno;
		set_stop = setno;
	    }
	    for (j = set_start; j <= set_stop; j++) {
		if (isactive_set(k, j)) {
                    fprintf(cp, "@TARGET S%d\n", j);
		    fprintf(cp, "@TYPE %s\n", set_types(dataset_type(k, j)));
		    x = getx(k, j);
		    y = gety(k, j);
		    n = getsetlength(k, j);
		    switch (dataset_type(k, j)) {
		    case XY:
			for (i = 0; i < n; i++) {
			    fprintf(cp, format, x[i], y[i]);
			    fputc('\n', cp);
			}
			break;
		    case XYDX:
		    case XYDY:
		    case XYZ:
		    case XYRT:
			dx = getcol(k, j, 2);
			for (i = 0; i < n; i++) {
			    fprintf(cp, "%g %g %g", x[i], y[i], dx[i]);
			    fputc('\n', cp);
			}
			break;
		    case POLY:
			dx = getcol(k, j, 2);
			for (i = 0; i < n; i++) {
			    fprintf(cp, "%g %g %d", x[i], y[i], (int) dx[i]);
			    fputc('\n', cp);
			}
			break;
		    case XYDXDX:
		    case XYDYDY:
		    case XYDXDY:
		    case XYUV:
			dx = getcol(k, j, 2);
			dy = getcol(k, j, 3);
			for (i = 0; i < n; i++) {
			    fprintf(cp, "%g %g %g %g", x[i], y[i], dx[i], dy[i]);
			    fputc('\n', cp);
			}
			break;
		    case XYHILO:
			dx = getcol(k, j, 2);
			dy = getcol(k, j, 3);
			dz = getcol(k, j, 4);
			for (i = 0; i < n; i++) {
			    fprintf(cp, "%g %g %g %g %g", x[i], y[i], dx[i], dy[i], dz[i]);
			    fputc('\n', cp);
			}
			break;
		    case XYBOX:
			dx = getcol(k, j, 2);
			dy = getcol(k, j, 3);
			dz = getcol(k, j, 4);
			for (i = 0; i < n; i++) {
			    fprintf(cp, "%g %g %g %g %d", x[i], y[i], dx[i], dy[i], (int) dz[i]);
			    fputc('\n', cp);
			}
			break;
		    case XYBOXPLOT:
			dx = getcol(k, j, 2);
			dy = getcol(k, j, 3);
			dz = getcol(k, j, 4);
			dw = getcol(k, j, 5);
			for (i = 0; i < n; i++) {
			    fprintf(cp, "%g %g %g %g %g %g", x[i], y[i], dx[i], dy[i], dz[i], dw[i]);
			    fputc('\n', cp);
			}
			break;
		    }
		    fprintf(cp, "&\n");
		}
	    }
	}
    }
    fclose(cp);
    cg = save_cg;
    return 0;
}

void putparms(int gno, FILE * pp, int embed)
{
    int i, j, k, ming, maxg;
    int ps, pt, gh, gl, gt, fx, fy, px, py;
    double dsx, dsy;
    char embedstr[2], tmpstr1[128], tmpstr2[128];
    framep f;
    legend leg;
    labels lab;
    plotarr p;
    tickmarks t;
    world w;
    view v;
    char *p1, *p2, *tmpbuf;

    if (embed) {
	strcpy(embedstr, "@");
    } else {
	embedstr[0] = 0;
    }
    fprintf(pp, "# ACE/gr parameter file\n");
    fprintf(pp, "#\n");
    fprintf(pp, "# (Converted by grconvert)\n");

    fprintf(pp, "%sversion 40001\n", embedstr);

    /* Print some global variables. Added by Henrik Seidel Tue Jun 10 16:47:14 MET DST 1997 */
    switch (page_layout) {
        case FREE:       strcpy(tmpstr1, "free"); break;
        case LANDSCAPE:  strcpy(tmpstr1, "landscape"); break;
        case PORTRAIT:   strcpy(tmpstr1, "portrait"); break;
        case FIXED:      strcpy(tmpstr1, "fixed"); break;
        default:         strcpy(tmpstr1, "free"); break;
    }
    fprintf(pp, "%spage layout %s\n", embedstr, tmpstr1);
    fprintf(pp, "%sps linewidth begin %d\n", embedstr, pslwbegin);
    fprintf(pp, "%sps linewidth increment %d\n", embedstr, pslwincr);
    fprintf(pp, "%shardcopy device %d\n", embedstr, hdevice);
    if (*description) {
        tmpbuf = (char *) malloc ((size_t) (strlen (description) + 1));
        if (tmpbuf == NULL) {
            errmsg("Error: Unable to malloc temporary in putparms()");
            return;
        }
        strcpy (tmpbuf, description);
        p1 = tmpbuf;
        while ((p2 = strchr (p1, '\n')) != NULL) {
            *p2 = 0;
            fprintf (pp, "%sdescription \"%s\"\n", embedstr, escapequotes(p1));
            *p2 = '\n';
            p1 = p2;
            p1++;
        }
        if (*p1)
            fprintf (pp, "%sdescription \"%s\"\n", embedstr, escapequotes(p1));
        free (tmpbuf);
    }
    /* End of added globals */

    fprintf(pp, "%spage %d\n", embedstr, (int) (scrollper * 100));
    fprintf(pp, "%spage inout %d\n", embedstr, (int) (shexper * 100));
    fprintf(pp, "%slink page %s\n", embedstr, scrolling_islinked ? "on" : "off");

    fprintf(pp, "%sdefault linestyle %d\n", embedstr, grdefaults.lines);
    fprintf(pp, "%sdefault linewidth %d\n", embedstr, grdefaults.linew);
    fprintf(pp, "%sdefault color %d\n", embedstr, grdefaults.color);
    fprintf(pp, "%sdefault char size %f\n", embedstr, grdefaults.charsize);
    fprintf(pp, "%sdefault font %d\n", embedstr, grdefaults.font);
    fprintf(pp, "%sdefault font source %d\n", embedstr, grdefaults.fontsrc);
    fprintf(pp, "%sdefault symbol size %f\n", embedstr, grdefaults.symsize);
    put_annotation(gno, pp, embed);
    put_region(pp, embed);
    if (gno == -1) {
	maxg = maxgraph - 1;
	ming = 0;
    } else {
	maxg = gno;
	ming = gno;
    }
    for (k = ming; k <= maxg; k++) {
	if (isactive_graph(k)) {
	    gno = k;
	    gh = g[gno].hidden;
	    gl = g[gno].label;
	    gt = g[gno].type;
	    ps = g[gno].pointset;
	    pt = g[gno].pt_type;
	    dsx = g[gno].dsx;
	    dsy = g[gno].dsy;
	    fx = g[gno].fx;
	    fy = g[gno].fy;
	    px = g[gno].px;
	    py = g[gno].py;

	    fprintf(pp, "%swith g%1d\n", embedstr, gno);

	    fprintf(pp, "%sg%1d %s\n", embedstr, gno, on_or_off(g[gno].active));
	    fprintf(pp, "%sg%1d label %s\n", embedstr, gno, on_or_off(gl));
	    fprintf(pp, "%sg%1d hidden %s\n", embedstr, gno, gh ? "true" : "false");
	    fprintf(pp, "%sg%1d type %s\n", embedstr, gno, graph_types(g[gno].type, 1));
	    fprintf(pp, "%sg%1d autoscale type %s\n", embedstr, gno, g[gno].auto_type == AUTO ? "AUTO" : "SPEC");
	    fprintf(pp, "%sg%1d fixedpoint %s\n", embedstr, gno, on_or_off(ps));
	    fprintf(pp, "%sg%1d fixedpoint type %d\n", embedstr, gno, pt);
	    fprintf(pp, "%sg%1d fixedpoint xy %f, %f\n", embedstr, gno, dsx, dsy);
	    strcpy(tmpstr1, get_format_types(fx));
	    strcpy(tmpstr2, get_format_types(fy));
	    fprintf(pp, "%sg%1d fixedpoint format %s %s\n", embedstr, gno, tmpstr1, tmpstr2);
	    fprintf(pp, "%sg%1d fixedpoint prec %d, %d\n", embedstr, gno, px, py);

	    get_graph_world(gno, &w);
	    fprintf(pp, "%s    world xmin %.12g\n", embedstr, w.xg1);
	    fprintf(pp, "%s    world xmax %.12g\n", embedstr, w.xg2);
	    fprintf(pp, "%s    world ymin %.12g\n", embedstr, w.yg1);
	    fprintf(pp, "%s    world ymax %.12g\n", embedstr, w.yg2);

	    for (i = 0; i < g[gno].ws_top; i++) {
		fprintf(pp, "%s    stack world %.9g, %.9g, %.9g, %.9g tick %g, %g, %g, %g\n", embedstr,
			g[gno].ws[i].w.xg1, g[gno].ws[i].w.xg2, g[gno].ws[i].w.yg1, g[gno].ws[i].w.yg2,
			g[gno].ws[i].t[0].xg1, g[gno].ws[i].t[0].xg2, g[gno].ws[i].t[0].yg1, g[gno].ws[i].t[0].yg2);
	    }

	    get_graph_view(gno, &v);
	    fprintf(pp, "%s    view xmin %f\n", embedstr, v.xv1);
	    fprintf(pp, "%s    view xmax %f\n", embedstr, v.xv2);
	    fprintf(pp, "%s    view ymin %f\n", embedstr, v.yv1);
	    fprintf(pp, "%s    view ymax %f\n", embedstr, v.yv2);

	    get_graph_labels(gno, &lab);
	    fprintf(pp, "%s    title \"%s\"\n", embedstr, escapequotes(lab.title.s));
	    fprintf(pp, "%s    title font %d\n", embedstr, lab.title.font);
	    fprintf(pp, "%s    title size %f\n", embedstr, lab.title.charsize);
	    fprintf(pp, "%s    title color %d\n", embedstr, lab.title.color);
	    fprintf(pp, "%s    title linewidth %d\n", embedstr, lab.title.linew);
	    fprintf(pp, "%s    subtitle \"%s\"\n", embedstr, escapequotes(lab.stitle.s));
	    fprintf(pp, "%s    subtitle font %d\n", embedstr, lab.stitle.font);
	    fprintf(pp, "%s    subtitle size %f\n", embedstr, lab.stitle.charsize);
	    fprintf(pp, "%s    subtitle color %d\n", embedstr, lab.stitle.color);
	    fprintf(pp, "%s    subtitle linewidth %d\n", embedstr, lab.stitle.linew);

	    for (i = 0; i < g[gno].maxplot; i++) {
		get_graph_plotarr(gno, i, &p);
		if (isactive_set(gno, i)) {
/* setting the type here causes big problems
		    fprintf(pp, "%s    s%1d type %s\n", embedstr, i, (char *) set_types(p.type));
*/
		    fprintf(pp, "%s    s%1d symbol %d\n", embedstr, i, p.sym);
		    fprintf(pp, "%s    s%1d symbol size %f\n", embedstr, i, p.symsize);
		    fprintf(pp, "%s    s%1d symbol fill %d\n", embedstr, i, p.symfill);
		    fprintf(pp, "%s    s%1d symbol color %d\n", embedstr, i, p.symcolor);
		    fprintf(pp, "%s    s%1d symbol linewidth %d\n", embedstr, i, p.symlinew);
		    fprintf(pp, "%s    s%1d symbol linestyle %d\n", embedstr, i, p.symlines);
		    fprintf(pp, "%s    s%1d symbol center %s\n", embedstr, i, p.symdot ? "true" : "false");
		    fprintf(pp, "%s    s%1d symbol char %d\n", embedstr, i, p.symchar);
		    fprintf(pp, "%s    s%1d skip %d\n", embedstr, i, p.symskip);
		    fprintf(pp, "%s    s%1d linestyle %d\n", embedstr, i, p.lines);
		    fprintf(pp, "%s    s%1d linewidth %d\n", embedstr, i, p.linew);
		    fprintf(pp, "%s    s%1d color %d\n", embedstr, i, p.color);
		    fprintf(pp, "%s    s%1d fill %d\n", embedstr, i, p.fill);
		    fprintf(pp, "%s    s%1d fill with %s\n", embedstr, i,
			    p.fillusing == COLOR ? "color" : "pattern");
		    fprintf(pp, "%s    s%1d fill color %d\n", embedstr, i, p.fillcolor);
		    fprintf(pp, "%s    s%1d fill pattern %d\n", embedstr, i, p.fillpattern);
		    switch (p.errbarxy) {
		    case TOP:
			fprintf(pp, "%s    s%1d errorbar type TOP\n", embedstr, i);
			break;
		    case BOTTOM:
			fprintf(pp, "%s    s%1d errorbar type BOTTOM\n", embedstr, i);
			break;
		    case LEFT:
			fprintf(pp, "%s    s%1d errorbar type LEFT\n", embedstr, i);
			break;
		    case RIGHT:
			fprintf(pp, "%s    s%1d errorbar type RIGHT\n", embedstr, i);
			break;
		    case BOTH:
			fprintf(pp, "%s    s%1d errorbar type BOTH\n", embedstr, i);
			break;
		    }
		    fprintf(pp, "%s    s%1d errorbar length %f\n", embedstr, i, p.errbarper);
		    fprintf(pp, "%s    s%1d errorbar linewidth %d\n", embedstr, i, p.errbar_linew);
		    fprintf(pp, "%s    s%1d errorbar linestyle %d\n", embedstr, i, p.errbar_lines);
		    fprintf(pp, "%s    s%1d errorbar riser %s\n", embedstr, i, p.errbar_riser == ON ? "on" : "off");
		    fprintf(pp, "%s    s%1d errorbar riser linewidth %d\n", embedstr, i, p.errbar_riser_linew);
		    fprintf(pp, "%s    s%1d errorbar riser linestyle %d\n", embedstr, i, p.errbar_riser_lines);
		    fprintf(pp, "%s    s%1d xyz %f, %f\n", embedstr, i, p.zmin, p.zmax);
		    if (is_hotlinked(gno, i)) {
			fprintf(pp, "%s    s%1d link %s \"%s\"\n", embedstr, i,
			  p.hotsrc == DISK ? "disk" : "pipe", p.hotfile);
		    }
		    fprintf(pp, "%s    s%1d comment \"%s\"\n", embedstr, i, p.comments);
		}
	    }

	    for (i = 0; i < MAXAXES; i++) {
		switch (i) {
		case 0:
		    get_graph_tickmarks(gno, &t, X_AXIS);
		    if (t.active == OFF) {
			fprintf(pp, "%s    xaxis off\n", embedstr);
			continue;
		    }
		    sprintf(buf, "%s    xaxis ", embedstr);
		    break;
		case 1:
		    get_graph_tickmarks(gno, &t, Y_AXIS);
		    if (t.active == OFF) {
			fprintf(pp, "%s    yaxis off\n", embedstr);
			continue;
		    }
		    sprintf(buf, "%s    yaxis ", embedstr);
		    break;
		case 2:
		    get_graph_tickmarks(gno, &t, ZX_AXIS);
		    if (t.active == OFF) {
			fprintf(pp, "%s    zeroxaxis off\n", embedstr);
			continue;
		    }
		    sprintf(buf, "%s    zeroxaxis ", embedstr);
		    break;
		case 3:
		    get_graph_tickmarks(gno, &t, ZY_AXIS);
		    if (t.active == OFF) {
			fprintf(pp, "%s    zeroyaxis off\n", embedstr);
			continue;
		    }
		    sprintf(buf, "%s    zeroyaxis ", embedstr);
		    break;
		}

		fprintf(pp, "%s tick %s\n", buf, on_or_off(t.active));
		fprintf(pp, "%s tick major %.12g\n", buf, t.tmajor);
		fprintf(pp, "%s tick minor %.12g\n", buf, t.tminor);
		fprintf(pp, "%s tick offsetx %f\n", buf, t.offsx);
		fprintf(pp, "%s tick offsety %f\n", buf, t.offsy);
/* DEFUNCT
		fprintf(pp, "%s tick alt %s\n", buf, on_or_off(t.alt));
		fprintf(pp, "%s tick min %.12g\n", buf, t.tmin);
		fprintf(pp, "%s tick max %.12g\n", buf, t.tmax);
*/

		fprintf(pp, "%s label \"%s\"\n", buf, t.label.s);
		if (t.label_layout == PERP) {
		    fprintf(pp, "%s label layout perp\n", buf);
		} else {
		    fprintf(pp, "%s label layout para\n", buf);
		}
		if (t.label_place == AUTO) {
		    fprintf(pp, "%s label place auto\n", buf);
		} else {
		    fprintf(pp, "%s label place spec\n", buf);
		    fprintf(pp, "%s label place %f, %f\n", buf, t.label.x, t.label.y);
		}
		fprintf(pp, "%s label char size %f\n", buf, t.label.charsize);
		fprintf(pp, "%s label font %d\n", buf, t.label.font);
		fprintf(pp, "%s label color %d\n", buf, t.label.color);
		fprintf(pp, "%s label linewidth %d\n", buf, t.label.linew);

		fprintf(pp, "%s ticklabel %s\n", buf, on_or_off(t.tl_flag));
		if (t.tl_type == AUTO) {
		    fprintf(pp, "%s ticklabel type auto\n", buf);
		} else {
		    fprintf(pp, "%s ticklabel type spec\n", buf);
		}
		fprintf(pp, "%s ticklabel prec %d\n", buf, t.tl_prec);
		fprintf(pp, "%s ticklabel format %s\n", buf, get_format_types(t.tl_format));
		fprintf(pp, "%s ticklabel append \"%s\"\n", buf, t.tl_appstr);
		fprintf(pp, "%s ticklabel prepend \"%s\"\n", buf, t.tl_prestr);
		switch (t.tl_layout) {
		case HORIZONTAL:
		    fprintf(pp, "%s ticklabel layout horizontal\n", buf);
		    break;
		case VERTICAL:
		    fprintf(pp, "%s ticklabel layout vertical\n", buf);
		    break;
		case SPEC:
		    fprintf(pp, "%s ticklabel layout spec\n", buf);
		    fprintf(pp, "%s ticklabel angle %d\n", buf, t.tl_angle);
		    break;
		}
		fprintf(pp, "%s ticklabel skip %d\n", buf, t.tl_skip);
		fprintf(pp, "%s ticklabel stagger %d\n", buf, t.tl_staggered);
		switch (t.tl_op) {
		case TOP:
		    fprintf(pp, "%s ticklabel op top\n", buf);
		    break;
		case BOTTOM:
		    fprintf(pp, "%s ticklabel op bottom\n", buf);
		    break;
		case LEFT:
		    fprintf(pp, "%s ticklabel op left\n", buf);
		    break;
		case RIGHT:
		    fprintf(pp, "%s ticklabel op right\n", buf);
		    break;
		case BOTH:
		    fprintf(pp, "%s ticklabel op both\n", buf);
		    break;
		}
		switch (t.tl_sign) {
		case NORMAL:
		    fprintf(pp, "%s ticklabel sign normal\n", buf);
		    break;
		case ABSOLUTE:
		    fprintf(pp, "%s ticklabel sign absolute\n", buf);
		    break;
		case NEGATE:
		    fprintf(pp, "%s ticklabel sign negate\n", buf);
		    break;
		}
		fprintf(pp, "%s ticklabel start type %s\n", buf, t.tl_starttype == AUTO ? "auto" : "spec");
		fprintf(pp, "%s ticklabel start %f\n", buf, t.tl_start);
		fprintf(pp, "%s ticklabel stop type %s\n", buf, t.tl_stoptype == AUTO ? "auto" : "spec");
		fprintf(pp, "%s ticklabel stop %f\n", buf, t.tl_stop);
		fprintf(pp, "%s ticklabel char size %f\n", buf, t.tl_charsize);
		fprintf(pp, "%s ticklabel font %d\n", buf, t.tl_font);
		fprintf(pp, "%s ticklabel color %d\n", buf, t.tl_color);
		fprintf(pp, "%s ticklabel linewidth %d\n", buf, t.tl_linew);

		fprintf(pp, "%s tick major %s\n", buf, on_or_off(t.t_flag));
		fprintf(pp, "%s tick minor %s\n", buf, on_or_off(t.t_mflag));
		fprintf(pp, "%s tick default %d\n", buf, t.t_num);
		switch (t.t_inout) {
		case IN:
		    fprintf(pp, "%s tick in\n", buf);
		    break;
		case OUT:
		    fprintf(pp, "%s tick out\n", buf);
		    break;
		case BOTH:
		    fprintf(pp, "%s tick both\n", buf);
		    break;
		}
		fprintf(pp, "%s tick major color %d\n", buf, t.t_color);
		fprintf(pp, "%s tick major linewidth %d\n", buf, t.t_linew);
		fprintf(pp, "%s tick major linestyle %d\n", buf, t.t_lines);
		fprintf(pp, "%s tick minor color %d\n", buf, t.t_mcolor);
		fprintf(pp, "%s tick minor linewidth %d\n", buf, t.t_mlinew);
		fprintf(pp, "%s tick minor linestyle %d\n", buf, t.t_mlines);
		fprintf(pp, "%s tick log %s\n", buf, on_or_off(t.t_log));
		fprintf(pp, "%s tick size %f\n", buf, t.t_size);
		fprintf(pp, "%s tick minor size %f\n", buf, t.t_msize);
		fprintf(pp, "%s bar %s\n", buf, on_or_off(t.t_drawbar));
		fprintf(pp, "%s bar color %d\n", buf, t.t_drawbarcolor);
		fprintf(pp, "%s bar linestyle %d\n", buf, t.t_drawbarlines);
		fprintf(pp, "%s bar linewidth %d\n", buf, t.t_drawbarlinew);
		fprintf(pp, "%s tick major grid %s\n", buf, on_or_off(t.t_gridflag));
		fprintf(pp, "%s tick minor grid %s\n", buf, on_or_off(t.t_mgridflag));
		switch (t.t_op) {
		case TOP:
		    fprintf(pp, "%s tick op top\n", buf);
		    break;
		case BOTTOM:
		    fprintf(pp, "%s tick op bottom\n", buf);
		    break;
		case LEFT:
		    fprintf(pp, "%s tick op left\n", buf);
		    break;
		case RIGHT:
		    fprintf(pp, "%s tick op right\n", buf);
		    break;
		case BOTH:
		    fprintf(pp, "%s tick op both\n", buf);
		    break;
		}
		if (t.t_type == AUTO) {
		    fprintf(pp, "%s tick type auto\n", buf);
		} else {
		    fprintf(pp, "%s tick type spec\n", buf);
		}
		fprintf(pp, "%s tick spec %d\n", buf, t.t_spec);
		for (j = 0; j < t.t_spec; j++) {
		    fprintf(pp, "%s tick %d, %g\n", buf, j, t.t_specloc[j]);
		    fprintf(pp, "%s ticklabel %d, \"%s\"\n", buf, j, t.t_speclab[j].s);
		}
	    }

	    get_graph_legend(gno, &leg);
	    fprintf(pp, "%s    legend %s\n", embedstr, on_or_off(leg.active));
	    fprintf(pp, "%s    legend loctype %s\n", embedstr, w_or_v(leg.loctype));
	    fprintf(pp, "%s    legend layout %d\n", embedstr, leg.layout);
	    fprintf(pp, "%s    legend vgap %d\n", embedstr, leg.vgap);
	    fprintf(pp, "%s    legend hgap %d\n", embedstr, leg.hgap);
	    fprintf(pp, "%s    legend length %d\n", embedstr, leg.len);
	    fprintf(pp, "%s    legend box %s\n", embedstr, on_or_off(leg.box));
	    fprintf(pp, "%s    legend box fill %s\n", embedstr, on_or_off(leg.box));
	    fprintf(pp, "%s    legend box fill with %s\n", embedstr, leg.boxfillusing == COLOR ? "color" : "pattern");
	    fprintf(pp, "%s    legend box fill color %d\n", embedstr, leg.boxfillcolor);
	    fprintf(pp, "%s    legend box fill pattern %d\n", embedstr, leg.boxfillpat);
	    fprintf(pp, "%s    legend box color %d\n", embedstr, leg.boxlcolor);
	    fprintf(pp, "%s    legend box linewidth %d\n", embedstr, leg.boxlinew);
	    fprintf(pp, "%s    legend box linestyle %d\n", embedstr, leg.boxlines);
	    fprintf(pp, "%s    legend x1 %.12g\n", embedstr, leg.legx);
	    fprintf(pp, "%s    legend y1 %.12g\n", embedstr, leg.legy);
	    fprintf(pp, "%s    legend font %d\n", embedstr, leg.font);
	    fprintf(pp, "%s    legend char size %f\n", embedstr, leg.charsize);
	    fprintf(pp, "%s    legend linestyle %d\n", embedstr, leg.lines);
	    fprintf(pp, "%s    legend linewidth %d\n", embedstr, leg.linew);
	    fprintf(pp, "%s    legend color %d\n", embedstr, leg.color);
	    for (i = 0; i < maxplot; i++) {
		if (isactive_set(gno, i)) {
		    if (strlen(g[gno].p[i].lstr)) {
			fprintf(pp, "%s    legend string %d \"%s\"\n", 
				embedstr, i, escapequotes(g[gno].p[i].lstr));
		    }
		}
	    }

	    get_graph_framep(gno, &f);
	    fprintf(pp, "%s    frame %s\n", embedstr, on_or_off(f.active));
	    fprintf(pp, "%s    frame type %d\n", embedstr, f.type);
	    fprintf(pp, "%s    frame linestyle %d\n", embedstr, f.lines);
	    fprintf(pp, "%s    frame linewidth %d\n", embedstr, f.linew);
	    fprintf(pp, "%s    frame color %d\n", embedstr, f.color);
	    fprintf(pp, "%s    frame fill %s\n", embedstr, on_or_off(f.fillbg));
	    fprintf(pp, "%s    frame background color %d\n", embedstr, f.bgcolor);
	}
    }
}

static void put_annotation(int gno, FILE * pp, int embed)
{
    int i;
    boxtype b;
    linetype l;
    ellipsetype e;
    plotstr s;
    char embedstr[2];

    if (embed) {
	strcpy(embedstr, "@");
    } else {
	embedstr[0] = 0;
    }
    for (i = 0; i < maxboxes; i++) {
	get_graph_box(i, &b);
	if (b.active == ON) {
	    fprintf(pp, "%swith box\n", embedstr);
	    fprintf(pp, "%s    box on\n", embedstr);
	    fprintf(pp, "%s    box loctype %s\n", embedstr, w_or_v(b.loctype));
	    if (b.loctype == WORLD) {
		fprintf(pp, "%s    box g%1d\n", embedstr, b.gno);
	    }
	    fprintf(pp, "%s    box %.12g, %.12g, %.12g, %.12g\n", embedstr, b.x1, b.y1, b.x2, b.y2);
	    fprintf(pp, "%s    box linestyle %d\n", embedstr, b.lines);
	    fprintf(pp, "%s    box linewidth %d\n", embedstr, b.linew);
	    fprintf(pp, "%s    box color %d\n", embedstr, b.color);
	    switch (b.fill) {
	    case NONE:
		fprintf(pp, "%s    box fill none\n", embedstr);
		break;
	    case COLOR:
		fprintf(pp, "%s    box fill color\n", embedstr);
		break;
	    case PATTERN:
		fprintf(pp, "%s    box fill pattern\n", embedstr);
		break;
	    }
	    fprintf(pp, "%s    box fill color %d\n", embedstr, b.fillcolor);
	    fprintf(pp, "%s    box fill pattern %d\n", embedstr, b.fillpattern);
	    fprintf(pp, "%sbox def\n", embedstr);
	}
    }

    for (i = 0; i < maxellipses; i++) {
	get_graph_ellipse(i, &e);
	if (e.active == ON) {
	    fprintf(pp, "%swith ellipse\n", embedstr);
	    fprintf(pp, "%s    ellipse on\n", embedstr);
	    fprintf(pp, "%s    ellipse loctype %s\n", embedstr, w_or_v(e.loctype));
	    if (e.loctype == WORLD) {
		fprintf(pp, "%s    ellipse g%1d\n", embedstr, e.gno);
	    }
	    fprintf(pp, "%s    ellipse %.12g, %.12g, %.12g, %.12g\n", embedstr, e.x1, e.y1, e.x2, e.y2);
	    fprintf(pp, "%s    ellipse linestyle %d\n", embedstr, e.lines);
	    fprintf(pp, "%s    ellipse linewidth %d\n", embedstr, e.linew);
	    fprintf(pp, "%s    ellipse color %d\n", embedstr, e.color);
	    switch (e.fill) {
	    case NONE:
		fprintf(pp, "%s    ellipse fill none\n", embedstr);
		break;
	    case COLOR:
		fprintf(pp, "%s    ellipse fill color\n", embedstr);
		break;
	    case PATTERN:
		fprintf(pp, "%s    ellipse fill pattern\n", embedstr);
		break;
	    }
	    fprintf(pp, "%s    ellipse fill color %d\n", embedstr, e.fillcolor);
	    fprintf(pp, "%s    ellipse fill pattern %d\n", embedstr, e.fillpattern);
	    fprintf(pp, "%sellipse def\n", embedstr);
	}
    }
   for (i = 0; i < maxlines; i++) {
	get_graph_line(i, &l);
	if (l.active == ON) {
	    fprintf(pp, "%swith line\n", embedstr);
	    fprintf(pp, "%s    line on\n", embedstr);
	    fprintf(pp, "%s    line loctype %s\n", embedstr, w_or_v(l.loctype));
	    if (l.loctype == WORLD) {
		fprintf(pp, "%s    line g%1d\n", embedstr, l.gno);
	    }
	    fprintf(pp, "%s    line %.12g, %.12g, %.12g, %.12g\n", embedstr, l.x1, l.y1, l.x2, l.y2);
	    fprintf(pp, "%s    line linewidth %d\n", embedstr, l.linew);
	    fprintf(pp, "%s    line linestyle %d\n", embedstr, l.lines);
	    fprintf(pp, "%s    line color %d\n", embedstr, l.color);
	    fprintf(pp, "%s    line arrow %d\n", embedstr, l.arrow);
	    fprintf(pp, "%s    line arrow size %f\n", embedstr, l.asize);
	    fprintf(pp, "%s    line arrow type %d\n", embedstr, l.atype);
	    fprintf(pp, "%sline def\n", embedstr);
	}
    }

    for (i = 0; i < maxstr; i++) {
	get_graph_string(i, &s);
	if (s.active == ON && s.s[0]) {
	    fprintf(pp, "%swith string\n", embedstr);
	    fprintf(pp, "%s    string on\n", embedstr);
	    fprintf(pp, "%s    string loctype %s\n", embedstr, w_or_v(s.loctype));
	    if (s.loctype == WORLD) {
		fprintf(pp, "%s    string g%1d\n", embedstr, s.gno);
	    }
	    fprintf(pp, "%s    string %.12g, %.12g\n", embedstr, s.x, s.y);
	    fprintf(pp, "%s    string linewidth %d\n", embedstr, s.linew);
	    fprintf(pp, "%s    string color %d\n", embedstr, s.color);
	    fprintf(pp, "%s    string rot %d\n", embedstr, s.rot);
	    fprintf(pp, "%s    string font %d\n", embedstr, s.font);
	    fprintf(pp, "%s    string just %d\n", embedstr, s.just);
	    fprintf(pp, "%s    string char size %f\n", embedstr, s.charsize);
	    fprintf(pp, "%sstring def \"%s\"\n", embedstr, escapequotes(s.s));
	}
    }
}

static void put_region(FILE * pp, int embed)
{
    int i, j;
    char embedstr[2];

    if (embed) {
	strcpy(embedstr, "@");
    } else {
	embedstr[0] = 0;
    }
    for (i = 0; i < MAXREGION; i++) {
	if (rg[i].active == ON) {
	    fprintf(pp, "%sr%1d ON\n", embedstr, i);
	    switch (rg[i].type) {
	    case ABOVE:
		fprintf(pp, "%sr%1d type above\n", embedstr, i);
		break;
	    case BELOW:
		fprintf(pp, "%sr%1d type below\n", embedstr, i);
		break;
	    case LEFT:
		fprintf(pp, "%sr%1d type left\n", embedstr, i);
		break;
	    case RIGHT:
		fprintf(pp, "%sr%1d type right\n", embedstr, i);
		break;
	    case POLYI:
		fprintf(pp, "%sr%1d type polyi\n", embedstr, i);
		break;
	    case POLYO:
		fprintf(pp, "%sr%1d type polyo\n", embedstr, i);
		break;
	    }
	    fprintf(pp, "%sr%1d linestyle %d\n", embedstr, i, rg[i].lines);
	    fprintf(pp, "%sr%1d linewidth %d\n", embedstr, i, rg[i].linew);
	    fprintf(pp, "%sr%1d color %d\n", embedstr, i, rg[i].color);
	    if (rg[i].type != POLYI && rg[i].type != POLYO) {
		fprintf(pp, "%sr%1d line %.12g, %.12g, %.12g, %.12g\n", embedstr, i, rg[i].x1, rg[i].y1, rg[i].x2, rg[i].y2);
	    } else {
		if (rg[i].x != NULL) {
		    for (j = 0; j < rg[i].n; j++) {
			fprintf(pp, "%sr%1d xy %.12g, %.12g\n", embedstr, i, rg[i].x[j], rg[i].y[j]);
		    }
		}
	    }
	    for (j = 0; j < maxgraph; j++) {
		if (rg[i].linkto[j] == TRUE) {
		    fprintf(pp, "%slink r%1d to g%1d\n", embedstr, i, j);
		}
	    }
	}
    }
}

