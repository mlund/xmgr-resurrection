#####################################################
# Makefile for grconvert -- VMS version (R.N.)      #
#####################################################

# not tested (RN)


TOP = [-]
HERE = SYS$DISK:[]
SRCDIR = [-.SRC]
VMSDIR = [-.SRC.VMS]
RM = DELETE/log

include $(TOP)Make.conf

O = .obj
EXE = .exe

# .SUFFIXES : .c $(O)

PROG = grconvert$(EXE)

SRCS = grconvert.c defaults.c readbin.c writeasc.c util.c

OBJS = grconvert$(O) defaults$(O) readbin$(O) writeasc$(O) util$(O)

# CFLAGS = $(CFLAGS0) -I$(TOP) -I. -I$(TOP)/src -DGRCONVERT
CFLAGS = /INCLUDE=($(TOP),$(HERE),$(SRCDIR),$(VMSDIR))/DEF=GRCONVERT=1

LIBS = $(NOGUI_LIBS)

VMSOBJ = ,$(VMSDIR)vms.opt/opt 
# VMSOBJ = ,$(VMSDIR)multinet.opt/opt 

all : $(PROG)
	@ !

$(PROG) : $(OBJS)
#	$(CC) -o $@ $(OBJS) $(LIBS)
	$(LINK) /EXE=$@ $+ $(LIBS) $(VMSOBJ)

tests : dummy

links : $(PROG)
	$(INSTALL) -d -m 755 $(PREFIX)/bin
	cd $(PREFIX)/bin; $(RM) $(PROG); $(LN_S) $(GR_HOME)/bin/$(PROG) $(PROG)
clean :
	$(RM) $(OBJS) 

distclean :
	$(RM) $(PROG) $(OBJS)
	
devclean :
	$(RM) $(PROG) $(OBJS)

install : $(PROG)
	$(INSTALL) -d -m 755 $(GR_HOME)/bin
	$(INSTALL) -s -m 755 $(PROG) $(GR_HOME)/bin/$(PROG)

dummy :

