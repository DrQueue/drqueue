#
# THIS IS A PYTHON SCRIPT FILE
# 
# Default configuration for 3Delight script generator
# 
# Python variables
# SCENE, RENDERDIR, XSI_DIR, RF_OWNER, DRQUEUE_PASS, DRQUEUE_RUNSCRIPT, RESX, RESY,
# DRQUEUE_IMAGE, DRQUEUE_IMAGEEXT, DRQUEUE_SKIPFRAMES, DRQUEUE_SKIPFRAMES, DRQUEUE_SCRIPTRUN
# 
# shell variables
# DRQUEUE_BIN, DRQUEUE_ETC, DRQUEUE_OS, DRQUEUE_FRAME, DRQUEUE_ENDFRAME, DRQUEUE_BLOCKSIZE
#

#
# For platform dependend environment setting a form like this
# can be used :
#
# if DRQUEUE_OS == "LINUX":
#    # Environment for Linux
# elsif DRQUEUE_OS == "IRIX":
#    # Environment for Irix
# else
#    # Some error messages
#

import os,signal,subprocess,sys

os.umask(0)

# fetch DrQueue environment
DRQUEUE_ETC = os.getenv("DRQUEUE_ETC")
DRQUEUE_BIN = os.getenv("DRQUEUE_BIN")
DRQUEUE_OS = os.getenv("DRQUEUE_OS")
DRQUEUE_FRAME = os.getenv("DRQUEUE_FRAME")
DRQUEUE_ENDFRAME = os.getenv("DRQUEUE_ENDFRAME")
DRQUEUE_BLOCKSIZE = os.getenv("DRQUEUE_BLOCKSIZE")


if DRQUEUE_OS == "WINDOWS":
	# convert to windows path with drive letter
	SCENE = subprocess.Popen(["cygpath.exe", "-w "+SCENE], stdout=subprocess.PIPE).communicate()[0]
	RENDERDIR = subprocess.Popen(["cygpath.exe", "-w "+RENDERDIR], stdout=subprocess.PIPE).communicate()[0]
	XSI_DIR = subprocess.Popen(["cygpath.exe", "-w "+XSI_DIR], stdout=subprocess.PIPE).communicate()[0]
	

BLOCK = DRQUEUE_FRAME + DRQUEUE_BLOCKSIZE - 1

if BLOCK > DRQUEUE_ENDFRAME:
	BLOCK = DRQUEUE_ENDFRAME


if (RESX != -1) && (RESY != -1):
	res_args="-resolutionX "+RESX+" -resolutionY "+RESY
else:
	res_args=""

if DRQUEUE_PASS != "":
	pass_args="-pass "+DRQUEUE_PASS
else:
	pass_args=""


ENGINE_PATH=XSI_PATH+"xsibatch"

os.chdir(RENDERDIR)


command = ENGINE_PATH+" -r -scene "+SCENE+" -verbose prog -startframe "+DRQUEUE_FRAME+" -endframe "+BLOCK+" "+pass_args+" "+res_args+" -skip "+DRQUEUE_SKIPFRAMES


print command
sys.stdout.flush()

p = subprocess.Popen(command, shell=True)
sts = os.waitpid(p.pid, 0)

# This should requeue the frame if failed
if sts[1] != 0:
	print "Requeueing frame..."
	os.kill(os.getppid(), signal.SIGINT)
	exit(1)
else:
	#if DRQUEUE_OS != "WINDOWS" then:
	# The frame was rendered properly
	# We don't know the output image name. If we knew we could set this correctly
	# chown_block RF_OWNER RD/IMAGE DRQUEUE_FRAME BLOCK 

	# change userid and groupid
	#chown 1002:1004 $SCENE:h/*
	print "Finished."
#
# Notice that the exit code of the last command is received by DrQueue
#
