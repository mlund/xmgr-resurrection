/*
 * dlmodule.c - DLL stuff for ACE/gr
 * The following interfaces are supported:
 *  + dlopen() (Linux, SunOS, Solaris, OSF, IRIX, AIX-4, UnixWare, ...)
 *  + shl_load() (HP/UX)
 *  + AIX-3 - there is a free dlopen() emulation library
 *  - VMS, OS2 ??
 */

#include <config.h>

#if defined(HAVE_DLOPEN)
#  include <dlfcn.h>
#endif

#if defined(HAVE_SHL_LOAD)
#  include <dl.h>
#endif

#include <string.h>

#include "dlmodule.h"

#include "defines.h"
#include "globals.h"
#include "protos.h"

int load_module(char *fname, char *dl_function, char *dl_key, int dl_type)
{
#if defined(HAVE_DL)

    int dlflag;
    void *handle;
    char *error;
    symtab_entry newkey;
    
    if ((dl_type < 0) || (dl_key == NULL) || (dl_function == NULL)) {
        errmsg("Improper call to load_module()");
	return (1);
    }
    
#if defined(HAVE_DLOPEN)
    if (dl_load_fast == TRUE) {
        dlflag = RTLD_LAZY;
    } else {
        dlflag = RTLD_NOW;
    }
    
    handle = (void *) dlopen (fname, dlflag);
    if (!handle) {
        errmsg ((char *) dlerror());
        return (1);
    }
    
    newkey.fnc = (double (*)()) dlsym(handle, dl_function);
    if ((error = (char *) dlerror()) != NULL) {
        errmsg(error);
        dlclose(handle);
        return (1);
    }

#endif /* end dlopen interface */

#if defined(HAVE_SHL_LOAD)

    if (dl_load_fast == TRUE) {
        dlflag = BIND_DEFERRED;
    } else {
        dlflag = BIND_IMMEDIATE;
    }
    
    handle = (void *) shl_load (fname, dlflag, 0L);
    if (!handle) {
#if defined(HAVE_STRERROR)
        errmsg (strerror(errno));
#else
        errmsg ("DL module initialization failed");
#endif
        return (1);
    }
    
    if (shl_findsym(handle, dl_function, TYPE_UNDEFINED, &newkey.fnc) != NULL) {
#if defined(HAVE_STRERROR)
        errmsg (strerror(errno));
#else
        errmsg("Error while resolving symbol");
#endif
        shl_unload(handle);
        return (1);
    }

#endif /* end shl_load interface */

    newkey.type = dl_type;
    newkey.s = malloc(strlen(dl_key) + 1);
    strcpy(newkey.s, dl_key);
    lowtoupper(newkey.s);
    
    if (addto_symtab(newkey) != 0){
        return (1);
    } else {
        return (0);
    }

#else /* no support for DL */
    errmsg("No support for DL modules on your OS");
    return (1);
#endif
}
