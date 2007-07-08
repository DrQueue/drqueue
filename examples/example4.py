#!/usr/bin/env python
import sys
sys.path.insert(0,'..')
import drqueue
import os

print "Master: %s\n"%(os.environ["DRQUEUE_MASTER"])

# First we receive the list of slaves
computer_list = drqueue.request_computer_list (drqueue.CLIENT)
print "Computers connected to the master:"
for computer in computer_list:
    # We can check if the computer is enabled looking into computer.limits.enabled (0 if disabled)
    print "Computer: %s Enabled: %s"%(computer.hwinfo.name,(lambda x: x and "Yes" or "No")(computer.limits.enabled))
    if computer.limits.enabled:
        # If the computer is enabled we disable it
        computer.request_disable(drqueue.CLIENT)
        print "\tDisabled !"
    else:
        # And if it is disabled we enable it
        computer.request_enable(drqueue.CLIENT)
        print "\tEnabled !"

