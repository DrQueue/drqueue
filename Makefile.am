AM_CPPFLAGS = -I$(srcdir)/libdrqueue -D_GNU_SOURCE -D_NO_COMPUTER_POOL_SEMAPHORES -D_NO_COMPUTER_SEMAPHORES

bin_PROGRAMS = slave master
slave_SOURCES = slave.c slave.h
slave_LDADD = libdrqueue/libdrqueue.la
master_SOURCES = master.c master.h
master_LDADD = libdrqueue/libdrqueue.la

SUBDIRS = libdrqueue drqman
EXTRA_DIST = bootstrap configure
