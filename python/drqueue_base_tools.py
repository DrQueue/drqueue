#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''Base helpter tools for installation and configuration of a yet to be built DrQueue module, daemon or tool.'''
#
#
# © Copyright 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
#
# This file is part of DrQueue
#
# DrQueue is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# DrQueue is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# $Id$
#

import string,os
import re

class version:
    '''Handles version numbers and strings based on the C header file'''
    keywords = {'VERSION_MAJOR':0,'VERSION_MINOR':1,'VERSION_PATCH':2,'VERSION_POST':3,'VERSION_PRE':4}
    def __init__(self,path=os.path.join('..','libdrqueue','common.h')):
        self.path=path
        try:
            file_object=open(self.path)
        except IOError, e:
            print "Could not open file '%s': %s"%(self.path,e)
        self.lines=file_object.read().splitlines()
        file_object.close()
        for line in self.lines:
            words = line.split()
            if len(words) != 3:
                continue
            keyword = words[1]
            val = words[2]
            if keyword not in self.keywords.keys():
                continue
            else:
                if self.keywords[keyword] == 0:
                    self.major=val
                if self.keywords[keyword] == 1:
                    self.minor=val
                if self.keywords[keyword] == 2:
                    self.patch=val
                if self.keywords[keyword] == 3:
                    self.post=val
                if self.keywords[keyword] == 4:
                    self.pre=val

                    
    def get_prepost_char(self):
        if self.pre != '0':
            return 'c'
        elif self.post != '0':
            return 'p'
        else:
            return ''

    def get_prepost_number(self):
        if self.pre != '0':
            return self.pre
        elif self.post != '0':
            return self.post
        else:
            return ''

    def __str__(self):
        base = "%s.%s.%s"%(self.major,self.minor,self.patch)
        if self.pre or self.post:
            base = "%s%s%s"%(base,self.get_prepost_char(),self.get_prepost_number())
        return base

    def __repr__(self):
        return self.__str__()

if __name__=='__main__':
    v = version()
    print "Version read: %s"%(v)
    
        

