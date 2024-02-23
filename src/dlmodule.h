/*
 * dlmodule.h - header for DLL stuff for ACE/gr
 */
#include <config.h>

#include "defines.h"

#if defined(HAVE_DLOPEN) || defined(HAVE_SHL_LOAD)
#define HAVE_DL
#endif

int load_module(char* fname, char* dl_function, char* dl_key, int dl_type);
