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

from string import Template

class FormattedVariable():
    """Variables defined into jobscripts, both custom and autoupdated"""

    def __init__(self,name,value,format="%s",format_kws={},update=False,
                 update_callback=None,padding=0):
        self.name=name
        self.value=value
        self.update=update
        self.update_callback=update_callback
        self.padding=padding
        self.format=format
        self.format_kws=format_kws
        
    def __str__(self):
        result = "%s=%s"%(self._get_format_name(),self._get_format_value())
        return result

    def _get_format_name(self):
        self.format_name = Template("$name").substitute(name=self.name)
        return self.format_name

    def _get_format_value(self):
        format=self._get_format()
        self.format_value = Template("$format").substitute(format=format)%(self.value)
        return self.format_value

    def _get_format(self):
        """Using the given base format string, applies the substitutions according to format_kws"""
        format = Template(self.format)
        return format.substitute(**self.format_kws)

    @property
    def format_name():
        "Returns the formated variable name"
        def fget(self):
            return self._get_format_name()
    
    @property
    def format_value():
        "Returns the formated variable value"
        def fget(self):
            return self._get_format_value()

if __name__ == '__main__':
    e = FormattedVariable('var1',2)
    e.format='%0${padding}i'
    e.format_kws={'padding':6}
    print e
    print e.format_name
    print e.format_value
