# $Id$

CC = gcc
CPP = g++
OBJS_LIBDRQUEUE = computer_info.o computer_status.o task.o logger.o communications.o \
			computer.o request.o semaphores.o job.o drerrno.o database.o common.o \
			mayasg.o blendersg.o bmrtsg.o pixiesg.o
LDFLAGS =

ifeq ($(origin INSTROOT),undefined)
INSTROOT = /lot/s800/HOME/RENDUSR/drqueue
endif

ifeq ($(origin INSTUID),undefined)
INSTUID = drqueue
endif

ifeq ($(origin INSTGID),undefined)
INSTGID = drqueue
endif

#Figure out OS-specific Configuration parameters
ifeq ($(origin systype),undefined)
 systype=$(shell uname -s)
endif

ifeq ($(systype),Linux)
 CFLAGS = -DCOMM_REPORT -Wall -I. -D__LINUX -g -O2
 CPPFLAGS = -D__CPLUSPLUS -DCOMM_REPORT -Wall -I. -D__LINUX -g -O2
 MAKE = make
else 
 ifeq ($(systype),IRIX)
	CFLAGS = -DCOMM_REPORT -Wall -I. -D__IRIX -g -O2
  CPPFLAGS = -D__CPLUSPLUS -DCOMM_REPORT -Wall -I. -D__IRIX -g -O2
	MAKE = /usr/freeware/bin/gmake
 else
	ifeq ($(systype),Darwin)
	 CFLAGS = -DCOMM_REPORT -Wall -I. -D__OSX -g -O2
   CPPFLAGS = -D__CPLUSPLUS -DCOMM_REPORT -Wall -I. -D__OSX -g -O2
	 MAKE = make
	else
	 ifeq ($(systype),FreeBSD)
	  CFLAGS = -DCOMM_REPORT -Wall -I. -D__FREEBSD -g -O2
    CPPFLAGS = -D__CPLUSPLUS -DCOMM_REPORT -Wall -I. -D__FREEBSD -g -O2
	  MAKE = gmake
	 else
$(error Cannot make DrQueue -- systype "$(systype)" is unknown)
	 endif
	endif
 endif	
endif

ifneq ($(origin LIBWRAP),undefined)
 CFLAGS += -DLIBWRAP
 LDFLAGS += -lwrap
endif

#abstract make targets
.PHONY: default all install miniinstall irix_install linux_install doc tags clean

all: base drqman

base: slave master requeue sendjob jobfinfo listjobs blockhost

install: miniinstall $(systype)_install 


drqman: libdrqueue.a
	$(MAKE) -C drqman

IRIX_install:
	#FIXME This doesn't work, but I (Jorge) have no access to an Irix box by now.
	install -d -u rendusr -g staff -m 0777 $(INSTROOT)/bin
	su rendusr -c "install -m 0777 -u rendusr -g nisuser ./slave ./master $(INSTROOT)/bin/"
	install -d -u rendusr -g staff -m 0777 $(INSTROOT)/etc
	su rendusr -c "cd etc; install -m 0777 -u rendusr -g nisuser ./maya.sg ./drqman.rc $(INSTROOT)/etc/"
	cp ./drqman/drqman /usr/local/software

Linux_install:
	install -d -m 0777 $(INSTROOT)/tmp
	install -d -m 0777 $(INSTROOT)/logs
	install -d -m 0755 $(INSTROOT)/bin
	install -d -m 0755 $(INSTROOT)/etc
	install -d -m 0777 $(INSTROOT)/db
	install -d -m 0777 $(INSTROOT)/contrib
	cp ./bin/* $(INSTROOT)/bin/ || exit 0
	cp ./etc/* $(INSTROOT)/etc/ || exit 0
	cp ./contrib/* $(INSTROOT)/contrib/ || exit 0
	chmod 0755 $(INSTROOT)/bin/* || exit 0
	chmod 0755 $(INSTROOT)/contrib/* || exit 0
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/bin/*
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/contrib/*

FreeBSD_install:
	install -d -m 0777 $(INSTROOT)/tmp
	install -d -m 0777 $(INSTROOT)/logs
	install -d -m 0755 $(INSTROOT)/bin
	install -d -m 0755 $(INSTROOT)/etc
	install -d -m 0777 $(INSTROOT)/db
	install -d -m 0777 $(INSTROOT)/contrib
	cp ./bin/* $(INSTROOT)/bin/ || exit 0
	cp ./etc/* $(INSTROOT)/etc/ || exit 0
	cp ./contrib/* $(INSTROOT)/contrib/ || exit 0
	chmod 0755 $(INSTROOT)/bin/* || exit 0
	chmod 0755 $(INSTROOT)/contrib/* || exit 0
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/bin/*
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/contrib/*

Darwin_install:
	install -d -m 0777 $(INSTROOT)/tmp
	install -d -m 0777 $(INSTROOT)/logs
	install -d -m 0755 $(INSTROOT)/bin
	install -d -m 0755 $(INSTROOT)/etc
	install -d -m 0777 $(INSTROOT)/db
	install -d -m 0777 $(INSTROOT)/contrib
	cp ./bin/* $(INSTROOT)/bin/ || exit 0
	cp ./etc/* $(INSTROOT)/etc/ || exit 0
	cp ./contrib/* $(INSTROOT)/contrib/ || exit 0
	chmod 0755 $(INSTROOT)/bin/* || exit 0
	chmod 0755 $(INSTROOT)/contrib/* || exit 0
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/bin/*
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/contrib/*

miniinstall: base
ifeq ($(systype),IRIX)
	install -root $(PWD) -d -m 0755 bin
	install -root $(PWD) -m 0755 -f /bin -src slave slave.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src master master.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src requeue requeue.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src jobfinfo jobfinfo.$(systype)
	test -x ./drqman/drqman && install -root $(PWD) -m 0755 -f /bin -src drqman/drqman drqman.$(systype) || test 1
else
	install -d -m 0755 bin
	install -m 0755 -p ./slave bin/slave.$(systype)
	install -m 0755 -p ./master bin/master.$(systype)
	install -m 0755 -p ./requeue bin/requeue.$(systype)
	install -m 0755 -p ./jobfinfo bin/jobfinfo.$(systype)
	test -x ./drqman/drqman && install -m 0755 -p ./drqman/drqman bin/drqman.$(systype) || exit 0
endif

doc:
	cxref *.[ch] drqman/*.[ch] -all-comments -xref-all -index-all -R/home/jorge/prog/drqueue -O/home/jorge/prog/drqueue/doc -html32 -D__LINUX

tags:
	etags *.[ch] drqman/*.[ch]

clean:
	rm -fR *.o *~ libdrqueue.a slave master sendjob requeue jobfinfo listjobs TAGS tmp/* logs/* db/* bin/*.$(systype)
	rm -fR blockhost
	$(MAKE) -C drqman clean

#actual object make targets

libdrqueue.a : $(OBJS_LIBDRQUEUE) libdrqueue.h
	ar sq $@ $(OBJS_LIBDRQUEUE)
slave: slave.o libdrqueue.a
master: master.o libdrqueue.a
requeue: requeue.o libdrqueue.a
requeue.o: requeue.c
	$(CC) -c $(CFLAGS) -o $@ $<
jobfinfo: jobfinfo.o libdrqueue.a
jobfinfo.o: jobfinfo.c
	$(CC) -c $(CFLAGS) -o $@ $<
listjobs: listjobs.o libdrqueue.a
listjobs.o: listjobs.c
	$(CC) -c $(CFLAGS) -o $@ $<
blockhost: blockhost.o libdrqueue.a
blockhost.o: blockhost.c
	$(CC) -c $(CFLAGS) -o $@ $<

sendjob: sendjob.o libdrqueue.a
	$(CPP) $(CPPDFLAGS) -o $@ sendjob.o libdrqueue.a

libdrqueue.h: computer_info.h computer_status.h task.h logger.h communications.h \
			computer.h request.h semaphores.h job.h drerrno.h database.h common.h

%.o: %.c %.h constants.h
	$(CC) -c $(CFLAGS) -o $@ $<


