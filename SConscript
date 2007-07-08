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

Import ('env','env_lib','libdrqueue','libdrqueue_src')
import distutils.sysconfig,os,sys,glob
env_lib.Append(CPPPATH=[distutils.sysconfig.get_python_inc()] +
                       ['../libdrqueue','./libdrqueue'])

vars = distutils.sysconfig.get_config_vars('CC', 'CXX', 'OPT', 
                'BASECFLAGS', 'CCSHARED', 'LDSHARED', 'SO')
for i in range(len(vars)):
    if vars[i] is None:
        vars[i] = ""

def customize_vars (previous=vars):
    for var in previous:
        if var == "BASECFLAGS":
            var += custom_definitions(global_definitions)
            print "New BASECFLAGS : " + var
    return previous
   
(cc, cxx, opt, basecflags, ccshared, ldshared, so_ext) = customize_vars(vars)
env_lib.Append (SWIGFLAGS=Split('-python -Ilibdrqueue') + 
                                env_lib['CPPDEFINES'] )
libpy = env_lib.SharedLibrary("drqueue",
                    [os.path.abspath('./src/drqueue/base/libdrqueue.i')] + 
                    libdrqueue_src,
                    #LIBPATH=[distutils.sysconfig.get_python_lib()],
                    #LIBS=['python'+distutils.sysconfig.get_python_version()],
                    #SHLINK=ldshared,
                    #SHLINKFLAGS=[],
                    SHLIBPREFIX="_",
                    SHLIBSUFFIX=so_ext)
Default(libpy)
