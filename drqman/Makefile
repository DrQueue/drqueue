#
# $Id$
#

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

GTK_PREFIX ?= /usr/local/gtk
#GTK_PATH ?= $(GTK_PREFIX)/bin
GTK_PATH ?= /usr/bin

kversion = $(shell uname -r)
universal = 0
ifeq ($(kversion),7.9.0)
SDK = /Developer/SDKs/MacOSX10.3.9.sdk
else
SDK = /Developer/SDKs/MacOSX10.4u.sdk
universal = 1
endif

#Figure out OS-specific Configuration parameters
ifeq ($(origin systype),undefined)
systype := $(shell bash -c "source ../bin/shlib; get_env_kernel")
machinetype := $(shell bash -c "source ../bin/shlib; get_env_machine")
endif

CPPFLAGS ?= -D_NO_COMPUTER_SEMAPHORES -D_NO_COMPUTER_POOL_SEMAPHORES -D_GNU_SOURCE -DCOMM_REPORT -I.. -I../libdrqueue
CFLAGS ?= -g -O3 -Wall
CXXFLAGS ?= $(CFLAGS) $(CPPFLAGS) -D__CPLUSPLUS

ifeq ($(systype),Linux)
 CPPFLAGS += -D__LINUX
 CFLAGS += `pkg-config --cflags gtk+-2.0`
 LDFLAGS += `pkg-config --libs gtk+-2.0`
 ifeq ($(machinetype),i686)
  ARCHFLAGS ?= -march=i686 -m32
 else
  ifeq ($(machinetype),x86_64)
   CHOST ?= x86_64-pc-linux-gnu
   ARCHFLAGS ?= -march=x86-64 -m64 -pipe
  endif
 endif
 CXXFLAGS += $(CFLAGS)
else
 ifeq ($(systype),GNU__kFreeBSD)
  CPPFLAGS += -D__LINUX
  CFLAGS += -D__LINUX `pkg-config --cflags gtk+-2.0`
  LDFLAGS += `pkg-config --libs gtk+-2.0`
else 
 ifeq ($(systype),IRIX)
  CPPFLAGS += -D__IRIX
  CFLAGS += `pkg-config --cflags gtk+-2.0`
  LDFLAGS += `pkg-config --libs gtk+-2.0`
 else
  ifeq ($(systype),Darwin)
   CPPFLAGS += -D__OSX
   ifeq ($(universal),1)
      CFLAGS +=-isysroot ${SDK}
      #LDFLAGS +=-Wl,-syslibroot,${SDK}
      ARCHFLAGS += -arch ppc -arch i386
   else 
     ifeq ($(machinetype),Power_Macintosh)
      ARCHFLAGS += -arch ppc
     else
      ARCHFLAGS += -arch i386
     endif
   endif
   CFLAGS += `pkg-config --cflags gtk+-2.0`
   LDFLAGS += `pkg-config --libs gtk+-2.0`
   CFLAGS += $(ARCHFLAGS)
  else
   ifeq ($(systype),FreeBSD)
    CPPFLAGS += -D__FREEBSD
    CFLAGS += `pkg-config --cflags gtk+-2.0`
    LDFLAGS += `pkg-config --libs gtk+-2.0`
    MAKE ?= gmake
   else
    ifeq ($(systype),CYGWIN_NT-5.1)
     CPPFLAGS += -D__CYGWIN 
     CFLAGS += $(CPPFLAGS) `$(GTK_PATH)/pkg-config.exe --define-variable=prefix=$(GTK_PREFIX) --cflags gtk+-2.0`
     LDFLAGS += `$(GTK_PATH)/pkg-config.exe --define-variable=prefix=$(GTK_PREFIX) --libs gtk+-2.0` -mwindows
    else
$(error Cannot make DrQueueManager -- systype "$(systype)" is unknown)
    endif
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

.PHONY: clean tags

all: drqman

#libdrqueue.i386: ../libdrqueue.a
#	lipo -output $@ -extract $(machinetype) $^
#	ranlib $@  

drqman: $(OBJS) ../libdrqueue.a
	$(CC) -o $@ $^ $(LDFLAGS)


%.o : %.c %.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *.exe *~ drqman
