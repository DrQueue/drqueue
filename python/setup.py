#!/usr/bin/env python
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA	 02111-1307
# USA
# 
# $Id$
#

from setuptools import setup

import os
import glob
import sys
from setuptools import setup, find_packages, Extension
import platform
from distutils.core import setup, Extension
import distutils.sysconfig

def get_wordsize_flags():
  flagPrefix = '-Xcompiler'
  linkPrefix = '-Xlinker'
  arch=platform.machine()
  bitsFlag = []
  if arch == 'x86_64':
    os.environ['CFLAGS'] = '-m64 -march=x86-64 -pipe'
    bitsFlag = [ flagPrefix + ' -m64', flagPrefix + ' -march=x86_64']
  elif arch == 'Power Macintosh':
    os.environ['CFLAGS'] = '-mpowerpc -mtune=powerpc'
    bitsFlag = [ flagPrefix + ' -mpowerpc', flagPrefix + ' -mtune=powerpc']
  elif arch == 'ppc':
    # Linux ppc
    # let's set no flags for now
    bitsFlag = []
  else:
    if sys.platform == 'cygwin' or sys.platform == 'win32':
      print("Win32 Environment: %s"%(sys.platform,))
      #bitsFlag = [ flagPrefix + '-mdll',
      #             linkPrefix + '--no-undefined',
      #             linkPrefix + '--enable-runtime-pseudo-reloc' ]
  return bitsFlag

def get_define_macros():
  get_wordsize_flags()
  print("Platform is: ",sys.platform)
  l_define_macros=[('COMM_REPORT',None),('_GNU_SOURCE',None),
                   ('_NO_COMPUTER_POOL_SEMAPHORES',None),
                   ('_NO_COMPUTER_SEMAPHORES',None)]
  if sys.platform == "linux2":
    l_define_macros = l_define_macros + [('__LINUX',None)]
  elif sys.platform == "darwin":
    l_define_macros = l_define_macros + [('__OSX',None)]
  elif sys.platform == "cygwin" or sys.platform == "win32":
    l_define_macros = l_define_macros + [('__CYGWIN',None)]
  return l_define_macros

def get_abspath(path):
    newpath=os.path.abspath(path)
    return newpath

def get_abspath_glob(path):
    pathlist=glob.glob(path)
    rlist=[]
    for file in pathlist:
      rlist.append(get_abspath(file))
    return rlist
  
def get_swig_flags():
    swigflags = ['-I'+get_abspath('..'),
                 #'-I'+get_abspath(os.path.join('..','libdrqueue')),
                 '-I'+get_abspath(os.path.join('libdrqueue'))]
    if sys.platform == "linux2":
      swigflags = swigflags + ['-D__LINUX']
    elif sys.platform == "darwin":
      swigflags = swigflags + ['-D__OSX']
    elif sys.platform == "cygwin" or sys.platform == 'win32':
      swigflags = swigflags + ['-D__CYGWIN']
    return swigflags

def custom_win32_include_paths():
    paths = []
    paths += distutils.sysconfig.get_python_inc()
    return paths

def custom_include_paths():
    paths = []
    if sys.platform == 'win32':
        #paths += custom_win32_include_paths()
        # faked for cygwin
        paths += [get_abspath(os.path.join('/','cygwin','usr',
                              'include','python2.5'))]
    return paths

def custom_library_paths():
    paths = []
    if sys.platform == 'win32':
        paths += [get_abspath(os.path.join('/','cygwin','usr','lib'))]
        paths += [get_abspath(os.path.join('/','cygwin','lib','mingw'))]
    return paths

setup(
    name = "PyDrQueue",
    # TODO: get version from C header
    version = "0.64.4",
    # metadata for upload to PyPI
    # could also include long_description, download_url, classifiers, etc.
    author = "Jorge Daza",
    author_email = "jorge@drqueue.org",
    description = "DrQueue bindings with Python",
    license = "GPL General Public License version 2",
    url = "http://drqueue.org/",

    ext_modules=[Extension('drqueue.base._libdrqueue', 
                 ['src/drqueue/base/libdrqueue.i'] \
                 #+ get_abspath_glob(os.path.join('..','libdrqueue','*.c')) \
                 + get_abspath_glob(os.path.join('libdrqueue','*.c')),
                 libraries = [ 'python'+distutils.sysconfig.get_python_version() ],
                 library_dirs= [ distutils.sysconfig.get_python_lib() ] + custom_library_paths(),
                 define_macros=get_define_macros(),
                 include_dirs=[get_abspath('..'),
                               #get_abspath(os.path.join('..','libdrqueue')),  
                               get_abspath('libdrqueue')] +
                               custom_include_paths(),
                 swig_opts=get_swig_flags()),],
    #packages = find_packages(exclude='ez_setup.py'),
    packages = [ 'drqueue', 'drqueue.base' ],
    package_dir = { '' : 'src' },
    #eager_resources = [ 'base','_base' ],
    #zip_safe = False,
)


