# $Id: Makefile,v 1.9 2001/07/04 10:13:59 jorge Exp $

CC = gcc
OBJS_SLAVE = computer_info.o computer_status.o slave.o task.o logger.o communications.o \
		computer.o request.o semaphores.o job.o drerrno.o
OBJS_MASTER = master.o database.o logger.o communications.o computer.o semaphores.o request.o \
		computer_info.o computer_status.o task.o job.o drerrno.o computer.o
OBJS_SENDJOB = sendjob.o request.o job.o communications.o logger.o semaphores.o \
		computer_status.o computer_info.o task.o computer.o drerrno.o

LDFLAGS =
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
	make systype=irix all

all: slave master sendjob

slave:	$(OBJS_SLAVE)
master: $(OBJS_MASTER)
sendjob: $(OBJS_SENDJOB)
	gcc -o $@ $(OBJS_SENDJOB) $(LDFLAGS_CURSES)

task.o : task.h slave.h
computer_info.o : computer_info.h constants.h
computer_status.o : computer_status.h task.h logger.h
slave.o : computer.h logger.h slave.h request.h communications.h semaphores.h
logger.o : logger.h task.h job.h computer.h
computer.h : computer_info.h computer_status.h
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
