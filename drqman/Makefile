#
# $Id$
#

CC = gcc
OBJS = main.o notebook.o drqm_jobs.o drqm_request.o drqm_computers.o drqm_common.o drqm_info.o \
				drqm_jobs_maya.o drqm_jobs_mentalray.o drqm_jobs_blender.o drqm_jobs_bmrt.o \
				drqm_jobs_pixie.o drqm_jobs_jdd.o drqm_jobs_3delight.o

#Figure out OS-specific Configuration parameters
ifeq ($(origin systype),undefined)
 systype=$(shell uname -s)
endif

ifeq ($(systype),Linux)
 CFLAGS = -Wall -I.. -D__LINUX -g `pkg-config --cflags gtk+-2.0` -O2
 LDFLAGS = `pkg-config --libs gtk+-2.0`
 MAKE = make
else 
 ifeq ($(systype),IRIX)
  CFLAGS = -Wall -I.. -D__IRIX -g `pkg-config --cflags gtk+-2.0` -O2
  LDFLAGS = `pkg-config --libs gtk+-2.0`
  MAKE = /usr/freeware/bin/make
 else
  ifeq ($(systype),Darwin)
   CFLAGS = -Wall -I.. -D__OSX -g `pkg-config --cflags gtk+-2.0` -O2
   LDFLAGS = `pkg-config --libs gtk+-2.0`
   MAKE = make
  else
   ifeq ($(systype),FreeBSD)
    CFLAGS = -Wall -I.. -D__FREEBSD -g `pkg-config --cflags gtk+-2.0` -O2
    LDFLAGS = `pkg-config --libs gtk+-2.0`
    MAKE = gmake
   else
$(error Cannot make DrQueueManager -- systype "$(systype)" is unknown)
   endif
  endif
 endif	
endif

ifneq ($(origin LIBWRAP),undefined)
 CFLAGS += -DLIBWRAP
 LDFLAGS += -lwrap
endif


#abstract make targets

.PHONY: clean tags

all: drqman

drqman: $(OBJS) ../libdrqueue.a
	$(CC) -o $@ $^ $(OOBJS) $(LDFLAGS)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *~ drqman

