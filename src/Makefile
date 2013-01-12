#####################################################
# Makefile for XMGR4                                #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

GRBATCH=grbatch$(EXE)

CEPHES_LIB=$(TOP)/cephes/libcephes.a

CFLAGS=$(CFLAGS0) -I$(TOP) -I. -DGR_HOME=\"$(GR_HOME)\" \
	$(HELPVIEWER) $(NETCDF) -DPRINT_CMD=\"$(PRINT_CMD)\"

LIBS=$(GUI_LIBS) $(CEPHES_LIB) $(NETCDF_LIBS) $(NOGUI_LIBS) $(XDR_LIB) $(DL_LIB)

all : $(ACEGR)


include Make.common

include Make.dep


xmgr$(EXE) : $(GROBJS) $(PARS_C) $(GUIOBJS) $(CEPHES_LIB)
	$(CC) $(CFLAGS) $(GROBJS) $(GUIOBJS) -o $@ $(LDFLAGS) $(LIBS)

gr$(EXE) : $(GROBJS) $(PARS_C) $(CEPHES_LIB)
	$(CC) $(CFLAGS) $(GROBJS) -o $@ $(LDFLAGS) $(LIBS)


$(PARS_C) : $(PARS_Y)
	$(YACC) -t $?
	mv y.tab.c $@

install : $(ACEGR)
	$(MKINSTALLDIRS) $(GR_HOME)/bin
	$(INSTALL_PROGRAM) -s $(ACEGR) $(GR_HOME)/bin/$(ACEGR)
	cd $(GR_HOME)/bin; $(RM) $(GRBATCH); $(LN_S) $(ACEGR) $(GRBATCH)

tests : dummy

links : $(ACEGR)
	$(MKINSTALLDIRS) $(PREFIX)/bin
	cd $(PREFIX)/bin; $(RM) $(ACEGR); $(LN_S) $(GR_HOME)/bin/$(ACEGR) $(ACEGR)
	cd $(PREFIX)/bin; $(RM) $(GRBATCH); $(LN_S) $(GR_HOME)/bin/$(ACEGR) $(GRBATCH)

clean : 
	$(RM) $(GROBJS) $(GUIOBJS)

distclean : 
	$(RM) $(ACEGR) $(GROBJS) $(GUIOBJS) \
	tags *.orig *.*gr .gdb_history
	
devclean : 
	$(RM) $(ACEGR) $(GROBJS) $(GUIOBJS) \
	tags *.orig *.*gr .gdb_history $(PARS_C)

tags : $(GRSRCS) $(GUISRCS) $(PARS_Y)
	ctags $(GRSRCS) $(GUISRCS) $(PARS_Y)
	
lint :
	lint  $(GRINCS) $(GUIINCS) $(GRSRCS) $(GUISRCS)

rcs : Makefile pars.yacc $(GRINCS) $(GUIINCS) $(GRSRCS) $(GUISRCS)
	ci -l $?
	touch rcs

dummy :
