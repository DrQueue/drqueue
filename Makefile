# $Id: Makefile,v 1.6 2001/05/28 14:21:31 jorge Exp $

CC = gcc
OBJS_SLAVE = arch/computer_info.o arch/computer_status.o slave.o task.o logger.o communications.o \
		computer.o request.o semaphores.o job.o drerrno.o
OBJS_MASTER = master.o database.o logger.o communications.o computer.o semaphores.o request.o \
		arch/computer_info.o arch/computer_status.o task.o job.o drerrno.o
OBJS_SENDJOB = sendjob.o request.o job.o communications.o logger.o semaphores.o \
		arch/computer_status.o arch/computer_info.o task.o computer.o drerrno.o

LDFLAGS =
LDFLAGS_CURSES = -lcurses

ifeq ($(systype),linux)
	CFLAGS = -Wall -I. -D__LINUX -g -O2
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I. -D__IRIX -g -O2
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

task.o : task.h
arch/computer_info.o : arch/computer_info.h constants.h
arch/computer_status.o : arch/computer_status.h task.h logger.h
slave.o : computer.h logger.h slave.h request.h communications.h
logger.o : logger.h task.h job.h computer.h
computer.h : arch/computer_info.h arch/computer_status.h
master.o : database.h master.h communications.h logger.h request.h semaphores.h drerrno.h
database.o : database.h
communications.o : communications.h database.h semaphores.h logger.h job.h drerrno.h
computer.o : computer.h database.h
semaphores.o : semaphores.h
request.o : request.h database.h logger.h communications.h semaphores.h slave.h drerrno.h
sendjob.o : sendjob.h job.h request.h
job.o : job.h database.h constants.h slave.h logger.h
drerrno.o: drerrno.h

clean:
	rm -f *.o ./arch/*.o *~ ./arch/*~ slave master sendjob TAGS

tags:
	etags *.[ch] arch/*.[ch]
