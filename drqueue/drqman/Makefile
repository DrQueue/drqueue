#
# $Id: Makefile,v 1.18 2004/03/09 18:53:22 jorge Exp $
#

CC = gcc
OBJS = main.o notebook.o drqm_jobs.o drqm_request.o drqm_computers.o drqm_common.o drqm_info.o \
				drqm_jobs_maya.o drqm_jobs_blender.o drqm_jobs_bmrt.o \
				drqm_jobs_pixie.o

ifeq ($(systype),linux)
	CFLAGS = -Wall -I.. -D__LINUX -g `pkg-config --cflags gtk+-2.0` -O2
	LDFLAGS = `pkg-config --libs gtk+-2.0`
	MAKE = make
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I.. -D__IRIX -g `gtk-config --cflags`
	LDFLAGS = `gtk-config --libs`
	MAKE = /usr/freeware/bin/make
 else
 	ifeq ($(systype),osx)
	 CFLAGS = -Wall -I.. -D__OSX -g `pkg-config --cflags gtk+-2.0` -O2
	 LDFLAGS = `pkg-config --libs gtk+-2.0`
	 MAKE = make
  endif
 endif
endif

.PHONY: clean irix linux tags
linux: 
	$(MAKE) systype=linux all
irix:
	$(MAKE) systype=irix all
osx:
	$(MAKE) systype=osx all

all: drqman

drqman: $(OBJS) ../libdrqueue.a
	$(CC) -o $@ $^ $(OOBJS) $(LDFLAGS)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *~ drqman

