#
# THIS IS A PYTHON SCRIPT FILE
# 
# Default configuration for BMRT script generator
# 
# Python variables
# SCENE, DISP_STATS, VERBOSE, CUSTOM_BEEP, CUSTOM_CROP, CROP_XMIN, CROP_XMAX, CROP_YMIN, 
# CROP_YMAX, CUSTOM_SAMPLES, XSAMPLES, YSAMPLES, CUSTOM_RADIOSITY, RADIOSITY_SAMPLES, 
# CUSTOM_RAYSAMPLES, RAYSAMPLES
# 
# shell variables
# DRQUEUE_BLOCKSIZE, DRQUEUE_COMPID, DRQUEUE_ENDFRAME, DRQUEUE_ETC, DRQUEUE_FRAME,
# DRQUEUE_JOBID, DRQUEUE_JOBNAME, DRQUEUE_OS, DRQUEUE_OWNER, DRQUEUE_PADFRAME, 
# DRQUEUE_PADFRAMES, DRQUEUE_STARTFRAME, DRQUEUE_STEPFRAME
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
DRQUEUE_BLOCKSIZE = int(os.getenv("DRQUEUE_BLOCKSIZE"))
DRQUEUE_COMPID = int(os.getenv("DRQUEUE_COMPID"))
DRQUEUE_ENDFRAME = int(os.getenv("DRQUEUE_ENDFRAME"))
DRQUEUE_ETC = os.getenv("DRQUEUE_ETC")
DRQUEUE_FRAME = int(os.getenv("DRQUEUE_FRAME"))
DRQUEUE_JOBID = int(os.getenv("DRQUEUE_JOBID"))
DRQUEUE_JOBNAME = os.getenv("DRQUEUE_JOBNAME")
DRQUEUE_OS = os.getenv("DRQUEUE_OS")
DRQUEUE_OWNER = os.getenv("DRQUEUE_OWNER")
DRQUEUE_PADFRAME = int(os.getenv("DRQUEUE_PADFRAME"))
DRQUEUE_PADFRAMES = int(os.getenv("DRQUEUE_PADFRAMES"))
DRQUEUE_STARTFRAME = int(os.getenv("DRQUEUE_STARTFRAME"))
DRQUEUE_STEPFRAME = int(os.getenv("DRQUEUE_STEPFRAME"))


if DRQUEUE_OS == "WINDOWS":
	# convert to windows path with drive letter
	SCENE = subprocess.Popen(["cygpath.exe", "-w "+SCENE], stdout=subprocess.PIPE).communicate()[0]

BLOCK = DRQUEUE_FRAME + DRQUEUE_BLOCKSIZE - 1

if BLOCK > DRQUEUE_ENDFRAME:
	BLOCK = DRQUEUE_ENDFRAME

if ("CUSTOM_CROP" in locals()) and (CUSTOM_CROP == "yes"):
	crop_args="-crop "+CROP_XMIN+" "+CROP_XMAX+" "+CROP_YMIN+" "+CROP_YMAX
else:
	crop_args=""
	
if ("CUSTOM_SAMPLES" in locals()) and (CUSTOM_SAMPLES == "yes"):
	sample_args="-samples "+XSAMPLES+" "+YSAMPLES
else:
	sample_args=""

if ("DISP_STATS" in locals()) and (DISP_STATS == "yes"):
	stats_args="-stats"
else:
	stats_args=""

if ("VERBOSE" in locals()) and (VERBOSE == "yes"):
	verbose_args="-v"
else:
	verbose_args=""

if ("CUSTOM_RADIOSITY" in locals()) and (CUSTOM_RADIOSITY == "yes"):
	radiosity_args="-radio "+RADIOSITY_SAMPLES
else:
	radiosity_args=""

if ("CUSTOM_RAYSAMPLES" in locals()) and (CUSTOM_RAYSAMPLES == "yes"):
	rsamples_args="-rsamples "+RAYSAMPLES
else:
	rsamples_args=""
	
if ("CUSTOM_BEEP" in locals()) and (CUSTOM_BEEP == "yes"):
	beep_args="-beep"
else:
	beep_args=""
	

ENGINE_PATH="rendrib"

command = ENGINE_PATH+" -frames "+str(DRQUEUE_FRAME)+" "+str(BLOCK)+" "+crop_args+" "+stats_args+" "+verbose_args+" "+radiosity_args+" "+rsamples_args+" "+beep_args+" "+SCENE


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
