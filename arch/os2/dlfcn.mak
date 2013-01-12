CC= gcc
CFLAGS= -Zmt -O2
AR= ar rc

.SUFFIXES: .c .o .a

default: dlfcn.a

all: default

dlfcn.a: dlfcn.o
	$(AR) $@ $<

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
