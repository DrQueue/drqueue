#
# THIS IS A PYTHON SCRIPT FILE
# 
# Default configuration for Blender script generator
# 
# Python variables
# SCENE, RENDERDIR, RF_OWNER, RAYTRACE, ANTIALIAS, CUSTOM_BUCKET, BUCKETSIZE, 
# CUSTOM_LOD, LOD, CUSTOM_VARYAA, VARYAA, CUSTOM_BDEPTH, BDEPTH, CUSTOM_ZDEPTH, 
# ZDEPTH, CUSTOM_CRACKS, CRACKS, CUSTOM_QUALITY, QUALITY, CUSTOM_QFINER, QFINER, 
# CUSTOM_SMULTIPLIER, SMULTIPLIER, CUSTOM_MPCACHE, MPCACHE, CUSTOM_MCACHE, 
# MCACHE, CUSTOM_SMPOLYGON, SMPOLYGON, CUSTOM_WH, HEIGHT, WIDTH, CUSTOM_TYPE, CTYPE
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

if CUSTOM_BUCKET == "yes":
	bucket_args="-B "+BUCKETSIZE
else:
	bucket_args=""
	
if CUSTOM_LOD == "yes":
	lod_args="-L "+LOD
else:
	lod_args=""

if CUSTOM_VARYAA == "yes":
	varyaa_args="-v "+VARYAA
else:
	varyaa_args=""

if RAYTRACE == "yes":
	raytrace_args="-r"
else:
	raytrace_args=""

if ANTIALIAS == "yes":
	antialias_args="-A"
else:
	antialias_args=""

if CUSTOM_BDEPTH == "yes":
	bdepth_args="-b "+BDEPTH
else:
	bdepth_args=""
	
if CUSTOM_ZDEPTH == "yes":
	if ZDEPTH == "average":
		zdepth_args="-z"
	else:
		zdepth_args="-Z"
else:
	zdepth_args=""
	
if CUSTOM_CRACKS == "yes":
	cracks_args="-c "+CRACKS
else:
	cracks_args=""

if CUSTOM_QUALITY == "yes":
	quality_args="-q "+QUALITY
else:
	quality_args=""
	
if CUSTOM_QFINER == "yes":
	qfiner_args="-Q "+QFINER
else:
	qfiner_args=""
	
if CUSTOM_SMULTIPLIER == "yes":
	smultiplier_args="-s "+SMULTIPLIER
else:
	smultiplier_args=""
	
if CUSTOM_MPCACHE == "yes":
	mpcache_args="-G "+MPCACHE
else:
	mpcache_args=""
	
if CUSTOM_MCACHE == "yes":
	mcache_args="-G "+MCACHE
else:
	mcache_args=""
	
if CUSTOM_SMPOLYGON == "yes":
	smpolygon_args="-S "+SMPOLYGON
else:
	smpolygon_args=""
	
if CUSTOM_WH == "yes":
	width_args="-w "+WIDTH
	height_args="-w "+HEIGHT
else:
	width_args=""
	height_args=""
	
if CUSTOM_TYPE == "yes":
	type_args="."+CTYPE
else:
	type_args=""


ENGINE_PATH="mantra"

command = ENGINE_PATH+" -f "+SCENE+DRQUEUE_PADFRAME+".ifd "+antialias_args+" "+raytrace_args+" "+bucket_args+" "+lod_args+"  "+varyaa_args+" "+bdepth_args+" "+zdepth_args+" "+cracks_args+" "+quality_args+"  "+qfiner_args+" "+smultiplier_args+" "+mpcache_args+" "+mcache_args+" "+smpolygon_args+" "+width_args+" "+height_args+" "+RENDERDIR+DRQUEUE_PADFRAME+type_args


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
