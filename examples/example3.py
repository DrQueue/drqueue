#!/usr/bin/env python
import sys
sys.path.insert(0,'..')
import drqueue
import os

print "Master: %s\n"%(os.environ["DRQUEUE_MASTER"])

# First we receive the list of jobs
job_list = drqueue.request_job_list (drqueue.CLIENT)
#
# The result is a python list of job objects
#

# For every job on the list...
for job in job_list:
    # If the job is ACTIVE, that means that has processors assigned...
    if (job.status == drqueue.JOBSTATUS_ACTIVE):
        print "Stopping job: %s..."%(job.name,),
        # We request the master to stop the job.
        if job.request_stop(drqueue.CLIENT):
            print "Stopped"
        else:
            print "Failed"
    # If the job is STOPPED, that means that it's pending frames won't be dispatched... 
    if (job.status == drqueue.JOBSTATUS_STOPPED):
        print "Continuing job: %s..."%(job.name,),
        # We request the master to continue the job.
        if (job.request_continue(drqueue.CLIENT)):
            print "Continued"
        else:
            print "Failed"
    
        
                                                          
