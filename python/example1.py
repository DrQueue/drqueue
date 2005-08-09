#!/usr/bin/env python
import drqueue
import os

print "Master: %s\n"%(os.environ["DRQUEUE_MASTER"])

computer_list = drqueue.get_computer_list (drqueue.CLIENT)
print "Computers connected to the master:"
for computer in computer_list:
    print "ID: %3i  Name: %s | Enabled: %s"%(computer.hwinfo.id,computer.hwinfo.name.ljust(20),(lambda x: x and "Yes" or "No")(computer.limits.enabled)) 

print "\nJobs registered in the master:"
job_list = drqueue.get_job_list (drqueue.CLIENT)
for job in job_list:
    print "ID: %3i  Name: %s"%(job.id,job.name)
