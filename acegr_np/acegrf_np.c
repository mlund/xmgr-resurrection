#include <config.h>

#if defined(HAVE_F77)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "acegr_np.h"

int
#ifdef NEED_F77_UNDERSCORE
acegropenf_ (const int *arg)
#else
acegropenf (const int *arg)
#endif
{
    return (ACEgrOpen (*arg));
}

int
#ifdef NEED_F77_UNDERSCORE
acegrclosef_ (void)
#else
acegrclosef (void)
#endif
{
    return (ACEgrClose ());
}

int
#ifdef NEED_F77_UNDERSCORE
acegrflushf_ (void)
#else
acegrflushf (void)
#endif
{
    return (ACEgrFlush ());
}


int
#ifdef NEED_F77_UNDERSCORE
acegrcommandf_ (const char* arg, int length)
#else
acegrcommandf (const char* arg, int length)
#endif
{
    char* str;
    int res;

    str = (char*) malloc ((size_t) (length + 1));
    if (str == NULL) {
        fprintf (stderr, "ACEgrCommandf: Not enough memory\n");
        return (-1);
    }
    strncpy (str, arg, length);
    str[length] = 0;
    res = ACEgrCommand (str);
    free (str);
    return (res);
}

#endif /* HAVE_F77 */

