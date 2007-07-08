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
# -*- coding: UTF-8 -*-
#

class EnvironmentVarioble:
    """Variables defined into jobscripts, both custom and autoupdated"""
    def __init__(self,name,value,format="%s",update=True,update_callback=None):
        self.name=name
        self.format=format
        self.value=value
        self.update=update
        self.update_callback=update_callback

    def __str__(self):
        value="%%"%(self.format,self.value)
        complete="%s=%s"%(self.name,value)
        return complete
