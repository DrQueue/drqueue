# $Id: Makefile,v 1.3 2001/05/02 16:12:32 jorge Exp $

CC = gcc
OBJS_SLAVE = arch/computer_info.o arch/computer_status.o slave.o task.o logger.o
OBJS_MASTER = master.o database.o logger.o
LDFLAGS =

ifeq ($(systype),linux)
	CFLAGS = -Wall -I. -D__LINUX -g -O2
else 
 ifeq ($systype),irix)
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
slave.o : computer.h logger.h slave.h
logger.o : logger.h task.h job.h computer.h
computer.h : arch/computer_info.h arch/computer_status.h
master.o : database.h master.h
database.o : database.h

clean:
	rm -f *.o ./arch/*.o *~ ./arch/*~ slave master