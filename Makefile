# $Id: Makefile,v 1.22 2001/09/01 16:31:41 jorge Exp $

CC = gcc
OBJS_LIBDRQUEUE = computer_info.o computer_status.o task.o logger.o communications.o \
			computer.o request.o semaphores.o job.o drerrno.o database.o
LDFLAGS =

INSTROOT = /lot/s800/HOME/RENDUSR/drqueue
IRIX_INSTROOT = $(INSTROOT)/irix
LINUX_INSTROOT = $(INSTROOT)/linux

ifeq ($(systype),linux)
	CFLAGS = -Wall -I. -D__LINUX -g -O2
	MAKE = make
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I. -D__IRIX -g -O2
	MAKE = /usr/freeware/bin/gmake
 endif
endif

.PHONY: clean irix linux tags irix_install

linux: 
	$(MAKE) systype=linux all
	(cd drqman; $(MAKE) linux)

irix:
	$(MAKE) systype=irix all
	(cd drqman; $(MAKE) irix)

irix_install: irix
	install -d -u rendusr -g staff -m 0777 $(IRIX_INSTROOT)/bin
	install -u rendusr -g staff ./slave ./master ./drqman/drqman $(IRIX_INSTROOT)/bin/

linux_install: linux
	install -d -o rendusr -g staff -m 0777 $(LINUX_INSTROOT)/bin
	install -p ./slave ./master ./drqman/drqman $(LINUX_INSTROOT)/bin/

all: slave master sendjob

libdrqueue.a : $(OBJS_LIBDRQUEUE) libdrqueue.h
	ar sq $@ $(OBJS_LIBDRQUEUE)
slave: slave.o libdrqueue.a
master: master.o libdrqueue.a
sendjob: sendjob.o libdrqueue.a

libdrqueue.h: computer_info.h computer_status.h task.h logger.h communications.h \
			computer.h request.h semaphores.h job.h drerrno.h database.h

%.o: %.c %.h constants.h
	$(CC) -c $(CFLAGS) -o $@ $<
clean:
	rm -f *.o *~ libdrqueue.a slave master sendjob TAGS
	(cd drqman; $(MAKE) clean)

tags:
	etags *.[ch] drqman/*.[ch]

