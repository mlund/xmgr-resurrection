/* $Id: drawticks.c,v 1.5 1995/06/08 01:37:21 pturner Exp pturner $
 *
 * Draw ticks and tick labels
 */

#include <config.h>
#include <cmath.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "externs.h"
#include "draw.h"
#include "protos.h"

extern double devcharsize;

void drawaxes(int gno) {
    int i;

    for (i = 0; i < MAXAXES; i++) {
        if (g[gno].t[i].active) {
            if (g[gno].t[i].t_drawbar) {
                if (i % 2 == 0) {
                    drawxaxisbar(gno, i);
                } else {
                    drawyaxisbar(gno, i);
                }
            }
            if (g[gno].t[i].t_flag) {
                if (i % 2 == 0) {
                    drawxtickmarks(gno, i);
                } else {
                    drawytickmarks(gno, i);
                }
            }
            if (g[gno].t[i].tl_flag) {
                if (i % 2 == 0) {
                    drawxticklabels(gno, i);
                } else {
                    drawyticklabels(gno, i);
                }
            }
        }
    }
}

void drawxaxisbar(int gno, int caxis) {
    tickmarks t;
    world w;
    view v;
    double top, bot;
    double delb = 0.0, delt = 0.0, vyb, vyt, vx, ofx, ofb, oft;
    double start, stop;

    get_graph_tickmarks(gno, &t, caxis);
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);

    /* set start and end for axis bar */
    start = w.xg1;
    stop = w.xg2;
    if (caxis == ZX_AXIS) {
        top = 0.0;
        bot = 0.0;
    } else {
        if (isyreversed(gno)) {
            top = w.yg1;
            bot = w.yg2;
        } else {
            top = w.yg2;
            bot = w.yg1;
        }
    }

    /* if offsx or offsy != 0.0 then adjust */
    vyb = v.yv1;
    vyt = v.yv2;
    vx = v.xv1;
    delb = 0.0;
    delt = 0.0;
    if (fabs(t.offsx) > 0.001 && fabs(t.offsx) < 1.0) {
        if (isyreversed(gno)) {
            vyb = v.yv2 - t.offsx;
        } else {
            vyb = v.yv1 - t.offsx;
        }
        view2world(vx, vyb, &ofx, &ofb);
        delb = bot - ofb;
    }
    if (fabs(t.offsy) > 0.001 && fabs(t.offsy) < 1.0) {
        if (isyreversed(gno)) {
            vyt = v.yv1 + t.offsy;
        } else {
            vyt = v.yv2 + t.offsy;
        }
        view2world(vx, vyt, &ofx, &oft);
        delt = oft - top;
    }
    setcolor(t.t_drawbarcolor);
    setlinestyle(t.t_drawbarlines);
    setlinewidth(t.t_drawbarlinew);
    setclipping(0);
    if (t.t_op == PLACE_BOTTOM || t.t_op == PLACE_BOTH) {
        my_move2(start, bot - delb);
        my_draw2(stop, bot - delb);
    }
    if (t.t_op == PLACE_TOP || t.t_op == PLACE_BOTH) {
        my_move2(start, top + delt);
        my_draw2(stop, top + delt);
    }
    setclipping(1);
}

void drawyaxisbar(int gno, int caxis) {
    tickmarks t;
    world w;
    view v;
    double top, bot;
    double delr = 0.0, dell = 0.0, vxl, vxr, vy, ofy, ofr, ofl;
    double start, stop;

    get_graph_tickmarks(gno, &t, caxis);
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);

    if (isyreversed(gno)) {
        start = w.yg2;
        stop = w.yg1;
    } else {
        start = w.yg1;
        stop = w.yg2;
    }
    if (caxis == ZY_AXIS) {
        top = 0.0;
        bot = 0.0;
    } else {
        if (isxreversed(gno)) {
            top = w.xg1;
            bot = w.xg2;
        } else {
            top = w.xg2;
            bot = w.xg1;
        }
    }

    /* if offsx or offsy != 0.0 then adjust */
    vxl = v.xv1;
    vxr = v.xv2;
    vy = v.yv1;
    dell = 0.0;
    delr = 0.0;
    /* left offset */
    if (fabs(t.offsx) > 0.001 && fabs(t.offsx) < 1.0) {
        if (isxreversed(gno)) {
            vxl = v.xv2 - t.offsx;
        } else {
            vxl = v.xv1 - t.offsx;
        }
        view2world(vxl, vy, &ofl, &ofy);
        dell = bot - ofl;
    }
    /* right offset */
    if (fabs(t.offsy) > 0.001 && fabs(t.offsy) < 1.0) {
        if (isxreversed(gno)) {
            vxr = v.xv1 + t.offsy;
        } else {
            vxr = v.xv2 + t.offsy;
        }
        view2world(vxr, vy, &ofr, &ofy);
        delr = ofr - top;
    }
    setcolor(t.t_drawbarcolor);
    setlinestyle(t.t_drawbarlines);
    setlinewidth(t.t_drawbarlinew);
    setclipping(0);
    if (t.t_op == PLACE_LEFT || t.t_op == PLACE_BOTH) {
        my_move2(bot - dell, start);
        my_draw2(bot - dell, stop);
    }
    if (t.t_op == PLACE_RIGHT || t.t_op == PLACE_BOTH) {
        my_move2(top + delr, start);
        my_draw2(top + delr, stop);
    }
    setclipping(1);
}

/* create format string */
void create_ticklabel(int form, int prec, double loc, char* s) {
    char format[256];
    double tmp;
    int m, d, y, h, mm;
    double sec;
    int itmp, isign = 0;
    extern char* months[];
    extern char* monthl[];
    extern char* dayofweekstrs[];
    extern char* dayofweekstrl[];

    strcpy(format, "%.*lf");
    switch (form) {
    case FORMAT_DECIMAL:
        sprintf(s, format, prec, loc);
        tmp = atof(s); /* fix reverse axes problem when loc == -0.0 */
        if (tmp == 0.0) {
            strcpy(format, "%.*lf");
            loc = 0.0;
            sprintf(s, format, prec, loc);
        }
        break;
    case FORMAT_EXPONENTIAL:
        strcpy(format, "%.*le");
        sprintf(s, format, prec, loc);
        tmp = atof(s); /* fix reverse axes problem when loc == -0.0 */
        if (tmp == 0.0) {
            strcpy(format, "%.*le");
            loc = 0.0;
            sprintf(s, format, prec, loc);
        }
        break;
    case FORMAT_POWER:
        if (loc < 0.0) {
            isign = 1;
            loc = log10(-loc);
            strcpy(format, "-10\\S%.*lf\\N");
        } else if (loc == 0.0) {
            sprintf(format, "%.*f", prec, 0.0);
        } else {
            loc = log10(loc);
            strcpy(format, "10\\S%.*lf\\N");
        }
        sprintf(s, format, prec, loc);
        break;
    case FORMAT_GENERAL:
        strcpy(format, "%.*lg");
        sprintf(s, format, prec, loc);
        tmp = atof(s);
        if (tmp == 0.0) {
            strcpy(format, "%lg");
            loc = 0.0;
            sprintf(s, format, loc);
        }
        break;
    case FORMAT_DDMMYY:
        strcpy(format, "%d-%d-%d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (y >= 1900 && y < 2000) {
            y -= 1900;
        }
        sprintf(s, format, d, m, y);
        break;
    case FORMAT_MMDDYY:
        strcpy(format, "%d-%d-%d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (y >= 1900 && y < 2000) {
            y -= 1900;
        }
        sprintf(s, format, m, d, y);
        break;
    case FORMAT_YYMMDD:
        strcpy(format, "%d-%d-%d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (y >= 1900 && y < 2000) {
            y -= 1900;
        }
        sprintf(s, format, y, m, d);
        break;
    case FORMAT_MMYY:
        strcpy(format, "%d-%d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (y >= 1900 && y < 2000) {
            y -= 1900;
        }
        sprintf(s, format, m, y);
        break;
    case FORMAT_MMDD:
        strcpy(format, "%d-%d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        sprintf(s, format, m, d);
        break;
    case FORMAT_MONTHDAY:
        strcpy(format, "%s-%d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (m - 1 < 0 || m - 1 > 11) {
        } else {
            sprintf(s, format, months[m - 1], d);
        }
        break;
    case FORMAT_DAYMONTH:
        strcpy(format, "%d-%s");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (m - 1 < 0 || m - 1 > 11) {
        } else {
            sprintf(s, format, d, months[m - 1]);
        }
        break;
    case FORMAT_MONTHS:
        strcpy(format, "%s");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (m - 1 < 0 || m - 1 > 11) {
        } else {
            sprintf(s, format, months[m - 1]);
        }
        break;
        break;
    case FORMAT_MONTHSY:
        strcpy(format, "%s-%d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (m - 1 < 0 || m - 1 > 11) {
        } else {
            sprintf(s, format, months[m - 1], y);
        }
        break;
        break;
    case FORMAT_MONTHL:
        strcpy(format, "%s");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (m - 1 < 0 || m - 1 > 11) {
        } else {
            sprintf(s, format, monthl[m - 1]);
        }
        break;
        break;
    case FORMAT_DAYOFWEEKS:
        strcpy(format, "%s");
        itmp = dayofweek(loc);
        if ((itmp < 0) | (itmp > 6)) {
        } else {
            sprintf(s, format, dayofweekstrs[dayofweek(loc)]);
        }
        break;
    case FORMAT_DAYOFWEEKL:
        strcpy(format, "%s");
        itmp = dayofweek(loc);
        if ((itmp < 0) | (itmp > 6)) {
        } else {
            sprintf(s, format, dayofweekstrl[dayofweek(loc)]);
        }
        break;
    case FORMAT_DAYOFYEAR:
        strcpy(format, "%d");
        sprintf(s, format, getndays(loc));
        break;
    case FORMAT_HMS:
        strcpy(format, "%02d:%02d:%02d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        sprintf(s, format, h, mm, (int)sec);
        break;
    case FORMAT_MMDDHMS:
        strcpy(format, "%d-%d %02d:%02d:%02d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (y >= 1900 && y < 2000) {
            y -= 1900;
        }
        sprintf(s, format, m, d, h, mm, (int)sec);
        break;
    case FORMAT_MMDDYYHMS:
        strcpy(format, "%d-%d-%d %02d:%02d:%02d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (y >= 1900 && y < 2000) {
            y -= 1900;
        }
        sprintf(s, format, m, d, y, h, mm, (int)sec);
        break;
    case FORMAT_YYMMDDHMS:
        strcpy(format, "%d-%d-%d %02d:%02d:%02d");
        calcdate(loc, &m, &d, &y, &h, &mm, &sec);
        if (y >= 1900 && y < 2000) {
            y -= 1900;
        }
        sprintf(s, format, y, m, d, h, mm, (int)sec);
        break;
    case FORMAT_DEGREESLON:
        if (loc < 0.0) {
            loc *= -1.0;
            strcpy(format, "%.*lfW");
        } else if (loc > 0.0) {
            strcpy(format, "%.*lfE");
        } else {
            strcpy(format, "0");
        }
        sprintf(s, format, prec, loc);
        break;
    case FORMAT_DEGREESMMLON:
        if (loc < 0.0) {
            loc *= -1.0;
            strcpy(format, "%d %.*lf' W");
        } else if (loc > 0.0) {
            strcpy(format, "%d %.*lf' E");
        } else {
            strcpy(format, "0 0'");
        }
        y = loc;
        sec = (loc - y) * 60.0;
        sprintf(s, format, y, prec, sec);
        break;
    case FORMAT_DEGREESMMSSLON:
        if (loc < 0.0) {
            loc *= -1.0;
            strcpy(format, "%d %d' %.*lf\" W");
        } else if (loc > 0.0) {
            strcpy(format, "%d %d' %.*lf\" E");
        } else {
            strcpy(format, "0 0' 0\"");
        }
        y = loc;
        sec = (loc - y) * 3600.0;
        m = sec / 60.0;
        sec = (sec - m * 60);
        sprintf(s, format, y, m, prec, sec);
        break;
    case FORMAT_MMSSLON:
        if (loc < 0.0) {
            loc *= -1.0;
            strcpy(format, "%d' %.*lf\" W");
        } else if (loc > 0.0) {
            strcpy(format, "%d' %.*lf\" E");
        } else {
            strcpy(format, "0 0' 0\"");
        }
        y = loc;
        sec = (loc - y) * 3600.0;
        m = sec / 60.0;
        sec = (sec - m * 60);
        sprintf(s, format, m, prec, sec);
        break;
    case FORMAT_DEGREESLAT:
        if (loc < 0.0) {
            loc *= -1.0;
            strcpy(format, "%.*lfS");
        } else if (loc > 0.0) {
            strcpy(format, "%.*lfN");
        } else {
            strcpy(format, "0");
        }
        sprintf(s, format, prec, loc);
        break;
    case FORMAT_DEGREESMMLAT:
        if (loc < 0.0) {
            loc *= -1.0;
            strcpy(format, "%d %.*lf' S");
        } else if (loc > 0.0) {
            strcpy(format, "%d %.*lf' N");
        } else {
            strcpy(format, "0 0'");
        }
        y = loc;
        sec = (loc - y) * 60.0;
        sprintf(s, format, y, prec, sec);
        break;
    case FORMAT_DEGREESMMSSLAT:
        if (loc < 0.0) {
            loc *= -1.0;
            strcpy(format, "%d %d' %.*lf\" S");
        } else if (loc > 0.0) {
            strcpy(format, "%d %d' %.*lf\" N");
        } else {
            strcpy(format, "0 0' 0\"");
        }
        y = loc;
        sec = (loc - y) * 3600.0;
        m = sec / 60.0;
        sec = (sec - m * 60);
        sprintf(s, format, y, m, prec, sec);
        break;
    case FORMAT_MMSSLAT:
        if (loc < 0.0) {
            loc *= -1.0;
            strcpy(format, "%d' %.*lf\" S");
        } else if (loc > 0.0) {
            strcpy(format, "%d' %.*lf\" N");
        } else {
            strcpy(format, "0 0' 0\"");
        }
        y = loc;
        sec = (loc - y) * 3600.0;
        m = sec / 60.0;
        sec = (sec - m * 60);
        sprintf(s, format, m, prec, sec);
        break;
    default:
        sprintf(s, format, prec, loc);
        break;
    }
}

void drawxticklabels(int gno, int caxis) {
    tickmarks t;
    world w;
    view v;
    char s[256];
    double prevloc = 0.0, loc, xsav, x, top, bot;
    int i = 0, ifudge, nticks, bjust = 2, tjust = 2;
    int tang = 0;
    double delb = 0.0, delt = 0.0, vyb, vyt, vx, ofx, ofb, oft;
    double start, stop;
    double ttmp; /* DEFUNCT for alternate mapping */
    double fnticks;
    double eps = 0.0001;

    get_graph_tickmarks(gno, &t, caxis);
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);
    t.tl_skip++;
    ifudge = (int)(t.tl_vgap * stringextenty(t.tl_charsize * devcharsize, "Ny"));

    /* set start and end for ticks */
    start = w.xg1;
    stop = w.xg2;
    ttmp = t.tmajor;
    if (caxis == ZX_AXIS) {
        top = 0.0;
        bot = 0.0;
    } else {
        if (isyreversed(gno)) {
            top = w.yg1;
            bot = w.yg2;
        } else {
            top = w.yg2;
            bot = w.yg1;
        }
    }
    if (t.tl_starttype == TYPE_SPEC) {
        if (t.tl_start >= w.xg1) {
            start = t.tl_start;
        }
    }
    if (t.tl_stoptype == TYPE_SPEC) {
        if (t.tl_stop <= w.xg2) {
            stop = t.tl_stop;
        }
    }
    /* how many ticklabels */
    if (islogx(gno)) {
        fnticks = fabs((log10(stop) - log10(start)) / ttmp) + 1;
    } else {
        fnticks = fabs((stop - start) / ttmp) + 1;
    }
    nticks = (int)fnticks;
    if (nticks + 1 < fnticks + eps) {
        nticks++;
    }

    /* if offsx or offsy != 0.0 then adjust */
    vyb = v.yv1;
    vyt = v.yv2;
    vx = v.xv1;
    delb = 0.0;
    delt = 0.0;
    if (fabs(t.offsx) > 0.001 && fabs(t.offsx) < 1.0) {
        if (isyreversed(gno)) {
            vyb = v.yv2 - t.offsx;
        } else {
            vyb = v.yv1 - t.offsx;
        }
        view2world(vx, vyb, &ofx, &ofb);
        delb = bot - ofb;
    }
    if (fabs(t.offsy) > 0.001 && fabs(t.offsy) < 1.0) {
        if (isyreversed(gno)) {
            vyt = v.yv1 + t.offsy;
        } else {
            vyt = v.yv2 + t.offsy;
        }
        view2world(vx, vyt, &ofx, &oft);
        delt = oft - top;
    }
    setcharsize(t.tl_charsize);
    setfont(t.tl_font);
    setcolor(t.tl_color);
    setlinestyle(1);
    setlinewidth(t.tl_linew);

    if (t.tl_type == TYPE_SPEC) {
        nticks = t.t_spec;
    }
    /* tick label justification */
    switch (t.tl_layout) {
    case TICKS_HORIZONTAL:
        tang = 0;
        break;
    case TICKS_VERTICAL:
        tang = 90;
        bjust = 1;
        tjust = 0;
        break;
    case TYPE_SPEC:
        tang = t.tl_angle;
        if (tang > 0 && tang < 180) {
            bjust = 1;
            tjust = 0;
        } else if (tang > 180 && tang < 360) {
            bjust = 0;
            tjust = 1;
        } else if (tang == 0 || tang == 180 || tang == 360) {
            bjust = 2;
            tjust = 2;
        }
        break;
    }
    if (nticks > 500) {
        errmsg("Too many ticks ( > 500 ), autoscaling");
        autoscale_graph(gno, caxis);
        return;
    }
    /* draw tick labels */
    for (i = 0; i < nticks; i += t.tl_skip) {
        if (t.t_type == TYPE_SPEC) {
            loc = t.t_specloc[i];
        } else {
            if (islogx(gno)) {
                ttmp = ceil(log10(start));
                loc = pow(10.0, ttmp + i * t.tmajor);
            } else {
                loc = start + i * t.tmajor;
            }
        }
        if (loc < (w.xg1 - eps) || loc > (w.xg2 + eps)) {
            continue;
        }
        x = loc;

        if (t.tl_type == TYPE_SPEC) {
            strcpy(s, t.t_speclab[i].s);
            strcat(s, "\\N\\4");
        } else {
            switch (t.tl_sign) {
            case SIGN_NORMAL:
                break;
            case SIGN_ABSOLUTE:
                loc = fabs(loc);
                break;
            case SIGN_NEGATE:
                loc = -loc;
                break;
            }
            create_ticklabel(t.tl_format, t.tl_prec, loc, s);
        }
        if (t.tl_prestr[0]) {
            char tmpbuf[300];

            strcpy(tmpbuf, t.tl_prestr);
            strcat(tmpbuf, s);
            strcpy(s, tmpbuf);
        }
        if (t.tl_appstr[0]) {
            strcat(s, t.tl_appstr);
        }
        if (t.tl_loc == LABEL_BETWEEN) { /* for centered labels */
            if (i == 0) {
                prevloc = x;
                continue;
            } else {
                xsav = x;
                x = (x + prevloc) / 2.0;
                prevloc = xsav;
            }
        }
        if (t.tl_op == PLACE_BOTTOM || t.tl_op == PLACE_BOTH) {
            (*devwritestr)((*devconvx)(x), (*devconvy)(bot - delb) - ifudge * (1 + i % (t.tl_staggered + 1)), tang, s,
                           bjust, 1);
        }
        if (t.tl_op == PLACE_TOP || t.tl_op == PLACE_BOTH) {
            (*devwritestr)((*devconvx)(x), (*devconvy)(top + delt) + ifudge * (1 + i % (t.tl_staggered + 1)), tang, s,
                           tjust, 0);
        }
    }

    if (t.label.s[0]) {
        int lang1 = 0, ljust1 = 2;
        int lang2 = 0, ljust2 = 2;
        double tmp = ((*devconvx)(g[gno].w.xg2) + (*devconvx)(g[gno].w.xg1)) / 2.0;

        switch (t.label_layout) {
        case LAYOUT_PARALLEL:
            if (t.tl_op == PLACE_BOTTOM || t.tl_op == PLACE_BOTH) {
                lang1 = 0;
                ljust1 = 2;
            }
            if (t.tl_op == PLACE_TOP || t.tl_op == PLACE_BOTH) {
                lang2 = 0;
                ljust2 = 2;
            }
            break;
        case LAYOUT_PERPENDICULAR:
            if (t.tl_op == PLACE_BOTTOM || t.tl_op == PLACE_BOTH) {
                lang1 = 90;
                ljust1 = 1;
            }
            if (t.tl_op == PLACE_TOP || t.tl_op == PLACE_BOTH) {
                lang2 = 90;
                ljust2 = 0;
            }
            break;
        }

        setcharsize(t.label.charsize);
        setfont(t.label.font);
        setcolor(t.label.color);
        setlinestyle(1);
        setlinewidth(t.label.linew);
        if (t.tl_op == PLACE_BOTTOM || t.tl_op == PLACE_BOTH) {
            if (t.label_place == TYPE_SPEC) {
                if (isyreversed(gno)) {
                    vyb = v.yv2 - t.label.y;
                } else {
                    vyb = v.yv1 - t.label.y;
                }

                vx = (v.xv1 + v.xv2) / 2 + t.label.x;
                view2world(vx, vyb, &ofx, &ofb);
                (*devwritestr)((*devconvx)(ofx), (*devconvy)(ofb), lang1, t.label.s, ljust1, 0);
            } else {
                (*devwritestr)((int)(tmp), (*devconvy)(bot - delb) - 3 * ifudge, lang1, t.label.s, ljust1, 0);
            }
        }
        if (t.tl_op == PLACE_TOP || t.tl_op == PLACE_BOTH) {
            if (t.label_place == TYPE_SPEC) {
                if (isyreversed(gno)) {
                    vyb = v.yv1 + t.label.y;
                } else {
                    vyb = v.yv2 + t.label.y;
                }

                vx = (v.xv1 + v.xv2) / 2 + t.label.x;
                view2world(vx, vyb, &ofx, &ofb);
                (*devwritestr)((*devconvx)(ofx), (*devconvy)(ofb), lang2, t.label.s, ljust2, 0);
            } else {
                (*devwritestr)((int)(tmp), (*devconvy)(top + delt) + 3 * ifudge, lang2, t.label.s, ljust2, 0);
            }
        }
    }
}

void drawyticklabels(int gno, int caxis) {
    tickmarks t;
    world w;
    view v;
    char s[256];
    double loc, y, top, bot;
    int i = 0, ifudge, ix, nticks;
    int ljust = 1, rjust = 0;
    int tang = 0, ylabpos = 0;
    double delr = 0.0, dell = 0.0, vxl, vxr, vy, ofy, ofr, ofl;
    double start, stop;
    double ttmp; /* DEFUNCT for alternate mapping */
    double fnticks;
    double eps = 0.0001;

    get_graph_tickmarks(gno, &t, caxis);
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);
    t.tl_skip++;
    ifudge = (int)(t.tl_hgap * stringextentx(t.tl_charsize * devcharsize, "M"));

    /* set start and end for ticks */
    start = w.yg1;
    stop = w.yg2;
    ttmp = t.tmajor;
    if (caxis == ZY_AXIS) {
        top = 0.0;
        bot = 0.0;
    } else {
        if (isxreversed(gno)) {
            top = w.xg1;
            bot = w.xg2;
        } else {
            top = w.xg2;
            bot = w.xg1;
        }
    }
    if (t.tl_starttype == TYPE_SPEC) {
        if (t.tl_start >= w.yg1) {
            start = t.tl_start;
        }
    }
    if (t.tl_stoptype == TYPE_SPEC) {
        if (t.tl_stop <= w.yg2) {
            stop = t.tl_stop;
        }
    }
    /* how many ticks and which direction to increment */
    if (islogy(gno)) {
        fnticks = fabs((log10(stop) - log10(start)) / ttmp) + 1;
    } else {
        fnticks = fabs((stop - start) / ttmp) + 1;
    }
    nticks = (int)fnticks;
    if (nticks + 1 < fnticks + eps) {
        nticks++;
    }

    t.tmajor = fabs(t.tmajor);

    /* if offsx or offsy != 0.0 then adjust */
    vxl = v.xv1;
    vxr = v.xv2;
    vy = v.yv1;
    dell = 0.0;
    delr = 0.0;
    if (fabs(t.offsx) > 0.001 && fabs(t.offsx) < 1.0) {
        if (isxreversed(gno)) {
            vxl = v.xv2 - t.offsx;
        } else {
            vxl = v.xv1 - t.offsx;
        }
        view2world(vxl, vy, &ofl, &ofy);
        dell = bot - ofl;
    }
    if (fabs(t.offsy) > 0.001 && fabs(t.offsy) < 1.0) {
        if (isxreversed(gno)) {
            vxr = v.xv1 + t.offsy;
        } else {
            vxr = v.xv2 + t.offsy;
        }
        view2world(vxr, vy, &ofr, &ofy);
        delr = ofr - top;
    }
    setcharsize(t.tl_charsize);
    setfont(t.tl_font);
    setcolor(t.tl_color);
    setlinestyle(1);
    setlinewidth(t.tl_linew);

    if (t.tl_type == TYPE_SPEC) {
        nticks = t.t_spec;
    }
    /* tick label justification */
    switch (t.tl_layout) {
    case TICKS_HORIZONTAL:
        tang = 0;
        ljust = 1;
        rjust = 0;
        break;
    case TICKS_VERTICAL:
        tang = 90;
        ljust = 2;
        rjust = 2;
        break;
    case TYPE_SPEC:
        tang = t.tl_angle;
        if (tang > 0 && tang < 90) {
            ljust = 1;
            rjust = 0;
        } else if (tang > 90 && tang < 270) {
            ljust = 0;
            rjust = 1;
        } else if (tang == 90 || tang == 270) {
            ljust = 2;
            rjust = 2;
        } else if (tang == 0 || tang == 360) {
            ljust = 1;
            rjust = 0;
        }
        break;
    }
    if (nticks > 500) {
        errmsg("Too many ticks ( > 500 ), autoscaling");
        autoscale_graph(gno, caxis);
        return;
    }
    /* draw tick labels */
    for (i = 0; i < nticks; i += t.tl_skip) {
        if (t.t_type == TYPE_SPEC) {
            loc = t.t_specloc[i];
        } else {
            if (islogy(gno)) {
                ttmp = ceil(log10(start));
                loc = pow(10.0, ttmp + i * t.tmajor);
            } else {
                loc = start + i * t.tmajor;
            }
        }
        if (loc < (w.yg1 - eps) || loc > (w.yg2 + eps)) {
            continue;
        }
        y = loc;

        if (t.tl_type == TYPE_SPEC) {
            strcpy(s, t.t_speclab[i].s);
            strcat(s, "\\N\\4");
        } else {
            switch (t.tl_sign) {
            case SIGN_NORMAL:
                break;
            case SIGN_ABSOLUTE:
                loc = fabs(loc);
                break;
            case SIGN_NEGATE:
                loc = -loc;
                break;
            }
            create_ticklabel(t.tl_format, t.tl_prec, loc, s);
        }
        if (t.tl_prestr[0]) {
            char tmpbuf[300];

            strcpy(tmpbuf, t.tl_prestr);
            strcat(tmpbuf, s);
            strcpy(s, tmpbuf);
        }
        if (t.tl_appstr[0]) {
            strcat(s, t.tl_appstr);
        }
        if (t.t_inout == TICKS_BOTH || t.t_inout == TICKS_OUT) {
            ix = ifudge * 2;
        } else {
            ix = ifudge;
        }
        if (t.tl_op == PLACE_LEFT || t.tl_op == PLACE_BOTH) {
            if (tang == 90) {
                ylabpos = 2 * ix;
                (*devwritestr)((*devconvx)(bot - dell) - ix, (*devconvy)(y), tang, s, ljust, 1);
            } else {
                ix = ifudge + stringextentx(t.tl_charsize * devcharsize, s);
                if (ix > ylabpos) {
                    ylabpos = ix;
                }
                (*devwritestr)((*devconvx)(bot - dell) - ifudge, (*devconvy)(y), tang, s, ljust, 1);
            }
        }
        if (t.tl_op == PLACE_RIGHT || t.tl_op == PLACE_BOTH) {
            if (tang == 90) {
                ylabpos = 2 * ix;
                (*devwritestr)((*devconvx)(top + delr) + ix, (*devconvy)(y), tang, s, rjust, 0);
            } else {
                ix = ifudge + stringextentx(t.tl_charsize * devcharsize, s);
                if (ix > ylabpos) {
                    ylabpos = ix;
                }
                (*devwritestr)((*devconvx)(top + delr) + ifudge, (*devconvy)(y), tang, s, rjust, 1);
            }
        }
    }
    /* TODO axis label layout */
    if (t.label.s[0]) {
        double tmp;
        int lang1 = 0, ljust1 = 2;
        int lang2 = 0, ljust2 = 2;

        tmp = ((*devconvy)(g[gno].w.yg2) + (*devconvy)(g[gno].w.yg1)) / 2.0;
        switch (t.label_layout) {
        case LAYOUT_PARALLEL:
            if (t.tl_op == PLACE_LEFT || t.tl_op == PLACE_BOTH) {
                lang1 = 90;
                ljust1 = 2;
            }
            if (t.tl_op == PLACE_RIGHT || t.tl_op == PLACE_BOTH) {
                lang2 = 90;
                /*
                        lang2 = 270;
                */
                ljust2 = 2;
            }
            break;
        case LAYOUT_PERPENDICULAR:
            if (t.tl_op == PLACE_LEFT || t.tl_op == PLACE_BOTH) {
                lang1 = 0;
                ljust1 = 1;
            }
            if (t.tl_op == PLACE_RIGHT || t.tl_op == PLACE_BOTH) {
                lang2 = 0;
                ljust2 = 0;
            }
            break;
        }

        setcharsize(t.label.charsize);
        setfont(t.label.font);
        setcolor(t.label.color);
        setlinestyle(1);
        setlinewidth(t.label.linew);
        if (t.tl_op == PLACE_LEFT || t.tl_op == PLACE_BOTH) {
            if (t.label_place == TYPE_SPEC) {
                if (isxreversed(gno)) {
                    vxl = v.xv2 - t.label.x;
                } else {
                    vxl = v.xv1 - t.label.x;
                }
                vy = (v.yv1 + v.yv2) / 2 + t.label.y;
                view2world(vxl, vy, &ofl, &ofy);
                (*devwritestr)((*devconvx)(ofl), (*devconvy)(ofy), lang1, t.label.s, ljust1, 0);
            } else {
                (*devwritestr)((*devconvx)(bot - dell) - (int)(1.4 * ylabpos), (int)(tmp), lang1, t.label.s, ljust1, 0);
            }
        }
        if (t.tl_op == PLACE_RIGHT || t.tl_op == PLACE_BOTH) {
            if (t.label_place == TYPE_SPEC) {
                if (isxreversed(gno)) {
                    vxl = v.xv1 + t.label.x;
                } else {
                    vxl = v.xv2 + t.label.x;
                }
                vy = (v.yv1 + v.yv2) / 2 + t.label.y;
                view2world(vxl, vy, &ofl, &ofy);
                (*devwritestr)((*devconvx)(ofl), (*devconvy)(ofy), lang2, t.label.s, ljust2, 0);
            } else {
                (*devwritestr)((*devconvx)(top + delr) + (int)(1.4 * ylabpos), (int)(tmp), lang2, t.label.s, ljust2, 0);
            }
        }
    }
}

/*
 */
void drawxtickmarks(int gno, int caxis) {
    tickmarks t;
    world w;
    view v;
    double top = 0, bot = 0, s, start, stop, step;
    double delb = 0.0, delt = 0.0, vyb, vyt, vx, ofx, ofb, oft;
    double ttmp1, ttmp2; /* DEFUNCT for alternate mapping */

    int axis, nticks, i, j, k, logloop, kstart;
    int yinvflag = isyreversed(gno) ? 1 : 0;
    double ticl[2];
    double fnticks;
    double eps = 0.0001;

    get_graph_tickmarks(gno, &t, caxis);
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);
    axis = caxis % 2; /* 0 if an axis along x, 1 if along y */
    start = w.xg1;
    stop = w.xg2;
    ticl[0] = t.t_size;
    ticl[1] = t.t_msize;
    for (i = 1; i >= 0; i--) {
        if (i == 1) {
            setcolor(t.t_mcolor);
            setlinestyle(t.t_mlines);
            setlinewidth(t.t_mlinew);
        }
        if (i == 0) {
            setcolor(t.t_color);
            setlinestyle(t.t_lines);
            setlinewidth(t.t_linew);
        }
        setticksize(ticl[i], 1.0);
        start = w.xg1;
        stop = w.xg2;
        if (t.tl_starttype == TYPE_SPEC) {
            if (i == 0) {
                if (t.tl_start >= w.xg1 && t.tl_start <= w.xg2) {
                    start = t.tl_start;
                }
            } else {
                if (t.tl_start >= w.xg1 && t.tl_start <= w.xg2) {
                    start = t.tl_start - (int)((t.tl_start - w.xg1) / t.tminor) * t.tminor;
                }
            }
        }
        if (t.tl_stoptype == TYPE_SPEC) {
            if (i == 0) {
                if (t.tl_stop <= w.xg2 && t.tl_stop >= w.xg1) {
                    stop = t.tl_stop;
                }
            } else {
                if (t.tl_stop <= w.xg2 && t.tl_stop >= w.xg1) {
                    stop = t.tl_stop + (int)((w.xg2 - t.tl_stop) / t.tminor) * t.tminor;
                }
            }
        }
        switch (caxis) {
        case X_AXIS:
            top = w.yg2;
            bot = w.yg1;
            break;
        case ZX_AXIS:
            top = 0.0;
            bot = 0.0;
            break;
        }
        ttmp1 = t.tmajor;
        ttmp2 = t.tminor;

        if (i == 0) {
            step = ttmp1;
        } else {
            step = ttmp2;
        }

        /* if offsx or offsy != 0.0 then adjust  */
        vyb = v.yv1;
        vyt = v.yv2;
        vx = v.xv1;
        delb = 0.0;
        delt = 0.0;
        if (fabs(t.offsx) > 0.001 && fabs(t.offsx) < 1.0) {
            if (isyreversed(gno)) {
                vyb = v.yv2 - t.offsx;
            } else {
                vyb = v.yv1 - t.offsx;
            }
            view2world(vx, vyb, &ofx, &ofb);
            delb = bot - ofb;
        }
        if (fabs(t.offsy) > 0.001 && fabs(t.offsy) < 1.0) {
            if (isyreversed(gno)) {
                vyt = v.yv1 + t.offsy;
            } else {
                vyt = v.yv2 + t.offsy;
            }
            view2world(vx, vyt, &ofx, &oft);
            delt = oft - top;
        }
        /* determine the number of tick marks */
        if (t.t_type == TYPE_SPEC) {
            fnticks = t.t_spec;
        } else if (islogx(gno) && i == 0) {
            fnticks = fabs((log10(stop) - log10(start)) / step) + 1;
        } else if (islogx(gno) && i == 1) {
            fnticks = fabs((log10(stop) - log10(start)) / ttmp1) + 1;
        } else {
            fnticks = fabs((stop - start) / step) + 1;
        }
        nticks = (int)fnticks;
        if (nticks + 1 < fnticks + eps) {
            nticks++;
        }

        if (islogx(gno)) {
            s = pow(10.0, floor(log10(start)));
        } else {
            s = start;
        }

        if (nticks > 500) {
            errmsg("Too many ticks ( > 500 ), autoscaling");
            autoscale_graph(gno, caxis);
            return;
        }
        if (((t.t_op == PLACE_BOTTOM && !yinvflag) || ((t.t_op == PLACE_TOP) && yinvflag)) || t.t_op == PLACE_BOTH) {
            for (j = 0; j < nticks && start <= stop; j++) {
                logloop = 5;
                kstart = 0;
                if (t.t_type == TYPE_SPEC) {
                    start = t.t_specloc[j];
                    if (start < (w.xg1 - eps) || start > (w.xg2 + eps)) {
                        continue;
                    }
                } else {
                    if (islogx(gno) && i == 0) {
                        start = pow(10.0, log10(s) + j * t.tmajor);
                    } else if (islogx(gno) && i == 1) {
                        logloop = t.tminor;
                        s = pow(10.0, floor(log10(s)));
                        start = pow(10.0, log10(s) + j * t.tmajor);
                        kstart = 0;
                    } else {
                        start = s + j * step;
                    }
                }
                if (logloop != 0) {
                    for (k = kstart; k < 10 / logloop - ((10 % logloop) ? 0 : 1); k++) {
                        if (islogx(gno) && i == 1) {
                            start = ((k + 1) * logloop) * pow(10.0, log10(s) + j * t.tmajor);
                            if (start > stop) {
                                goto skip1;
                            }
                        }
                        if (start < (w.xg1 - eps) || start > (w.xg2 + eps)) {
                            continue;
                        }
                        if (t.t_gridflag && i == 0) {
                            my_move2(start, bot);
                            my_draw2(start, top);
                        } else if (t.t_mgridflag && i == 1) {
                            my_move2(start, bot);
                            my_draw2(start, top);
                        }
                        switch (t.t_inout) {
                        case TICKS_IN: /* draw up from y1 */
                            drawtic(start, bot - delb, axis, yinvflag ? 1 : 0);
                            break;
                        case TICKS_OUT: /* draw down from y2 */
                            drawtic(start, bot - delb, axis, yinvflag ? 0 : 1);
                            break;
                        case TICKS_BOTH: /* draw up from y1 then draw down
                                          * from y2 */
                            drawtic(start, bot - delb, axis, yinvflag ? 1 : 0);
                            drawtic(start, bot - delb, axis, yinvflag ? 0 : 1);
                            break;
                        }
                    }
                }
            }
        }
    skip1:;

        start = s;
        if (((t.t_op == PLACE_TOP && !yinvflag) || ((t.t_op == PLACE_BOTTOM) && yinvflag)) || t.t_op == PLACE_BOTH) {
            for (j = 0; j < nticks && start <= stop; j++) {
                logloop = 5;
                kstart = 0;
                if (t.t_type == TYPE_SPEC) {
                    start = t.t_specloc[j];
                    if (start < (w.xg1 - eps) || start > (w.xg2 + eps)) {
                        continue;
                    }
                } else {
                    if (islogx(gno) && i == 0) {
                        start = pow(10.0, log10(s) + j * t.tmajor);
                    } else if (islogx(gno) && i == 1) {
                        logloop = t.tminor;
                        start = pow(10.0, log10(s) + j * t.tmajor);
                        kstart = 0;
                    } else {
                        start = s + j * step;
                    }
                }
                if (logloop != 0) {
                    for (k = kstart; k < 10 / logloop - ((10 % logloop) ? 0 : 1); k++) {
                        if (islogx(gno) && i == 1) {
                            start = ((k + 1) * logloop) * pow(10.0, log10(s) + j * t.tmajor);
                            if (start > stop) {
                                goto skip2;
                            }
                        }
                        if (start < (w.xg1 - eps) || start > (w.xg2 + eps)) {
                            continue;
                        }
                        switch (t.t_inout) {
                        case TICKS_IN:
                            drawtic(start, top + delt, axis, yinvflag ? 0 : 1); /* draw down from y2 */
                            break;
                        case TICKS_OUT:
                            drawtic(start, top + delt, axis, yinvflag ? 1 : 0); /* draw up from y2 */
                            break;
                        case TICKS_BOTH:
                            drawtic(start, top + delt, axis, yinvflag ? 0 : 1); /* draw down from y2 */
                            drawtic(start, top + delt, axis, yinvflag ? 1 : 0); /* draw up from y2 */
                            break;
                        }
                    }
                }
            }
        }
    skip2:;
    }
}

/*
 */
void drawytickmarks(int gno, int caxis) {
    tickmarks t;
    world w;
    view v;
    double top = 0, bot = 0, s, start, stop, step;

    double delr = 0.0, dell = 0.0, vxl, vxr, vy, ofy, ofr, ofl;

    double ttmp1, ttmp2; /* DEFUNCT for alternate mapping */

    int axis, nticks, i, j, k, logloop, kstart;
    int xinvflag = isxreversed(gno) ? 1 : 0;
    double ticl[2];
    double fnticks;
    double eps = 0.0001;

    get_graph_tickmarks(gno, &t, caxis);
    get_graph_world(gno, &w);
    get_graph_view(gno, &v);
    axis = caxis % 2; /* 0 if an axis along x, 1 if along y */
    start = w.yg1;
    stop = w.yg2;
    ticl[0] = t.t_size;
    ticl[1] = t.t_msize;
    for (i = 1; i >= 0; i--) {
        if (i == 1) {
            setcolor(t.t_mcolor);
            setlinestyle(t.t_mlines);
            setlinewidth(t.t_mlinew);
        }
        if (i == 0) {
            setcolor(t.t_color);
            setlinestyle(t.t_lines);
            setlinewidth(t.t_linew);
        }
        setticksize(1.0, ticl[i]);
        start = w.yg1;
        stop = w.yg2;
        if (t.tl_starttype == TYPE_SPEC) {
            if (i == 0) {
                if (t.tl_start >= w.yg1 && t.tl_start <= w.yg2) {
                    start = t.tl_start;
                }
            } else {
                if (t.tl_start >= w.yg1 && t.tl_start <= w.yg2) {
                    start = t.tl_start - (int)((t.tl_start - w.yg1) / t.tminor) * t.tminor;
                }
            }
        }
        if (t.tl_stoptype == TYPE_SPEC) {
            if (i == 0) {
                if (t.tl_stop <= w.yg2 && t.tl_stop >= w.yg1) {
                    stop = t.tl_stop;
                }
            } else {
                if (t.tl_stop <= w.yg2 && t.tl_stop >= w.yg1) {
                    stop = t.tl_stop + (int)((w.yg2 - t.tl_stop) / t.tminor) * t.tminor;
                }
            }
        }
        switch (caxis) {
        case Y_AXIS:
            top = w.xg2;
            bot = w.xg1;
            break;
        case ZY_AXIS:
            top = 0.0;
            bot = 0.0;
            break;
        }

        ttmp1 = t.tmajor;
        ttmp2 = t.tminor;

        if (i == 0) {
            step = ttmp1;
        } else {
            step = ttmp2;
        }

        /* if offsx or offsy != 0.0 then adjust  */
        vxl = v.xv1;
        vxr = v.xv2;
        vy = v.yv1;
        dell = 0.0;
        delr = 0.0;
        if (fabs(t.offsx) > 0.001 && fabs(t.offsx) < 1.0) {
            if (isxreversed(gno)) {
                vxl = v.xv2 - t.offsx;
            } else {
                vxl = v.xv1 - t.offsx;
            }
            view2world(vxl, vy, &ofl, &ofy);
            dell = bot - ofl;
        }
        if (fabs(t.offsy) > 0.001 && fabs(t.offsy) < 1.0) {
            if (isxreversed(gno)) {
                vxr = v.xv1 + t.offsx;
            } else {
                vxr = v.xv2 + t.offsx;
            }
            view2world(vxr, vy, &ofr, &ofy);
            delr = ofr - top;
        }
        /* determine the number of tick marks */
        if (t.t_type == TYPE_SPEC) {
            fnticks = t.t_spec;
        } else if (islogy(gno) && i == 0) {
            fnticks = fabs((log10(stop) - log10(start)) / step) + 1;
        } else if (islogy(gno) && i == 1) {
            fnticks = fabs((log10(stop) - log10(start)) / ttmp1) + 1;
        } else {
            fnticks = fabs((stop - start) / step) + 1;
        }
        nticks = (int)fnticks;
        if (nticks + 1 < fnticks + eps) {
            nticks++;
        }

        if (islogy(gno)) {
            s = pow(10.0, floor(log10(start)));
        } else {
            s = start;
        }
        if (nticks > 500) {
            errmsg("Too many ticks ( > 500 ), autoscaling");
            autoscale_graph(gno, caxis);
            return;
        }
        if (((t.t_op == PLACE_LEFT && !xinvflag) || ((t.t_op == PLACE_RIGHT) && xinvflag)) || t.t_op == PLACE_BOTH) {
            for (j = 0; j < nticks && start <= stop; j++) {
                logloop = 5;
                kstart = 0;
                if (t.t_type == TYPE_SPEC) {
                    start = t.t_specloc[j];
                    if (start < (w.yg1 - eps) || start > (w.yg2 + eps)) {
                        continue;
                    }
                } else {
                    if (islogy(gno) && i == 0) {
                        start = pow(10.0, log10(s) + j * t.tmajor);
                    } else if (islogy(gno) && i == 1) {
                        logloop = t.tminor;
                        start = pow(10.0, log10(s) + j * t.tmajor);
                        kstart = 0;
                    } else {
                        start = s + j * step;
                    }
                }

                if (logloop != 0) {
                    for (k = kstart; k < 10 / logloop - ((10 % logloop) ? 0 : 1); k++) {
                        if (islogy(gno) && i == 1) {
                            start = ((k + 1) * logloop) * pow(10.0, log10(s) + j * t.tmajor);
                            if (start > stop) {
                                goto skip1;
                            }
                        }
                        if (start < (w.yg1 - eps) || start > (w.yg2 + eps)) {
                            continue;
                        }
                        if (t.t_gridflag && i == 0) {
                            my_move2(bot, start);
                            my_draw2(top, start);
                        } else if (t.t_mgridflag && i == 1) {
                            my_move2(bot, start);
                            my_draw2(top, start);
                        }
                        switch (t.t_inout) {
                        case TICKS_IN:
                            drawtic(bot - dell, start, axis, xinvflag ? 1 : 0); /* draw up from y1 */
                            break;
                        case TICKS_OUT:
                            drawtic(bot - dell, start, axis, xinvflag ? 0 : 1); /* draw down from y2 */
                            break;
                        case TICKS_BOTH:
                            drawtic(bot - dell, start, axis, xinvflag ? 1 : 0); /* draw up from y1 */
                            drawtic(bot - dell, start, axis, xinvflag ? 0 : 1); /* draw down from y2 */
                            break;
                        }
                    }
                }
            }
        }
    skip1:;

        start = s;
        if (((t.t_op == PLACE_RIGHT && !xinvflag) || ((t.t_op == PLACE_LEFT) && xinvflag)) || t.t_op == PLACE_BOTH) {
            for (j = 0; j < nticks && start <= stop; j++) {
                logloop = 5;
                kstart = 0;
                if (t.t_type == TYPE_SPEC) {
                    start = t.t_specloc[j];
                    if (start < (w.yg1 - eps) || start > (w.yg2 + eps)) {
                        continue;
                    }
                } else {
                    if (islogy(gno) && i == 0) {
                        start = pow(10.0, log10(s) + j * t.tmajor);
                    } else if (islogy(gno) && i == 1) {
                        logloop = t.tminor;
                        start = pow(10.0, log10(s) + j * t.tmajor);
                        kstart = 0;
                    } else {
                        start = s + j * step;
                    }
                }
                if (logloop != 0) {
                    for (k = kstart; k < 10 / logloop - ((10 % logloop) ? 0 : 1); k++) {
                        if (islogy(gno) && i == 1) {
                            start = ((k + 1) * logloop) * pow(10.0, log10(s) + j * t.tmajor);
                            if (start > stop) {
                                goto skip2;
                            }
                        }
                        if (start < (w.yg1 - eps) || start > (w.yg2 + eps)) {
                            continue;
                        }
                        switch (t.t_inout) {
                        case TICKS_IN:
                            drawtic(top + delr, start, axis, xinvflag ? 0 : 1); /* draw down from y2 */
                            break;
                        case TICKS_OUT:
                            drawtic(top + delr, start, axis, xinvflag ? 1 : 0); /* draw up from y2 */
                            break;
                        case TICKS_BOTH:
                            drawtic(top + delr, start, axis, xinvflag ? 0 : 1); /* draw down from y2 */
                            drawtic(top + delr, start, axis, xinvflag ? 1 : 0); /* draw up from y2 */
                            break;
                        }
                    }
                }
            }
        }
    skip2:;
    }
}

/* TODO */
int check_nticks(int gno, int axis, double gmin, double gmax, double tm, int maxnt) { return 1; }
