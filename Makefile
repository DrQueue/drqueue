# $Id: Makefile,v 1.2 2001/04/26 16:06:22 jorge Exp $

CC = gcc
OBJS_SLAVE = arch/computer_info.o arch/computer_status.o slave.o task.o logger.o
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

all: slave

slave:	$(OBJS_SLAVE)

task.o : task.h
arch/computer_info.o : arch/computer_info.h constants.h
arch/computer_status.o : arch/computer_status.h task.h logger.h
slave.o : computer.h logger.h
logger.o : logger.h task.h
computer.h : arch/computer_info.h arch/computer_status.h

clean:
	rm -f *.o ./arch/*.o *~ ./arch/*~ slave