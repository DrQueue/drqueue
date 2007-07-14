#!/usr/bin/env python
#
# -*- coding: UTF-8 -*-
#
# Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Jorge Daza Garcia-Blanes
#
"""This tool will try to set a minimun set of default environment variables to
have at least a local running master/slave and will write them in bash format
either to file received as first argument or to the one defined as
'output_file' in case of missing that argument"""

import platform
import os
from ConfigParser import SafeConfigParser
import re,sys

output_file='default_env'
default_location='/usr/local'
default_name='drqueue'
default_display=':0'
default_master='127.0.0.1'

def get_full_installation_path(location=default_location,name=default_name):
    return os.path.join(location,name)

def check_if_installed_on(location):
    result = None
    full_path = get_full_installation_path()
    if os.path.isdir(full_path):
        print "Found installation root on: %s"%(full_path)
        result = full_path
    else:
        print "ERROR: Installation path not found"
    return result

def write_environment_file(output_file=output_file):
    try:
        f=open(output_file,"w+")
    except:
        print "ERROR: could not create file"
    f.write("export DRQUEUE_ROOT=%s%s"%(final_path,os.linesep))
    f.write("export DRQUEUE_MASTER=%s%s"%(final_master,os.linesep))
    if final_display:
        f.write("export DISPLAY=%s%s"%(final_display,os.linesep))
    f.write("export PATH=$PATH:%s%s"%(os.path.join(final_path,'bin'),os.linesep))
    f.close()

# If we receive an argument we set it as the default output file
try:
    if sys.argv[1]:
        output_file=sys.argv[1]
except:
    pass

# DRQUEUE_ROOT
final_path=get_full_installation_path()
try:
    os.environ['DRQUEUE_ROOT']
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


# DISPLAY
final_display=None
try:
    os.environ['DISPLAY']
except:
    print "DISPLAY variable not set, setting to default: %s"
    final_display=default_display
else:
    print "DISPLAY already set to: %s"%(os.environ['DISPLAY'])

# DRQUEUE_MASTER
final_master=default_master
try:
    final_master=os.environ['DRQUEUE_MASTER']
    print "DRQUEUE_MASTER was already set to '%s'. Keeping."%(final_master)
except:
    print "DRQUEUE_MASTER will be set to: %s"%(final_master)
    
write_environment_file(output_file)

print """******
After running this program successfully a file '%s' should have
been generated with a get of default guessed environment variables.
Please check it, and after any needed correction the way of using it
would be to source it either directly from the command line or from
.bash_profile or .bashrc to avoid repetition.

That'd be:
$ source %s

Or:
$ . %s
******"""%(output_file,output_file,output_file)
