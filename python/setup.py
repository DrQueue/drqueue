#!/usr/bin/env python

import os
import glob
import sys
import distutils
from distutils.core import setup, Extension

def get_define_macros():
  print "Platform is: ",sys.platform
  l_define_macros=[('COMM_REPORT',None),('_GNU_SOURCE',None)]
  if sys.platform == "linux2":
    l_define_macros = l_define_macros + [('__LINUX',None)]
  elif sys.platform == "darwin":
    l_define_macros = l_define_macros + [('__OSX',None)]
  return l_define_macros

def get_abspath(path):
  #print "Given path: %s"%(path,)
  newpath=os.path.abspath(path)
  #print "Absolute path: %s"%(newpath)
  return newpath

def get_abspath_glob(path):
  pathlist=glob.glob(path)
  rlist=[]
  for file in pathlist:
    rlist.append(get_abspath(file))
  return rlist
  
def get_swig_flags():
  swigflags = ['-I'+get_abspath('..'),'-I'+get_abspath(os.path.join('..','libdrqueue'))]
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
       license='GPL General Public License version 2',
       url='http://drqueue.org/',
       ext_modules=[Extension('_drqueue', ['drqueue.i'] + get_abspath_glob(os.path.join('..','libdrqueue','*.c')),
                              define_macros=get_define_macros(),
                              include_dirs=[get_abspath('..'),get_abspath(os.path.join('..','libdrqueue'))],
                              swig_opts=get_swig_flags())],
       py_modules=['drqueue'])

