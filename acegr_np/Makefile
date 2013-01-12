#####################################################
# Makefile for libacegr_np                          #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c .f $(O)

LIB = libacegr_np.a

CFLAGS=$(CFLAGS0) -I$(TOP) -I. 

LIBS = $(NOGUI_LIBS)

LIBOBJS = acegr_np$(O) acegrf_np$(O)
TESTS=test_np testf_np

all : $(LIB)

tests : $(TESTS)
	-@PATH=../src:$$PATH; export PATH; \
	for i in $(TESTS); do echo Running $$i; ./$$i; done

$(LIB) : $(LIBOBJS) 
	$(RM) $@
	$(AR) cr $@ $(LIBOBJS)
	$(RANLIB) $@

$(LIBOBJS) : $(TOP)/config.h acegr_np.h

test_np : test_np$(O)
	$(CC) $(CFLAGS) -o $@ $? -L. -lacegr_np

testf_np : testf_np$(O)
	-$(FC) $(FFLAGS) -o $@ $? -L. -lacegr_np

test_np$(O) : $(LIB)
testf_np$(O) : $(LIB)

install : $(LIB)
	$(MKINSTALLDIRS) $(GR_HOME)/lib
	$(INSTALL_DATA) $(LIB) $(GR_HOME)/lib/$(LIB)
	$(MKINSTALLDIRS) $(GR_HOME)/include
	$(INSTALL_DATA) acegr_np.h $(GR_HOME)/include/acegr_np.h

links : $(LIB)
	$(MKINSTALLDIRS) $(PREFIX)/lib
	cd $(PREFIX)/lib; $(RM) $(LIB); $(LN_S) $(GR_HOME)/lib/$(LIB) $(LIB)
	$(MKINSTALLDIRS) $(PREFIX)/include
	cd $(PREFIX)/include; $(RM) acegr_np.h; $(LN_S) $(GR_HOME)/include/acegr_np.h acegr_np.h
	
clean :
	$(RM) *.o sample.gr $(TESTS)

distclean :
	$(RM) *.o sample.gr $(LIB) $(TESTS)
	
devclean :
	$(RM) *.o sample.gr $(LIB) $(TESTS)

