#
# $Id: Makefile,v 1.5 2001/07/19 09:06:26 jorge Exp $
#

CC = gcc
OBJS = main.o notebook.o drqm_jobs.o drqm_request.o drqm_computers.o

ifeq ($(systype),linux)
	CFLAGS = -Wall -I.. -D__LINUX -g `gtk-config --cflags`
	LDFLAGS = `gtk-config --libs`
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I.. -D__IRIX -g `gtk-config --cflags`
	LDFLAGS = `gtk-config --libs`
 endif
endif

.PHONY: clean irix linux tags
linux: 
	make systype=linux all
irix:
	/usr/freeware/bin/gmake systype=irix all

all: drqman

drqman: $(OBJS) ../libdrqueue.a
	cc -o $@ $^ $(OOBJS) $(LDFLAGS)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *~ drqman

