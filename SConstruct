import sys
import glob
import os

env = Environment ()

env.Append (CCFLAGS=Split ('-DCOMM_REPORT -D_GNU_SOURCE -D_NO_COMPUTER_POOL_SEMAPHORES -Wall -g -O0'))

print "Platform is: ",sys.platform

if sys.platform == "linux2":
	env.Append (CCFLAGS = Split ('-D__LINUX'))
elif sys.platform == "darwin":
	#env.Append (CCFLAGS = Split ('-D__OSX -no-cpp-precomp -fstrict-aliasing -mno-fused-madd'))
	env.Append (CCFLAGS = Split ('-D__OSX'))
else:
	print "Unknown platform: %s"%(sys.platform,)
	exit (1)

env.Append (CXXFLAGS='-D__CPLUSPLUS')
env.Append (CPPPATH=['.','./libdrqueue'])

#
# libdrqueue.a
#
libdrqueue_src = glob.glob("./libdrqueue/*.c")
libdrqueue = env.StaticLibrary ('drqueue', libdrqueue_src)

#
# drqman
#
drqman_c = glob.glob ("drqman/*.c")
gtkcflags = os.popen('pkg-config --cflags gtk+-2.0').read()
gtklibs = os.popen('pkg-config --libs gtk+-2.0').read()
env_gtkstuff = env.Copy ()
env_gtkstuff.Append (CCFLAGS = Split(gtkcflags))
env_gtkstuff.Append (LINKFLAGS = Split(gtklibs))
drqman = env_gtkstuff.Program ('drqman/drqman',drqman_c, LIBS=['drqueue',], LIBPATH=['.',])

#
# Main
#
master = env.Program ('master.c', LIBS=['drqueue',], LIBPATH=['.',])
slave = env.Program ('slave.c', LIBS=['drqueue',], LIBPATH=['.',])

#
# Tools
#
jobfinfo = env.Program ('jobfinfo.c', LIBS=['drqueue',], LIBPATH=['.',])
jobinfo = env.Program ('jobinfo.c', LIBS=['drqueue',], LIBPATH=['.',])
compinfo = env.Program ('compinfo.c', LIBS=['drqueue',], LIBPATH=['.',])
requeue = env.Program ('requeue.c', LIBS=['drqueue',], LIBPATH=['.',])
cfgreader = env.Program ('cfgreader.c', LIBS=['drqueue',], LIBPATH=['.',])
cjob = env.Program ('cjob.c', LIBS=['drqueue',], LIBPATH=['.',])
blockhost = env.Program ('blockhost.c', LIBS=['drqueue',], LIBPATH=['.',])
sendjob = env.Program ('sendjob.cpp', LIBS=['drqueue',], LIBPATH=['.',])
