#
# $Id$
#

ifeq ($(origin GTK_PATH),undefined)
GTK_PATH = /usr/local/gtk/bin
endif

CC = gcc
OBJS = main.o notebook.o drqm_jobs.o drqm_request.o drqm_computers.o drqm_common.o drqm_info.o \
				drqm_jobs_maya.o drqm_jobs_mentalray.o drqm_jobs_blender.o drqm_jobs_bmrt.o \
				drqm_jobs_pixie.o drqm_jobs_jdd.o drqm_jobs_3delight.o drqm_jobs_lightwave.o drqm_cygwin.o \
				drqm_jobs_aqsis.o drqm_autorefresh.o drqm_jobs_aftereffects.o drqm_jobs_shake.o drqm_jobs_terragen.o \
				drqm_jobs_nuke.o drqm_jobs_turtle.o drqm_jobs_mantra.o drqm_jobs_xsi.o

#Figure out OS-specific Configuration parameters
ifeq ($(origin systype),undefined)
 systype=$(shell uname -s)
endif

CFLAGS = -I../libdrqueue -I.. -Wall -g -O2

ifeq ($(systype),Linux)
 CPPFLAGS += -D__LINUX
 CFLAGS += -D__LINUX `pkg-config --cflags gtk+-2.0`
 LDFLAGS = `pkg-config --libs gtk+-2.0`
 MAKE = make
else 
 ifeq ($(systype),IRIX)
  CPPFLAGS += -D__IRIX
  CFLAGS += -D__IRIX `pkg-config --cflags gtk+-2.0`
  LDFLAGS = `pkg-config --libs gtk+-2.0`
  MAKE = /usr/freeware/bin/make
 else
  ifeq ($(systype),Darwin)
   CPPFLAGS += -D__OSX
   CFLAGS += -D__OSX `pkg-config --cflags gtk+-2.0`
   LDFLAGS = `pkg-config --libs gtk+-2.0`
   MAKE = make
  else
   ifeq ($(systype),FreeBSD)
    CPPFLAGS += -D__FREEBSD
    CFLAGS += -D__FREEBSD `pkg-config --cflags gtk+-2.0`
    LDFLAGS = `pkg-config --libs gtk+-2.0`
    MAKE = gmake
   else
    ifeq ($(systype),CYGWIN_NT-5.1)
     CPPFLAGS += -D__CYGWIN
     CFLAGS += -D__CYGWIN `$(GTK_PATH)/pkg-config --cflags gtk+-2.0` -mms-bitfields
     LDFLAGS = `$(GTK_PATH)/pkg-config --libs gtk+-2.0`
     UIFLAGS = -e _mainCRTStartup -mwindows ../contrib/windows/Resources/drqueue.res 
     MAKE = make
    else
$(error Cannot make DrQueueManager -- systype "$(systype)" is unknown)
    endif
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

ifeq ($(systype),CYGWIN_NT-5.1)

../contrib/windows/Resources/drqueue.res: ../contrib/windows/Resources/drqueue.rc
	$(MAKE) -C ../contrib/windows/Resources
 
drqman: $(OBJS) ../libdrqueue.a ../contrib/windows/Resources/drqueue.res 
	$(CC) -o $@ $(OBJS) ../libdrqueue.a $(LDFLAGS) $(UIFLAGS) 

else

drqman: $(OBJS) ../libdrqueue.a
	$(CC) -o $@ $^ $(OOBJS) $(LDFLAGS)

endif

%.o : %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *.exe *~ drqman
