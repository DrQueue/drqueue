#
# $Id: Makefile,v 1.1 2001/07/13 15:23:33 jorge Exp $
#

CFLAGS = `gtk-config --cflags` -Wall -g
LDFLAGS = `gtk-config --libs`

OBJS = main.o notebook.o drqm_jobs.o

all: drqman

drqman: $(OBJS)
	cc -o $@ $^ $(OOBJS) $(LDFLAGS)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *~ drqman

