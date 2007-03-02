EXTRA_DIST = bootstrap
SUBDIRS = libdrqueue . drqman
AM_CPPFLAGS = -I$(srcdir)/libdrqueue

bin_PROGRAMS = slave master
slave_SOURCES = slave.c slave.h
slave_LDADD = libdrqueue/libdrqueue.a
master_SOURCES = master.c master.h
master_LDADD = libdrqueue/libdrqueue.a
