#!/usr/bin/env python
# 
# Copyright (C) 2001,2002,2003,2004,2005,2006 Jorge Daza Garcia-Blanes
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

from ez_setup import use_setuptools
use_setuptools()

import os
import glob
import sys
import setuptools
from setuptools import setup, find_packages, Extension
import platform

def get_wordsize_flags():
  flagPrefix = '-Xcompiler'
  arch=platform.machine()
  if arch == 'i386':
    os.environ['CFLAGS'] = '-m32 -march=i386'
    bitsFlag = [ flagPrefix + ' -m32', flagPrefix + ' -march=i386']
  elif arch == 'i686':
    os.environ['CFLAGS'] = '-m32 -march=i686'
    bitsFlag = [ flagPrefix + ' -m32', flagPrefix + ' -march=i686']
  elif arch == 'x86_64':
    os.environ['CFLAGS'] = '-m64 -march=athlon64'
    bitsFlag = [ flagPrefix + ' -m64', flagPrefix + ' -march=athlon64']
  elif arch == 'Power Macintosh':
    os.environ['CFLAGS'] = '-mpowerpc -mtune=powerpc'
    bitsFlag = [ flagPrefix + ' -mpowerpc', flagPrefix + ' -mtune=powerpc']
  else:
    print "Machine not listed: %s"%(arch,)
    sys.exit(1)
  return bitsFlag


def get_define_macros():
  get_wordsize_flags()
  print "Platform is: ",sys.platform
  l_define_macros=[('COMM_REPORT',None),('_GNU_SOURCE',None),('_NO_COMPUTER_SEMAPHORES',None),('_NO_COMPUTER_POOL_SEMAPHORES',None)]
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

setup(
    name = "drqueue",
    version = "0.64.2c0",
    packages = find_packages(),

    ext_modules=[Extension('_drqueue', ['drqueue.i'] + get_abspath_glob(os.path.join('..','libdrqueue','*.c')),
      define_macros=get_define_macros(),
      include_dirs=[get_abspath('..'),get_abspath(os.path.join('..','libdrqueue'))],
      swig_opts=get_swig_flags())],

    py_modules = ['drqueue',],

    # Project uses reStructuredText, so ensure that the docutils get
    # installed or upgraded on the target machine
    #install_requires = ['docutils>=0.3'],

    package_data = {
        # If any package contains *.txt or *.rst files, include them:
        '': ['*.txt', '*.rst'],
    },

    # metadata for upload to PyPI
    author = "Jorge Daza",
    author_email = "jorge@drqueue.org",
    description = "DrQueue bindings with Python",
    license = "GPL General Public License version 2",
    url = "http://drqueue.org/",

    # could also include long_description, download_url, classifiers, etc.
)

