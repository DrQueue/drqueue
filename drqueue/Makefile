# $Id: Makefile,v 1.31 2002/05/17 16:02:05 jorge Exp $

CC = gcc
OBJS_LIBDRQUEUE = computer_info.o computer_status.o task.o logger.o communications.o \
			computer.o request.o semaphores.o job.o drerrno.o database.o common.o \
			mayasg.o
LDFLAGS =

INSTROOT = /lot/s800/HOME/RENDUSR/drqueue

ifeq ($(systype),linux)
	CFLAGS = -DCOMM_REPORT -Wall -I. -D__LINUX -g -O2
	MAKE = make
else 
 ifeq ($(systype),irix)
	CFLAGS = -DCOMM_REPORT -Wall -I. -D__IRIX -g -O2
	MAKE = /usr/freeware/bin/gmake
 endif
endif

.PHONY: clean irix linux tags irix_install doc

linux: 
	$(MAKE) systype=linux all
	(cd drqman; $(MAKE) linux)

irix:
	$(MAKE) systype=irix all
	(cd drqman; $(MAKE) irix)

irix_install: irix
	install -d -u rendusr -g staff -m 0777 $(INSTROOT)/bin
	su rendusr -c "install -m 0777 -u rendusr -g nisuser ./slave ./master $(IRIX_INSTROOT)/bin/"
	install -d -u rendusr -g staff -m 0777 $(INSTROOT)/etc
	su rendusr -c "cd etc; install -m 0777 -u rendusr -g nisuser ./maya.sg ./drqman.rc $(IRIX_INSTROOT)/etc/"
	cp ./drqman/drqman /usr/local/software

linux_install: linux
#	install -v -d -m 0777 -o rendusr -g 103 $(INSTROOT)/bin
	cp ./slave $(INSTROOT)/bin/slave.linux
	cp ./master $(INSTROOT)/bin/master.linux
	cp ./drqman/drqman $(INSTROOT)/bin/drqman.linux
	cp ./bin/slave.sh $(INSTROOT)/bin/slave.sh
	cp ./bin/master.sh $(INSTROOT)/bin/master.sh
	cp ./bin/path2unix.pl $(INSTROOT)/bin/path2unix.pl
	chmod 0777 $(INSTROOT)/bin/*.linux $(INSTROOT)/bin/*.sh $(INSTROOT)/bin/*.pl
	chown rendusr.103 $(INSTROOT)/bin/*.linux $(INSTROOT)/bin/*.sh $(INSTROOT)/bin/*.pl

linux_miniinstall: linux
	install -d -m 0777 ./bin
	install -p ./slave ./master ./drqman/drqman ./bin/

irix_miniinstall: irix
	install -d -m 0777 ./bin
	install -p -m 0777 ./slave ./master ./drqman/drqman ./bin/

doc:
	cxref *.[ch] drqman/*.[ch] -all-comments -xref-all -index-all -R/home/jorge/prog/drqueue -O/home/jorge/prog/drqueue/doc -html32 -D__LINUX

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

