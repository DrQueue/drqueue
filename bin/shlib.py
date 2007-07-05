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
#
# -*- coding: UTF-8 -*-
#

import sys,platform,os,re,signal
from optparse import OptionParser

class shhelper:
    global_pid = 0
    def __init__(self,basetool=os.path.split(sys.argv[0])[1],escape=False,underscore=True):
        self.basetool=basetool
        self.machine=self.machine(escape,underscore)
        self.kernel=self.kernel(escape,underscore)
        self.os_info=self.os_info(escape,underscore)

    def report(self):
        print "Basetool: %s"%(self.basetool)
        print "Machine: %s"%(self.machine)
        print "Kernel:  %s"%(self.kernel)
        if self.os_info['kernel'] == 'Linux':
            print "Linux distro: %s version %s"%(self.os_info['distro'][0],self.os_info['distro'][1])
        elif self.os_info['kernel'] == 'Darwin':
            print "Darwin version: %s running on %s"%(self.os_info['mac_ver'][0],self.os_info['mac_ver'][2])
        else:
            print "OS Information not available"
            
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

    def os_info(self,escape,underscore):
        """If available will store the distribution specific details into 'distro' ('Fedora','7','Moonshine')"""
        result=[]
        if self.kernel == 'Linux':
            distro = platform.dist()
            result = { 'kernel':'Linux', 'distro':distro }
        elif self.kernel == 'Darwin':
            distro = platform.mac_ver()
            result = { 'kernel':'Darwin', 'mac_ver':distro }
        else:
            result = { 'kernel':'Unknown kernel' }
        return result

    def get_full_exec(self,basetool):
        try:
            basepath = os.environ['DRQUEUE_ROOT']
        except:
            print u"ERROR: DRQUEUE_ROOT environment variable not set. Exiting."
            sys.exit(1)
        basepath = os.path.join(basepath,'bin')
        basename = "%s.%s.%s"%(basetool,self.kernel,self.machine)
        return os.path.join(basepath,basename)

    def run_with_args(self,basetool,options,args):
        exec_name = self.get_full_exec(basetool)
        if options.daemon:
            print "Running as daemon"
            os.close(0)
            os.close(1)
            os.close(2)
            signal.signal(SIGHUP,SIG_IGN)
        try:
            print u"Spawning process: %s with args %s"%(exec_name,args)
            global_pid = os.spawnvp(os.P_NOWAIT,exec_name,args)
        except:
            print u"Daemon Interrumpted"
            global_pid = 0
        else:
            print u"Daemon pid: %i"%(global_pid,)
        signal.signal(signal.SIGINT,kill_daemon)
        signal.signal(signal.SIGCLD,daemon_finished)
        if options.filename:
            try:
                pid_file = open (options.filename,'w+')
            except:
                print u"ERROR: Could not create pid file on: '%s'"%(options.filename,)
                os.kill(self.pid,signal.SIGINT)
                sys.exit(1)
            pid_file.write("%i"%(global_pid,))
            pid_file.close()

    def wait_forever(self,basetool,options,args):
        (old_pid,status) = os.wait()
        while old_pid:
            print "Print process %i arrived with status %i, starting over..."%(old_pid,status)
            self.run_with_args(basetool,options,args)

def kill_daemon(signal,stack):
    os.kill(global_pid,signal.SIGINT)
    # Leaving 15 seconds to die cleanly
    os.alarm(15)
    signal.signal(SIGALARM,kill_daemon_kill)

def kill_daemon_kill(signal,stack):
    os.kill(global_pid,signal.SIGKILL)

def daemon_finished(signal,stack):
    print "Daemon exited for unknown reasons"
    sys.exit(1)

    
def main(basetool):
    helper = shhelper()
    parser = OptionParser()
    parser.add_option("-p","--pid",dest="filename",help="Writes the pid of the main daemon into FILE",metavar="FILE")
    parser.add_option("-d","--daemon",action="store_const", const=1, dest="daemon", help="Runs the daemon in the background")
    (options,args) = parser.parse_args()
    helper.run_with_args(basetool,options,args)
    helper.wait_forever(basetool,options,args)

if __name__ == '__main__':
    main(sys.argv[1])
