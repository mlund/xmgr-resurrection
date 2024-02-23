/* $Id: files.c,v 1.4 1995/06/02 03:23:59 pturner Exp pturner $
 *
 * read data files
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "globals.h"
#include "protos.h"

#ifdef VMS
#include "vms_unix.h"
#endif

#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
#include "netcdf.h"
#endif

#define MAXERR 50
#define MAX_LINE_LEN 512
/*
 * number of doubles to allocate for each call to realloc
 */
#define BUFSIZE 512

int realtime = 0;
int change_gno;      /* if the graph number changes on read in */
static int cur_gno;  /* if the graph number changes on read in */
int change_type;     /* current set type */
static int cur_type; /* current set type */

static int readerror = 0; /* number of errors */
static int readline = 0;  /* line number in file */

int readpoly(int gno, char* fn, FILE* fp, int readone);
int readrawspice(int gno, char* fn, FILE* fp);

int is_old_bin_format(char* fn);

int getdata(int gno, char* fn, int src, int type) {
    FILE* fp = NULL;
    int retval = -1;
    struct stat statb;

    switch (src) {
    case SOURCE_DISK:
        /* check to make sure this is a file and not a dir */
        if (stat(fn, &statb)) {
            sprintf(buf, "Can't open file %s", fn);
            errmsg(buf);
            return 0;
        }
        if (!S_ISREG(statb.st_mode)) {
            sprintf(buf, "File %s is not a regular file", fn);
            errmsg(buf);
            return 0;
        }
#if defined(HAVE_XDR)
        if (is_old_bin_format(fn)) {
            char pipename[MAXPATHLEN];
            sprintf(pipename, "%s/bin/grconvert %s -", xmgrdir, fn);
            if (yesno("File format no longer supported - reformat?", NULL, NULL, NULL)) {
                getdata(gno, pipename, SOURCE_PIPE, SET_XY);
                return 1;
            } else {
                return 0;
            }
        }
#endif
        fp = fopen(fn, "r");
        readline = 0;
        break;
    case SOURCE_PIPE:
        fp = (FILE*)popen(fn, "r");
        readline = 0;
        break;
    case SOURCE_STDIN:
        fp = stdin;
        readline = 0;
        break;
    default:
        errmsg("Wrong call to getdata()");
        return 0;
    }
    if (fp == NULL) {
        sprintf(buf, "Can't open file %s", fn);
        errmsg(buf);
        return 0;
    }
    cur_gno = gno;
    change_type = cur_type = type;
    set_work_pending(TRUE);
    while (retval == -1) {
        retval = 0;
        switch (cur_type) {
        case SET_XY:
            retval = readxy(cur_gno, fn, fp, 0);
            break;
        case SET_POLY:
            retval = readpoly(cur_gno, fn, fp, 0);
            break;
        case SET_NXY:
            retval = readnxy(cur_gno, fn, fp);
            break;
        case SET_IHL:
            retval = readihl(cur_gno, fn, fp);
            break;
        case SET_BIN:
            retval = readbinary(cur_gno, fn, fp);
            break;
        case SET_XYDX:
        case SET_XYDY:
        case SET_XYDXDX:
        case SET_XYDYDY:
        case SET_XYDXDY:
        case SET_XYZ:
        case SET_XYRT:
        case SET_XYHILO:
        case SET_XYBOXPLOT:
        case SET_XYUV:
        case SET_XYBOX:
            retval = readxxyy(cur_gno, fn, fp, cur_type);
            break;
        case SET_XYSTRING:
            retval = readxystring(cur_gno, fn, fp, 0);
            break;
        case SET_BLOCK:
            retval = readblockdata(cur_gno, fn, fp);
            break;
        case SET_RAWSPICE:
            retval = readrawspice(cur_gno, fn, fp);
            break;
        }
    }
    if (src == SOURCE_PIPE) {
        pclose(fp);
    } else {
        if (fp != stdin) { /* leave stdin open */
            fclose(fp);
        }
    }
    set_work_pending(FALSE);
#ifndef NONE_GUI
    update_status_popup(NULL, NULL, NULL);
#endif
    return retval;
}

/*
 * read a line parsing x as given by xformat
 */
int scanline(char* buf, int xformat, double* x, double* y) {
    int pstat, mo, da, yr, hr, mi;
    double sec;
    switch (xformat) {
    case FORMAT_GENERAL:
    case FORMAT_DECIMAL:
        return sscanf(buf, "%lf %lf", x, y);
        break;
    case FORMAT_HMS:
        if ((pstat = sscanf(buf, "%d:%d:%lf %lf", &hr, &mi, &sec, y)) != 4) {
            return 0;
        } else {
            mo = 1;
            da = 1;
            yr = 1970;
            *x = julday(mo, da, yr, 12, 0, 0.0);
            printf("%d %d %d %f %f\n", mo, da, yr, *x, *y);
            return 4;
        }
        break;
    case FORMAT_MMDDYY:
        if ((pstat = sscanf(buf, "%d-%d-%d %lf", &mo, &da, &yr, y)) != 4) {
            return 0;
        } else {
            *x = julday(mo, da, yr, 12, 0, 0.0);
            printf("%d %d %d %f %f\n", mo, da, yr, *x, *y);
            return 4;
        }
        break;
    case FORMAT_YYMMDD:
        if ((pstat = sscanf(buf, "%d-%d-%d %lf", &yr, &mo, &da, y)) != 4) {
            return 0;
        } else {
            *x = julday(mo, da, yr, 12, 0, 0.0);
            printf("%d %d %d %f %f\n", mo, da, yr, *x, *y);
            return 4;
        }
        break;
    case FORMAT_MMDDYYHMS:
        if ((pstat = sscanf(buf, "%d-%d-%d %d:%d:%lf %lf", &mo, &da, &yr, &hr, &mi, &sec, y)) != 7) {
            return 0;
        } else {
            *x = julday(mo, da, yr, hr, mi, sec);
            return 7;
        }
        break;
    case FORMAT_YYMMDDHMS:
        if ((pstat = sscanf(buf, "%d-%d-%d %d:%d:%lf %lf", &yr, &mo, &da, &hr, &mi, &sec, y)) != 7) {
            return 0;
        } else {
            *x = julday(mo, da, yr, hr, mi, sec);
            return 7;
        }
        break;
    default:
        return sscanf(buf, "%lf %lf", x, y);
        break;
    }
}

/*
 * read file type xy
 */
int readxy(int gno, char* fn, FILE* fp, int readone) {
    extern int readxformat; /* TODO to globals.h */
    int i = 0, ll, j, pstat, readset = 0, retval = 0;
    double *x, *y;

    x = (double*)calloc(BUFSIZE, sizeof(double));
    y = (double*)calloc(BUFSIZE, sizeof(double));
    if (x == NULL || y == NULL) {
        errmsg("Insufficient memory for set");
        cxfree(x);
        cxfree(y);
        return (0);
    }
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        readline++;
        ll = strlen(buf);
        if ((ll > 0) && (buf[ll - 1] != '\n')) { /* must have a newline
                                                  * char at end of line */
            readerror++;
            fprintf(stderr, "No newline at line #%1d: %s\n", readline, buf);
            if (readerror > MAXERR) {
                if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                    cxfree(x);
                    cxfree(y);
                    return (0);
                } else {
                    readerror = 0;
                }
            }
            continue;
        }
        if (buf[0] == '#') {
            continue;
        }
        if (strlen(buf) < 2) { /* blank line */
            continue;
        }
        if (buf[0] == '@') {
            change_gno = -1;
            change_type = cur_type;
            read_param(buf + 1);
            if (change_gno >= 0) {
                cur_gno = gno = change_gno;
            }
            if (change_type != cur_type) {
                cur_type = change_type;
                retval = -1;
                break; /* exit this module and store any set */
            }
            continue;
        }
        convertchar(buf);
        /* count the number of items scanned */
        if ((pstat = scanline(buf, readxformat, &x[i], &y[i])) >= 1) {
            /* supply x if missing (y winds up in x) */
            if (pstat == 1) {
                y[i] = x[i];
                x[i] = i;
            }
            if (realtime == 1 && inwin) {
                drawpolysym(&x[i], &y[i], 1, 3, 0, 0, 1.0);
            }
            /* got x and y so increment */
            i++;
            if (i % BUFSIZE == 0) {
                x = (double*)realloc(x, (i + BUFSIZE) * sizeof(double));
                y = (double*)realloc(y, (i + BUFSIZE) * sizeof(double));
            }
        } else {
            if (i != 0) {
                if ((j = nextset(gno)) == -1) {
                    cxfree(x);
                    cxfree(y);
                    return (readset);
                }
                activateset(gno, j);
                settype(gno, j, SET_XY);
                setcol(gno, x, j, i, 0);
                setcol(gno, y, j, i, 1);
                if (!strlen(getcomment(gno, j))) {
                    setcomment(gno, j, fn);
                }
                log_results(fn);
                updatesetminmax(gno, j);
                if (realtime == 2 && inwin) {
                    drawsetxy(gno, &g[gno].p[j], j);
                }
                readset++;
            } else {
                readerror++;
                fprintf(stderr, "Error at line #%1d: %s", readline, buf);
                if (readerror > MAXERR) {
                    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                        cxfree(x);
                        cxfree(y);
                        return (0);
                    } else {
                        readerror = 0;
                    }
                }
            }
            i = 0;
            x = (double*)calloc(BUFSIZE, sizeof(double));
            y = (double*)calloc(BUFSIZE, sizeof(double));
            if (x == NULL || y == NULL) {
                errmsg("Insufficient memory for set");
                cxfree(x);
                cxfree(y);
                return (readset);
            }
            if (readone) {
                return (-2);
            }
        }
    }
    if (i != 0) {
        if ((j = nextset(gno)) == -1) {
            cxfree(x);
            cxfree(y);
            return (readset);
        }
        activateset(gno, j);
        settype(gno, j, SET_XY);
        setcol(gno, x, j, i, 0);
        setcol(gno, y, j, i, 1);
        if (!strlen(getcomment(gno, j))) {
            setcomment(gno, j, fn);
        }
        log_results(fn);
        updatesetminmax(gno, j);
        if (realtime == 2 && inwin) {
            /*
             * TODO ??? drawsetxy(g[gno].p[j]);
             */
        }
        readset++;
    } else {
        cxfree(x);
        cxfree(y);
    }
    if (retval == -1) {
        return retval;
    } else {
        return readset;
    }
}

int remove_missing_data(double* x, double* y, double* flag, int n) {
    int i, cnt = 0, iflag = 0;
    extern double missing; /* TODO - move to globals.h */
    for (i = 0; i < n; i++) {
        if (x[i] != missing && y[i] != missing) {
            x[cnt] = x[i];
            y[cnt] = y[i];
            if (iflag) {
                flag[cnt] = 1.0;
            } else {
                iflag = 1;
                flag[cnt] = 0.0;
            }
            cnt++;
        } else {
            iflag = 0;
        }
    }
    return cnt;
}

/*
 * read file type SET_POLY
 */
int readpoly(int gno, char* fn, FILE* fp, int readone) {
    int i = 0, ll, j, pstat, readset = 0, retval = 0;
    double *x, *y, *flag;

    x = (double*)calloc(BUFSIZE, sizeof(double));
    y = (double*)calloc(BUFSIZE, sizeof(double));
    flag = (double*)calloc(BUFSIZE, sizeof(double));
    if (x == NULL || y == NULL || flag == NULL) {
        errmsg("Insufficient memory for set");
        cxfree(x);
        cxfree(y);
        cxfree(flag);
        return (0);
    }
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        readline++;
        ll = strlen(buf);
        if ((ll > 0) && (buf[ll - 1] != '\n')) { /* must have a newline
                                                  * char at end of line */
            readerror++;
            fprintf(stderr, "No newline at line #%1d: %s\n", readline, buf);
            if (readerror > MAXERR) {
                if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                    cxfree(x);
                    cxfree(y);
                    cxfree(flag);
                    return (0);
                } else {
                    readerror = 0;
                }
            }
            continue;
        }
        if (buf[0] == '#') {
            continue;
        }
        if (strlen(buf) < 2) { /* blank line */
            continue;
        }
        if (buf[0] == '@') {
            change_gno = -1;
            change_type = cur_type;
            read_param(buf + 1);
            if (change_gno >= 0) {
                cur_gno = gno = change_gno;
            }
            if (change_type != cur_type) {
                cur_type = change_type;
                retval = -1;
                break; /* exit this module and store any set */
            }
            continue;
        }
        convertchar(buf);
        /* count the number of items scanned */
        if ((pstat = sscanf(buf, "%lf %lf %lf", &x[i], &y[i], &flag[i])) >= 1) {
            /* supply x if missing (y winds up in x) */
            if (pstat == 1) {
                y[i] = x[i];
                x[i] = i;
            }
            if (realtime == 1 && inwin) {
                drawpolysym(&x[i], &y[i], 1, 3, 0, 0, 1.0);
            }
            /* got x and y so increment */
            i++;
            if (i % BUFSIZE == 0) {
                x = (double*)realloc(x, (i + BUFSIZE) * sizeof(double));
                y = (double*)realloc(y, (i + BUFSIZE) * sizeof(double));
                flag = (double*)realloc(flag, (i + BUFSIZE) * sizeof(double));
            }
        } else {
            if (i != 0) {
                int ntmp;
                if ((j = nextset(gno)) == -1) {
                    cxfree(x);
                    cxfree(y);
                    cxfree(flag);
                    return (readset);
                }
                ntmp = remove_missing_data(x, y, flag, i);
                if (i != ntmp) {
                    i = ntmp;
                    x = (double*)realloc(x, ntmp * sizeof(double));
                    y = (double*)realloc(y, ntmp * sizeof(double));
                    flag = (double*)realloc(flag, ntmp * sizeof(double));
                }
                if (i > 0) {
                    activateset(gno, j);
                    settype(gno, j, SET_POLY);
                    setcol(gno, x, j, i, 0);
                    setcol(gno, y, j, i, 1);
                    setcol(gno, flag, j, i, 2);
                    if (!strlen(getcomment(gno, j))) {
                        setcomment(gno, j, fn);
                    }
                    log_results(fn);
                    updatesetminmax(gno, j);
                    readset++;
                }
            } else {
                readerror++;
                fprintf(stderr, "Error at line #%1d: %s", readline, buf);
                if (readerror > MAXERR) {
                    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                        cxfree(x);
                        cxfree(y);
                        cxfree(flag);
                        return (0);
                    } else {
                        readerror = 0;
                    }
                }
            }
            i = 0;
            x = (double*)calloc(BUFSIZE, sizeof(double));
            y = (double*)calloc(BUFSIZE, sizeof(double));
            flag = (double*)calloc(BUFSIZE, sizeof(double));
            if (x == NULL || y == NULL) {
                errmsg("Insufficient memory for set");
                cxfree(x);
                cxfree(y);
                cxfree(flag);
                return (readset);
            }
            if (readone) {
                return (-2);
            }
        }
    }
    if (i != 0) {
        int ntmp;
        if ((j = nextset(gno)) == -1) {
            cxfree(x);
            cxfree(y);
            cxfree(flag);
            return (readset);
        }
        ntmp = remove_missing_data(x, y, flag, i);
        if (i != ntmp) {
            i = ntmp;
            x = (double*)realloc(x, ntmp * sizeof(double));
            y = (double*)realloc(y, ntmp * sizeof(double));
            flag = (double*)realloc(flag, ntmp * sizeof(double));
        }
        if (i > 0) {
            activateset(gno, j);
            settype(gno, j, SET_POLY);
            setcol(gno, x, j, i, 0);
            setcol(gno, y, j, i, 1);
            setcol(gno, flag, j, i, 2);
            if (!strlen(getcomment(gno, j))) {
                setcomment(gno, j, fn);
            }
            log_results(fn);
            updatesetminmax(gno, j);
            readset++;
        }
    } else {
        cxfree(x);
        cxfree(y);
        cxfree(flag);
    }
    if (retval == -1) {
        return retval;
    } else {
        return readset;
    }
}

/*
 * read the first set found in a file to set setno
 */
int read_set_fromfile(int gno, int setno, char* fn, int src) {
    FILE* fp = NULL;
    struct stat statb;
    int readline = 0;
    int i = 0, pstat, retval = 0;
    double *x, *y;

    switch (src) {
    case SOURCE_DISK:
        /* check to make sure this is a file and not a dir */
        if (stat(fn, &statb)) {
            sprintf(buf, "Can't get information about file %s", fn);
            errmsg(buf);
            return 0;
        }
        if (!S_ISREG(statb.st_mode)) {
            sprintf(buf, "File %s is not a regular file", fn);
            errmsg(buf);
            return 0;
        }
        fp = fopen(fn, "r");
        break;
    case SOURCE_PIPE:
        fp = (FILE*)popen(fn, "r");
        break;
    case 2:
        fp = stdin;
        break;
    }
    if (fp == NULL) {
        sprintf(buf, "Can't open file %s", fn);
        errmsg(buf);
        return 0;
    }
    softkillset(gno, setno);
    x = (double*)calloc(BUFSIZE, sizeof(double));
    y = (double*)calloc(BUFSIZE, sizeof(double));
    if (x == NULL || y == NULL) {
        errmsg("Insufficient memory for set");
        cxfree(x);
        cxfree(y);
        goto breakout;
    }
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        readline++;
        if (buf[strlen(buf) - 1] != '\n') { /* must have a newline char
                                             * at end of line */
            readerror++;
            fprintf(stderr, "No newline at line #%1d: %s", readline, buf);
            continue;
        }
        if (buf[0] == '#') {
            continue;
        }
        if (buf[0] == '@') {
            continue;
        }
        convertchar(buf);
        /* count the number of items scanned */
        if ((pstat = sscanf(buf, "%lf %lf", &x[i], &y[i])) >= 1) {
            /* supply x if missing (y winds up in x) */
            if (pstat == 1) {
                y[i] = x[i];
                x[i] = i;
            }
            i++;
            if (i % BUFSIZE == 0) {
                x = (double*)realloc(x, (i + BUFSIZE) * sizeof(double));
                y = (double*)realloc(y, (i + BUFSIZE) * sizeof(double));
            }
        }
    }
    activateset(gno, setno);
    settype(gno, setno, SET_XY);
    setcol(gno, x, setno, i, 0);
    setcol(gno, y, setno, i, 1);
    if (!strlen(getcomment(gno, setno))) {
        setcomment(gno, setno, fn);
    }
    log_results(fn);
    updatesetminmax(gno, setno);
    retval = 1;

breakout:;

    if (src == SOURCE_PIPE) {
        pclose(fp);
    } else {
        if (fp != stdin) { /* leave stdin open */
            fclose(fp);
        }
    }
    return retval;
}

/*
 * read IHL format
 */
int readihl(int gno, char* fn, FILE* fp) {
    int i, j, pstat, npts;
    double *x, *y, tmp;

    i = 0;
    pstat = 0;
    if ((j = nextset(gno)) == -1) {
        return 0;
    }
    if (fgets(buf, MAX_LINE_LEN, fp) == NULL) {
        errmsg("Can't read from file");
        killset(gno, j);
        return 0;
    }
    readline++;
    pstat = sscanf(buf, "%d", &npts);
    if (npts == 0) {
        errmsg("Number of points = 0");
        killset(gno, j);
        return 0;
    }
    activateset(gno, j);
    settype(gno, j, SET_XY);
    setlength(gno, j, npts);
    if (!strlen(getcomment(gno, j))) {
        setcomment(gno, j, fn);
    }
    log_results(fn);
    x = getx(gno, j);
    y = gety(gno, j);
    for (i = 0; i < npts; i++) {
        if (fgets(buf, MAX_LINE_LEN, fp) == NULL) {
            errmsg("Premature EOF");
            updatesetminmax(gno, j);
            return 1;
        }
        readline++;
        convertchar(buf);
        pstat = sscanf(buf, "%lf %lf %lf", &tmp, &x[i], &y[i]);
    }
    updatesetminmax(gno, j);
    return 1;
}

/*
 * read x1 y1 y2 ... y30 formatted files
 * note that the maximum number of sets is 30
 */
#define MAXSETN 30

int readnxy(int gno, char* fn, FILE* fp) {
    int i, j, pstat, cnt, scnt[MAXSETN], setn[MAXSETN], retval = 0;
    double *x[MAXSETN], *y[MAXSETN], xval, yr[MAXSETN];
    char *s, buf[1024], tmpbuf[1024];
    int do_restart = 0;

    /* if more than one set of nxy data is in the file,
     * leap to here after each is read - the goto is at the
     * bottom of this module.
     */
restart:;

    i = 0;
    pstat = 0;
    cnt = 0;
    while ((fgets(buf, MAX_LINE_LEN, fp) != NULL) && ((buf[0] == '#') || (buf[0] == '@'))) {
        readline++;
        if (buf[0] == '@') {
            change_gno = -1;
            read_param(buf + 1);
            if (change_gno >= 0) {
                cur_gno = gno = change_gno;
            }
        }
    }
    convertchar(buf);

    /*
     * count the columns
     */
    strcpy(tmpbuf, buf);
    s = tmpbuf;
    while ((s = strtok(s, " \t\n")) != NULL) {
        cnt++;
        s = NULL;
    }
    if (cnt > maxplot) {
        errmsg("Maximum number of columns exceeded, reading first 31");
        cnt = 31;
    }
    s = buf;
    s = strtok(s, " \t\n");
    if (s == NULL) {
        errmsg("Read ended by a blank line at or near the beginning of file");
        return 0;
    }
    pstat = sscanf(s, "%lf", &xval);
    if (pstat == 0) {
        errmsg("Read ended, non-numeric found on line at or near beginning of file");
        return 0;
    }
    s = NULL;
    for (j = 0; j < cnt - 1; j++) {
        s = strtok(s, " \t\n");
        if (s == NULL) {
            yr[j] = 0.0;
            errmsg("Number of items in column incorrect");
        } else {
            yr[j] = atof(s);
        }
        s = NULL;
    }
    if (cnt > 1) {
        for (i = 0; i < cnt - 1; i++) {
            if ((setn[i] = nextset(gno)) == -1) {
                for (j = 0; j < i; j++) {
                    killset(gno, setn[j]);
                }
                return 0;
            }
            activateset(gno, setn[i]);
            settype(gno, setn[i], SET_XY);
            x[i] = (double*)calloc(BUFSIZE, sizeof(double));
            y[i] = (double*)calloc(BUFSIZE, sizeof(double));
            if (x[i] == NULL || y[i] == NULL) {
                errmsg("Insufficient memory for set");
                cxfree(x[i]);
                cxfree(y[i]);
                for (j = 0; j < i + 1; j++) {
                    killset(gno, setn[j]);
                }
                return (0);
            }
            *(x[i]) = xval;
            *(y[i]) = yr[i];
            scnt[i] = 1;
        }
        while (!do_restart && (fgets(buf, MAX_LINE_LEN, fp) != NULL)) {
            readline++;
            if (buf[0] == '#') {
                continue;
            }
            if (strlen(buf) < 2) {
                continue;
            }
            if (buf[0] == '@') {
                change_gno = -1;
                change_type = cur_type;
                read_param(buf + 1);
                if (change_gno >= 0) {
                    cur_gno = gno = change_gno;
                }
                if (change_type != cur_type) {
                    cur_type = change_type;
                    retval = -1;
                    break; /* exit this module and store any set */
                }
                continue;
            }
            convertchar(buf);
            s = buf;
            s = strtok(s, " \t\n");
            if (s == NULL) {
                continue;
            }
            /* check for set separator */
            pstat = sscanf(s, "%lf", &xval);
            if (pstat == 0) {
                do_restart = 1;
                continue;
            } else {
                s = NULL;
                for (j = 0; j < cnt - 1; j++) {
                    s = strtok(s, " \t\n");
                    if (s == NULL) {
                        yr[j] = 0.0;
                        errmsg("Number of items in column incorrect");
                    } else {
                        yr[j] = atof(s);
                    }
                    s = NULL;
                }
                for (i = 0; i < cnt - 1; i++) {
                    *(x[i] + scnt[i]) = xval;
                    *(y[i] + scnt[i]) = yr[i];
                    scnt[i]++;
                    if (scnt[i] % BUFSIZE == 0) {
                        x[i] = (double*)realloc(x[i], (scnt[i] + BUFSIZE) * sizeof(double));
                        y[i] = (double*)realloc(y[i], (scnt[i] + BUFSIZE) * sizeof(double));
                    }
                }
            }
        }
        for (i = 0; i < cnt - 1; i++) {
            setcol(gno, x[i], setn[i], scnt[i], 0);
            setcol(gno, y[i], setn[i], scnt[i], 1);
            sprintf(buf, "%s:%d", fn, i + 1); /* identify column # in comment */
            setcomment(gno, setn[i], buf);
            log_results(fn);
            updatesetminmax(gno, setn[i]);
        }
        if (!do_restart) {
            if (retval == -1) {
                return retval;
            } else {
                return 1;
            }
        } else {
            do_restart = 0;
            goto restart;
        }
    }
    return 0;
}

int readbinary(int gno, char* fn, FILE* fp) {
    int i, j, setn, nsets = 0, npts;
    double *x, *y;
    float *xf, *yf;

    fread(&nsets, sizeof(int), 1, fp);
    if (nsets > g[gno].maxplot) {
        sprintf(buf, "Not enough sets: have %d, need %d", g[gno].maxplot, nsets);
        errmsg(buf);
        return 0;
    }
    for (i = 0; i < nsets; i++) {
        fread(&npts, sizeof(int), 1, fp);
        if (npts > 0) {
            x = (double*)calloc(npts, sizeof(double));
            if (x == NULL) {
                errmsg("Can't calloc in readbinary");
                return 0;
            }
            y = (double*)calloc(npts, sizeof(double));
            if (y == NULL) {
                errmsg("Can't calloc in readbinary");
                cxfree(x);
                return 0;
            }
            xf = (float*)calloc(npts, sizeof(float));
            if (xf == NULL) {
                errmsg("Can't calloc in readbinary");
                return 0;
            }
            yf = (float*)calloc(npts, sizeof(float));
            if (yf == NULL) {
                errmsg("Can't calloc in readbinary");
                cxfree(xf);
                return 0;
            }
            fread(xf, sizeof(float), npts, fp);
            fread(yf, sizeof(float), npts, fp);
            for (j = 0; j < npts; j++) {
                x[j] = xf[j];
                y[j] = yf[j];
            }
            free(xf);
            free(yf);
            if ((setn = nextset(gno)) == -1) {
                cxfree(x);
                cxfree(y);
                return 0;
            }
            activateset(gno, setn);
            settype(gno, setn, SET_XY);
            setcol(gno, x, setn, npts, 0);
            setcol(gno, y, setn, npts, 1);
            if (!strlen(getcomment(gno, j))) {
                setcomment(gno, setn, fn);
            }
            log_results(fn);
            updatesetminmax(gno, setn);
        }
    }
    return 1;
}

int readxystring(int gno, char* fn, FILE* fp, int readone) {
    int i = 0, ll, j, pstat, readset = 0, retval = 0;
    double *x, *y;
    char *s, *s1, *s2, **strs;

    x = (double*)calloc(BUFSIZE, sizeof(double));
    y = (double*)calloc(BUFSIZE, sizeof(double));
    strs = (char**)calloc(BUFSIZE, sizeof(char*));
    if (x == NULL || y == NULL || strs == NULL) {
        errmsg("Insufficient memory for set");
        cxfree(x);
        cxfree(y);
        cxfree(strs);
        return (0);
    }
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        readline++;
        ll = strlen(buf);
        if ((ll > 0) && (buf[ll - 1] != '\n')) { /* must have a newline
                                                  * char at end of line */
            readerror++;
            fprintf(stderr, "No newline at line #%1d: %s\n", readline, buf);
            if (readerror > MAXERR) {
                if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                    cxfree(x);
                    cxfree(y);
                    cxfree(strs);
                    return (0);
                } else {
                    readerror = 0;
                }
            }
            continue;
        }
        if (buf[0] == '#') {
            continue;
        }
        if (strlen(buf) < 2) { /* blank line */
            continue;
        }
        if (buf[0] == '@') {
            change_gno = -1;
            change_type = cur_type;
            read_param(buf + 1);
            if (change_gno >= 0) {
                cur_gno = gno = change_gno;
            }
            if (change_type != cur_type) {
                cur_type = change_type;
                retval = -1;
                break; /* exit this module and store any set */
            }
            continue;
        }
        /* count the number of items scanned */
        if ((pstat = sscanf(buf, "%lf %lf", &x[i], &y[i])) >= 1) {
            /* supply x if missing (y winds up in x) */
            if (pstat == 1) {
                y[i] = x[i];
                x[i] = i;
            }
            /* get the string portion */
            buf[strlen(buf) - 1] = 0; /* remove newline */
            s1 = strrchr(buf, '"');   /* find last quote */
            s2 = strchr(buf, '"');    /* find first quote */
            if (s1 != s2) {           /* a quoted string */
                s = s1;
                s[0] = 0; /* terminate the string here */
                s = s2;
                s++; /* increment to the first char */
                strs[i] = (char*)malloc((strlen(s) + 1) * sizeof(char));
                strcpy(strs[i], s);
                if (realtime == 1 && inwin) {
                    drawpolysym(&x[i], &y[i], 1, 3, 0, 0, 1.0);
                }
                /* got x and y so increment */
                i++;
                if (i % BUFSIZE == 0) {
                    x = (double*)realloc(x, (i + BUFSIZE) * sizeof(double));
                    y = (double*)realloc(y, (i + BUFSIZE) * sizeof(double));
                    strs = (char**)realloc(strs, (i + BUFSIZE) * sizeof(char*));
                }
            } else {
                readerror++;
                fprintf(stderr, "Error at line #%1d: %s\n", readline, buf);
                if (readerror > MAXERR) {
                    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                        cxfree(x);
                        cxfree(y);
                        cxfree(strs);
                        return (0);
                    } else {
                        readerror = 0;
                    }
                }
            }
        } else {
            if (i != 0) {
                if ((j = nextset(gno)) == -1) {
                    cxfree(x);
                    cxfree(y);
                    return (readset);
                }
                activateset(gno, j);
                settype(gno, j, SET_XYSTRING);
                setcol(gno, x, j, i, 0);
                setcol(gno, y, j, i, 1);
                g[gno].p[j].s = strs;
                if (!strlen(getcomment(gno, j))) {
                    setcomment(gno, j, fn);
                }
                log_results(fn);
                updatesetminmax(gno, j);
                if (realtime == 2 && inwin) {
                    drawsetxy(gno, &g[gno].p[j], j);
                }
                readset++;
            } else {
                readerror++;
                fprintf(stderr, "Error at line #%1d: %s", readline, buf);
                if (readerror > MAXERR) {
                    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                        cxfree(x);
                        cxfree(y);
                        cxfree(strs);
                        return (0);
                    } else {
                        readerror = 0;
                    }
                }
            }
            i = 0;
            x = (double*)calloc(BUFSIZE, sizeof(double));
            y = (double*)calloc(BUFSIZE, sizeof(double));
            strs = (char**)calloc(BUFSIZE, sizeof(char*));
            if (x == NULL || y == NULL) {
                errmsg("Insufficient memory for set");
                cxfree(x);
                cxfree(y);
                return (readset);
            }
            if (readone) {
                return (-2);
            }
        }
    }

    if (i != 0) {
        if ((j = nextset(gno)) == -1) {
            cxfree(x);
            cxfree(y);
            cxfree(strs);
            return (readset);
        }
        activateset(gno, j);
        settype(gno, j, SET_XYSTRING);
        setcol(gno, x, j, i, 0);
        setcol(gno, y, j, i, 1);
        g[gno].p[j].s = strs;
        if (!strlen(getcomment(gno, j))) {
            setcomment(gno, j, fn);
        }
        log_results(fn);
        updatesetminmax(gno, j);
        if (realtime == 2 && inwin) {
            /*
             * TODO ??? drawsetxy(g[gno].p[j]);
             */
        }
        readset++;
    } else {

        cxfree(x);
        cxfree(y);
        cxfree(strs);
    }

    if (retval == -1) {
        return retval;
    } else {

        return readset;
    }
}

/*
 * read file types using dx and/or dy
 */
int readxxyy(int gno, char* fn, FILE* fp, int type) {
    int i = 0, j = 0, pstat, readset = 0, retval = 0;
    double *x, *y, *dx, *dy, *dz, *dw;
    double xtmp, ytmp, dxtmp, dytmp, dztmp, dwtmp;

    x = y = dx = dy = dz = dw = NULL;
    x = (double*)calloc(BUFSIZE, sizeof(double));
    y = (double*)calloc(BUFSIZE, sizeof(double));
    switch (type) {
    case SET_XYZ:
    case SET_XYRT:
    case SET_XYDX:
    case SET_XYDY:
        dx = (double*)calloc(BUFSIZE, sizeof(double));
        break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
    case SET_XYUV:
        dx = (double*)calloc(BUFSIZE, sizeof(double));
        dy = (double*)calloc(BUFSIZE, sizeof(double));
        break;
    case SET_XYHILO:
    case SET_XYBOX:
        dx = (double*)calloc(BUFSIZE, sizeof(double));
        dy = (double*)calloc(BUFSIZE, sizeof(double));
        dz = (double*)calloc(BUFSIZE, sizeof(double));
        break;
    case SET_XYBOXPLOT:
        dx = (double*)calloc(BUFSIZE, sizeof(double));
        dy = (double*)calloc(BUFSIZE, sizeof(double));
        dz = (double*)calloc(BUFSIZE, sizeof(double));
        dw = (double*)calloc(BUFSIZE, sizeof(double));
        break;
    default:
        dx = (double*)calloc(BUFSIZE, sizeof(double));
        dy = (double*)calloc(BUFSIZE, sizeof(double));
        break;
    }
    if (x == NULL || y == NULL) {
        errmsg("Insufficient memory for set");
        cxfree(x);
        cxfree(y);
        cxfree(dx);
        cxfree(dy);
        cxfree(dz);
        cxfree(dw);
        return (0);
    }
    while (fgets(buf, MAX_LINE_LEN, fp) != NULL) {
        readline++;
        if (buf[0] == '#') {
            continue;
        }
        if (strlen(buf) < 2) {
            continue;
        }
        if (buf[0] == '@') {
            change_gno = -1;
            change_type = cur_type;
            read_param(buf + 1);
            if (change_gno >= 0) {
                cur_gno = gno = change_gno;
            }
            if (change_type != cur_type) {
                if (change_type != cur_type) {
                    cur_type = change_type;
                    retval = -1;
                    break; /* exit this module and store any set */
                }
            }
            continue;
        }
        convertchar(buf);
        /* count the number of items scanned */
        if ((pstat = sscanf(buf, "%lf %lf %lf %lf %lf %lf", &xtmp, &ytmp, &dxtmp, &dytmp, &dztmp, &dwtmp)) >= 1) {
            /* got x and y so increment */
            x[i] = xtmp;
            y[i] = ytmp;
            if (type == SET_XYDX || type == SET_XYDY || type == SET_XYZ || type == SET_XYRT) {
                dx[i] = dxtmp;
            } else if (type == SET_XYHILO || type == SET_XYBOX) {
                dx[i] = dxtmp;
                dy[i] = dytmp;
                dz[i] = dztmp;
            } else if (type == SET_XYBOXPLOT) {
                dx[i] = dxtmp;
                dy[i] = dytmp;
                dz[i] = dztmp;
                dw[i] = dwtmp;
            } else {
                dx[i] = dxtmp;
                dy[i] = dytmp;
            }
            i++;
            if (i % BUFSIZE == 0) {
                x = (double*)realloc(x, (i + BUFSIZE) * sizeof(double));
                y = (double*)realloc(y, (i + BUFSIZE) * sizeof(double));
                switch (type) {
                case SET_XYDX:
                case SET_XYDY:
                case SET_XYZ:
                case SET_XYRT:
                    dx = (double*)realloc(dx, (i + BUFSIZE) * sizeof(double));
                    break;
                case SET_XYDXDX:
                case SET_XYDYDY:
                case SET_XYDXDY:
                case SET_XYUV:
                    dx = (double*)realloc(dx, (i + BUFSIZE) * sizeof(double));
                    dy = (double*)realloc(dy, (i + BUFSIZE) * sizeof(double));
                    break;
                case SET_XYHILO:
                case SET_XYBOX:
                    dx = (double*)realloc(dx, (i + BUFSIZE) * sizeof(double));
                    dy = (double*)realloc(dy, (i + BUFSIZE) * sizeof(double));
                    dz = (double*)realloc(dz, (i + BUFSIZE) * sizeof(double));
                    break;
                case SET_XYBOXPLOT:
                    dx = (double*)realloc(dx, (i + BUFSIZE) * sizeof(double));
                    dy = (double*)realloc(dy, (i + BUFSIZE) * sizeof(double));
                    dz = (double*)realloc(dz, (i + BUFSIZE) * sizeof(double));
                    dw = (double*)realloc(dz, (i + BUFSIZE) * sizeof(double));
                    break;
                default:
                    dx = (double*)realloc(dx, (i + BUFSIZE) * sizeof(double));
                    dy = (double*)realloc(dy, (i + BUFSIZE) * sizeof(double));
                    break;
                }
            }
        } else {
            if (i != 0) {
                if ((j = nextset(gno)) == -1) {
                    cxfree(x);
                    cxfree(y);
                    cxfree(dx);
                    cxfree(dy);
                    cxfree(dz);
                    cxfree(dw);
                    return readset;
                }
                activateset(gno, j);
                settype(gno, j, type);
                setcol(gno, x, j, i, 0);
                setcol(gno, y, j, i, 1);
                setcol(gno, dx, j, i, 2);
                setcol(gno, dy, j, i, 3);
                setcol(gno, dz, j, i, 4);
                setcol(gno, dw, j, i, 5);
                if (!strlen(getcomment(gno, j))) {
                    setcomment(gno, j, fn);
                }
                log_results(fn);
                updatesetminmax(gno, j);
                readset++;
            } else {
                readerror++;
                fprintf(stderr, "Error at line #%1d: %s", readline, buf);
                if (readerror > MAXERR) {
                    if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                        cxfree(x);
                        cxfree(y);
                        cxfree(dx);
                        cxfree(dy);
                        cxfree(dz);
                        cxfree(dw);
                        return (0);
                    } else {
                        readerror = 0;
                    }
                }
            }
            i = 0;
            x = (double*)calloc(BUFSIZE, sizeof(double));
            y = (double*)calloc(BUFSIZE, sizeof(double));
            switch (type) {
            case SET_XYDX:
            case SET_XYZ:
            case SET_XYRT:
            case SET_XYDY:
                dx = (double*)calloc(BUFSIZE, sizeof(double));
                break;
            case SET_XYDXDX:
            case SET_XYDYDY:
            case SET_XYDXDY:
            case SET_XYUV:
                dx = (double*)calloc(BUFSIZE, sizeof(double));
                dy = (double*)calloc(BUFSIZE, sizeof(double));
                break;
            case SET_XYHILO:
            case SET_XYBOX:
                dx = (double*)calloc(BUFSIZE, sizeof(double));
                dy = (double*)calloc(BUFSIZE, sizeof(double));
                dz = (double*)calloc(BUFSIZE, sizeof(double));
                break;
            case SET_XYBOXPLOT:
                dx = (double*)calloc(BUFSIZE, sizeof(double));
                dy = (double*)calloc(BUFSIZE, sizeof(double));
                dz = (double*)calloc(BUFSIZE, sizeof(double));
                dw = (double*)calloc(BUFSIZE, sizeof(double));
                break;
            default:
                dx = (double*)calloc(BUFSIZE, sizeof(double));
                dy = (double*)calloc(BUFSIZE, sizeof(double));
                break;
            }
            if (x == NULL || y == NULL) {
                errmsg("Insufficient memory for set");
                cxfree(x);
                cxfree(y);
                cxfree(dx);
                cxfree(dy);
                cxfree(dz);
                cxfree(dw);
                killset(gno, j);
                return (readset);
            }
        }
    }
    if (i != 0) {
        if ((j = nextset(gno)) == -1) {
            cxfree(x);
            cxfree(y);
            cxfree(dx);
            cxfree(dy);
            cxfree(dz);
            cxfree(dw);
            return readset;
        }
        activateset(gno, j);
        settype(gno, j, type);
        setcol(gno, x, j, i, 0);
        setcol(gno, y, j, i, 1);
        setcol(gno, dx, j, i, 2);
        setcol(gno, dy, j, i, 3);
        setcol(gno, dz, j, i, 4);
        setcol(gno, dw, j, i, 5);
        if (!strlen(getcomment(gno, j))) {
            setcomment(gno, j, fn);
        }
        log_results(fn);
        updatesetminmax(gno, j);
        readset++;
    } else {
        cxfree(x);
        cxfree(y);
        cxfree(dx);
        cxfree(dy);
        cxfree(dz);
        cxfree(dw);
    }
    if (retval == -1) {
        return retval;
    } else {
        return readset;
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
    if (ncols < maxplot) {
        ncols = maxplot;
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

/*
 * read block data
 */
int readblockdata(int gno, char* fn, FILE* fp) {
    int i = 0, j, k, ncols = 0, pstat;
    int first = 1, readerror = 0;
    double** data = NULL;
    char tmpbuf[2048], *s, tbuf[256];
    int linecount = 0;

    i = 0;
    pstat = 0;
    while ((s = fgets(buf, MAX_LINE_LEN, fp)) != NULL) {
        readline++;
        linecount++;
        if (buf[0] == '#') {
            continue;
        }
        if (buf[0] == '@') {
            read_param(buf + 1);
            continue;
        }
        if ((int)strlen(buf) > 1) {
            convertchar(buf);
            if (first) { /* count the number of columns */
                ncols = 0;
                strcpy(tmpbuf, buf);
                s = tmpbuf;
                while (*s == ' ' || *s == '\t' || *s == '\n') {
                    s++;
                }
                while ((s = strtok(s, " \t\n")) != NULL) {
                    ncols++;
                    s = NULL;
                }
                if (ncols < 1 || ncols > maxblock) {
                    errmsg("Column count incorrect");
                    return 0;
                }
                data = (double**)malloc(sizeof(double*) * maxblock);
                if (data == NULL) {
                    errmsg("Can't allocate memory for block data");
                    return 0;
                }
                for (j = 0; j < ncols; j++) {
                    data[j] = (double*)calloc(BUFSIZE, sizeof(double));
                    if (data[j] == NULL) {
                        errmsg("Insufficient memory for block data");
                        for (k = 0; k < j; k++) {
                            cxfree(data[k]);
                        }
                        cxfree(data);
                        return 0;
                    }
                }
                first = 0;
            }
            s = buf;
            while (*s == ' ' || *s == '\t' || *s == '\n') {
                s++;
            }
            for (j = 0; j < ncols; j++) {
                s = strtok(s, " \t\n");
                if (s == NULL) {
                    data[j][i] = 0.0;
                    sprintf(tbuf, "Number of items in column incorrect at line %d, line skipped", linecount);
                    errmsg(tbuf);
                    readerror++;
                    if (readerror > MAXERR) {
                        if (yesno("Lots of errors, abort?", NULL, NULL, NULL)) {
                            for (k = 0; k < ncols; k++) {
                                cxfree(data[k]);
                            }
                            cxfree(data);
                            return (0);
                        } else {
                            readerror = 0;
                        }
                    }
                    /* skip the rest */
                    goto bustout;
                } else {
                    data[j][i] = atof(s);
                }
                s = NULL;
            }
            i++;
            if (i % BUFSIZE == 0) {
                for (j = 0; j < ncols; j++) {
                    data[j] = (double*)realloc(data[j], (i + BUFSIZE) * sizeof(double));
                    if (data[j] == NULL) {
                        errmsg("Insufficient memory for block data");
                        for (k = 0; k < j; k++) {
                            cxfree(data[k]);
                        }
                        cxfree(data);
                        return 0;
                    }
                }
            }
        }
    bustout:;
    }
    for (j = 0; j < ncols; j++) {
        blockdata[j] = data[j];
    }
    cxfree(data);
    blocklen = i;
    blockncols = ncols;
    return 1;
}

void create_set_fromblock(int gno, int type, char* cols) {
    int i;
    int setno, graphno;
    int cx, cy, c1, c2, c3, c4;
    double *tx, *ty, *t2, *t3, *t4, *t5;
    int nc, *coli;
    char *s, buf[256];
    strcpy(buf, cols);
    s = buf;
    c1 = c2 = c3 = c4 = nc = 0;
    coli = (int*)malloc(maxblock * sizeof(int*));
    while ((s = strtok(s, ":")) != NULL) {
        coli[nc] = atoi(s);
        coli[nc]--;
        nc++;
        s = NULL;
    }
    if (nc == 0) {
        errmsg("No columns scanned in column string");
        free(coli);
        return;
    }
    for (i = 0; i < nc; i++) {
        if (coli[i] < 0 || coli[i] >= blockncols) {
            errmsg("Incorrect column specification");
            free(coli);
            return;
        }
    }

    cx = coli[0];
    cy = coli[1];
    if (cx >= blockncols) {
        errmsg("Column for X exceeds the number of columns in block data");
        free(coli);
        return;
    }
    if (cy >= blockncols) {
        errmsg("Column for Y exceeds the number of columns in block data");
        free(coli);
        return;
    }
    switch (type) {
    case SET_XY:
        break;
    case SET_XYRT:
    case SET_XYDX:
    case SET_XYDY:
    case SET_XYZ:
        c1 = coli[2];
        if (c1 >= blockncols) {
            errmsg("Column for E1 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
        c1 = coli[2];
        c2 = coli[3];
        if (c1 >= blockncols) {
            errmsg("Column for E1 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        if (c2 >= blockncols) {
            errmsg("Column for E2 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        break;
    case SET_XYHILO:
    case SET_XYBOX:
        c1 = coli[2];
        c2 = coli[3];
        c3 = coli[4];
        if (c1 >= blockncols) {
            errmsg("Column for E1 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        if (c2 >= blockncols) {
            errmsg("Column for E2 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        if (c3 >= blockncols) {
            errmsg("Column for E3 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        break;
    case SET_XYBOXPLOT:
        c1 = coli[2];
        c2 = coli[3];
        c3 = coli[4];
        c4 = coli[5];
        if (c1 >= blockncols) {
            errmsg("Column for E1 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        if (c2 >= blockncols) {
            errmsg("Column for E2 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        if (c3 >= blockncols) {
            errmsg("Column for E3 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        if (c4 >= blockncols) {
            errmsg("Column for E4 exceeds the number of columns in block data");
            free(coli);
            return;
        }
        break;
    }
    setno = -1;
    graphno = -1;

    if (graphno == -1) {
        graphno = cg;
    }
    if (setno == -1) {
        setno = nextset(graphno);
    }
    if (setno == -1) {
        return;
    }
    if (!g[graphno].active) {
        set_graph_active(graphno);
    }
    activateset(graphno, setno);
    settype(graphno, setno, type);

    tx = (double*)calloc(blocklen, sizeof(double));
    ty = (double*)calloc(blocklen, sizeof(double));
    for (i = 0; i < blocklen; i++) {
        tx[i] = blockdata[cx][i];
        ty[i] = blockdata[cy][i];
    }
    setcol(graphno, tx, setno, blocklen, 0);
    setcol(graphno, ty, setno, blocklen, 1);

    switch (type) {
    case SET_XY:
        sprintf(buf, "Cols %d %d", cx + 1, cy + 1);
        break;
    case SET_XYRT:
    case SET_XYDX:
    case SET_XYDY:
    case SET_XYZ:
        sprintf(buf, "Cols %d %d %d", cx + 1, cy + 1, c1 + 1);
        t2 = (double*)calloc(blocklen, sizeof(double));
        for (i = 0; i < blocklen; i++) {
            t2[i] = blockdata[c1][i];
        }
        setcol(graphno, t2, setno, blocklen, 2);
        break;
    case SET_XYDXDX:
    case SET_XYDYDY:
    case SET_XYDXDY:
        sprintf(buf, "Cols %d %d %d %d", cx + 1, cy + 1, c1 + 1, c2 + 1);
        t2 = (double*)calloc(blocklen, sizeof(double));
        t3 = (double*)calloc(blocklen, sizeof(double));
        for (i = 0; i < blocklen; i++) {
            t2[i] = blockdata[c1][i];
            t3[i] = blockdata[c2][i];
        }
        setcol(graphno, t2, setno, blocklen, 2);
        setcol(graphno, t3, setno, blocklen, 3);
        break;
    case SET_XYHILO:
    case SET_XYBOX:
        sprintf(buf, "Cols %d %d %d %d %d", cx + 1, cy + 1, c1 + 1, c2 + 1, c3 + 1);
        t2 = (double*)calloc(blocklen, sizeof(double));
        t3 = (double*)calloc(blocklen, sizeof(double));
        t4 = (double*)calloc(blocklen, sizeof(double));
        for (i = 0; i < blocklen; i++) {
            t2[i] = blockdata[c1][i];
            t3[i] = blockdata[c2][i];
            t4[i] = blockdata[c3][i];
        }
        setcol(graphno, t2, setno, blocklen, 2);
        setcol(graphno, t3, setno, blocklen, 3);
        setcol(graphno, t4, setno, blocklen, 4);
        break;
    case SET_XYBOXPLOT:
        sprintf(buf, "Cols %d %d %d %d %d %d", cx + 1, cy + 1, c1 + 1, c2 + 1, c3 + 1, c4 + 1);
        t2 = (double*)calloc(blocklen, sizeof(double));
        t3 = (double*)calloc(blocklen, sizeof(double));
        t4 = (double*)calloc(blocklen, sizeof(double));
        t5 = (double*)calloc(blocklen, sizeof(double));
        for (i = 0; i < blocklen; i++) {
            t2[i] = blockdata[c1][i];
            t3[i] = blockdata[c2][i];
            t4[i] = blockdata[c3][i];
            t5[i] = blockdata[c4][i];
        }
        setcol(graphno, t2, setno, blocklen, 2);
        setcol(graphno, t3, setno, blocklen, 3);
        setcol(graphno, t4, setno, blocklen, 4);
        setcol(graphno, t5, setno, blocklen, 5);
        break;
    }

    free(coli);
    setcomment(graphno, setno, buf);
    log_results(buf);
    updatesetminmax(graphno, setno);

#ifndef NONE_GUI
    update_status_popup(NULL, NULL, NULL);
    drawgraph();
#endif
}

/**/
/* Reads a rawspicefile */
/**/

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/* ASSUMED RAW FILE FORMAT:

    Both Spice and CAZM produce this same file format.  It looks as
    follows:

    Title
    Date
    Name
    Flags
    No. Variables: 3
    No. Points: 00061
    Command
    Variables:
        0  time time
        1  v(2) voltage
        2  v(5) voltage
    Values:
    0	0e0
        0e0
        0e0
    1	5e-10
        0e0
        0e0
    2	1e-9
        0e0
        4.7e-1

    For now, the only lines I pay attention to are "No. Variables",
    "No. Points", "Variables", and "Values".  I scan for those exact
    words and then assume that the information to follow is in the
    format above.

    Multiple runs may be present in a file.

    If Variable 0 is time, then I assume this is a TRANSIENT run,
    else if Variable 0 is Frequency, then I assume this is an AC
    run, else I assume it is a TRANSFER run.
*/

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/*
 * note that the maximum number of sets is 30
 */
#define MAXSETN 30

int readrawspice(int gno, char* fn, FILE* fp) {
    int i, j, setn[MAXSETN];
    double *x[MAXSETN], *y[MAXSETN], xval, yval;
    char inputlinetype[BUFSIZE];
    char inputline[BUFSIZE];
    char tmpstring1[BUFSIZE], tmpstring2[BUFSIZE];
    char* truncated;
    int numvariables, numpoints, tmpint;
    numvariables = 0;
    numpoints = 0;

    while (fgets(inputline, BUFSIZE, fp) != NULL) {
        readline++;
        strcpy(inputlinetype, "");
        sscanf(inputline, "%s%s", inputlinetype, tmpstring1);
        if (strcmp(inputlinetype, "Title:") == 0) {
            truncated = &inputline[6];
            for (i = 0; i < (int)strlen(truncated); i++) {
                if (truncated[i] == '\n') {
                    truncated[i] = '\0';
                    break;
                }
            }
            set_plotstr_string(&g[gno].labs.title, truncated);
        }
        if (strcmp(inputlinetype, "Date:") == 0) {
            truncated = &inputline[5];
            for (i = 0; i < (int)strlen(truncated); i++) {
                if (truncated[i] == '\n') {
                    truncated[i] = '\0';
                    break;
                }
            }
            set_plotstr_string(&g[gno].labs.stitle, truncated);
        }
        if (strcmp(inputlinetype, "No.") == 0) {
            if (strcmp(tmpstring1, "Variables:") == 0) {
                sscanf(inputline, "%s%s%d", inputlinetype, tmpstring1, &numvariables);
                /*				printf("%d variables\n",numvariables);*/
            }
        }
        /**/
        /* Accounts for this variant .....*/
        /**/
        /*Variables:    0    TIME    seconds */
        /*			1        nearend vlts */
        /**/
        /**/
        if ((strcmp(inputlinetype, "Variables:") == 0) && (numvariables != 0) &&
            (int)sscanf(inputline, "%s%d%s%s", inputlinetype, &tmpint, tmpstring1, tmpstring2) == 4) {
            /* Read off x axis title and ingore for now */
            sscanf(inputline, "%s%d%s%s", inputlinetype, &tmpint, tmpstring1, tmpstring2);
            for (i = 0; i < numvariables - 1; i++) {
                if ((setn[i] = nextset(gno)) == -1) {
                    for (j = 0; j < i; j++) {
                        killset(gno, setn[j]);
                    }
                    return 0;
                }
                fgets(inputline, BUFSIZE, fp);
                readline++;
                sscanf(inputline, "%d%s%s", &tmpint, tmpstring1, tmpstring2);
                strcat(tmpstring1, "   ");
                strcat(tmpstring1, tmpstring2);
                activateset(gno, setn[i]);
                settype(gno, setn[i], SET_XY);
                setcomment(gno, setn[i], (char*)tmpstring1);
                log_results(tmpstring1);
            }
        }
        /**/
        /* Accounts for this variant .....*/
        /**/
        /*Variables:                      */
        /*             0    TIME    seconds */
        /*			1        nearend vlts */
        /**/
        /**/
        else if ((strcmp(inputlinetype, "Variables:") == 0) && (numvariables != 0) &&
                 (int)(sscanf(inputline, "%s%d%s%s", inputlinetype, &tmpint, tmpstring1, tmpstring2) == 1)) {
            /* Read off x axis title and ingore for now */
            fgets(inputline, BUFSIZE, fp);
            readline++;
            sscanf(inputline, "%s%d%s%s", inputlinetype, &tmpint, tmpstring1, tmpstring2);
            for (i = 0; i < numvariables - 1; i++) {
                if ((setn[i] = nextset(gno)) == -1) {
                    for (j = 0; j < i; j++) {
                        killset(gno, setn[j]);
                    }
                    return 0;
                }
                fgets(inputline, BUFSIZE, fp);
                readline++;
                sscanf(inputline, "%d%s%s", &tmpint, tmpstring1, tmpstring2);
                strcat(tmpstring1, "   ");
                strcat(tmpstring1, tmpstring2);
                activateset(gno, setn[i]);
                settype(gno, setn[i], SET_XY);
                setcomment(gno, setn[i], (char*)tmpstring1);
                log_results(tmpstring1);
            }
        }
        if (strcmp(inputlinetype, "Values:") == 0) {
            /**/
            /* Read in actual values until end of file */
            /**/
            for (i = 0; i < numvariables - 1; i++) {
                /**/
                /* Allocate initial memory for each array */
                /**/
                x[i] = (double*)calloc(BUFSIZE, sizeof(double));
                y[i] = (double*)calloc(BUFSIZE, sizeof(double));
                if (x[i] == NULL || y[i] == NULL) {
                    errmsg("Insufficient memory for set; Clearing data");
                    cxfree(x[i]);
                    cxfree(y[i]);
                    for (j = 0; j < i + 1; j++) {
                        killset(gno, setn[j]);
                    }
                    return 0;
                }
            }
            while (fgets(inputline, BUFSIZE, fp) != NULL) {
                readline++;
                /**/
                /* If not an incremental line, grab another line */
                /**/
                while (sscanf(inputline, "%d%lf", &tmpint, &xval) != 2) {
                    if (fgets(inputline, BUFSIZE, fp) == NULL) {
                        readline++;
                        /**/
                        /* EOF or error in obtaining another line */
                        /**/
                        break;
                    }
                }
                for (j = 0; j < numvariables - 1; j++) {
                    fgets(inputline, BUFSIZE, fp);
                    readline++;
                    sscanf(inputline, "%lf", &yval);
                    x[j][numpoints] = xval;
                    y[j][numpoints] = yval;
                }
                numpoints++;
                /**/
                /* If I run out of space, add more at end of arrays */
                /**/
                if (numpoints % BUFSIZE == 0) {
                    for (j = 0; j < numvariables - 1; j++) {
                        x[j] = (double*)realloc(x[j], (numpoints + BUFSIZE) * sizeof(double));
                        y[j] = (double*)realloc(y[j], (numpoints + BUFSIZE) * sizeof(double));
                    }
                }
                strcpy(inputline, "");
            }
            for (i = 0; i < numvariables - 1; i++) {
                setcol(gno, x[i], setn[i], numpoints, 0);
                setcol(gno, y[i], setn[i], numpoints, 1);
                updatesetminmax(gno, setn[i]);
            }
        }
    }
    sprintf(tmpstring1, "%d sets of %d data points read", numvariables, numpoints);
    echomsg(tmpstring1);
    return 1;
}

/*
 * write out all sets in binary
 */
void do_writesets_binary(int gno, int setno, char* fn) {
    int i, j, n, scnt;
    FILE* cp;
    double *x, *y;
    float *xf, *yf;
    char s[192];

    if (fn == NULL || !fn[0]) {
        errmsg("Define file name first");
        return;
    }
    if (!isactive_graph(gno)) {
        sprintf(s, "Graph %d is inactive", gno);
        errmsg(s);
        return;
    }
    if (fexists(fn)) {
        return;
    }
    if ((cp = fopen(fn, "w")) == NULL) {
        sprintf(s, "Unable to open file %s", fn);
        errmsg(s);
        return;
    }
    scnt = 0;
    for (i = 0; i < g[gno].maxplot; i++) {
        if (isactive_set(gno, i) && getsetlength(gno, i)) {
            scnt++;
        }
    }
    fwrite(&scnt, sizeof(int), 1, cp);
    for (j = 0; j < g[gno].maxplot; j++) {
        if (isactive_set(gno, j)) {
            x = getx(gno, j);
            y = gety(gno, j);
            n = getsetlength(gno, j);
            xf = (float*)calloc(n, sizeof(float));
            yf = (float*)calloc(n, sizeof(float));
            for (i = 0; i < n; i++) {
                xf[i] = x[i];
                yf[i] = y[i];
            }
            fwrite(&n, sizeof(int), 1, cp);
            fwrite(xf, sizeof(float), n, cp);
            fwrite(yf, sizeof(float), n, cp);
            free(xf);
            free(yf);
        }
    }
    fclose(cp);
}

void outputset(int gno, int setno, char* fname, char* dformat) {
    int i, n;
    FILE* cp;
    double *x, *y, *dx, *dy, *dz, *dw;
    char format[256];
    if (fname == NULL) {
        cp = stdout;
    } else if ((cp = fopen(fname, "w")) == NULL) {
        char s[256];
        sprintf(s, "Unable to open file %s", fname);
        errmsg(s);
        return;
    }
    if (dformat == NULL) {
        strcpy(format, "%lf %lf");
    } else {
        strcpy(format, dformat);
    }
    if (isactive_set(cg, setno)) {
        x = getx(cg, setno);
        y = gety(cg, setno);
        n = getsetlength(cg, setno);
        switch (dataset_type(cg, setno)) {
        case SET_XY:
            for (i = 0; i < n; i++) {
                fprintf(cp, format, x[i], y[i]);
                fputc('\n', cp);
            }
            break;
        case SET_XYDX:
        case SET_XYDY:
        case SET_XYZ:
        case SET_XYRT:
            dx = getcol(cg, setno, 2);
            for (i = 0; i < n; i++) {
                fprintf(cp, "%g %g %g", x[i], y[i], dx[i]);
                fputc('\n', cp);
            }
            break;
        case SET_POLY:
            dx = getcol(cg, setno, 2);
            for (i = 0; i < n; i++) {
                fprintf(cp, "%g %g %d", x[i], y[i], (int)dx[i]);
                fputc('\n', cp);
            }
            break;
        case SET_XYDXDX:
        case SET_XYDYDY:
        case SET_XYDXDY:
        case SET_XYUV:
            dx = getcol(gno, setno, 2);
            dy = getcol(gno, setno, 3);
            for (i = 0; i < n; i++) {
                fprintf(cp, "%g %g %g %g", x[i], y[i], dx[i], dy[i]);
                fputc('\n', cp);
            }
            break;
        case SET_XYHILO:
            dx = getcol(gno, setno, 2);
            dy = getcol(gno, setno, 3);
            dz = getcol(gno, setno, 4);
            for (i = 0; i < n; i++) {
                fprintf(cp, "%g %g %g %g %g", x[i], y[i], dx[i], dy[i], dz[i]);
                fputc('\n', cp);
            }
            break;
        case SET_XYBOXPLOT:
            dx = getcol(gno, setno, 2);
            dy = getcol(gno, setno, 3);
            dz = getcol(gno, setno, 4);
            dw = getcol(gno, setno, 5);
            for (i = 0; i < n; i++) {
                fprintf(cp, "%g %g %g %g %g %g", x[i], y[i], dx[i], dy[i], dz[i], dw[i]);
                fputc('\n', cp);
            }
            break;
        case SET_XYBOX:
            dx = getcol(gno, setno, 2);
            dy = getcol(gno, setno, 3);
            dz = getcol(gno, setno, 4);
            for (i = 0; i < n; i++) {
                fprintf(cp, "%g %g %g %g %d", x[i], y[i], dx[i], dy[i], (int)dz[i]);
                fputc('\n', cp);
            }
            break;
        }
    }
    if (fname != NULL) {
        fclose(cp);
    }
}

/*
 * write out a set
 */
int do_writesets(int gno, int setno, int embed, char* fn, char* format) {
    int i, j, k, n, which_graph = gno, save_cg = cg, start, stop, set_start, set_stop;
    FILE* cp;
    double *x, *y, *dx, *dy, *dz, *dw;

    if (!fn[0]) {
        errmsg("Define file name first");
        return 1;
    }
    if (fexists(fn)) {
        return 1;
    }
    if ((cp = fopen(fn, "w")) == NULL) {
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
                    case SET_XY:
                        for (i = 0; i < n; i++) {
                            fprintf(cp, format, x[i], y[i]);
                            fputc('\n', cp);
                        }
                        break;
                    case SET_XYDX:
                    case SET_XYDY:
                    case SET_XYZ:
                    case SET_XYRT:
                        dx = getcol(k, j, 2);
                        for (i = 0; i < n; i++) {
                            fprintf(cp, "%g %g %g", x[i], y[i], dx[i]);
                            fputc('\n', cp);
                        }
                        break;
                    case SET_POLY:
                        dx = getcol(k, j, 2);
                        for (i = 0; i < n; i++) {
                            fprintf(cp, "%g %g %d", x[i], y[i], (int)dx[i]);
                            fputc('\n', cp);
                        }
                        break;
                    case SET_XYDXDX:
                    case SET_XYDYDY:
                    case SET_XYDXDY:
                    case SET_XYUV:
                        dx = getcol(k, j, 2);
                        dy = getcol(k, j, 3);
                        for (i = 0; i < n; i++) {
                            fprintf(cp, "%g %g %g %g", x[i], y[i], dx[i], dy[i]);
                            fputc('\n', cp);
                        }
                        break;
                    case SET_XYHILO:
                        dx = getcol(k, j, 2);
                        dy = getcol(k, j, 3);
                        dz = getcol(k, j, 4);
                        for (i = 0; i < n; i++) {
                            fprintf(cp, "%g %g %g %g %g", x[i], y[i], dx[i], dy[i], dz[i]);
                            fputc('\n', cp);
                        }
                        break;
                    case SET_XYBOX:
                        dx = getcol(k, j, 2);
                        dy = getcol(k, j, 3);
                        dz = getcol(k, j, 4);
                        for (i = 0; i < n; i++) {
                            fprintf(cp, "%g %g %g %g %d", x[i], y[i], dx[i], dy[i], (int)dz[i]);
                            fputc('\n', cp);
                        }
                        break;
                    case SET_XYBOXPLOT:
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

#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)

/*
 * read a variable from netcdf file into a set in graph gno
 * xvar and yvar are the names for x, y in the netcdf file resp.
 * return 0 on fail, return 1 if success.
 *
 * if xvar == NULL, then load the index of the point to x
 *
 */
int readnetcdf(int gno, int setno, char* netcdfname, char* xvar, char* yvar, int nstart, int nstop, int nstride) {
    int cdfid; /* netCDF id */
    int i, n;
    double *x, *y;
    float *xf, *yf;
    short *xs, *ys;
    long *xl, *yl;

    /* variable ids */
    int x_id = -1, y_id;

    /* variable shapes */
    long start[2];
    long count[2];

    nc_type xdatatype = 0;
    nc_type ydatatype = 0;
    int xndims, xdim[10], xnatts;
    int yndims, ydim[10], ynatts;
    long nx, ny;

    extern int ncopts;
    ncopts = 0; /* no crash on error */

    /*
     * get a set if on entry setno == -1, if setno=-1, then fail
     */
    if (setno == -1) {
        if ((setno = nextset(gno)) == -1) {
            return 0;
        }
    } else {
        if (isactive_set(cg, setno)) {
            killset(gno, setno);
        }
    }
    /*
     * open the netcdf file and locate the variable to read
     */
    if ((cdfid = ncopen(netcdfname, NC_NOWRITE)) == -1) {
        errmsg("Can't open file.");
        return 0;
    }
    if (xvar != NULL) {
        if ((x_id = ncvarid(cdfid, xvar)) == -1) {
            char ebuf[256];
            sprintf(ebuf, "readnetcdf(): No such variable %s for X", xvar);
            errmsg(ebuf);
            return 0;
        }
        ncvarinq(cdfid, x_id, NULL, &xdatatype, &xndims, xdim, &xnatts);
        ncdiminq(cdfid, xdim[0], NULL, &nx);
        if (xndims != 1) {
            errmsg("Number of dimensions for X must be 1.");
            return 0;
        }
    }
    if ((y_id = ncvarid(cdfid, yvar)) == -1) {
        char ebuf[256];
        sprintf(ebuf, "readnetcdf(): No such variable %s for Y", yvar);
        errmsg(ebuf);
        return 0;
    }
    ncvarinq(cdfid, y_id, NULL, &ydatatype, &yndims, ydim, &ynatts);
    ncdiminq(cdfid, ydim[0], NULL, &ny);
    if (yndims != 1) {
        errmsg("Number of dimensions for Y must be 1.");
        return 0;
    }
    if (xvar != NULL) {
        n = nx < ny ? nx : ny;
    } else {
        n = ny;
    }
    if (n <= 0) {
        errmsg("Length of dimension == 0.");
        return 0;
    }
    /*
     * allocate for this set
     */
    x = (double*)calloc(n, sizeof(double));
    y = (double*)calloc(n, sizeof(double));
    if (x == NULL || y == NULL) {
        errmsg("Insufficient memory for set");
        cxfree(x);
        cxfree(y);
        ncclose(cdfid);
        return 0;
    }
    start[0] = 0;
    count[0] = n; /* This will retrieve whole file, modify
                   * these values to get subset. This will only
                   * work for single-dimension vars.  You need
                   * to add dims to start & count for
                   * multi-dimensional. */

    /*
     * read the variables from the netcdf file
     */
    if (xvar != NULL) {
        /* TODO should check for other data types here */
        /* TODO should check for NULL on the callocs() */
        /* TODO making assumptions about the sizes of shorts and longs */
        switch (xdatatype) {
        case NC_SHORT:
            xs = (short*)calloc(n, sizeof(short));
            ncvarget(cdfid, x_id, start, count, (void*)xs);
            for (i = 0; i < n; i++) {
                x[i] = xs[i];
            }
            free(xs);
            break;
        case NC_LONG:
            xl = (long*)calloc(n, sizeof(long));
            ncvarget(cdfid, x_id, start, count, (void*)xl);
            for (i = 0; i < n; i++) {
                x[i] = xl[i];
            }
            free(xl);
            break;
        case NC_FLOAT:
            xf = (float*)calloc(n, sizeof(float));
            ncvarget(cdfid, x_id, start, count, (void*)xf);
            for (i = 0; i < n; i++) {
                x[i] = xf[i];
            }
            free(xf);
            break;
        case NC_DOUBLE:
            ncvarget(cdfid, x_id, start, count, (void*)x);
            break;
        default:
            errmsg("Data type not supported");
            cxfree(x);
            cxfree(y);
            ncclose(cdfid);
            return 0;
            break;
        }
    } else { /* just load index */
        for (i = 0; i < n; i++) {
            x[i] = i + 1;
        }
    }
    switch (ydatatype) {
    case NC_SHORT:
        ys = (short*)calloc(n, sizeof(short));
        ncvarget(cdfid, y_id, start, count, (void*)ys);
        for (i = 0; i < n; i++) {
            y[i] = ys[i];
        }
        break;
    case NC_LONG:
        yl = (long*)calloc(n, sizeof(long));
        ncvarget(cdfid, y_id, start, count, (void*)yl);
        for (i = 0; i < n; i++) {
            y[i] = yl[i];
        }
        break;
    case NC_FLOAT:
        /* TODO should check for NULL here */
        yf = (float*)calloc(n, sizeof(float));
        ncvarget(cdfid, y_id, start, count, (void*)yf);
        for (i = 0; i < n; i++) {
            y[i] = yf[i];
        }
        free(yf);
        break;
    case NC_DOUBLE:
        ncvarget(cdfid, y_id, start, count, (void*)y);
        break;
    default:
        errmsg("Data type not supported");
        cxfree(x);
        cxfree(y);
        ncclose(cdfid);
        return 0;
        break;
    }
    ncclose(cdfid);

    /*
     * initialize stuff for the newly created set
     */
    activateset(gno, setno);
    settype(gno, setno, SET_XY);
    setcol(gno, x, setno, n, 0);
    setcol(gno, y, setno, n, 1);

    sprintf(buf, "File %s x = %s y = %s", netcdfname, xvar == NULL ? "Index" : xvar, yvar);
    setcomment(gno, setno, buf);
    log_results(buf);
    updatesetminmax(gno, setno);
    return 1;
}

int write_netcdf(int gno, int setno, char* fname) {

    char buf[512];
    int ncid; /* netCDF id */
    int i, j;
    /* dimension ids */
    int n_dim;
    /* variable ids */
    int x_id, y_id;
    int dims[1];
    long len[1];
    long it = 0;
    double *x, *y, x1, x2, y1, y2;
    ncid = nccreate(fname, NC_CLOBBER);
    ncattput(ncid, NC_GLOBAL, "Contents", NC_CHAR, 11, (void*)"xmgr sets");
    for (i = 0; i < maxgraph; i++) {
        if (isactive_graph(i)) {
            for (j = 0; j < g[i].maxplot; j++) {
                if (isactive_set(i, j)) {
                    char s[64];

                    sprintf(buf, "g%d_s%d_comment", i, j);
                    ncattput(ncid, NC_GLOBAL, buf, NC_CHAR, strlen(getcomment(i, j)), (void*)getcomment(i, j));

                    sprintf(buf, "g%d_s%d_type", i, j);
                    strcpy(s, set_types(dataset_type(i, j)));
                    ncattput(ncid, NC_GLOBAL, buf, NC_CHAR, strlen(s), (void*)s);

                    sprintf(buf, "g%d_s%d_n", i, j);
                    n_dim = ncdimdef(ncid, buf, getsetlength(i, j));
                    dims[0] = n_dim;
                    getsetminmax(i, j, &x1, &x2, &y1, &y2);
                    sprintf(buf, "g%d_s%d_x", i, j);
                    x_id = ncvardef(ncid, buf, NC_DOUBLE, 1, dims);
                    ncattput(ncid, x_id, "min", NC_DOUBLE, 1, (void*)&x1);
                    ncattput(ncid, x_id, "max", NC_DOUBLE, 1, (void*)&x2);
                    dims[0] = n_dim;
                    sprintf(buf, "g%d_s%d_y", i, j);
                    y_id = ncvardef(ncid, buf, NC_DOUBLE, 1, dims);
                    ncattput(ncid, y_id, "min", NC_DOUBLE, 1, (void*)&y1);
                    ncattput(ncid, y_id, "max", NC_DOUBLE, 1, (void*)&y2);
                }
            }
        }
    }
    ncendef(ncid);
    ncclose(ncid);
    if ((ncid = ncopen(fname, NC_WRITE)) == -1) {
        errmsg("Can't open file.");
        return 1;
    }
    for (i = 0; i < maxgraph; i++) {
        if (isactive_graph(i)) {
            for (j = 0; j < g[i].maxplot; j++) {
                if (isactive_set(i, j)) {
                    len[0] = getsetlength(i, j);
                    x = getx(i, j);
                    y = gety(i, j);
                    sprintf(buf, "g%d_s%d_x", i, j);
                    x_id = ncvarid(ncid, buf);
                    sprintf(buf, "g%d_s%d_y", i, j);
                    y_id = ncvarid(ncid, buf);
                    ncvarput(ncid, x_id, &it, len, (void*)x);
                    ncvarput(ncid, y_id, &it, len, (void*)y);
                }
            }
        }
    }

    ncclose(ncid);
    return 0;
}

/* TODO */
/*
 * int write_set_netcdf(int gno, int setno, int ncid)
 * {
 * }
 */

#endif /* HAVE_NETCDF */

#if defined(HAVE_XDR)
#ifndef VMS
#include <rpc/rpc.h>
#else
#ifndef __ALPHA
#define _XOPEN_SOURCE_EXTENDED 1
#endif
#ifdef MULTINET
#include <types.h>
#define DONT_DECLARE_MALLOC
#include "multinet_root:[multinet.include.rpc]rpc.h"
#else
#include <ucx$rpcxdr.h>
#endif
#endif

#define PARMS_MAGIC 1002003
#define HEADERLENGTH 64

int is_old_bin_format(char* fn) {
    char buf[256];
    int magic;
    FILE* fp;
    XDR xdrs;

    if ((fp = fopen(fn, "r")) == NULL) {
        return 0;
    }

    xdrstdio_create(&xdrs, fp, XDR_DECODE);
    xdr_vector(&xdrs, buf, HEADERLENGTH, sizeof(char), (xdrproc_t)xdr_char);
    fclose(fp);
    sscanf(buf, "%d", &magic);
    if (magic == PARMS_MAGIC) {
        return 1;
    } else {
        return 0;
    }
}
#endif
