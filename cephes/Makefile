#####################################################
# Makefile for Cephes math library                  #
#####################################################
# You should not change anything here.              #
# Please read INSTALL file in the upper directory   #
#####################################################

TOP=..

include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

CFLAGS=$(CFLAGS0) -I$(TOP) -I. 

LIB=libcephes.a

all : $(LIB)

include Make.common

include Make.dep

$(LIB) : $(OBJS)
	$(AR) cr $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

install : $(LIB)

tests : dummy

links : dummy

clean :
	$(RM) $(LIB) $(OBJS)

distclean :
	$(RM) $(LIB) $(OBJS)

devclean :
	$(RM) $(LIB) $(OBJS)

dummy :
