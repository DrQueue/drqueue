SUBDIRS = libdrqueue . drqman
AM_CPPFLAGS = -I$(srcdir)/libdrqueue -D_GNU_SOURCE -D_NO_COMPUTER_POOL_SEMAPHORES -D_NO_COMPUTER_SEMAPHORES @my_sys@

bin_PROGRAMS = slave master
slave_SOURCES = slave.c
slave_LDADD = libdrqueue/libdrqueue.la
master_SOURCES = master.c
master_LDADD = libdrqueue/libdrqueue.la

EXTRA_DIST = bootstrap configure
