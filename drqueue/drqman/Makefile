#
# $Id: Makefile,v 1.6 2001/07/20 08:36:45 jorge Exp $
#

CC = gcc
OBJS = main.o notebook.o drqm_jobs.o drqm_request.o drqm_computers.o

ifeq ($(systype),linux)
	CFLAGS = -Wall -I.. -D__LINUX -g `gtk-config --cflags`
	LDFLAGS = `gtk-config --libs`
	MAKE = make
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I.. -D__IRIX -g `gtk-config --cflags`
	LDFLAGS = `gtk-config --libs`
	MAKE = /usr/freeware/bin/make
 endif
endif

.PHONY: clean irix linux tags
linux: 
	$(MAKE) systype=linux all
irix:
	$(MAKE) systype=irix all

all: drqman

drqman: $(OBJS) ../libdrqueue.a
	cc -o $@ $^ $(OOBJS) $(LDFLAGS)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *~ drqman

