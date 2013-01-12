#####################################################
# Makefile for documentation of XMGR                #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

DOCS = CHANGES.html dblclick.html intro.html plot.html trans.html \
	FAQ.html edcrset.html main.html points.html usage.html \
	file.html nohelp.html regions.html view.html \
	acegr_np.html fonts.html npipe.html sets.html xmgr.html \
	commands.html graphops.html options.html test_np.c \
	data.html hotkeys.html page.html testf_np.f dlmodule.html

all : dummy

clean : dummy

distclean : dummy

devclean : dummy

install : $(DOCS)
	$(MKINSTALLDIRS) $(GR_HOME)/doc
	for i in $(DOCS); do $(INSTALL_DATA) $$i $(GR_HOME)/doc; done

links : dummy

tests : dummy

dummy :
