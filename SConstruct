import sys
import glob
import os

env = Environment (CC = 'gcc', 
										CCFLAGS=Split ('-DCOMM_REPORT -Wall -g -O2 -I.'))


print "Platform is: ",sys.platform
if sys.platform == "linux2":
	env.Append (CCFLAGS = '-D__LINUX')
elif sys.platform == "darwin":
	env.Append (CCFLAGS = '-D__OSX')

env.Append (CXXFLAGS='-D__CPLUSPLUS')

libdrqueue = Split ("""computer_info.c computer_status.c task.c logger.c communications.c
      computer.c request.c semaphores.c job.c drerrno.c database.c common.c
      mayasg.c blendersg.c bmrtsg.c pixiesg.c""")
env.Library ('libdrqueue.a', libdrqueue)

drqman_c = glob.glob ("drqman/*.c")
gtkcflags = os.popen('pkg-config --cflags gtk+-2.0').read()
gtklibs = os.popen('pkg-config --libs gtk+-2.0').read()
env_gtkstuff = env.Copy ()
env_gtkstuff.Append (CCFLAGS = Split(gtkcflags))
env_gtkstuff.Append (LINKFLAGS = Split(gtklibs))
drqman = env_gtkstuff.Program ('drqman/drqman',drqman_c, LIBS=['libdrqueue.a'], LIBPATH=['.'])

env.Program ('master.c', LIBS=['libdrqueue.a'], LIBPATH=['.'])
env.Program ('slave.c', LIBS=['libdrqueue.a'], LIBPATH=['.'])
env.Program ('jobfinfo.c', LIBS=['libdrqueue.a'], LIBPATH=['.'])
env.Program ('requeue.c', LIBS=['libdrqueue.a'], LIBPATH=['.'])
env.Program ('cjob.c', LIBS=['libdrqueue.a'], LIBPATH=['.'])
env.Program ('blockhost.c', LIBS=['libdrqueue.a'], LIBPATH=['.'])

env.Program ('sendjob.cpp', LIBS=['libdrqueue.a'], LIBPATH=['.'])
