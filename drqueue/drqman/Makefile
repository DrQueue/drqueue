#
# $Id: Makefile,v 1.15 2003/12/18 20:39:41 jorge Exp $
#

CC = gcc
OBJS = main.o notebook.o drqm_jobs.o drqm_request.o drqm_computers.o drqm_common.o drqm_info.o \
				drqm_jobs_maya.o drqm_jobs_blender.o drqm_jobs_bmrt.o

ifeq ($(systype),linux)
	CFLAGS = -Wall -I.. -D__LINUX -g `pkg-config --cflags gtk+` -O2
	LDFLAGS = `pkg-config --libs gtk+`
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
	$(CC) -o $@ $^ $(OOBJS) $(LDFLAGS)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *~ drqman

