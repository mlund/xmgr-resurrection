/*
 *
 * Misc. utilities and support routines
 *
 */

#include <config.h>

#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "globals.h"
#include "protos.h"

/*
 * Change suffix on printer output filename to newsuf
 */
void change_suffix(char* s, char* newsuf) {
    int i;

    /* get suffix */
    i = strlen(s);
    do {
    } while (--i >= 0 && s[i] != '.');

    if (i < 0 || !strcmp(s + i, newsuf)) /* don't do a thing */
        return;

    /* check if known suffix before changing it */
    if (!strcmp(s + i, hp_suffix) || !strcmp(s + i, ps_suffix) || !strcmp(s + i, leaf_suffix) ||
        !strcmp(s + i, mif_suffix) || !strcmp(s + i, eps_suffix) || !strcmp(s + i, pstex_suffix))
        strcpy(s + i, newsuf);
}

/*
 * set the current print options
 */
void set_printer(int device, char* prstr) {
    if (device == GR_FILE) {
        if (prstr != NULL) {
            strcpy(printstr, prstr);
        }
        ptofile = TRUE;
    } else {
        switch (device) {
        case GR_PS_L:
        case GR_PS_P:
            if (prstr != NULL) {
                strcpy(ps_prstr, prstr);
            }
            curprint = ps_prstr;
            if (!epsflag)
                cursuffix = ps_suffix;
            else
                cursuffix = eps_suffix;
            break;
        case GR_MIF_L:
        case GR_MIF_P:
            if (prstr != NULL) {
                strcpy(mif_prstr, prstr);
            }
            curprint = mif_prstr;
            cursuffix = mif_suffix;
            break;
        case GR_HPGL_L:
        case GR_HPGL_P:
            if (prstr != NULL) {
                strcpy(hp_prstr, prstr);
            }
            curprint = hp_prstr;
            cursuffix = hp_suffix;
            break;
        case GR_LEAF_L:
        case GR_LEAF_P:
            if (prstr != NULL) {
                strcpy(leaf_prstr, prstr);
            }
            curprint = leaf_prstr;
            cursuffix = leaf_suffix;
            break;
        case GR_PSTEX_INC:
        case GR_PSTEX_L:
        case GR_PSTEX_P:
            ptofile = TRUE;
            cursuffix = pstex_suffix;
            break;
        case 15:
        case 16:
            break;
        default:
            sprintf(buf, "Unknown printer device %d, printer unchanged", device);
            errmsg(buf);
            return;
            break;
        }
        hdevice = device;
        /*
         * 	ptofile = FALSE;
         */
    }
#ifndef NONE_GUI
    update_printer_setup();
#endif
}

/* scrunch a pair of integer arrays */
void scrunch_points(int* x, int* y, int* n) {
    int i, cnt = 0;
    cnt = 0;
    for (i = 0; i < *n - 1; i++) {
        if (x[cnt] == x[i + 1] && y[cnt] == y[i + 1]) {
        } else {
            cnt++;
            x[cnt] = x[i + 1];
            y[cnt] = y[i + 1];
        }
    }
    cnt++;
    if (cnt < 2) {
        cnt = 2;
        x[1] = x[*n + 1];
        y[1] = y[*n + 1];
    }
    *n = cnt;
}

/*
 * stuff results, etc. into a text window
 */
void log_results(char* buf) {
    char tmpbuf[512];
    if (logwindow) {
        strcpy(tmpbuf, buf);
        if (tmpbuf[strlen(tmpbuf) - 1] != '\n') {
            strcat(tmpbuf, "\n");
        }
        stufftext(tmpbuf, 1);
    }
}

void errmsg(char* buf) {
#ifdef NONE_GUI
    printf("%s\n", buf);
#else
    if (inwin) {
        errwin(buf);
    } else {
        printf("%s\n", buf);
    }
#endif
}

int yesnoterm(char* msg) { return 1; }

int yesno(char* msg, char* s1, char* s2, char* help_anchor) {
    if (noask) {
        return 1;
    }
#ifdef NONE_GUI
    return (yesnoterm(msg));
#else
    if (inwin) {
        return (yesnowin(msg, s1, s2, help_anchor));
    } else {
        return (yesnoterm(msg));
    }
#endif
}

int fexists(char* to) {
    struct stat stto;
    char tbuf[256];

    if (stat(to, &stto) == 0) {
        sprintf(tbuf, "Overwrite %s?", to);
        if (!yesno(tbuf, NULL, NULL, NULL)) {
            return (1);
        }
        return (0);
    }
    return (0);
}

void stufftext(char* s, int sp) {
#ifdef NONE_GUI
    printf("%s", buf);
#else
    stufftextwin(s, sp);
#endif
}

char* mybasename(char* s) {
    int start, end;
    static char basename[256];

    end = strlen(s) - 1;
    if (end == 0 && *s == '/') { /* root is a special case */
        basename[0] = '/';
        return basename;
    }

    /* strip trailing white space and slashes */
    while (s[end] == '/' || s[end] == ' ' || s[end] == '\t')
        end--;
    /* find start of basename */
    start = end;
    do {
        start--;
    } while (start >= 0 && s[start] != '/');

    strncpy(basename, s + (start + 1), end - start);
    basename[end - start] = '\0';
    return basename;
}

int isdir(char* f) {
    struct stat st;

    stat(f, &st);
    return (S_ISDIR(st.st_mode));
}

int set_workingdir(char* wd) {
    char buf2[MAXPATHLEN];
    strcpy(buf2, wd);
    if (buf2[0] == '~') {
        expand_tilde(buf2);
    }
    if (chdir(buf2) >= 0) {
        strcpy(workingdir, buf2);
        return 0;
    } else {
        return -1;
    }
}

/* TODO this needs some work */
void expand_tilde(char* buf) {
    char buf2[MAXPATHLEN];
    char* home;
    if (buf[0] == '~') {
        if (strlen(buf) == 1) {
            home = getenv("HOME");
            if (home == NULL) {
                errmsg("Couldn't find $HOME!");
                return;
            } else {
                strcpy(buf, home);
                strcat(buf, "/");
            }
        } else if (buf[1] == '/') {
            home = getenv("HOME");
            if (home == NULL) {
                errmsg("Couldn't find $HOME!");
                return;
            }
            strcpy(buf2, home);
            strcat(buf2, "/");
            strcat(buf2, buf + 1);
            strcpy(buf, buf2);
        } else {
            char tmp[128], *pp = tmp, *q = buf + 1;
            struct passwd* pent;

            while (*q && (*q != '/')) {
                *pp++ = *q++;
            }
            *pp = 0;
            if ((pent = getpwnam(tmp)) != NULL) {
                strcpy(buf2, pent->pw_dir);
                strcat(buf2, "/");
                strcat(buf2, q);
                strcpy(buf, buf2);
            } else {
                errmsg("No user by that name");
            }
        }
    }
}

void echomsg(char* msg) {
    if (inwin) {
#ifndef NONE_GUI
        set_left_footer(msg);
#endif
    } else {
        printf("%s\n", msg);
    }
}

#ifndef HAVE_STRDUP

/* This is stolen from rotated.c */

/**************************************************************************/
/* Routine to mimic `strdup()' (some machines don't have it)              */
/**************************************************************************/

char* strdup(char* str) {
    char* s;

    if (str == NULL)
        return NULL;

    s = (char*)malloc((unsigned)(strlen(str) + 1));
    if (s != NULL)
        strcpy(s, str);

    return s;
}

#endif

static void update_timestamp(void) {
    struct tm tm;
    time_t time_value;
    char* str;

    (void)time(&time_value);
    tm = *localtime(&time_value);
    str = asctime(&tm);
    if (str[strlen(str) - 1] == '\n') {
        str[strlen(str) - 1] = '\0';
    }
    set_plotstr_string(&timestamp, str);
}

/*
 * dirtystate routines
 */

static int dirtystate = 0;

void set_dirtystate(void) {
    if (dirtystate >= 0) {
        dirtystate++;
        update_timestamp();
        /*
         * TODO:
         * 	if ( (dirtystate > SOME_LIMIT) ||
         *           (current_time - autosave_time > ANOTHER_LIMIT) ) {
         * 	    autosave();
         * 	}
         */
    }
}

void clear_dirtystate(void) { dirtystate = 0; }

void lock_dirtystate(void) { dirtystate = -1; }

int is_dirtystate(void) { return (dirtystate); }
