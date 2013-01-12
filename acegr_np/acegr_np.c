/*************************************************************************/
/*  (C) 04.08.1997 Henrik Seidel (HS)                                    */
/*  <henrik@itb.biologie.hu-berlin.de>                                   */
/*************************************************************************/

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif

#include "acegr_np.h"

/* static global variables */
static char* buf = NULL;               /* global write buffer */
static int bufsize;                    /* size of the global write buffer */
static int bufsizeforce;               /* threshold for forcing a flush */
static char fifo[MAXPATHLEN] = "";     /* name of the fifo */
static int fd_fifo = -1;               /* file descriptor of the fifo */
static pid_t pid = (pid_t) -1;         /* pid of acegr */

/* function prototypes of static functions */
static void
#if (defined(HAVE_ON_EXIT))
unlink_fifo (int status, void* arg)
#else
unlink_fifo (void)
#endif
{
    /* Delete the named pipe if it exists */
    if (*fifo != 0) {
        unlink (fifo);
        *fifo = 0;
    }
}

static void
handler (const int signum)
{
    /* Call the exit procedure to ensure proper deleting of the named */
    /* pipe (we registered unlink_fifo with atexit) */
    exit (EXIT_FAILURE);
}

int
ACEgrOpen (const int arg)
{
    /* Set the buffer sizes according to arg */
    if (arg < 64) {
        fprintf (stderr, "The buffer size in ACEgrOpen should be >= 64\n");
        return (-1);
    }
    bufsize = arg;
    bufsizeforce = arg / 2;
    
    /* Don't exit on SIGPIPE */
    signal (SIGPIPE, SIG_IGN);
    
    /* Make sure that fifo is removed on exit and interruptions */
#if (defined(HAVE_ON_EXIT))
    on_exit (unlink_fifo, NULL);
#else
    atexit (unlink_fifo);
#endif
    signal (SIGHUP, handler);
    signal (SIGINT, handler);
    signal (SIGQUIT, handler);
    signal (SIGTERM, handler);
    signal (SIGCHLD, handler);

    /* Make the named pipe */
    if (mkfifo (tmpnam(fifo), 0600)) {
        perror ("ACEgrOpen");
        return (-1);
    }

    /* Fork a subprocess for starting acegr */
    pid = vfork ();
    if (pid == (pid_t) (-1)) {
        perror ("ACEgrOpen");
        return (-1);
    }

    /* If we are the child, replace ourselves with acegr */
    if (pid == (pid_t) 0) {
        /* we use a timer delay of 0.9 seconds -- it should not be an
           integer number since this might produce race conditions
           with sleep() */
        int retval = execlp ("xmgr", "xmgr", "-noask", "-npipe",
                             fifo, "-timer", "900", (char *) NULL);
        if (retval == -1) {
            perror ("ACEgrOpen: Could not start xmgr");
            return (-1);
        }
    }

    /* We are the parent -> open the fifo for writing and allocate the */
    /* write buffer */
    fd_fifo = open (fifo, O_WRONLY);
    if (fd_fifo == -1) {
        perror ("ACEgrOpen");
        return (-1);
    }
    buf = (char *) malloc ((size_t) bufsize);
    if (buf == NULL) {
        fprintf (stderr, "ACEgrOpen: Not enough memory\n");
        return (-1);
    }
    *buf = 0;
    return (0);
}

int
ACEgrClose (void)
{
    /* Don't exit when our child "acegr" exits */
    signal (SIGCHLD, SIG_IGN);
    /* Tell acegr to exit and wait until it did so */
    if (ACEgrCommand ("exit") == -1)
	return (-1);
    if (ACEgrFlush () == EOF)
	return (-1);
    waitpid (pid, NULL, 0);
    /* Close the fifo and free the buffer */
    if (close (fd_fifo) == -1)
	return (-1);
    free (buf);
    return (0);
}

int
ACEgrFlush (void)
{
    int loop = 0;
    int len, res;

    len = strlen (buf);
    while (loop < 30) {
        res = write (fd_fifo, buf, len);
        if (res == len) {
            /* We successfully wrote everything -> clear the buffer
               and return */
            *buf = 0;
            return (0);
        } else {
            /* There was an error, we could not write the buffer */
            if (errno == EPIPE) {
                /* Wait a second, since every 1.8 seconds there is a 0.9 s
                   time window for writing */
                sleep (1);
            } else {
                /* There was an error we cannot handle */
                perror ("ACEgrFlush");
                return (EOF);
            }
        }
        loop++;
    }
    fprintf (stderr, "ACEgrFlush: ran into eternal loop\n");
    return (EOF);
}

int
ACEgrPrintf (const char* fmt, ...)
{
    va_list ap;
    char* str;
    int nchar;
    
    /* Allocate a new string buffer for the function arguments */
    str = (char *) malloc ((size_t) bufsize);
    if (str == (char *) NULL) {
        fprintf (stderr, "ACEgrPrintf: Not enough memory\n");
        return (0);
    }
    /* Print to the string buffer according to the function arguments */
    va_start (ap, fmt);
#if defined(HAVE_VSNPRINTF)
    nchar = vsnprintf (str, bufsize - 2, fmt, ap);
#else
    nchar = vsprintf (str, fmt, ap);
#endif
    va_end (ap);
    nchar++;               /* This is for the appended "\n" */
    if (ACEgrCommand (str) == -1) {
        fprintf (stderr, "ACEgrPrintf: Error while calling ACEgrCommand\n");
        nchar = 0;
    }
    free (str);
    return (nchar);
}

int
ACEgrCommand (const char* cmd)
{
    int len, res;
    
    /* Append the new string to the global write buffer */
    strncat (buf, cmd, bufsize);
    strncat (buf, "\n", bufsize);
    len = strlen (buf);
    
    /* Try to send the global write buffer to acegr */
    res = write (fd_fifo, buf, len);
    if (res < len) {
        /* We could not write the buffer */
        if (errno == EPIPE) {
            /* The reason is that we got a SIGPIPE, we can handle
               this. If the filling of the global write buffer exceeds
               some threshold, flush the buffer. This involves waiting
               for a time window to write to acegr. */
            if (strlen (buf) >= bufsizeforce) {
                if (ACEgrFlush () == EOF) {
                    fprintf (stderr,
                             "ACEgrCommand: Can't flush write buffer\n");
                    return (-1);
                }
            }
        } else {
            /* There was some error we cannot handle */
            perror ("ACEgrCommand");
            return (-1);
        }
    } else {
        /* We could write the entire buffer -> clear it */
        *buf = 0;
    }
    return (0);
}
