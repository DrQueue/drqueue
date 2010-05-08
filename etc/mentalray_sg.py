#
# THIS IS A PYTHON SCRIPT FILE
# 
# Default configuration for MentalRay script generator
# 
# Python variables
# SCENE, RENDERDIR, RF_OWNER, FFORMAT, RESX, RESY, CAMERA, DRQUEUE_IMAGE, RENDER_TYPE
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
	

BLOCK = DRQUEUE_FRAME + DRQUEUE_BLOCKSIZE - 1

if BLOCK > DRQUEUE_ENDFRAME:
	BLOCK = DRQUEUE_ENDFRAME


if DRQUEUE_IMAGE != "":
	image_args="-im "+DRQUEUE_IMAGE
else:
	image_args=""

if CAMERA != "":
	camera_args="-cam "+CAMERA
else:
	camera_args=""

if (RESX != -1) and (RESY != -1):
	res_args="-x "+RESX+" -y "+RESY
else:
	res_args=""

if FFORMAT != "":
	format_args="-of "+FFORMAT
else:
	format_args=""

if RENDERDIR != "":
	os.chdir(RENDERDIR)


ENGINE_PATH="ray"


# extra stuff for rendering single images in a couple of parts
if RENDER_TYPE == "single image":
	# calculate parts to render
	for line in open(SCENE):
		if "resolution" in line:
			res_arr = line.split()
			if res_arr[0] == "resolution"
				scene_height = res_arr[2]
				scene_width = res_arr[1]
	
	part_height = scene_height / (DRQUEUE_ENDFRAME + 1)
	height_high = scene_height - (DRQUEUE_FRAME * part_height)
	height_low = height_high - part_height

	print "rendering dimensions: 0 "+height_low+" "+scene_width+" "+height_high

	# generate frame filename
	for line in open(SCENE):
		if "resolution" in line:
			if "." in line:
				res_arr = line.split()
				outputname = string.replace(res_arr[3], "\"", "")
	
	basename, extension = os.path.splitext(outputname)
	framename = basename+"_"+string.zfill(DRQUEUE_FRAME, 4)+"."+extension
	
	command = ENGINE_PATH+" -window 0 "+height_low+" "+scene_width+" "+height_high+" "+SCENE+" -file_name "+framename

else:
	command = ENGINE_PATH+" "+SCENE+" -render "+DRQUEUE_FRAME+" "+BLOCK


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
