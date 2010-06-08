#
# THIS IS A PYTHON SCRIPT FILE
# 
# Default configuration for MentalRay script generator
# 
# Python variables
# SCENE, RENDERDIR, RF_OWNER, FFORMAT, RESX, RESY, CAMERA, DRQUEUE_IMAGE, RENDER_TYPE
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
	RENDERDIR = subprocess.Popen(["cygpath.exe", "-w "+RENDERDIR], stdout=subprocess.PIPE).communicate()[0]
	

BLOCK = DRQUEUE_FRAME + DRQUEUE_BLOCKSIZE - 1

if BLOCK > DRQUEUE_ENDFRAME:
	BLOCK = DRQUEUE_ENDFRAME


if ("DRQUEUE_IMAGE" in locals()) and (DRQUEUE_IMAGE != ""):
	image_args="-im "+DRQUEUE_IMAGE
else:
	image_args=""

if ("CAMERA" in locals()) and (CAMERA != ""):
	camera_args="-cam "+CAMERA
else:
	camera_args=""

if ("RESX" in locals()) and ("RESX" in locals()) and (int(RESX) > 0) and (int(RESY) > 0):
	res_args="-x "+RESX+" -y "+RESY
else:
	res_args=""

if ("FFORMAT" in locals()) and (FFORMAT != ""):
	format_args="-of "+FFORMAT
else:
	format_args=""

if ("RENDERDIR" in locals()) and (RENDERDIR != ""):
	os.chdir(RENDERDIR)


ENGINE_PATH="ray"


# extra stuff for rendering single images in a couple of parts
if RENDER_TYPE == "single image":
	# calculate parts to render
	for line in open(SCENE):
		if "resolution" in line:
			res_arr = line.split()
			if res_arr[0] == "resolution":
				scene_height = res_arr[2]
				scene_width = res_arr[1]
	
	part_height = scene_height / (DRQUEUE_ENDFRAME + 1)
	height_high = scene_height - (DRQUEUE_FRAME * part_height)
	height_low = height_high - part_height

	print("rendering dimensions: 0 "+height_low+" "+scene_width+" "+height_high)

	# generate frame filename
	for line in open(SCENE):
		if "resolution" in line:
			if "." in line:
				res_arr = line.split()
				outputname = string.replace(res_arr[3], "\"", "")
	
	basename, extension = os.path.splitext(outputname)
	framename = basename+"_"+string.zfill(DRQUEUE_FRAME, 4)+"."+extension
	
	command = ENGINE_PATH+" -window 0 "+str(height_low)+" "+str(scene_width)+" "+str(height_high)+" "+SCENE+" -file_name "+framename

else:
	command = ENGINE_PATH+" "+SCENE+" -render "+str(DRQUEUE_FRAME)+" "+str(BLOCK)


print(command)
sys.stdout.flush()

p = subprocess.Popen(command, shell=True)
sts = os.waitpid(p.pid, 0)

# This should requeue the frame if failed
if sts[1] != 0:
	print("Requeueing frame...")
	os.kill(os.getppid(), signal.SIGINT)
	exit(1)
else:
	#if DRQUEUE_OS != "WINDOWS" then:
	# The frame was rendered properly
	# We don't know the output image name. If we knew we could set this correctly
	# chown_block RF_OWNER RD/IMAGE DRQUEUE_FRAME BLOCK 

	# change userid and groupid
	#chown 1002:1004 $SCENE:h/*
	print("Finished.")
#
# Notice that the exit code of the last command is received by DrQueue
#
