#
# THIS IS A PYTHON SCRIPT FILE
# 
# Default configuration for Mantra script generator
# 
# Python variables
# SCENE, RENDERDIR, RF_OWNER, RAYTRACE, ANTIALIAS, CUSTOM_BUCKET, BUCKETSIZE, 
# CUSTOM_LOD, LOD, CUSTOM_VARYAA, VARYAA, CUSTOM_BDEPTH, BDEPTH, CUSTOM_ZDEPTH, 
# ZDEPTH, CUSTOM_CRACKS, CRACKS, CUSTOM_QUALITY, QUALITY, CUSTOM_QFINER, QFINER, 
# CUSTOM_SMULTIPLIER, SMULTIPLIER, CUSTOM_MPCACHE, MPCACHE, CUSTOM_MCACHE, 
# MCACHE, CUSTOM_SMPOLYGON, SMPOLYGON, CUSTOM_WH, HEIGHT, WIDTH, CUSTOM_TYPE, CTYPE
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

if ("CUSTOM_BUCKET" in locals()) and (CUSTOM_BUCKET == "yes"):
	bucket_args="-B "+BUCKETSIZE
else:
	bucket_args=""

if ("CUSTOM_LOD" in locals()) and (CUSTOM_LOD == "yes"):	
	lod_args="-L "+LOD
else:
	lod_args=""

if ("CUSTOM_VARYAA" in locals()) and (CUSTOM_VARYAA == "yes"):
	varyaa_args="-v "+VARYAA
else:
	varyaa_args=""

if ("RAYTRACE" in locals()) and (RAYTRACE == "yes"):
	raytrace_args="-r"
else:
	raytrace_args=""

if ("ANTIALIAS" in locals()) and (ANTIALIAS == "yes"):
	antialias_args="-A"
else:
	antialias_args=""

if ("CUSTOM_BDEPTH" in locals()) and (CUSTOM_BDEPTH == "yes"):
	bdepth_args="-b "+BDEPTH
else:
	bdepth_args=""

if ("CUSTOM_ZDEPTH" in locals()) and (CUSTOM_ZDEPTH == "yes"):	
	if ZDEPTH == "average":
		zdepth_args="-z"
	else:
		zdepth_args="-Z"
else:
	zdepth_args=""

if ("CUSTOM_CRACKS" in locals()) and (CUSTOM_CRACKS == "yes"):	
	cracks_args="-c "+CRACKS
else:
	cracks_args=""

if ("CUSTOM_QUALITY" in locals()) and (CUSTOM_QUALITY == "yes"):
	quality_args="-q "+QUALITY
else:
	quality_args=""

if ("CUSTOM_QFINER" in locals()) and (CUSTOM_QFINER == "yes"):	
	qfiner_args="-Q "+QFINER
else:
	qfiner_args=""

if ("CUSTOM_SMULTIPLIER" in locals()) and (CUSTOM_SMULTIPLIER == "yes"):	
	smultiplier_args="-s "+SMULTIPLIER
else:
	smultiplier_args=""

if ("CUSTOM_MPCACHE" in locals()) and (CUSTOM_MPCACHE == "yes"):	
	mpcache_args="-G "+MPCACHE
else:
	mpcache_args=""

if ("CUSTOM_MCACHE" in locals()) and (CUSTOM_MCACHE == "yes"):	
	mcache_args="-G "+MCACHE
else:
	mcache_args=""

if ("CUSTOM_SMPOLYGON" in locals()) and (CUSTOM_SMPOLYGON == "yes"):	
	smpolygon_args="-S "+SMPOLYGON
else:
	smpolygon_args=""

if ("CUSTOM_WH" in locals()) and (CUSTOM_WH == "yes"):	
	width_args="-w "+WIDTH
	height_args="-w "+HEIGHT
else:
	width_args=""
	height_args=""

if ("CUSTOM_TYPE" in locals()) and (CUSTOM_TYPE == "yes"):	
	type_args="."+CTYPE
else:
	type_args=""


ENGINE_PATH="mantra"

command = ENGINE_PATH+" -f "+str(SCENE+DRQUEUE_PADFRAME)+".ifd "+antialias_args+" "+raytrace_args+" "+bucket_args+" "+lod_args+"  "+varyaa_args+" "+bdepth_args+" "+zdepth_args+" "+cracks_args+" "+quality_args+"  "+qfiner_args+" "+smultiplier_args+" "+mpcache_args+" "+mcache_args+" "+smpolygon_args+" "+width_args+" "+height_args+" "+RENDERDIR+str(DRQUEUE_PADFRAME)+type_args


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
