/* vms_unix.c */
/* Rolf Niepraschk, 11/97, niepraschk@ptb.de */

/* some suggestions comes from the gawk VMS port -- R.N. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <starlet.h>
#include <jpidef.h>
#include <ssdef.h>
#include <lib$routines.h>
#include <descrip.h>
#include <clidef.h>
#include <errno.h>
#include <math.h>


/* this is a "system" function for VMS because 
   system("spawn/nowait mosaic ...") dosn't work -- why not? -- RN */
int system_spawn(const char *command)
{
  $DESCRIPTOR(dstr, ""); int retval;
  
  dstr.dsc$a_pointer = malloc(1024);
  strcpy(dstr.dsc$a_pointer, command);
  dstr.dsc$w_length = strlen(dstr.dsc$a_pointer);

  retval = lib$spawn(&dstr, 0, 0, &CLI$M_NOWAIT); 
  
  free(dstr.dsc$a_pointer);
  
  return retval; 
}

#if __VMS_VER < 70000000 

  typedef struct
  {
    unsigned short buffer_length, item_code;
    char    *buffer;
    int     *return_len;
  } itmlst_item;

  char *getlogin()
  {
    int ret, i; itmlst_item itmlst[2];
    static char username[13];

    itmlst[0].buffer_length = sizeof(username)-1;
    itmlst[0].item_code = JPI$_USERNAME;
    itmlst[0].buffer = username;
    itmlst[0].return_len = NULL;

    itmlst[1].buffer_length = 0;
    itmlst[1].item_code = 0;
    itmlst[1].buffer = NULL;
    itmlst[1].return_len = NULL;

    ret = sys$getjpiw(NULL, NULL, NULL, itmlst, NULL, NULL, NULL);

    if ((ret && 1) == 0) strcpy(username, "");
    else
    {
      for (i=0; i<sizeof(username); i++)
      {
	if (username[i] == ' ')
	{
          username[i] = '\0'; break;
	}
      }
      if (i == sizeof(username)) username[--i] = '\0';
    }

    return username; 
  }

#endif

/* Print error message.  `s1' is printf control string, `s2' is arg for it. */

extern char *progname;

void error (char *s1, char * s2)
{
  fprintf (stderr, "%s: ", progname);
  fprintf (stderr, s1, s2);
  fprintf (stderr, "\n");
}

/* Print error message and exit.  */

void fatal (char *s1, char *s2)
{
  error (s1, s2);
  exit (1);
}

/* Like malloc but get fatal error if memory is exhausted.  */

char *xmalloc (unsigned int size)
{
  char *result = (char *) malloc (size);
  if (result == NULL)
    fatal ("virtual memory exhausted", 0);
  return result;
}


