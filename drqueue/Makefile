# $Id: Makefile,v 1.26 2001/09/05 09:43:53 jorge Exp $

CC = gcc
OBJS_LIBDRQUEUE = computer_info.o computer_status.o task.o logger.o communications.o \
			computer.o request.o semaphores.o job.o drerrno.o database.o common.o \
			mayasg.o
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
	su rendusr -c "install -m 0777 -u rendusr -g staff ./slave ./master $(IRIX_INSTROOT)/bin/"
	cp ./drqman/drqman /usr/local/software

linux_install: linux
	install -d -o rendusr -g staff -m 0777 $(LINUX_INSTROOT)/bin
	install -p ./slave ./master ./drqman/drqman $(LINUX_INSTROOT)/bin/

linux_miniinstall: linux
	install -d -m 0777 ./bin
	install -p ./slave ./master ./drqman/drqman ./bin/

irix_miniinstall: irix
	install -d -m 0777 ./bin
	install -p -m 0777 ./slave ./master ./drqman/drqman ./bin/

all: slave master sendjob

libdrqueue.a : $(OBJS_LIBDRQUEUE) libdrqueue.h
	ar sq $@ $(OBJS_LIBDRQUEUE)
slave: slave.o libdrqueue.a
master: master.o libdrqueue.a
sendjob: sendjob.o libdrqueue.a

libdrqueue.h: computer_info.h computer_status.h task.h logger.h communications.h \
			computer.h request.h semaphores.h job.h drerrno.h database.h common.h

%.o: %.c %.h constants.h
	$(CC) -c $(CFLAGS) -o $@ $<
clean:
	rm -f *.o *~ libdrqueue.a slave master sendjob TAGS
	(cd drqman; $(MAKE) clean)

tags:
	etags *.[ch] drqman/*.[ch]

