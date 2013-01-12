#####################################################
# Makefile for examples of XMGR                     #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

DOTEST = dotest$(BAT)

EXAMPLES = altaxis.par logtest.par tbar3.dat times.dat \
	au.d manygraphs.d terr.d tinset.d \
	axes.par mlo.dat terr2.d tlog.demo \
	bar.d explain.par moresyms.dat test.com tmc.c \
	bar2.d fills.dat props.gr test.dat tpipe.d \
	bars.d graphs.par regions.par test1.par tstack.dat \
	brw.dat hilo.dat slideshow.d test2.d txyr.dat \
	co2.all log.d stackedb.d tfonts.par \
	co2.par logistic.d symslines.dat tforms.par

all : dummy

clean : dummy

distclean : dummy

devclean : dummy

install : $(EXAMPLES) $(DOTEST)
	$(MKINSTALLDIRS) $(GR_HOME)/examples
	for i in $(EXAMPLES); do $(INSTALL_DATA) $$i $(GR_HOME)/examples; done
	$(INSTALL_PROGRAM) $(DOTEST) $(GR_HOME)/examples/$(DOTEST)

links : dummy

tests : $(EXAMPLES) $(DOTEST)
	./$(DOTEST)

dummy :
