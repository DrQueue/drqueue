# $Id$

CC = gcc
CXX = g++
OBJS_LIBDRQUEUE = computer_info.o computer_status.o task.o logger.o communications.o \
			computer.o request.o semaphores.o job.o drerrno.o database.o common.o \
			generalsg.o mantrasg.o aqsissg.o mayasg.o mentalraysg.o blendersg.o bmrtsg.o pixiesg.o 3delightsg.o \
			lightwavesg.o aftereffectssg.o shakesg.o terragensg.o nukesg.o xsisg.o turtlesg.o envvars.o

ifeq ($(origin INSTROOT),undefined)
INSTROOT = /usr/local/drqueue
endif

ifeq ($(origin INSTUID),undefined)
INSTUID = drqueue
endif

ifeq ($(origin INSTGID),undefined)
INSTGID = drqueue 
endif

ifeq ($(origin DOTNETPATH),undefined)
DOTNETPATH = C:/WINDOWS/Microsoft.NET/Framework/v1.1.4322
endif

ifeq ($(origin NSISPATH),undefined)
NSISPATH = C:/Program\ Files/NSIS
endif

#Figure out OS-specific Configuration parameters
ifeq ($(origin systype),undefined)
 systype=$(shell uname -s)
endif

CFLAGS = -I. -g -O2 -Wall
CPPFLAGS = -D_GNU_SOURCE -DCOMM_REPORT
CXXFLAGS = $(CFLAGS) -D__CPLUSPLUS 

ifeq ($(systype),Linux)
 CPPFLAGS += -D__LINUX
 MAKE = make
else 
 ifeq ($(systype),IRIX)
	CFLAGS += -DCOMM_REPORT -Wall -I. -D__IRIX -g -O2
	CPPFLAGS += -D__CPLUSPLUS -DCOMM_REPORT -Wall -I. -D__IRIX -g -O2
	MAKE = /usr/freeware/bin/gmake
 else
	ifeq ($(systype),Darwin)
   CFLAGS += -DCOMM_REPORT -Wall -I. -D__OSX -g -O2
   CPPFLAGS += -D__CPLUSPLUS -DCOMM_REPORT -Wall -I. -D__OSX -g -O2
	 MAKE = make
	else
	 ifeq ($(systype),FreeBSD)
    CFLAGS = -DCOMM_REPORT -Wall -I. -D__FREEBSD -g -O2
    CPPFLAGS = -D__CPLUSPLUS -DCOMM_REPORT -Wall -I. -D__FREEBSD -g -O2
	  MAKE = gmake
	 else
	  ifeq ($(systype),CYGWIN_NT-5.1)
	   CFLAGS = -DCOMM_REPORT -Wall -I. -D__CYGWIN -g -O2 
     CPPFLAGS = -D__CPLUSPLUS -DCOMM_REPORT -Wall -I. -D__CYGWIN -g -O2
	   MAKE = make
	   UIFLAGS += -e _mainCRTStartup -mwindows contrib/windows/Resources/drqueue.res 
		else	
			$(error Cannot make DrQueue -- systype "$(systype)" is unknown)
	  endif
	 endif
	endif
 endif	
endif

ifneq ($(origin LIBWRAP),undefined)
 CFLAGS += -DLIBWRAP
 LDFLAGS += -lwrap
endif

#abstract make targets
.PHONY: default all install miniinstall irix_install linux_install doc tags clean testing_env drqman

BASE_C_TOOLS = slave master requeue jobfinfo blockhost ctask cjob jobinfo compinfo
BASE_CXX_TOOLS = sendjob

all: base drqman/drqman

drqman: drqman/drqman

base: $(BASE_C_TOOLS) $(BASE_CXX_TOOLS)

install: miniinstall $(systype)_install 

drqman/drqman: libdrqueue.a
	$(MAKE) -C drqman

testing_env:
	mkdir tmp logs db
	chmod 777 tmp

IRIX_install:
	install -d -m 0777 $(INSTROOT)/tmp
	install -d -m 0777 $(INSTROOT)/logs
	install -d -m 0755 $(INSTROOT)/bin
	install -d -m 0755 $(INSTROOT)/etc
	install -d -m 0777 $(INSTROOT)/db
	install -d -m 0777 $(INSTROOT)/contrib
	cp -r ./bin/* $(INSTROOT)/bin/ || exit 0
	cp ./etc/* $(INSTROOT)/etc/ || exit 0
	cp ./contrib/* $(INSTROOT)/contrib/ || exit 0
	chmod -R 0755 $(INSTROOT)/bin/* || exit 0
	chmod 0755 $(INSTROOT)/contrib/* || exit 0
	chown -R $(INSTUID):$(INSTGID) $(INSTROOT)/bin/*
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/contrib/*

Linux_install:
	install -d -m 0777 $(INSTROOT)/tmp
	install -d -m 0777 $(INSTROOT)/logs
	install -d -m 0755 $(INSTROOT)/bin
	install -d -m 0755 $(INSTROOT)/etc
	install -d -m 0777 $(INSTROOT)/db
	install -d -m 0777 $(INSTROOT)/contrib
	cp -r ./bin/* $(INSTROOT)/bin/ || exit 0
	cp ./etc/* $(INSTROOT)/etc/ || exit 0
	cp ./contrib/* $(INSTROOT)/contrib/ || exit 0
	chmod -R 0755 $(INSTROOT)/bin/* || exit 0
	chmod 0755 $(INSTROOT)/contrib/* || exit 0
	chown -R $(INSTUID):$(INSTGID) $(INSTROOT)/bin/*
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/contrib/*

CYGWIN_NT-5.1_install:
	install -d -m 0777 $(INSTROOT)/tmp
	install -d -m 0777 $(INSTROOT)/logs
	install -d -m 0755 $(INSTROOT)/bin
	install -d -m 0755 $(INSTROOT)/etc
	install -d -m 0777 $(INSTROOT)/db
	install -d -m 0777 $(INSTROOT)/contrib
	install -d -m 0777 $(INSTROOT)/contrib/windows
	install -d -m 0777 $(INSTROOT)/contrib/windows/Installer
	cp -r ./bin/*.exe $(INSTROOT)/bin/ || exit 0
	cp /usr/sbin/cygserver $(INSTROOT)/bin || exit 0
	cp `which expr.exe` $(INSTROOT)/bin || exit 0
	cp `which tcsh.exe` $(INSTROOT)/bin || exit 0
	cp `which cygpath.exe` $(INSTROOT)/bin || exit 0
	cp ./etc/* $(INSTROOT)/etc/ || exit 0
	sh ./contrib/windows/build_services.sh $(PWD)/contrib/windows $(DOTNETPATH) 
	cp ./contrib/* $(INSTROOT)/contrib/ || exit 0
	cp ./contrib/windows/*.exe $(INSTROOT)/contrib/windows || exit 0
	cp ./contrib/windows/Installer/* $(INSTROOT)/contrib/windows/installer || exit 0
	cp COPYING $(INSTROOT)/
	chmod -R 0755 $(INSTROOT)/bin/* || exit 0
	chmod 0755 $(INSTROOT)/contrib/* || exit 0
	sh contrib/windows/install_dlls.sh $(INSTROOT)/bin
	$(NSISPATH)/makensis.exe `cygpath -w $(INSTROOT)/contrib/windows/Installer/installer.nsi`
	mv $(INSTROOT)/contrib/windows/Installer/Install.exe $(INSTROOT)/contrib/drqueue-setup.exe  

FreeBSD_install:
	install -d -m 0777 $(INSTROOT)/tmp
	install -d -m 0777 $(INSTROOT)/logs
	install -d -m 0755 $(INSTROOT)/bin
	install -d -m 0755 $(INSTROOT)/etc
	install -d -m 0777 $(INSTROOT)/db
	install -d -m 0777 $(INSTROOT)/contrib
	cp -r ./bin/* $(INSTROOT)/bin/ || exit 0
	cp ./etc/* $(INSTROOT)/etc/ || exit 0
	cp ./contrib/* $(INSTROOT)/contrib/ || exit 0
	chmod -R 0755 $(INSTROOT)/bin/* || exit 0
	chmod 0755 $(INSTROOT)/contrib/* || exit 0
	chown -R $(INSTUID):$(INSTGID) $(INSTROOT)/bin/*
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/contrib/*

Darwin_install:
	install -d -m 0777 $(INSTROOT)/tmp
	install -d -m 0777 $(INSTROOT)/logs
	install -d -m 0755 $(INSTROOT)/bin
	install -d -m 0755 $(INSTROOT)/etc
	install -d -m 0777 $(INSTROOT)/db
	install -d -m 0777 $(INSTROOT)/contrib
	cp -r ./bin/* $(INSTROOT)/bin/ || exit 0
	cp ./etc/* $(INSTROOT)/etc/ || exit 0
	cp ./contrib/* $(INSTROOT)/contrib/ || exit 0
	chmod -R 0755 $(INSTROOT)/bin/* || exit 0
	chmod 0755 $(INSTROOT)/contrib/* || exit 0
	chown -R $(INSTUID):$(INSTGID) $(INSTROOT)/bin/*
	chown $(INSTUID):$(INSTGID) $(INSTROOT)/contrib/*

miniinstall: base
ifeq ($(systype),IRIX)
	install -root $(PWD) -d -m 0755 bin
	install -root $(PWD) -m 0755 -f /bin -src slave slave.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src master master.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src requeue requeue.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src jobfinfo jobfinfo.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src jobinfo jobinfo.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src blockhost blockhost.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src ctask ctask.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src cjob cjob.$(systype)
	install -root $(PWD) -m 0755 -f /bin -src sendjob sendjob.$(systype)
	test -x ./drqman/drqman && install -root $(PWD) -m 0755 -f /bin -src drqman/drqman drqman.$(systype) || test 1
else
 ifeq ($(systype),CYGWIN_NT-5.1)
	install -d -m 0755 bin
	install -m 0755 -p ./slave.exe bin/slave.exe
	install -m 0755 -p ./master.exe bin/master.exe
	install -m 0755 -p ./requeue.exe bin/requeue.exe
	install -m 0755 -p ./jobfinfo.exe bin/jobfinfo.exe
	install -m 0755 -p ./jobinfo.exe bin/jobfinfo.exe
	install -m 0755 -p ./blockhost.exe bin/blockhost.exe
	install -m 0755 -p ./ctask.exe bin/ctask.exe
	install -m 0755 -p ./cjob.exe bin/cjob.exe
	install -m 0755 -p ./sendjob.exe bin/sendjob.exe
	test -x ./drqman/drqman.exe && install -m 0755 -p ./drqman/drqman.exe bin/drqman.exe || exit 0
else
	install -d -m 0755 bin
	install -m 0755 -p ./slave bin/slave.$(systype)
	install -m 0755 -p ./master bin/master.$(systype)
	install -m 0755 -p ./requeue bin/requeue.$(systype)
	install -m 0755 -p ./jobfinfo bin/jobfinfo.$(systype)
	install -m 0755 -p ./jobinfo bin/jobinfo.$(systype)
	install -m 0755 -p ./blockhost bin/blockhost.$(systype)
	install -m 0755 -p ./ctask bin/ctask.$(systype)
	install -m 0755 -p ./cjob bin/cjob.$(systype)
	install -m 0755 -p ./sendjob bin/sendjob.$(systype)
	test -x ./drqman/drqman && install -m 0755 -p ./drqman/drqman bin/drqman.$(systype) || exit 0
endif
endif

doc:
	cxref *.[ch] drqman/*.[ch] -all-comments -xref-all -index-all -R/home/jorge/prog/drqueue -O/home/jorge/prog/drqueue/doc -html32 -D__LINUX

tags:
	etags *.[ch] drqman/*.[ch]

clean:
	rm -fR *.o *.exe *~ libdrqueue.a $(BASE_C_TOOLS) $(BASE_CXX_TOOLS) TAGS tmp/* logs/* db/* contrib/windows/*.exe bin/*.$(systype)
	$(MAKE) -C drqman clean

#actual object make targets
libdrqueue.h: $(OBJS_LIBDRQUEUE:.o=.h)
libdrqueue.a: $(OBJS_LIBDRQUEUE) libdrqueue.h
	ar sq $@ $(OBJS_LIBDRQUEUE)

ifeq ($(systype),CYGWIN_NT-5.1)
contrib/windows/Resources/drqueue.res: contrib/windows/Resources/drqueue.rc
	$(MAKE) -C contrib/windows/Resources
endif

%.c: %.h
%.cpp: %.h
%.o: %.c %.h constants.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

slave: slave.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@
master: master.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@
requeue: requeue.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@
jobinfo: jobinfo.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@
jobfinfo: jobfinfo.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@
blockhost: blockhost.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@
ctask: ctask.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@
cjob: cjob.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@
compinfo: compinfo.o libdrqueue.a
	$(CC) $(LDFLAGS) $^ -o $@

sendjob.o: sendjob.cpp sendjob.h
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
sendjob: sendjob.o libdrqueue.a
	$(CXX) $^ $(LDFLAGS) -o $@ 


