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
    def status(self):
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
        print '<td>' + self.status()
        print '<td>' + str(self.drjob.fleft)
        print '<td>' + str(self.drjob.fdone)
        print '</tr>'
        print '</div>'
    def ShowItem (self,itemname,itemvalue):
        print '<tr class="jobitemrow">'
        print '<td class="jobitemname">' + itemname + ':</td>'
        print '<td class="jobitemvalue">' + itemvalue + '</td>'
        print '</tr>'
    def show (self):
        print '<div class="job">'
        print '<h1 class="jobtitle">' + self.drjob.name + '(%s)'%(self.drjob.id,) + '</h1>'
        print '<table>'
        self.ShowItem("Status",self.status())
        self.ShowItem("Owner",self.drjob.owner)
        self.ShowItem("Command",self.drjob.cmd)
        self.ShowItem("Priority",str(self.drjob.priority))
        print '</table>'

class joblist:
    def __init__(self):
        try:
            self.drlist = drqueue.request_job_list(drqueue.CLIENT)
        except:
            self.drlist = None
            
    def showjobs (self):
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
            print '<p>Could not connect to master: %s'%(os.environ['DRQUEUE_MASTER'],)
        else:
            self.showjobs()
        print '</div>'



class computerlist:
    def __init__(self):
        pass
    def show(self):
        print '<div id="computerlist" class="joblist">'
        print '</div>'

class drkeewee:
    """This class implements the top of DrKeewee"""
    def __init__ (self):
        pass
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
    def showjob (self,path):
        if len(path) == 2:
            try:
                drjob = drqueue.job()
                drqueue.request_job_xfer(int(path[1]),drjob,drqueue.CLIENT)
                kwjob = job(drjob)
                kwjob.show()
            except:
                print "Could not retrieve job %s"%(path[1],)
    def showbody(self):
        print '<body>'
        self.showtitle()
        self.path = os.getenv('PATH_INFO','').split('/')[1:]
        if len(self.path) == 0:
            self.showlists()
        elif self.path[0] == 'job':
            self.showjob(self.path)
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
