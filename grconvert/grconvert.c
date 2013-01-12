/*
 * Utility to convert xmgr project files saved in old binary format
 */
#include <config.h>

#include <stdio.h>

#define MAIN

#include "grconvert.h"

static void usage(FILE *stream, char *progname);

int main(int argc, char *argv[])
{

    /* initialize plots, strings, graphs */
    set_program_defaults();

    if (argc != 3) {
        usage(stderr, argv[0]);
        return (1);
    }	
    if(!getbinary(cg, argv[1], 0)) {
	do_writesets(maxgraph, -1, 1, argv[2], sformat);
	return (0);
    } else {
        return (1);
    }
}

static void usage(FILE *stream, char *progname)
{
    fprintf(stream, "Usage: %s <infile> <outfile>\n", progname);
}
