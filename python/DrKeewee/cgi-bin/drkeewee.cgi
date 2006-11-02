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
# This program is distributed in the hope that it will be useful,
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
# This program returns the number of times a frame has been requeued.
# Useful for avoiding endless loops
#
import cgitb
cgitb.enable()

import cgi
import os
import sys
import drqueue
import time

class job:
    def __init__(self,drjob):
        self.drjob = drjob
    def Status(self):
        if self.drjob.status == drqueue.JOBSTATUS_WAITING:
            return "Waiting"
        elif self.drjob.status == drqueue.JOBSTATUS_ACTIVE:
            return "Active"
        elif self.drjob.status == drqueue.JOBSTATUS_STOPPED:
            return "Stopped"
        elif self.drjob.status == drqueue.JOBSTATUS_FINISHED:
            return "Finished"
        else:
            return "Unknown"
    def ShowMinRow (self):
        print '<div id="job" class="job">'
        print '<tr class="jobrow">'
        print '<td>' + str(self.drjob.id)
        print '<td><a href="%s/job/%i">'%(os.environ['SCRIPT_NAME'],self.drjob.id,) + self.drjob.name + '</a>' + '</td>'
        print '<td>' + self.drjob.owner + '</td>'
        print '<td>' + self.Status() + '</td>'
        print '<td>' + str(self.drjob.fleft) + '</td>'
        print '<td>' + str(self.drjob.fdone) + '</td>'
        print '<td>' + str(self.drjob.nprocs) + '</td>'
        print '<td>' + drqueue.time_str(self.drjob.avg_frame_time) + '</td>'
        if self.drjob.nprocs:
            print '<td>' + time.ctime(self.drjob.est_finish_time)  + '</td>'
        else:
            print '<td>Not running</td>'
        print '</tr>'
        print '</div>'
    def ShowJobItem (self,itemname,itemvalue):
        print '<tr class="jobitemrow">'
        print '<td class="jobitemname">' + itemname + ' : </td>'
        print '<td class="jobitemvalue">' + itemvalue + '</td>'
        print '</tr>'
    def Show (self):
        print '<div class="job">'
        print '<h1 class="jobtitle">Job : ' + self.drjob.name + '</h1>'
        print '<table>'
        self.ShowJobItem("Id",str(self.drjob.id))
        self.ShowJobItem("Status",self.Status())
        self.ShowJobItem("Owner",self.drjob.owner)
        self.ShowJobItem("Command",self.drjob.cmd)
        self.ShowJobItem("Priority",str(self.drjob.priority))
        print '</table>'
        self.ShowFrames()
    def ShowFrameMin (self,drframe,index):
        print '<div class="framemin">'
        print '<tr class="framerow">'
        print '<td><a href="%s/job/%i/frame/%i">'%(os.environ['SCRIPT_NAME'],self.drjob.id,drqueue.job_frame_index_to_number(self.drjob,index)) \
              + str(drqueue.job_frame_index_to_number(self.drjob,index)) +'</a></td>'
        print '<td>' + drqueue.job_frame_status_string(self.drframelist[index].status) + '</td>'
        print '<td>' + str(self.drframelist[index].exitcode) + '</td>'
        print '<td><a href="%s/computer/%i">'%(os.environ['SCRIPT_NAME'],self.drframelist[index].icomp,) \
              + str(self.drframelist[index].icomp) + '</a></td>'
        print '<td>' + str(self.drframelist[index].itask) + '</td>'
        print '<td>' + str(self.drframelist[index].requeued) + '</td>'
        if self.drframelist[index].start_time:
            print '<td>' + time.ctime(self.drframelist[index].start_time) + '</td>'
            print '<td>' + time.ctime(self.drframelist[index].end_time) + '</td>'
        else:
            print '<td>Not started</td>'
            print '<td>Not started</td>'
        print '</tr>'
        print '</div>'
    def ShowFrames (self):
        print '<div class="framelisttitle">'
        print 'Frame List'
        print '</div>'
        print '<table class="framelisttable">'
        print '<tr class="framerowtitle"><td>Number</td><td>Status</td><td>Exit code</td><td>Computer</td><td>Task</td><td>Requeued</td>\
        <td>Start time</td><td>Finish time</td></tr>'
        index = 0
        for drframe in self.drframelist:
            self.ShowFrameMin(drframe,index)
            index += 1
        print '</table>'
    def ShowFrame (self,frame):
        index = drqueue.job_frame_number_to_index(self.drjob,frame)
        print '<div class="frametitle">'
        print '<h1 class="jobtitle">Job : ' + self.drjob.name + '</h1>'
        print '<h2>Frame : %i</h2>'%(frame,) 
        print '<h2>Status : ' + drqueue.job_frame_status_string(self.drframelist[index].status) + '</h2>'
        print '<h3>Start Time: '
        if self.drframelist[index].start_time:
            print time.ctime(self.drframelist[index].start_time)
        else:
            print 'Not Started'
        print '</h3>'
        print '<h3>Finish time : '
        if self.drframelist[index].end_time:
            print time.ctime(self.drframelist[index].end_time)
        else:
            print 'Not Started'
        print '</div>'

class joblist:
    def __init__(self):
        try:
            self.drlist = drqueue.request_job_list(drqueue.CLIENT)
        except:
            self.drlist = None
            
    def ShowJobs (self):
        print '<div id="joblisttitle" class="joblisttitle">'
        print 'Job List'
        print '</div>'
        print '<table class="joblisttable">'
        print '<tr class="jobrowtitle"><td>Id</td><td>Name</td><td>Owner</td><td>Status</td><td>Frames left</td><td>Frames done</td>\
        <td>Frames running</td><td>Average frame time</td><td>Estimated end</td></tr>'
        for drjob in self.drlist:
            kwjob = job(drjob)
            kwjob.ShowMinRow()
        print '</table>'
            
    def show (self):
        print '<div id="joblist" class="joblist">'
        if self.drlist == None:
            print '<p class="error">Could not connect to master: %s'%(os.environ['DRQUEUE_MASTER'],)
        else:
            self.ShowJobs()
        print '</div>'

class task:
    def __init__(self,drtask,columns=None):
        self.drtask = drtask
        self.columns = []
        self.column = {}
        if not len(self.columns):
            self.columns = ['jobname','status','frame','ijob','itask','icomp']
        self.fill_column()
    def fill_column(self):
        for name in self.columns:
            self.column[name] = self.attribute_as_column(name)
        return self.column
    def attribute_as_column(self,name):
        if name == 'jobname':
            return '<td class="%s"><a href="%s/job/%i">%s</a></td>'%(name,os.environ['SCRIPT_NAME'],self.drtask.ijob,self.drtask.jobname,)
        elif name == 'ijob':
            return '<td class="%s">%i</td>'%(name,self.drtask.ijob,)
        elif name == 'itask':
            return '<td class="%s">%i</td>'%(name,self.drtask.itask,)
        elif name == 'status':
            return '<td class="%s">%s -> %i</td>'%(name,self.status_string(),self.drtask.status)
        elif name == 'icomp':
            return '<td class="%s"><a href="%s/computer/%i">%i</a></td>'%(name,os.environ['SCRIPT_NAME'],self.drtask.icomp,self.drtask.icomp)
        elif name == 'frame':
            return '<td class="%s"><a href="%s/job/%i/frame/%i">%i</a></td>'%(name,os.environ['SCRIPT_NAME'],self.drtask.ijob,self.drtask.frame,self.drtask.frame)
        else:
            return '<td class="empty">!!Task property unknown!!</td>'
    def as_row_titles(self):
        print '<tr class="taskrowtitle">'
        for title in self.column.keys():
            print '<th class="task-%s">%s</th>'%(title,title)
        print '</tr>'
    def as_row (self):
        print '<tr class="taskrow">'
        for column_name in self.column.keys():
            print self.column[column_name]
        print '</tr>'
    def status_string(self):
        return drqueue.task_status_string(self.drtask.status)
    
    
class tasklist:
    def __init__(self,computer):
        self.tasklist = []
        drtasks = computer.GetDRTasks()
        for drtask in drtasks:
            self.tasklist.append(task(drtask))
    def show_as_table (self):
        print '<table class="tasklisttable">'
        if len(self.tasklist):
            self.tasklist[0].as_row_titles()
        for task_item in self.tasklist:
            task_item.as_row()
        print '</table>'
    def show_as_div_table (self):
        print '<div class="tasklist">'
        self.show_as_table()
        print '</div>'
        
class computer:
    def __init__(self,drcomputer):
        self.drcomputer = drcomputer
        self.tasklist = tasklist(self)
    def GetDRTasks (self):
        tlist = []
        for i in range(drqueue.MAXTASKS):
            drtask = self.drcomputer.status.get_task(i)
            if drtask.used:
                tlist.append(drtask)
        return tlist

    def ShowTasks (self):
        print '<div class="tasklist">'
        print '<p class="tasklisttitle">Tasks</p>'
        self.tasklist.show_as_table()
        print '</div>'
        
    def ShowMinRow (self):
        print '<div id="computer" class="computer">'
        print '<tr class="computerrow">'
        print '<td>' + str(self.drcomputer.hwinfo.id)
        print '<td><a href="%s/computer/%i">'%(os.environ['SCRIPT_NAME'],self.drcomputer.hwinfo.id,) + self.drcomputer.hwinfo.name + '</a>' + '</td>'
        print '<td>'
        if self.drcomputer.limits.enabled:
            print 'Yes'
        else:
            print 'No'
        print '</td>'
        print '<td>' + str(self.drcomputer.status.ntasks) + '</td>'
        print '<td>' + str(self.drcomputer.status.get_loadavg(0)) + ',' \
              + str(self.drcomputer.status.get_loadavg(1)) + ','\
              + str(self.drcomputer.status.get_loadavg(2)) + '</td>'
        print '<td>' + self.Pools() + '</td>'
        print '</tr>'
        print '</div>'

    def OperatingSystem(self):
        if self.drcomputer.hwinfo.os == drqueue.OS_LINUX:
            return "Linux"
        elif self.drcomputer.hwinfo.os == drqueue.OS_IRIX:
            return "Irix"
        elif self.drcomputer.hwinfo.os == drqueue.OS_CYGWIN:
            return "Windows"
        elif self.drcomputer.hwinfo.os == drqueue.OS_OSX:
            return "Mac OS X"
        elif self.drcomputer.hwinfo.os == drqueue.OS_FREEBSD:
            return "FreeBSD"
        else:
            return "Unknown"

    def ShowComputerItem (self,itemname,itemvalue):
        print '<tr class="computeritemrow">'
        print '<td class="computeritemname">' + itemname + ' : </td>'
        print '<td class="computeritemvalue">' + itemvalue + '</td>'
        print '</tr>'

    def Show (self):
        print '<div class="computer">'
        print '<h1 class="computertitle">Computer : ' + self.drcomputer.hwinfo.name + '</h1>'
        print '<table>'
        self.ShowComputerItem("Name",self.drcomputer.hwinfo.name)
        self.ShowComputerItem("Id",str(self.drcomputer.hwinfo.id))
        self.ShowComputerItem("Load Average",str(self.drcomputer.status.get_loadavg(0)))
        self.ShowComputerItem("Tasks running",str(self.drcomputer.status.ntasks))
        self.ShowComputerItem("Operating System",str(self.OperatingSystem()))
        print '</table>'
        self.ShowTasks()
        
    def Pools(self):
        pools = ''
        for i in range (self.drcomputer.limits.npools):
            if i > 0:
                pools += ' , '
            pools += self.drcomputer.limits.get_pool(i).name
        return pools

class computerlist:
    def __init__(self):
        try:
            self.drlist = drqueue.request_computer_list(drqueue.CLIENT)
        except:
            self.drlist = None

    def ShowComputers(self):
        print '<div id="computerlisttitle" class="computerlisttitle">'
        print 'Computer List'
        print '</div>'
        print '<table class="computerlisttable">'
        print '<tr class="computerrowtitle"><td>Id</td><td>Name</td><td>Enabled</td><td>Running</td><td>Load Average</td><td>Pools</td>'
        for drcomputer in self.drlist:
            kwcomputer = computer(drcomputer)
            kwcomputer.ShowMinRow()
        print '</table>'
        
    def show(self):
        print '<div class="computerlist">'
        if self.drlist == None:
            print '<p class="error">Could not connect to master: %s'%(os.environ['DRQUEUE_MASTER'],)
        else:
            self.ShowComputers()
        print '</div>'

class drkeewee:
    """This class implements the top of DrKeewee, the cgi script that
    lets you manage DrQueue through a web server."""
    def showheader(self):
        print '<head><title>DrKeewee</title><link rel="stylesheet" href="/drkeewee.css" type="text/css"/><head>'
    def showtitle(self):
        print '<div class="title">'
        print '<span id="drkeewee"><a href="/">DrKeewee</a></span><div id="drqueue"><a href=http://www.drqueue.org/>DrQueue</a>\'s web service</div>'
        print '</div>'
    def showlists (self):
        self.joblist = joblist()
        self.computerlist = computerlist()
        self.joblist.show()
        self.computerlist.show()
    def ShowJob (self,path):
        try:
            drjob = drqueue.job()
            drqueue.request_job_xfer(int(path[1]),drjob,drqueue.CLIENT)
        except:
            print "Could not retrieve job %s"%(path[1],)
            return
        try:
            drframelist = drjob.request_frame_list(drqueue.CLIENT)
        except:
            print "Could not retrieve job %s frame list"%(path[1],)
            return
        kwjob = job(drjob)
        kwjob.drframelist = drframelist
        if len(path) == 2:
            kwjob.Show()
        elif len(path) == 4 and path[2] == 'frame':
            kwjob.ShowFrame(int(path[3]))

    def ShowComputer (self,path):
        try:
            drcomp = drqueue.computer()
            drqueue.request_comp_xfer(int(path[1]),drcomp,drqueue.CLIENT)
        except:
            print "Could not retrieve computer %s"%(path[1],)
            return
        kwcomp = computer(drcomp)
        kwcomp.Show()

    def showbody(self):
        print '<body>'
        self.showtitle()
        self.path = os.getenv('PATH_INFO','').split('/')[1:]
        if len(self.path) == 0:
            self.showlists()
        elif self.path[0] == 'job':
            self.ShowJob(self.path)
        elif self.path[0] == 'computer':
            self.ShowComputer(self.path)
        print '</body>'
    def show (self):
        print "Content-type: text/html"
        print
        print "<html>"
        self.showheader()
        self.showbody()
        print "</html>"
    
main = drkeewee()
main.show()
