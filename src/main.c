/*
 * ACE/gr - Graphics for Exploratory Data Analysis
 *
 * Copyright (c) 1991-95 Paul J Turner, Portland, OR
 * Copyright (c) 1996-98 ACE/gr Development Team
 * 
 * Currently maintained by Evgeny Stambulchik, Rehovot, Israel
 * 
 *                             All Rights Reserved
 *
 * Permission  to  use, copy, modify, and  distribute  this software  and  its
 * documentation  for any purpose and  without fee is hereby granted, provided
 * that  the above copyright notice  appear in  all copies and  that both that
 * copyright  notice  and   this  permission  notice   appear  in   supporting
 * documentation.
 *
 * PAUL J TURNER AND OTHER CONTRIBUTORS DISCLAIM ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING,  BUT  NOT LIMITED  TO, ALL  IMPLIED WARRANTIES OF
 * MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL PAUL J TURNER  OR  CURRENT
 * MAINTAINER  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR  OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * Comments, bug reports, etc to:
 *
 * http://plasma-gate.weizmann.ac.il/Xmgr/comments.html
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/* for globals.h */
#define MAIN

#include "globals.h"
#include "patchlevel.h"

#include "protos.h"

#ifdef VMS
#  include "vms_unix.h"
#endif

#ifdef NONE_GUI
char version[] = "gr";
#else
char version[] = "xmgr";
#endif

double missing = DATASET_MISSING;

static void usage(FILE *stream, char *progname);

int inpipe = 0;			/* if xmgr is to participate in a pipe */


int main(int argc, char *argv[])
{
    FILE *fp;
    int i, j;
    int gno;
    int cur_graph = cg;		/* default graph is graph 0 */
    int loadlegend = 0;		/* legend on and load file names */
    int gcols = 1, grows = 1;
    int grbatch = 0;		/* if executed as 'grbatch' then TRUE */
    int remove = 0;		/* remove file after read */
    int noprint = 0;		/* if grbatch, then don't print if true */
    extern int realtime;	/* if data is to be plotted as it is read in */
    extern char batchfile[];	/* name of file to execute */
    extern int density_flag;	/* temp, for density plots, defined in
				 * plotone.c */
#if defined(DEBUG)    
    extern int yydebug;
#endif

    char xvgrrc_file[MAXPATHLEN], *s;
    
    getcwd(currentdir, MAXPATHLEN);	/* get the starting directory */

#ifndef GR_HOME
    strcpy(xmgrdir, "/usr/local/xmgr");
#else
    strcpy(xmgrdir, GR_HOME);
#endif


    /*
     * if program name is grbatch then don't initialize the X toolkit - set
     * hardcopyflag for batch plotting
     */
    s = strrchr(argv[0], '/');
    if (s == NULL) {
	s = argv[0];
    } else {
	s++;
    }

    if (strcmp("grbatch", s) && strcmp("grbatch.exe", s)) {
	fprintf(stderr, "%s v%d.%d.%d %s\n", version, 
		MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER);
	fprintf(stderr, "(C) Copyright 1991-1995 Paul J Turner\n");
	fprintf(stderr, "(C) Copyright 1996-1998 ACE/gr Development Team\n");
	fprintf(stderr, "All Rights Reserved\n");

#ifndef NONE_GUI
	initialize_screen(&argc, argv);
#endif

    } else {
	grbatch = TRUE;
    }

    /* initialize plots, strings, graphs */
    set_program_defaults();

    /* initialize colormap data */
    initialize_cms_data();
    
    /* initialize the nonl-fit parameters */
    initialize_nonl();

    /* initialize the parser symbol table */
    init_symtab();
    
    /* initialize the rng */
    srand48(100L);

    /* initialize device, here tdevice is always 0 = Xlib */
    device = tdevice = 0;

    /* check for startup file in local directory */
    if ((fp = fopen(".xmgrrc", "r")) != NULL) {
	fclose(fp);
	getparms(".xmgrrc");
    } else {
	/* check for startup file in effective users home dir */
	if ((s = getenv("HOME")) != NULL) {
	    strcpy(xvgrrc_file, s);
	    strcat(xvgrrc_file, "/.xmgrrc");
	    if ((fp = fopen(xvgrrc_file, "r")) != NULL) {
		fclose(fp);
		getparms(xvgrrc_file);
	    }
	} else {
	    if (!grbatch) {
		fprintf(stderr, "Unable to read environment variable HOME, skipping startup file\n");
	    }
	}
    }
    plfile[0] = 0;		/* parameter file name */

    /*
     * xmgr home directory
     */
    if ((s = getenv("GR_HOME")) != NULL) {
	strcpy(xmgrdir, s);
    }
#ifndef GR_HELPVIEWER
    strcpy(help_viewer, "netscape -noraise -remote openURL\\(%s,newwindow\\) >>/dev/null 2>&1 || netscape %s");
#else
    strcpy(help_viewer, GR_HELPVIEWER);
#endif

    /*
     * check for changed help file viewer command
     */
    if ((s = getenv("GR_HELPVIEWER")) != NULL) {
	strcpy(help_viewer, s);
    }    

    /*
     * check for changed printer spooling strings
     */
    if ((s = getenv("GR_PS_PRSTR")) != NULL) {
	strcpy(ps_prstr, s);
    }
    if ((s = getenv("GR_MIF_PRSTR")) != NULL) {
	strcpy(mif_prstr, s);
    }
    if ((s = getenv("GR_HPGL_PRSTR")) != NULL) {
	strcpy(hp_prstr, s);
    }
    if ((s = getenv("GR_LEAF_PRSTR")) != NULL) {
	strcpy(leaf_prstr, s);
    }
    /*
     * check for changed hardcopy device
     */
    if ((s = getenv("GR_HDEV")) != NULL) {
	hdevice = atoi(s);
    }
    set_printer(hdevice, NULL);

    set_graph_active(cur_graph);

    if (argc >= 2) {
	for (i = 1; i < argc; i++) {
	    if (argv[i][0] == '-') {
		if (argmatch(argv[i], "-version", 5)) {
		    fprintf(stdout, "%s v%d.%d.%d %s\n", version, 
		    	MAJOR_REV, MINOR_REV, PATCHLEVEL, BETA_VER);
		    exit(0);
		}
#if defined(DEBUG)
		if (argmatch(argv[i], "-debug", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for debug flag\n");
			usage(stderr, argv[0]);
		    } else {
		    	debuglevel = atoi(argv[i]);
		    	if (debuglevel == 4) { 
			    /* turn on debugging in pars.y */
			    yydebug = 1;
		    	}
		    }
		} else
#endif
		       if (argmatch(argv[i], "-nosigcatch", 6)) {
		    sigcatch = 0;
		} else if (argmatch(argv[i], "-autoscale", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for autoscale flag, should be one of 'x', 'y', 'xy'\n");
			usage(stderr, argv[0]);
		    } else {
			if (!strcmp("x", argv[i])) {
			    g[cur_graph].autoscale = 1;
			} else if (!strcmp("y", argv[i])) {
			    g[cur_graph].autoscale = 2;
			} else if (!strcmp("xy", argv[i])) {
			    g[cur_graph].autoscale = 3;
			} else {
			    fprintf(stderr, "%s: Improper argument for -a flag should be one of 'x', 'y', 'xy'\n", argv[0]);
			    usage(stderr, argv[0]);
			}
		    }
		} else if (argmatch(argv[i], "-noauto", 7)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for no autoscale flag, should be one of 'x', 'y', 'xy'\n");
			usage(stderr, argv[0]);
		    } else {
			if (!strcmp("x", argv[i])) {
			    g[cur_graph].noautoscale = 1;
			} else if (!strcmp("y", argv[i])) {
			    g[cur_graph].noautoscale = 2;
			} else if (!strcmp("xy", argv[i])) {
			    g[cur_graph].noautoscale = 3;
			} else {
			    fprintf(stderr, "%s: Improper argument for -noauto flag should be one of 'x', 'y', 'xy'\n", argv[0]);
			    usage(stderr, argv[0]);
			}
		    }
		} else if (argmatch(argv[i], "-batch", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for batch file\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(batchfile, argv[i]);
		    }
		} else if (argmatch(argv[i], "-image", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for image file\n");
			usage(stderr, argv[0]);
		    } else {
			readimage = TRUE;
			strcpy(image_filename, argv[i]);
		    }
		} else if (argmatch(argv[i], "-imagexy", 8)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for imagexy\n");
			usage(stderr, argv[0]);
		    }
		    imagex = atoi(argv[i]);
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for imagexy\n");
			usage(stderr, argv[0]);
		    }
		    imagey = atoi(argv[i]);
		} else if (argmatch(argv[i], "-pipe", 5)) {
		    inpipe = TRUE;
/*
		    int flags = 0;
		    fcntl(0, F_SETFL, flags | O_NONBLOCK);
*/
		} else if (argmatch(argv[i], "-noprint", 8)) {
		    noprint = TRUE;
		} else if (argmatch(argv[i], "-logwindow", 5)) {
		    logwindow = TRUE;
		} else if (argmatch(argv[i], "-nologwindow", 7)) {
		    logwindow = FALSE;
		} else if (argmatch(argv[i], "-npipe", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for named pipe\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(pipe_name, argv[i]);
			named_pipe = 1;
		    }
#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
		} else if (argmatch(argv[i], "-netcdf", 7) || argmatch(argv[i], "-hdf", 4)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for netcdf file\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(netcdf_name, argv[i]);
			readcdf = 1;
		    }
		} else if (argmatch(argv[i], "-netcdfxy", 9) || argmatch(argv[i], "-hdfxy", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for netcdf X variable name\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(xvar_name, argv[i]);
		    }
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for netcdf Y variable name\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(yvar_name, argv[i]);
		    }
		    if (strcmp(xvar_name, "null")) {
			readnetcdf(cg, -1, netcdf_name, xvar_name, yvar_name, -1, -1, 1);
		    } else {
			readnetcdf(cg, -1, netcdf_name, NULL, yvar_name, -1, -1, 1);
		    }
#endif				/* HAVE_NETCDF */
		} else if (argmatch(argv[i], "-timer", 6)) {
		    extern int timer_delay;	/* TODO move to globals.h */

		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for time delay\n");
			usage(stderr, argv[0]);
		    } else {
			timer_delay = atoi(argv[i]);
		    }
		} else if (argmatch(argv[i], "-realtime", 5)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for realtime plotting\n");
			usage(stderr, argv[0]);
		    } else {
			realtime = atoi(argv[i]);
		    }
		} else if (argmatch(argv[i], "-maxsets", 8)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max number of sets\n");
			usage(stderr, argv[0]);
		    } else {
			maxplot = atoi(argv[i]);
			realloc_plots(maxplot);
		    }
		} else if (argmatch(argv[i], "-maxblock", 9)) {
		    int itmp;			
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max size of block data\n");
			usage(stderr, argv[0]);
		    } else {
			itmp = atoi(argv[i]);
			if (itmp < maxplot) {
			    itmp = maxplot;
			}
			alloc_blockdata(itmp);
		    }
		} else if (argmatch(argv[i], "-graphsets", 10)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max number of sets for he current graph\n");
			usage(stderr, argv[0]);
		    } else {
			realloc_graph_plots(cur_graph, atoi(argv[i]));
		    }
		} else if (argmatch(argv[i], "-maxgraph", 8)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max number of graphs\n");
			usage(stderr, argv[0]);
		    } else {
			maxgraph = atoi(argv[i]);
			realloc_graphs();
		    }
		} else if (argmatch(argv[i], "-maxlines", 8)) {
		    int itmp;
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max number of lines\n");
			usage(stderr, argv[0]);
		    } else {
			itmp = atoi(argv[i]);
			realloc_lines(itmp);
		    }
		} else if (argmatch(argv[i], "-maxboxes", 8)) {
		    int itmp;
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max number of boxes\n");
			usage(stderr, argv[0]);
		    } else {
			itmp = atoi(argv[i]);
			realloc_boxes(itmp);
		    }
		} else if (argmatch(argv[i], "-maxstr", 7)) {
		    int itmp;
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max number of strings\n");
			usage(stderr, argv[0]);
		    } else {
			itmp = atoi(argv[i]);
			realloc_strings(itmp);
		    }
		} else if (argmatch(argv[i], "-defaultcmap", 12)) {
		    use_defaultcmap = 0;
		} else if (argmatch(argv[i], "-vertext", 8)) {
		    use_xvertext = 1;
		} else if (argmatch(argv[i], "-timestamp", 10)) {
		    timestamp.active = TRUE;
		} else if (argmatch(argv[i], "-remove", 7)) {
		    remove = 1;
		} else if (argmatch(argv[i], "-fixed", 5)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for max number of strings\n");
			usage(stderr, argv[0]);
		    } else {
		        if (i == argc) {
			    fprintf(stderr, "Missing argument for max number of strings\n");
			    usage(stderr, argv[0]);
		        } else {
		            canvasw = atoi(argv[i]);
		    	    i++;
		    	    canvash = atoi(argv[i]);
		    	    page_layout = PAGE_FIXED;
			}
		    }
		} else if (argmatch(argv[i], "-landscape", 10)) {
		    page_layout = PAGE_LANDSCAPE;
		} else if (argmatch(argv[i], "-portrait", 9)) {
		    page_layout = PAGE_PORTRAIT;
		} else if (argmatch(argv[i], "-free", 5)) {
		    page_layout = PAGE_FREE;
		} else if (argmatch(argv[i], "-noask", 5)) {
		    noask = 1;
		} else if (argmatch(argv[i], "-mono", 5)) {
		    monomode = 1;
		} else if (argmatch(argv[i], "-bs", 3)) {
		    backingstore = 1;
		} else if (argmatch(argv[i], "-nobs", 5)) {
		    backingstore = 0;
		} else if (argmatch(argv[i], "-dc", 3)) {
		    allow_dc = 1;
		} else if (argmatch(argv[i], "-nodc", 5)) {
		    allow_dc = 0;
		} else if (argmatch(argv[i], "-redraw", 7)) {
		    auto_redraw = 1;
		} else if (argmatch(argv[i], "-noredraw", 9)) {
		    auto_redraw = 0;
		} else if (argmatch(argv[i], "-statusredraw", 7)) {
		    status_auto_redraw = 1;
		} else if (argmatch(argv[i], "-nostatusredraw", 9)) {
		    status_auto_redraw = 0;
		} else if (argmatch(argv[i], "-refresh", 7)) {
		    allow_refresh = 1;
		} else if (argmatch(argv[i], "-norefresh", 9)) {
		    allow_refresh = 0;
		} else if (argmatch(argv[i], "-freecolors", 11)) {
		    free_colors = 1;
		} else if (argmatch(argv[i], "-nofreecolors", 13)) {
		    free_colors = 0;
		} else if (argmatch(argv[i], "-maxcolors", 10)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for number of colors\n");
			usage(stderr, argv[0]);
		    }
		    maxcolors = atoi(argv[i]);
		    initialize_cms_data();
		    if (maxcolors > 256) {
			fprintf(stderr, "Number of colors exceeds 256, set to 256\n");
			maxcolors = 256;
		    }
		} else if (argmatch(argv[i], "-GXxor", 6)) {
		    invert = 0;
		} else if (argmatch(argv[i], "-GXinvert", 6)) {
		    invert = 1;
		} else if (argmatch(argv[i], "-eps", 4)) {
		    epsflag = 1;
		} else if (argmatch(argv[i], "-nops2", 4)) {
		    ps2flag = 0;
		} else if (argmatch(argv[i], "-device", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for hardcopy device select flag\n");
			usage(stderr, argv[0]);
		    }
		    set_printer(atoi(argv[i]), NULL);
		} else if (argmatch(argv[i], "-log", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for log plots flag\n");
			usage(stderr, argv[0]);
		    }
		    if (!strcmp("x", argv[i])) {
			g[cur_graph].type = GRAPH_LOGX;
		    } else if (!strcmp("y", argv[i])) {
			g[cur_graph].type = GRAPH_LOGY;
		    } else if (!strcmp("xy", argv[i])) {
			g[cur_graph].type = GRAPH_LOGXY;
		    } else {
			fprintf(stderr, "%s: Improper argument for -l flag should be one of 'x', 'y', 'xy'\n", argv[0]);
			g[cur_graph].type = GRAPH_XY;
		    }
		} else if (argmatch(argv[i], "-printfile", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing file name for printing\n");
			usage(stderr, argv[0]);
		    } else {
			set_printer(GR_FILE, argv[i]);
		    }
		} else if (argmatch(argv[i], "-hardcopy", 6)) {
		    grbatch = TRUE;
		} else if (argmatch(argv[i], "-pexec", 6)) {
		    if (i == argc) {
			fprintf(stderr, "Missing argument for exec\n");
			usage(stderr, argv[0]);
		    } else {
			char pstring[MAX_STRING_LENGTH];
			int ilen;

			i++;
			strcpy(pstring, argv[i]);
			ilen = strlen(pstring);
			pstring[ilen] = '\n';
			pstring[ilen + 1] = 0;
			read_param(pstring);
		    }
		} else if (argmatch(argv[i], "-graph", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameter for graph select\n");
			usage(stderr, argv[0]);
		    } else {
			sscanf(argv[i], "%d", &gno);
			if (gno >= 0 && gno < maxgraph) {
			    cg = cur_graph = gno;
			    set_graph_active(gno);
			} else {
			    fprintf(stderr, "Graph number must be between 0 and %d\n", maxgraph - 1);
			}
		    }
		} else if (argmatch(argv[i], "-block", 6)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameter for block data\n");
			usage(stderr, argv[0]);
		    } else {
			if (getdata(cur_graph, argv[i], cursource, SET_BLOCK)) {
			}
		    }
		} else if (argmatch(argv[i], "-bxy", 4)) {
		    char blocksetcols[32];
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameter for block data set creation\n");
			usage(stderr, argv[0]);
		    }
		    strcpy(blocksetcols, argv[i]);
		    create_set_fromblock(cg, curtype, blocksetcols);
		} else if (argmatch(argv[i], "-xy", 3)) {
		    curtype = SET_XY;
		} else if (argmatch(argv[i], "-poly", 5)) {
		    curtype = SET_POLY;
		} else if (argmatch(argv[i], "-bin", 4)) {
		    curtype = SET_BIN;
		} else if (argmatch(argv[i], "-xydx", 5)) {
		    curtype = SET_XYDX;
		} else if (argmatch(argv[i], "-xydy", 5)) {
		    curtype = SET_XYDY;
		} else if (argmatch(argv[i], "-xydxdx", 7)) {
		    curtype = SET_XYDXDX;
		} else if (argmatch(argv[i], "-xydydy", 7)) {
		    curtype = SET_XYDYDY;
		} else if (argmatch(argv[i], "-xydxdy", 7)) {
		    curtype = SET_XYDXDY;
		} else if (argmatch(argv[i], "-xyuv", 5)) {
		    curtype = SET_XYUV;
		} else if (argmatch(argv[i], "-xyz", 4)) {
		    curtype = SET_XYZ;
		} else if (argmatch(argv[i], "-xyd", 4)) {
		    curtype = SET_XYZ;
		    density_flag = TRUE;
		} else if (argmatch(argv[i], "-xybox", 6)) {
		    curtype = SET_XYBOX;
		} else if (argmatch(argv[i], "-boxplot", 8)) {
		    curtype = SET_XYBOXPLOT;
		} else if (argmatch(argv[i], "-xyr", 4)) {
		    curtype = SET_XYRT;
		} else if (argmatch(argv[i], "-ihl", 4)) {
		    curtype = SET_IHL;
		} else if (argmatch(argv[i], "-nxy", 4)) {
		    curtype = SET_NXY;
		} else if (argmatch(argv[i], "-hilo", 5)) {
		    curtype = SET_XYHILO;
		} else if (argmatch(argv[i], "-rawspice", 9)) {
		    curtype = SET_RAWSPICE;
		} else if (argmatch(argv[i], "-xystring", 9)) {
		    curtype = SET_XYSTRING;
		} else if (argmatch(argv[i], "-type", 2)) {
		    /* other file types here */
		    i++;
		    if (argmatch(argv[i], "bin", 3)) {
			curtype = SET_BIN;
		    } else if (argmatch(argv[i], "poly", 4)) {
			curtype = SET_POLY;
		    } else if (argmatch(argv[i], "xydx", 4)) {
			curtype = SET_XYDX;
		    } else if (argmatch(argv[i], "xydy", 4)) {
			curtype = SET_XYDY;
		    } else if (argmatch(argv[i], "xydxdx", 6)) {
			curtype = SET_XYDXDX;
		    } else if (argmatch(argv[i], "xydydy", 6)) {
			curtype = SET_XYDYDY;
		    } else if (argmatch(argv[i], "xydxdy", 6)) {
			curtype = SET_XYDXDY;
		    } else if (argmatch(argv[i], "xyz", 3)) {
			curtype = SET_XYZ;
		    } else if (argmatch(argv[i], "xyr", 3)) {
			curtype = SET_XYRT;
		    } else if (argmatch(argv[i], "hilo", 4)) {
			curtype = SET_XYHILO;
		    } else if (argmatch(argv[i], "boxplot", 4)) {
			curtype = SET_XYBOXPLOT;
		    } else {
			fprintf(stderr, "%s: Unknown file type '%s' assuming XY\n", argv[0], argv[i]);
			curtype = SET_XY;
		    }
		} else if (argmatch(argv[i], "-missing", 3)) {
		    if (i == argc) {
			fprintf(stderr, "Missing argument for missing data value\n");
			usage(stderr, argv[0]);
		    } else {
			i++;
		        missing = atof(argv[i]);
		    }
		} else if (argmatch(argv[i], "-graphtype", 7)) {
		    if (i == argc) {
			fprintf(stderr, "Missing argument for graph type\n");
			usage(stderr, argv[0]);
		    } else {
			i++;
			if (!strcmp("xy", argv[i])) {
			    g[cur_graph].type = GRAPH_XY;
			} else if (!strcmp("polar", argv[i])) {
			    g[cur_graph].type = GRAPH_POLAR;
			} else if (!strcmp("bar", argv[i])) {
			    g[cur_graph].type = GRAPH_BAR;
			} else if (!strcmp("hbar", argv[i])) {
			    g[cur_graph].type = GRAPH_HBAR;
			} else if (!strcmp("stackedbar", argv[i])) {
			    g[cur_graph].type = GRAPH_STACKEDBAR;
			} else if (!strcmp("stackedhbar", argv[i])) {
			    g[cur_graph].type = GRAPH_STACKEDHBAR;
			} else if (!strcmp("logx", argv[i])) {
			    g[cur_graph].type = GRAPH_LOGX;
			} else if (!strcmp("logy", argv[i])) {
			    g[cur_graph].type = GRAPH_LOGY;
			} else if (!strcmp("logxy", argv[i])) {
			    g[cur_graph].type = GRAPH_LOGXY;
			} else if (!strcmp("smith", argv[i])) {
			    g[cur_graph].type = GRAPH_SMITH;
			    g[cur_graph].w.xg1 = -1.0;
			    g[cur_graph].w.yg1 = -1.0;
			    g[cur_graph].w.xg2 = 1.0;
			    g[cur_graph].w.yg2 = 1.0;
			} else {
			    fprintf(stderr, "%s: Improper argument for -graphtype should be one of 'xy', 'logx', 'logy', 'logxy', 'bar', 'stackedbar'\n", argv[0]);
			    usage(stderr, argv[0]);
			}
		    }
		} else if (argmatch(argv[i], "-arrange", 7)) {
		    if (i == argc) {
			fprintf(stderr, "Missing argument for graph arrangement\n");
			usage(stderr, argv[0]);
		    } else {
			i++;
			grows = atoi(argv[i]);
			i++;
			gcols = atoi(argv[i]);
		    }
		} else if (argmatch(argv[i], "-cols", 5)) {
		    if (i == argc) {
			fprintf(stderr, "Missing argument for graph column arrangement\n");
			usage(stderr, argv[0]);
		    } else {
			i++;
			gcols = atoi(argv[i]);
		    }
		} else if (argmatch(argv[i], "-rows", 5)) {
		    if (i == argc) {
			fprintf(stderr, "Missing argument for graph row arrangement\n");
			usage(stderr, argv[0]);
		    } else {
			i++;
			grows = atoi(argv[i]);
		    }
		} else if (argmatch(argv[i], "-legend", 4)) {
		    if (i == argc) {
			fprintf(stderr, "Missing argument for -legend\n");
			usage(stderr, argv[0]);
		    } else {
			i++;
			if (!strcmp("load", argv[i])) {
			    loadlegend = TRUE;
			    g[cur_graph].l.active = TRUE;
			} else {
			    fprintf(stderr, "Improper argument for -legend\n");
			    usage(stderr, argv[0]);
			}
		    }
		} else if (argmatch(argv[i], "-rvideo", 7)) {
		    revflag = 1;
		} else if (argmatch(argv[i], "-param", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameter file name\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(plfile, argv[i]);
			if (!getparms(plfile)) {
			    g[cur_graph].parmsread = FALSE;
			    fprintf(stderr, "Unable to read parameter file %s\n", plfile);
			} else {
			    g[cur_graph].parmsread = TRUE;
			}
		    }
		} else if (argmatch(argv[i], "-results", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing results file name\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(resfile, argv[i]);
/*
*  open resfile if -results option given
*/
			if (!fexists(resfile)) {
			    if ((resfp = fopen(resfile, "w")) == NULL) {
				fprintf(stderr, "Unable to open file %s", resfile);
				exit(1);
			    }
			}
		    }
		} else if (argmatch(argv[i], "-saveall", 8)) {
		    char savefile[MAXPATHLEN];
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing save file name\n");
			usage(stderr, argv[0]);
		    } else {
			strcpy(savefile, argv[i]);
			do_writesets(maxgraph, -1, 1, savefile, sformat);
		    }
		} else if (argmatch(argv[i], "-wd", 3)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameters for working directory\n");
			usage(stderr, argv[0]);
		    } else {
			if (isdir(argv[i])) {
			    strcpy(buf, argv[i]);
			    if (chdir(buf) == 0) {
					getcwd(workingdir, 512);
			    } else {
					fprintf(stderr, "Can't change to directory %s, fatal error",
																buf);
					exit(1);
			    }
			} else {
			    fprintf(stderr, "%s is not a directory, fatal error\n",argv[i]);
			    exit(1);
			}
		    }
		} else if (argmatch(argv[i], "-source", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing argument for data source parameter\n");
			usage(stderr, argv[0]);
		    }
		    if (argmatch(argv[i], "pipe", 4)) {
			cursource = SOURCE_PIPE;
		    } else if (argmatch(argv[i], "disk", 4)) {
			cursource = SOURCE_DISK;
		    } else if (argmatch(argv[i], "stdin", 5)) {
			cursource = SOURCE_STDIN;
		    }
		    /* we are in a pipe */
		    if (cursource == SOURCE_STDIN) {
			if (getdata(cur_graph, "STDIN", SOURCE_STDIN, curtype)) {
			}
		    }
		} else if (argmatch(argv[i], "-viewport", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameters for viewport setting\n");
			usage(stderr, argv[0]);
		    } else {
			g[cur_graph].v.xv1 = atof(argv[i++]);
			g[cur_graph].v.yv1 = atof(argv[i++]);
			g[cur_graph].v.xv2 = atof(argv[i++]);
			g[cur_graph].v.yv2 = atof(argv[i]);
		    }
		} else if (argmatch(argv[i], "-world", 2)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing parameters for world setting\n");
			usage(stderr, argv[0]);
		    } else {
			g[cur_graph].w.xg1 = atof(argv[i++]);
			g[cur_graph].w.yg1 = atof(argv[i++]);
			g[cur_graph].w.xg2 = atof(argv[i++]);
			g[cur_graph].w.yg2 = atof(argv[i]);
		    }
		} else if (argmatch(argv[i], "-seed", 5)) {
		    i++;
		    if (i == argc) {
			fprintf(stderr, "Missing seed for srand48()\n");
			usage(stderr, argv[0]);
		    } else {
			srand48(atol(argv[i]));	/* note atol() */
		    }
		} else if (argmatch(argv[i], "-symcolorbug", 12)) {
		    symcolorbug = 1;
		} else if (argmatch(argv[i], "-help", 2)) {
		    usage(stdout, argv[0]);
		} else if (argmatch(argv[i], "-usage", 5)) {
		    usage(stdout, argv[0]);
		} else {
		    fprintf(stderr, "No option %s\n", argv[i]);
		    usage(stderr, argv[0]);
		}
	    } else {
		if (!(i == argc)) {
		    lock_dirtystate();
		    if (getdata(cur_graph, argv[i], cursource, curtype)) {
			strcpy(docname, argv[i]);
			if (remove) {
			    unlink(argv[i]);
			}
			clear_dirtystate();
		    } else {
		        clear_dirtystate();
			set_dirtystate();
		    }
		}		/* end if */
		remove = 0;
	    }			/* end else */
	}			/* end for */
    }				/* end if */
    for (i = 0; i < maxgraph; i++) {
	if (isactive_graph(i) && (activeset(i))) {
	    if (g[i].parmsread == FALSE) {
		if (g[i].noautoscale) {
		    switch (g[i].noautoscale) {
		    case 1:
			defaulty(i, -1);
			default_axis(i, g[i].auto_type, Y_AXIS);
			default_axis(i, g[i].auto_type, ZY_AXIS);
			break;
		    case 2:
			defaultx(i, -1);
			default_axis(i, g[i].auto_type, X_AXIS);
			default_axis(i, g[i].auto_type, ZX_AXIS);
			break;
		    case 3:
			break;
		    }
		} else {
		    defaultgraph(i);
		    default_axis(i, g[i].auto_type, X_AXIS);
		    default_axis(i, g[i].auto_type, ZX_AXIS);
		    default_axis(i, g[i].auto_type, Y_AXIS);
		    default_axis(i, g[i].auto_type, ZY_AXIS);
		}
	    }
/*
 * if auto scaling type set with -a option, then scale appropriate axis
 */
	    else {
		if (g[i].autoscale) {
		    switch (g[i].autoscale) {
		    case 1:
			defaultx(i, -1);
			default_axis(i, g[i].auto_type, X_AXIS);
			default_axis(i, g[i].auto_type, ZX_AXIS);
			break;
		    case 2:
			defaulty(i, -1);
			default_axis(i, g[i].auto_type, Y_AXIS);
			default_axis(i, g[i].auto_type, ZY_AXIS);
			break;
		    case 3:
			defaultgraph(i);
			default_axis(i, g[i].auto_type, X_AXIS);
			default_axis(i, g[i].auto_type, ZX_AXIS);
			default_axis(i, g[i].auto_type, Y_AXIS);
			default_axis(i, g[i].auto_type, ZY_AXIS);
			break;
		    }
		}
	    }
	}
    }				/* end for */
    cg = 0;			/* default is graph 0 */
/*
 * load legend
 */
    if (loadlegend) {
	for (i = 0; i < maxgraph; i++) {
	    if (isactive_graph(i) && (activeset(i))) {
		for (j = 0; j < maxplot; j++) {
		    if (isactive_set(i, j)) {
			strcpy(g[i].p[j].lstr, g[i].p[j].comments);
		    }
		}
	    }
	}
    }
/*
 * straighten our cursource if a pipe was used
 */
    if (cursource == 2) {
	cursource = SOURCE_DISK;
    }
/*
 * arrange graphs if grows & gcols set
 */
    arrange_graphs(grows, gcols);
/*
 * initialize the Hershey fonts
 */
    hselectfont(grdefaults.font);
/*
 * initialize the world and viewport
 */
    defineworld(g[cg].w.xg1, g[cg].w.yg1, g[cg].w.xg2, g[cg].w.yg2, islogx(cg), islogy(cg));
    viewport(g[cg].v.xv1, g[cg].v.yv1, g[cg].v.xv2, g[cg].v.yv2);
/*
 * if -h on command line or executed as grbatch just plot the graph and quit
 */
    if (grbatch || hardcopyflag) {
	if (hdevice == 0) {
	    fprintf(stderr,
		    "%s: Device 0 (Xlib) can't be used for batch plotting\n", argv[0]);
	    exit(1);
	}
	if (inpipe) {
	    getdata(cg, "STDIN", SOURCE_STDIN, SET_XY);
	    inpipe = 0;
	}
	if (batchfile[0]) {
	    getparms(batchfile);
	}
	if (!noprint) {
	    do_hardcopy();
	}
	if (resfp) {
	    fclose(resfp);
	}
	exit(0);
    }

/*
 * go window things up - do_main_loop is in x[v,m]gr.c
 */
    do_main_loop();
    return 0;
}

static void usage(FILE *stream, char *progname)
{
    fprintf(stream, "Usage of %s command line arguments: \n", progname);
    fprintf(stream, "-maxsets   [number_of_sets]           Set the number of data sets per graph\n");
    fprintf(stream, "                                        (minimum is 30)\n");
    fprintf(stream, "-maxgraph  [number_of_graphs]         Set the number of graphs for this session\n");
    fprintf(stream, "                                        (minimum is 10)\n");
    fprintf(stream, "-autoscale [x|y|xy]                   Override any parameter file settings\n");
    fprintf(stream, "-noauto    [x|y|xy]                   Supress autoscaling for the specified axis\n");
    fprintf(stream, "-arrange   [rows] [cols]              Arrange the graphs in a grid rows by cols\n");
    fprintf(stream, "-cols      [cols]\n");
    fprintf(stream, "-rows      [rows]\n");
    fprintf(stream, "-batch     [batch_file]               Execute batch_file on start up\n");
    fprintf(stream, "-noask                                Assume the answer is yes to all requests -\n");
    fprintf(stream, "                                        if the operation would overwrite a file,\n");
    fprintf(stream, "                                        xmgr will do so with out prompting\n");
    fprintf(stream, "-pipe                                 Read data from stdin on startup\n");
    fprintf(stream, "-npipe     [file]                     Read data from named pipe on startup\n");
    fprintf(stream, "-logwindow                            Open the log window\n");
    fprintf(stream, "-nologwindow                          No log window, overrides resource setting\n");
    fprintf(stream, "-device    [hardcopy device number]\n");
    fprintf(stream, "-hardcopy                             No interactive session, just print and\n");
    fprintf(stream, "                                        quit\n");
    fprintf(stream, "-eps                                  Set the PostScript driver to write EPS\n");
    fprintf(stream, "-nops2                                Do not use PostScript Level 2 features\n");
    fprintf(stream, "-log       [x|y|xy]                   Set the graph type to logarithmic\n");
    fprintf(stream, "-legend    [load]                     Turn the graph legend on\n");
    fprintf(stream, "-printfile [file for hardcopy output]\n");
    fprintf(stream, "-graph     [graph number]             Set the current graph number\n");
    fprintf(stream, "-graphsets [number_of_sets]           Set the number of data sets for the\n");
    fprintf(stream, "                                        current graph\n");
    fprintf(stream, "-graphtype [graph_type]               Set the type of the current graph\n");
    fprintf(stream, "-world     [xmin ymin xmax ymax]      Set the user coordinate system for the\n"); 
    fprintf(stream, "                                        current graph\n");
    fprintf(stream, "-view      [xmin ymin xmax ymax]      Set the viewport for the current graph\n");
    fprintf(stream, "-results   [results_file]             write the results from regression to\n");
    fprintf(stream, "                                        results_file\n");
    fprintf(stream, "-source    [disk|pipe|stdin]          Source of next data file\n");
    fprintf(stream, "-param     [parameter_file]           Load parameters from parameter_file to the\n");
    fprintf(stream, "                                        current graph\n");
    fprintf(stream, "-pexec     [parameter_string]         Interpret string as a parameter setting\n");
    fprintf(stream, "-type      [xy|xydx|xydy|xydxdx|xydydy|hilo] Set the type of the next data file\n");
    fprintf(stream, "-ihl       [ihl_file]                 Assume data file is in IHL format (local)\n");
    fprintf(stream, "-xy        [xy_file]                  Assume data file is in X Y format - sets\n");
    fprintf(stream, "                                        are separated by lines containing non-\n");
    fprintf(stream, "                                        numeric data\n");
    fprintf(stream, "-nxy       [nxy_file]                 Assume data file is in X Y1 Y2 Y3 ...\n"); 
    fprintf(stream, "                                        format\n");
    fprintf(stream, "-xydx      [xydx_file]                Assume data file is in X Y DX format\n");
    fprintf(stream, "-xydy      [xydy_file]                Assume data file is in X Y DY format\n");
    fprintf(stream, "-xydxdx    [xydxdx_file]              Assume data file is in X Y DX1 DX2 format\n");
    fprintf(stream, "-xydydy    [xydydy_file]              Assume data file is in X Y DY1 DY2 format\n");
    fprintf(stream, "-xydxdy    [xydxdy_file]              Assume data file is in X Y DX DY format\n");
    fprintf(stream, "-xyz       [xyz_file]                 Assume data file is in X Y Z format\n");
    fprintf(stream, "-xyd       [xyd_file]                 Assume data file is in X Y density format\n");
    fprintf(stream, "-xyr       [xyr_file]                 Assume data file is in X Y RADIUS format\n");
    fprintf(stream, "-rawspice  [rawspice_file]            Assume data is in rawspice format\n");
    fprintf(stream, "-block     [block_data]               Assume data file is block data\n");
    fprintf(stream, "-maxblock  [number_of_columns]        Set the number of columns for block data\n");
    fprintf(stream, "                                        (default is %d)\n", MAXPLOT);
    fprintf(stream, "-bxy       [x:y:etc.]                 Form a set from the current block data set\n");
    fprintf(stream, "                                        using the current set type from columns\n");
    fprintf(stream, "                                        given in the argument\n");
    fprintf(stream, "-hilo      [hilo_file]                Assume data is in X HI LO OPEN CLOSE\n"); 
    fprintf(stream, "                                        format\n");
    fprintf(stream, "-boxplot   [boxplot_file]             Assume data is in X MEDIAN Y1 Y2 Y3 Y4\n"); 
    fprintf(stream, "                                        format\n");
#if defined(HAVE_NETCDF) || defined(HAVE_MFHDF)
    fprintf(stream, "-netcdf    [netcdf file]              Assume data file is bnetCDF format\n");
    fprintf(stream, "-netcdfxy  [X var name] [Y var name]  If -netcdf was used previously, read from\n");
    fprintf(stream, "                                        the netCDF file, 'X var name' and 'Y\n"); 
    fprintf(stream, "                                        var name' and create a set. If 'X var\n");
    fprintf(stream, "                                        name' equals \"null\" then load the\n");
    fprintf(stream, "                                        index of Y to X\n");
#endif
    fprintf(stream, "-rvideo                               Exchange the color indices for black and\n");
    fprintf(stream, "                                        white\n");
    fprintf(stream, "-mono                                 Run %s in monochrome mode (affects the\n", progname);
    fprintf(stream, "                                        display only)\n");
    fprintf(stream, "-seed      [seed_value]               Integer seed for random number generator\n");
    fprintf(stream, "-GXxor                                Use xor to draw rubberband lines and graph\n");
    fprintf(stream, "                                        focus markers\n");
    fprintf(stream, "-GXinvert                             Use invert to draw rubberband lines and\n");
    fprintf(stream, "                                        graph focus markers\n");
    fprintf(stream, "-bs                                   Do backing store\n");
    fprintf(stream, "-nobs                                 Suppress backing store\n");
    fprintf(stream, "-dc                                   Allow double click operations on the\n");
    fprintf(stream, "                                        canvas\n");
    fprintf(stream, "-nodc                                 Ignore double clicks on the canvas\n");
    fprintf(stream, "-maxcolors  [max_colors]              Set the number of colors to allocate\n");
    fprintf(stream, "                                        (minimum is 17)\n");
    fprintf(stream, "-redraw                               Do a redraw for refreshing the canvas when\n");
    fprintf(stream, "                                        the server doesn't do backing store\n");
    fprintf(stream, "-noredraw                             Don't do a redraw for refreshing the\n");
    fprintf(stream, "                                        canvas when the server doesn't do\n");
    fprintf(stream, "                                        backing store\n");
    fprintf(stream, "-statusredraw                        Do a redraw after each action in the\n");
    fprintf(stream, "                                        status window\n");
    fprintf(stream, "-nostatusredraw                      Don't do a redraw after each action in the\n");
    fprintf(stream, "                                        status window\n");
    fprintf(stream, "-freecolors                           Attempt to free reallocated colors \n");
    fprintf(stream, "-nofreecolors                         Don't attempt to free reallocated colors\n");

    fprintf(stream, "-image     [image_file]               Argument is the name of an X Window dump\n");
    fprintf(stream, "                                        (.xwd format)\n");
    fprintf(stream, "-imagexy   [X] [Y]                    Arguments are the position of the image in\n");
    fprintf(stream, "                                        pixels, where (0,0) is the upper left\n");
    fprintf(stream, "                                        corner of the display and y increases\n");
    fprintf(stream, "                                        down the screen\n");
    fprintf(stream, "-noprint                              In batch mode, do not print\n");
    fprintf(stream, "-nosigcatch                           Don't catch signals\n");
#if defined(DEBUG)
    fprintf(stream, "-debug     [debug_level]              Set debugging options\n");
#endif
    fprintf(stream, "-usage|-help                          This message\n");
    fprintf(stream, "\n");
    fprintf(stream, " ** If it scrolls too fast, run `%s -help | more\' **\n", progname);
    exit(0);
}

/*
 * command loop
 */
void do_main_loop(void)
{
#ifdef NONE_GUI
    char pstring[MAX_STRING_LENGTH];
    int ilen;
    while (1) {
        printf("gr> ");
        fgets(pstring,MAX_STRING_LENGTH - 1,stdin);
        ilen = strlen(pstring);
        if (ilen == 0) {
            continue;
        }
        read_param(pstring);
    }
#else
    do_main_winloop();
#endif
}
