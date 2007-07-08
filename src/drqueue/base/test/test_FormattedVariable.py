"""This module tests classes and functions defined in FormatedVariable.py"""
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

import sys
sys.path.insert(1,'.')
from FormattedVariable import *
import unittest

class FormattedVariableTest(unittest.TestCase):

    def setUp(self):
        self.fvl = []
        # With padding, integers
        for i in range(5):
            self.fvl.append(FormattedVariable('Var%i'%(i,),i,format="%0${padding}i",format_kws={'padding':i*2}))
        # Strings
        for i in range(10)[5:]:
            self.fvl.append(FormattedVariable('Var%i'%(i,),"VarValue%i"%(i*2),format="%s"))

    def teststringrepr(self):
        i = 0
        for fv in self.fvl:
            if i < 5:
                self.assertEqual('%s'%fv,'Var%i=%0*i'%(i,i*2,i))
            else:
                self.assertEqual('%s'%fv,'Var%i=VarValue%i'%(i,i*2))
            i +=1

    def testvalueformatting(self):
        i = 0
        for fv in self.fvl:
            fv.format="${subst1}/${subst2}/%10s"
            fv.format_kws={'subst1':i,'subst2':i**2}
            fv.value=i**3
            self.assertEqual('%s'%fv,'Var%i=%i/%i/%10s'%(i,i,i**2,i**3))
            i += 1

if __name__ == '__main__':
    unittest.main()
