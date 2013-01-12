/* check_float.c -- Rolf Niepraschk 11/97, niepraschk@ptb.de */
/* test program for the cephes library definitions */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG2EA 0.44269504088896340735992
#define EQS(x, y) (strcmp(x, y) == 0)
#define ACCURACY "%1.4f"

typedef struct
{
  union {unsigned short s[5]; double d;} f;
  char *name;  
} XTYPE;

XTYPE X[] = { {{0037742,0124354,0122560,0057703}, "DEC"}, 
              {{0x0bf8,0x94ae,0x551d,0x3fdc}, "IBMPC"},
	      {{0x3fdc,0x551d,0x94ae,0x0bf8}, "MIEEE"},
	      {{0,0,0,0}, "UNK"},
	      {{0,0,0,0}, ""} 
	    };
	    
int main (int argc, char *argv[])
{
  int i; char TMPSTR[1024]; char LOG2EA_STR[80];
  i = 0;
  
  sprintf(LOG2EA_STR, ACCURACY, LOG2EA);
  
  for (i=0; *X[i].name; i++) 
  {
    sprintf(TMPSTR, ACCURACY, X[i].f.d);
    if (EQS(TMPSTR, LOG2EA_STR)) break;
  }
    
  if (*X[i].name) 
     printf("Your system needs \"#define %s = 1\"\n", X[i].name);
  else
     printf("Try to use \"#define %s = 1\"\n", X[--i].name); 
  
  
  exit(EXIT_SUCCESS);
}
