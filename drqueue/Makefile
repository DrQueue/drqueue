# $Id: Makefile,v 1.12 2001/07/11 14:50:28 jorge Exp $

CC = gcc
OBJS_LIBDRQUEUE = computer_info.o computer_status.o task.o logger.o communications.o \
			computer.o request.o semaphores.o job.o drerrno.o database.o
LDFLAGS = -ldrqueue -L.
LDFLAGS_CURSES = -lcurses

ifeq ($(systype),linux)
	CFLAGS = -Wall -I. -D__LINUX -g
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I. -D__IRIX -g
 endif
endif

.PHONY: clean irix linux tags
linux: 
	make systype=linux all
irix:
	/usr/freeware/bin/gmake systype=irix all

all: slave master sendjob

libdrqueue.a : $(OBJS_LIBDRQUEUE)
	ar q $@ $^
	ranlib $@
slave: slave.o libdrqueue.a
master: master.o libdrqueue.a
sendjob: sendjob.o libdrqueue.a

task.o : task.h slave.h
computer_info.o : computer_info.h constants.h
computer_status.o : computer_status.h task.h logger.h
slave.o : computer.h logger.h slave.h request.h communications.h semaphores.h
logger.o : logger.h task.h job.h computer.h
computer.h : computer_info.h computer_status.h logger.h
master.o : database.h master.h communications.h logger.h request.h semaphores.h drerrno.h
database.o : database.h computer.h
communications.o : communications.h database.h semaphores.h logger.h job.h drerrno.h task.h
computer.o : computer.h database.h
semaphores.o : semaphores.h
request.o : request.h database.h logger.h communications.h semaphores.h slave.h drerrno.h job.h
sendjob.o : sendjob.h job.h request.h
job.o : job.h database.h constants.h slave.h logger.h semaphores.h
drerrno.o: drerrno.h

clean:
	rm -f *.o *~ slave master sendjob TAGS

tags:
	etags *.[ch]
