#!/usr/bin/env python
"""This tool will try to set a minimun set of default environment variables to have at least a local running master/slave"""

import platform
import os
from ConfigParser import SafeConfigParser
import re,sys

default_location='/usr/local'
default_name='drqueue'
default_display=':0'
default_master='127.0.0.1'

def check_if_installed_on(location):
    result = None
    full_path = os.path.join(default_location,default_name)
    if os.path.isdir(full_path):
        print "Found installation root on: %s"%(full_path)
        result = full_path
    else:
        print "ERROR: Installation path not found"
    return result

def write_environment_file():
    try:
        f=open("default_env","w+")
    except:
        print "ERROR: could not create file"
    
    f.write("export DRQUEUE_ROOT=%s%s"%(final_path,os.linesep))
    f.write("export DISPLAY=%s%s"%(final_display,os.linesep))
    f.write("export PATH=$PATH:%s%s"%(final_path,os.linesep))
    f.close()


try:
    full_path = os.environ['DRQUEUE_ROOT']
except:
    location = default_location
    print "DRQUEUE_ROOT not set. Trying to read installation prefix"
    try:
        f = open("scons.conf","r")
        for line in f.readlines():
            if re.match ("^PREFIX=.*$",line):
                location = line.split('=')[1].strip()
                print "Found install location: %s"%(location)
                break
            else:
                print "Not found install location, using default: %s"%(location)
                location = default_location
        f.close()
    except:
        location = default_location
        print "No scons.conf found. Using default location: %s"%(location,)

    print "Checking if the location seems a DrQueue installation root directory"
    full_path = check_if_installed_on(location)
    if full_path:
        print "Setting DRQUEUE_ROOT to the location found"
        final_path = full_path
    else:
        print "ERROR: Could not guess DRQUEUE_ROOT. Aborting."
        sys.exit(1)
else:
    print "DRQUEUE_ROOT already set to: %s"%(os.environ['DRQUEUE_ROOT'])

final_display=default_display
try:
    final_display=os.environ['DISPLAY']
except:
    print "DISPLAY variable not set, setting to default: %s"
else:
    print "DISPLAY already set to: %s"%(os.environ['DISPLAY'])

write_environment_file()

