# vim:ft=python

import sys
import glob
import os
import platform

env = Environment (arch=platform.machine(),)
env.Append (CPPPATH=['.','./libdrqueue'])

print "Architecture: %s"%(env['arch'],)
if env['arch'] == 'i386':
  bitsFlag='-m32'
  env.Append (CCFLAGS = Split('-march=i386'),CXXFLAGS = env.subst('$CCFLAGS'))
elif env['arch'] == 'i686':
  bitsFlag='-m32'
  env.Append (CCFLAGS = Split('-march=i686'),CXXFLAGS = env.subst('$CCFLAGS'))
elif env['arch'] == 'x86_64':
  bitsFlag='-m64'
  env.Append (CCFLAGS = Split('-march=athlon64'),CXXFLAGS = env.subst('$CCFLAGS'))
elif env['arch'] == 'Power Macintosh':
  bitsFlag='-mpowerpc'
  env.Append (CCFLAGS = Split('-mtune=powerpc'))

#dict = env.Dictionary()
#keys = dict.keys()
#keys.sort()
#for key in keys:
#  print "construction variable = '%s', value = '%s'" % (key, dict[key])
   
env.Append (CCFLAGS = Split ('-DCOMM_REPORT -D_GNU_SOURCE -D_NO_COMPUTER_POOL_SEMAPHORES -D_NO_COMPUTER_SEMAPHORES -g -O0'),
            CXXFLAGS = ['-D__CPLUSPLUS',env.subst('$CCFLAGS')])
env.Append (CCFLAGS = [bitsFlag,])

print "Platform is: ",sys.platform
if sys.platform == "linux2":
	env.Append (CCFLAGS = Split ('-D__LINUX'))
  #if coreduo
  #env.Append(CCFLAGS = Split('-march=nocona -pipe -O2'), CXXFLAGS=Split('-march=nocona -O2 -pipe'))
elif sys.platform == "darwin":
	#env.Append (CCFLAGS = Split ('-D__OSX -no-cpp-precomp -fstrict-aliasing -mno-fused-madd'))
	env.Append (CCFLAGS = Split ('-D__OSX'))
else:
	print "Unknown platform: %s"%(sys.platform,)
	exit (1)

#
# libdrqueue.a
#
libdrqueue_src = glob.glob(os.path.join('.','libdrqueue','*.c'))
libdrqueue = env.StaticLibrary ('drqueue', libdrqueue_src)

#
# drqman
#
drqman_c = glob.glob (os.path.join('.','drqman','*.c'))
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
