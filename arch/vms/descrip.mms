#####################################################
# Top-level Makefile for XMGR4   (VMS)              #
#####################################################

# Rolf Niepraschk, 11/97, niepraschk@ptb.de

include Make.conf

CD = SET DEFAULT
TOP = [-]
ECHO = WRITE SYS$OUTPUT

ALL : CEPHES XMGR 
	@ !


.LAST
	 @ $(ECHO) ""
	 @ $(ECHO) "Done."

CEPHES :
	 @ $(CD) [.CEPHES]
	 @ $(MMS)/IGNORE=WARNING $(MMSQUALIFIERS) $(MMSTARGETS)
	 @ $(CD) $(TOP)

XMGR :
	 @ $(CD) [.SRC]
	 @ $(MMS)/IGNORE=WARNING $(MMSQUALIFIERS) $(MMSTARGETS)
	 @ $(CD) $(TOP)

clean : CEPHES XMGR
	@ !

distclean : CEPHES XMGR
	@ !
