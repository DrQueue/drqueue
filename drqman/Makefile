#
# $Id$
#

ifeq ($(origin GTK_PATH),undefined)
GTK_PATH = /usr/local/gtk/bin
$(warning Using default path for GTK_PATH with value $(GTK_PATH))
endif

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))


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
 ifeq ($(systype),IRIX)
  CPPFLAGS += -D__IRIX
  CFLAGS += `pkg-config --cflags gtk+-2.0`
  LDFLAGS += `pkg-config --libs gtk+-2.0`
#  MAKE = /usr/freeware/bin/make
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
#   MAKE = make
  else
   ifeq ($(systype),FreeBSD)
    CPPFLAGS += -D__FREEBSD
    CFLAGS += `pkg-config --cflags gtk+-2.0`
    LDFLAGS = `pkg-config --libs gtk+-2.0`
    MAKE = gmake
   else
    ifeq ($(systype),CYGWIN_NT-5.1)
     CPPFLAGS += -D__CYGWIN
     CFLAGS += -D__CYGWIN `$(GTK_PATH)/pkg-config --cflags gtk+-2.0` -mms-bitfields
     LDFLAGS = `$(GTK_PATH)/pkg-config --libs gtk+-2.0`
     UIFLAGS = -e _mainCRTStartup -mwindows ../contrib/windows/Resources/drqueue.res 
     MAKE = make
    else
$(error Cannot make DrQueueManager -- systype "$(systype)" is unknown)
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

ifeq ($(systype),CYGWIN_NT-5.1)

../contrib/windows/Resources/drqueue.res: ../contrib/windows/Resources/drqueue.rc
	$(MAKE) -C ../contrib/windows/Resources

drqman: $(OBJS) ../libdrqueue.a ../contrib/windows/Resources/drqueue.res 
	$(CC) -o $@ $(OBJS) libdrqueue.a $(LDFLAGS) $(UIFLAGS)

else

#libdrqueue.i386: ../libdrqueue.a
#	lipo -output $@ -extract $(machinetype) $^
#	ranlib $@  

drqman: $(OBJS) ../libdrqueue.a
	$(CC) -o $@ $^ $(LDFLAGS)

endif

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

.PHONY: clean

clean:
	rm -f *.o *.so *.exe *~ drqman
