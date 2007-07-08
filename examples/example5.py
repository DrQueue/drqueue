#!/usr/bin/env python
import sys
sys.path.insert(0,'..')
import drqueue
import os

print "Master: %s\n"%(os.environ["DRQUEUE_MASTER"])

job = drqueue.job()
job.name = "Test job"
job.send_to_queue()
