# $Id: Makefile,v 1.19 2001/08/31 15:07:31 jorge Exp $

CC = gcc
OBJS_LIBDRQUEUE = computer_info.o computer_status.o task.o logger.o communications.o \
			computer.o request.o semaphores.o job.o drerrno.o database.o
LDFLAGS =

INSTROOT = /lot/s800/HOME/RENDUSR/drqueue
IRIX_INSTROOT = $(INSTROOT)/irix
LINUX_INSTROOT = $(INSTROOT)/linux

ifeq ($(systype),linux)
	CFLAGS = -Wall -I. -D__LINUX -g
	MAKE = make
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I. -D__IRIX -g
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
# 	if [ -d $(IRIX_INSTROOT) ]; do mkdir -p $(IRIX_INSTROOT); done
	cp ./slave $(IRIX_INSTROOT)/bin
	cp ./master $(IRIX_INSTROOT)/bin
	cp ./drqman/drqman $(IRIX_INSTROOT)/bin
	cp ./drqman/drqman /usr/local/software

linux_install: linux
# 	if [ -d $(LINUX_INSTROOT) ]; do	mkdir -p $(LINUX_INSTROOT); done
	cp ./slave $(LINUX_INSTROOT)/bin
	cp ./master $(LINUX_INSTROOT)/bin
	cp ./drqman/drqman $(LINUX_INSTROOT)/bin

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

