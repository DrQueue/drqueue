import sys

env = Environment (CC = 'gcc', 
										CCFLAGS=Split ('-DCOMM_REPORT -Wall -g -O2'))


print "Platform is: ",sys.platform
if sys.platform == "linux2":
	env.Append (CCFLAGS = '-D__LINUX')

env_c = env.Copy ()
env_cpp = env.Copy ()
env_cpp.Append (CCFLAGS='-D__CPLUSPLUS')

libdrqueue = Split ("""computer_info.c computer_status.c task.c logger.c communications.c
      computer.c request.c semaphores.c job.c drerrno.c database.c common.c
      mayasg.c blendersg.c bmrtsg.c pixiesg.c""")
env_c.Library ('libdrqueue.a', libdrqueue)

env_c.Program ('master.c', LIBS=['libdrqueue.a'], LIBPATH=['.'])
env_c.Program ('slave.c', LIBS=['libdrqueue.a'], LIBPATH=['.'])

env_cpp.Program ('sendjob.cpp', LIBS=['libdrqueue.a'], LIBPATH=['.'])
