# $Id: Makefile,v 1.5 2001/05/09 10:53:08 jorge Exp $

CC = gcc
OBJS_SLAVE = arch/computer_info.o arch/computer_status.o slave.o task.o logger.o communications.o \
		computer.o request.o semaphores.o
OBJS_MASTER = master.o database.o logger.o communications.o computer.o semaphores.o request.o \
		arch/computer_info.o arch/computer_status.o task.o
LDFLAGS =

ifeq ($(systype),linux)
	CFLAGS = -Wall -I. -D__LINUX -g -O2
else 
 ifeq ($(systype),irix)
	CFLAGS = -Wall -I. -D__IRIX -g -O2
 endif
endif

.PHONY: clean irix linux
linux: 
	make systype=linux all
irix:
	make systype=irix all

all: slave master

slave:	$(OBJS_SLAVE)

master: $(OBJS_MASTER)

task.o : task.h
arch/computer_info.o : arch/computer_info.h constants.h
arch/computer_status.o : arch/computer_status.h task.h logger.h
slave.o : computer.h logger.h slave.h request.h communications.h
logger.o : logger.h task.h job.h computer.h
computer.h : arch/computer_info.h arch/computer_status.h
master.o : database.h master.h communications.h logger.h request.h semaphores.h
database.o : database.h
communications.o : communications.h database.h semaphores.h logger.h
computer.o : computer.h database.h
semaphores.o : semaphores.h
request.o : request.h database.h logger.h communications.h semaphores.h

clean:
	rm -f *.o ./arch/*.o *~ ./arch/*~ slave master