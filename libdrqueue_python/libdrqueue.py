#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''type info here'''
#
# Copyright (C) 2010 Andreas Schroeder
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

# might use: pointer, slavedb, computer, job, task, logger, communications, request, drerrno, database, semaphores, common, envvars, jobscript, config, list, computer_pool

# might also use: generalsg, mayasg, mentalraysg, blendersg, pixiesg, 3delightsg, lightwavesg, aftereffectssg, shakesg, terragensg, nukesg, aqsissg, mantrasg, turtlesg, xsisg, cinema4dsg, luxrendersg, vraysg, 3dsmaxsg

import common, communications
from computer import *
from task import *

##if defined __CYGWIN32__ && !defined __CYGWIN__
#/* For backwards compatibility with Cygwin b19 and
# *       earlier, we define __CYGWIN__ here, so that
# *             we can rely on checking just for that macro. */
##  define __CYGWIN__  __CYGWIN32__
##endif

##if defined _WIN32 && !defined __CYGWIN__
#/* Use Windows separators on all _WIN32 defining
# *       environments, except Cygwin. */
##  define DIR_SEPARATOR_CHAR        '\\'
##  define DIR_SEPARATOR_STR         "\\"
##  define PATH_SEPARATOR_CHAR       ';'
##  define PATH_SEPARATOR_STR        ";"
##endif
##ifndef DIR_SEPARATOR_CHAR
#/* Assume that not having this is an indicator that all
# *       are missing. */
##  define DIR_SEPARATOR_CHAR        '/'
##  define DIR_SEPARATOR_STR     "/"
##  define PATH_SEPARATOR_CHAR       ':'
##  define PATH_SEPARATOR_STR        ":"
##endif /* !DIR_SEPARATOR_CHAR */


#extern int drerrno;
#extern int phantom[2];



