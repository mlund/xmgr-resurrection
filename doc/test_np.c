#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <acegr_np.h>

#ifndef EXIT_SUCCESS
#  define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#  define EXIT_FAILURE -1
#endif

int
main (int argc, char* argv[])
{
    int i;

    /* Start xmgr with a buffer size of 2048 and open the pipe */
    if (ACEgrOpen(2048) == -1) {
        fprintf (stderr, "Can't run xmgr. \n");
        exit (EXIT_FAILURE);
    }
    
    /* Send some initialization commands to xmgr */
    ACEgrPrintf ("world xmax 100");
    ACEgrPrintf ("world ymax 10000");
    ACEgrPrintf ("xaxis tick major 20");
    ACEgrPrintf ("xaxis tick minor 10");
    ACEgrPrintf ("yaxis tick major 2000");
    ACEgrPrintf ("yaxis tick minor 1000");
    ACEgrPrintf ("sets symbol 2");
    ACEgrPrintf ("sets symbol fill 1");
    ACEgrPrintf ("sets symbol size 0.3");

    /* Display sample data */
    for (i = 1; i <= 100; i++) {
        ACEgrPrintf ("g0.s0 point %d, %d", i, i);
        ACEgrPrintf ("g0.s1 point %d, %d", i, i * i);
        /* Update the xmgr display after every ten steps */
        if (i % 10 == 0) {
            ACEgrPrintf ("redraw");
            /* Wait a second, just to simulate some time needed for
               calculations. Your real application shouldn't wait. */
            sleep (1);
        }
    }

    /* Tell xmgr to save the data */
    ACEgrPrintf ("saveall \"sample.gr\"");

    /* Flush the output buffer and close the pipe */
    ACEgrClose ();

    /* We are done */
    exit (EXIT_SUCCESS);
}

