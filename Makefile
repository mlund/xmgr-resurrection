#####################################################
# Top-level Makefile for XMGR4                      #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the current directory #
#####################################################

include Make.conf

subdirs : configure Make.conf
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE)); done

all : $(subdirs)

install : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) install); done

tests : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) tests); done

check : tests

links : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) links); done

clean : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean); done

distclean : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) distclean); done
	$(RM) config.log config.status config.cache config.h Make.conf

devclean : $(subdirs)
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) devclean); done
	$(RM) config.log config.status config.cache config.h Make.conf configure

changes : CHANGES

CHANGES : doc/CHANGES.html
	@lynx -dump $? > CHANGES

Make.conf : conf/Make.conf.in configure
	@echo
	@echo 'Please re-run ./configure'
	@echo
	@exit 1

configure : conf/configure.in
	autoconf $? > configure
	@echo
	@echo 'Please re-run ./configure'
	@echo
	@exit 1

dummy :

