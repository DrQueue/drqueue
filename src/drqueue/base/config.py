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
# $Id: /drqueue/remote/trunk/jobfinfo.c 2334 2005-07-05T03:50:01.502018Z jorge  $
#
# -*- coding: UTF-8 -*-
#

from ConfigParser import SafeConfigParser
import os

class Config(SafeConfigParser):
    def __init__(self,defaults=None):
        SafeConfigParser.__init__(self,defaults)
        
if __name__ == '__main__':
    cfg = Config()
    cfg.read('test.cfg')
    for (i,v) in cfg.items('master'):
        print "I: %s V: %s"%(i,v)
        
        
        
