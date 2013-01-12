/* vms_unix.h */
/* Rolf Niepraschk, 11/97, niepraschk@ptb.de */

#define unlink delete

char *xmalloc (unsigned int size);
void fatal (char *s1, char *s2);
void error (char *s1, char * s2);

#  if __VMS_VER < 70000000 
#    define pclose(x) close(x)
#    define popen(x, y) fopen(x, y)
#    define O_NONBLOCK O_NDELAY
     char *getlogin();
#  endif

int system_spawn(const char *command);

char* getlogin();
