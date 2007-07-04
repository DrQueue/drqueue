#!/usr/bin/env python
#
# Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
# USA
#
# $Id: /drqueue/remote/branches/0.65.x/jobinfo.c 1754 2007-01-27T05:35:05.735857Z jorge  $
#

import sys,platform,os,re

class shhelper:
    def __init__(self,basetool=os.path.split(sys.argv[0])[1],escape=False,underscore=True):
        self.basetool=basetool
        self.machine=self.machine(escape,underscore)
        self.kernel=self.kernel(escape,underscore)
        self.distro=self.distro(escape,underscore)

    def report(self):
        print "Basetool: %s"%(self.basetool)
        print "Machine: %s"%(self.machine)
        print "Kernel:  %s"%(self.kernel)
        print "Distro:  %s version %s"%(self.distro[0],self.distro[1])

    def machine(self,escape,underscore):
        machine = platform.machine()
        if underscore:
            machine = re.sub('\s','_',machine)
        if escape:
            machine = re.escape(machine)
        return machine

    def kernel(self,escape,underscore):
        kernel = platform.system()
        if underscore:
            kernel = re.sub('\s','_',kernel)
        if escape:
            kernel = re.escape(kernel)
        return kernel

    def distro(self,escape,underscore):
        """If available will store the distribution specific details into 'distro' ('Fedora','7','Moonshine')"""
        distro = platform.dist()
        return distro


if __name__ == '__main__':
    helper = shhelper()
    helper.report()
