#
# Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
#
# This file is part of DrQueue
#
# DrQueue is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# DrQueue is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
# USA
#
# $Id$
#

import sys
import glob
import os
import platform

def get_platform_name():
    name = sys.platform
    if name == 'win32':
       return 'cygwin'
    return name

def get_abspath_glob(path):
    pathlist=glob.glob(path)
    rlist=[]
    for file in pathlist:
      rlist.append(os.path.abspath(file))
    return rlist

# Construction environment for the library (doesn't link with itself)
env_lib = Environment ()
if sys.platform == 'win32':
    print "-> Win32 using Cygwin mode"
    Tool('mingw')(env_lib)

env_lib.Append (CPPPATH=['.','libdrqueue'])
env_lib.Append (CPPDEFINES = Split ('-DCOMM_REPORT -D_GNU_SOURCE ' \
                + '-D_NO_COMPUTER_POOL_SEMAPHORES -D_NO_COMPUTER_SEMAPHORES'),
                CPPFLAGS = Split ('-g -O0'),
                CXXFLAGS = ['-D__CPLUSPLUS',Split(env_lib.subst('$CCFLAGS')),
                            Split(env_lib.subst('$CPPDEFINES'))])

print "Platform is: ",sys.platform
if sys.platform == "linux2":
  env_lib.Append (CPPDEFINES = Split ('-D__LINUX'))
elif sys.platform == "darwin":
  env_lib.Append (CPPDEFINES = Split ('-D__OSX'))
elif sys.platform == "irix6":
  env_lib.Append (CPPDEFINES = Split ('-D__IRIX'))
  env_lib['CC'] = 'c99'
elif sys.platform == "cygwin":
  env_lib.Append (CPPDEFINES = Split ('-D__CYGWIN'))
  os.environ['PKG_CONFIG_PATH'] = 'C:\GTK\lib\pkgconfig'
elif sys.platform == "win32":
  env_lib.Append (CPPDEFINES = Split ('-D__CYGWIN'))
else:
  print "Unknown platform: %s"%(sys.platform,)
  exit (1)

# Base construction environment that links with the library
env = env_lib.Copy()
env.Append (LIBS = ['drqueue'],LIBPATH = ['libdrqueue'])

#
# libdrqueue.a
#
libdrqueue_src = get_abspath_glob(os.path.join('libdrqueue','*.c'))
libdrqueue = env_lib.Library ('libdrqueue/drqueue', libdrqueue_src)
Default (libdrqueue)

#
# Main
#
master = env.Program ('master.c')
Default (master)
slave = env.Program ('slave.c')
Default (slave)

## Build python modules
#Export ('env','env_lib','libdrqueue','libdrqueue_src')
#SConscript(['python/SConscript'])

#
# drqman
#
drqman_c = glob.glob (os.path.join('drqman','*.c'))
env_gtkstuff = env.Copy ()
env_gtkstuff.ParseConfig ('pkg-config --cflags --libs gtk+-2.0')
drqman = env_gtkstuff.Program ('drqman/drqman',drqman_c)
Default (drqman)

#
# Tools
#
cmdline_tools = [ 'jobfinfo','jobinfo','compinfo','requeue','cfgreader',
                  'cjob','blockhost','sendjob' ]
ctools = {}
for tool in cmdline_tools:
    print "Tool: %s"%(tool,)
    if tool != 'sendjob':
        ctools[tool] = env.Program (tool+'.c')
    else:
        ctools[tool ]= env.Program (tool+'.cpp')
    Default(ctools[tool])

