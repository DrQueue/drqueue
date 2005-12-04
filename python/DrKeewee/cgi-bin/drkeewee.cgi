#!/usr/bin/env python

import cgitb
cgitb.enable()

import cgi
import os
import sys
sys.path.insert(0,'..')
import drqueue

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
        print '<td><a href="%s/job/%i">'%(os.environ['SCRIPT_NAME'],self.drjob.id,) + self.drjob.name + '</a>'
        print '<td>' + self.Status()
        print '<td>' + str(self.drjob.fleft)
        print '<td>' + str(self.drjob.fdone)
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
        print '<td><a href="%s/job/%i/frame/%i">'%(os.environ['SCRIPT_NAME'],self.drjob.id,index) \
              + str(drqueue.job_frame_index_to_number(self.drjob,index)) +'</a></td>'
        print '<td>' + drqueue.job_frame_status_string(self.drframelist[index].status) + '</td>'
        print '<td>' + str(self.drframelist[index].exitcode) + '</td>'
        print '<td><a href="%s/computer/%i">'%(os.environ['SCRIPT_NAME'],self.drframelist[index].icomp,) \
              + str(self.drframelist[index].icomp) + '</a></td>'
        print '<td>' + str(self.drframelist[index].itask) + '</td>'
        print '<td>' + str(self.drframelist[index].requeued) + '</td>'
        print '</tr>'
        print '</div>'
    def ShowFrames (self):
        print '<div class="framelisttitle">'
        print 'Frame List'
        print '</div>'
        print '<table>'
        print '<tr class="framerowtitle"><td>Number</td><td>Status</td><td>Exit code</td><td>Computer</td><td>Task</td><td>Requeued</td></tr>'
        index = 0
        for drframe in self.drframelist:
            self.ShowFrameMin(drframe,index)
            index += 1
        print '</table>'
    def ShowFrame (self,index):
        print '<div class="frametitle">'
        print '<h1 class="jobtitle")>Job : ' + self.drjob.name
        print '<h2>Frame : ' + str(drqueue.job_frame_index_to_number(self.drjob,index))
        print '<h2>Status : ' + drqueue.job_frame_status_string(self.drframelist[index].status)
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
        print '<table>'
        print '<tr class="jobrowtitle"><td>Id</td><td>Name</td><td>Status</td><td>Frames left</td><td>Frames done</td></tr>'
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



class computerlist:
    def __init__(self):
        pass
    def show(self):
        print '<div id="computerlist" class="joblist">'
        print '</div>'

class drkeewee:
    """This class implements the top of DrKeewee, the cgi script that
    lets you manage DrQueue through a web server."""
    def showheader(self):
        print '<head><title>DrKeewee</title><link rel="stylesheet" href="/drkeewee.css" type="text/css"/><head>'
    def showtitle(self):
        print '<div class="title">'
        print '<h1>DrKeewee - <a href=http://www.drqueue.org/>DrQueue</a>\'s web service</h1>'
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
