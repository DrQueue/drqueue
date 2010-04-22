#
# THIS IS A PYTHON SCRIPT FILE
# 
# Default configuration for Blender script generator
# 
# Python variables
# SCENE, DISP_STATS, VERBOSE, CUSTOM_BEEP, CUSTOM_CROP, CROP_XMIN, CROP_XMAX, CROP_YMIN, 
# CROP_YMAX, CUSTOM_SAMPLES, XSAMPLES, YSAMPLES, CUSTOM_RADIOSITY, RADIOSITY_SAMPLES, 
# CUSTOM_RAYSAMPLES, RAYSAMPLES
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

BLOCK = DRQUEUE_FRAME + DRQUEUE_BLOCKSIZE - 1

if BLOCK > DRQUEUE_ENDFRAME:
	BLOCK = DRQUEUE_ENDFRAME

if CUSTOM_CROP == "yes":
	crop_args="-crop "+CROP_XMIN+" "+CROP_XMAX+" "+CROP_YMIN+" "+CROP_YMAX
else:
	crop_args=""
	
if CUSTOM_SAMPLES == "yes":
	sample_args="-samples "+XSAMPLES"+" "+YSAMPLES
else:
	sample_args=""

if DISP_STATS == "yes":
	stats_args="-stats"
else:
	stats_args=""

if VERBOSE == "yes":
	verbose_args="-v"
else:
	verbose_args=""

if CUSTOM_RADIOSITY == "yes":
	radiosity_args="-radio "+RADIOSITY_SAMPLES
else:
	radiosity_args=""

if CUSTOM_RAYSAMPLES == "yes":
	rsamples_args="-rsamples "+RAYSAMPLES
else:
	rsamples_args=""
	
if CUSTOM_BEEP == "yes":
	beep_args="-beep"
else:
	beep_args=""
	

ENGINE_PATH="rendrib"

command = ENGINE_PATH+" -frames "+DRQUEUE_FRAME+" "+BLOCK+" "+crop_args+" "+stats_args+" "+verbose_args+" "+radiosity_args+" "+rsamples_args+" "+beep_args+" "+SCENE


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
