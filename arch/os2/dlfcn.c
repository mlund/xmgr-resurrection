/* dlfcn.c */ 
/* Implementation of dlopen() interface for OS/2               */
/* This code is released into public domain                    */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* We only need parts of the whole OS/2-specific stuff */
#define INCL_DOSMODULEMGR     /* Module Manager values */
#define INCL_DOSERRORS        /* Error values */
#include <os2.h>

#define LM_LENGTH 256
#define MAXDLLOPEN 256

static ULONG LoadErrorLength = LM_LENGTH;
static UCHAR LoadError[LM_LENGTH] = "";
static int LoadErrorFlag = FALSE;

void *dlopen( const char *filename, int flag)

{
  HMODULE DLLHandle;
  APIRET rc;
  int DLLFlag = FALSE;
 
  rc = DosLoadModule( LoadError,
                      LoadErrorLength-1,
                      filename,
                      &DLLHandle);
  if (rc != NO_ERROR) {
    sprintf(LoadError,
            "DosLoadModule(%s) = %lu\n", filename, rc);
    LoadErrorFlag = TRUE;
    return NULL;
  }
  LoadErrorFlag = FALSE;
  return (void*)DLLHandle;
}


char *dlerror(void)
{
  if (!LoadErrorFlag) {
    return NULL;
    }
  else {
    LoadErrorFlag = FALSE;
    return LoadError;
    }
}


void *dlsym(void *handle, char *symbol)
{

  APIRET rc;  
  PFN FuncAddress;
  
  rc = DosQueryProcAddr( (HMODULE) handle,
                             0L,
                             symbol,
                             &FuncAddress);
  if (rc != NO_ERROR) {
    sprintf(LoadError,
           "DosQueryProcAddr(%s)=%lu\n", symbol, rc);
    LoadErrorFlag = TRUE;
   return NULL;
   }
  LoadErrorFlag = FALSE;
  return (void*)FuncAddress;
}


int dlclose( void *handle )

{
  APIRET rc;

  rc = DosFreeModule( (HMODULE)handle );
  if (rc != NO_ERROR) {
    sprintf(LoadError,
            "DosFreeModule()=%lu\n", rc);
    LoadErrorFlag = TRUE;
   return 2;
   }
  LoadErrorFlag = FALSE;
  return 0;
}
