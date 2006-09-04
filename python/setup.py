#!/usr/bin/env python

import os
import glob
import sys
import distutils
from distutils.core import setup, Extension



#if sys.platform == "linux2":
#	env.Append (CCFLAGS = '-D__LINUX')
#	env.Append (CPPPATH=['/usr/include/python2.4',"..","../libdrqueue"])
#	swigflags = swigflags + " -D__LINUX"
#elif sys.platform == "darwin":
#	env.Append (CCFLAGS = Split('-D__OSX -fno-common'))
#	env.Append (CPPPATH=['/sw/include/python2.4',"..","../libdrqueue"])
#	env.Append (LIBS=['python2.4'],LIBPATH=['/sw/lib/python2.4/config'])
#	swigflags = swigflags + " -D__OSX"


def get_define_macros():
  print "Platform is: ",sys.platform
  l_define_macros=[('COMM_REPORT',None),('_GNU_SOURCE',None)]
  if sys.platform == "linux2":
    l_define_macros = l_define_macros + [('__LINUX',None)]
  elif sys.platform == "darwin":
    l_define_macros = l_define_macros + [('__OSX',None)]
  return l_define_macros
  
def get_swig_flags():
  swigflags = ['-I..','-I../libdrqueue']
  if sys.platform == "linux2":
    swigflags = swigflags + ['-D__LINUX']
  elif sys.platform == "darwin":
    swigflags = swigflags + ['-D__OSX']
  return swigflags

setup (name='drqueue',
       version='0.1',
       description='DrQueue Python modules',
       author='Jorge Daza',
       author_email='jorge@drqueue.org',
       url='http://drqueue.org/',
       ext_modules=[Extension('drqueue',
                              define_macros=get_define_macros(),
                              include_dirs=['..',os.path.join('..','libdrqueue/')],
                              swig_opts=get_swig_flags(),            
                              sources=glob.glob(os.path.join('..','libdrqueue','*.c')) + ['drqueue.i'])])
