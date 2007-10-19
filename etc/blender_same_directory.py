#!/usr/bin/python

#
# same directory rendering for blender
# usage: curframe=N blender -b scene.blend -P blender_same_directory.py
# 
# 2007 kaazoo@konzept3d.com
#


import Blender
from Blender import Scene, Get, Noise, Load, sys, BGL, Draw, Window, Camera
import os
import sys

print '\nThis Python script will render your scene and place the output in the same directory.\n'

# get scene settings
scn = Scene.GetCurrent()
context = scn.getRenderingContext()

# Set the start and end frame to the current frame.
curframe = int(os.getenv('curframe'))
context.startFrame(curframe)
context.endFrame(curframe)
print 'curframe: ' + str(curframe) 

# get scenefile from args
scenefile = sys.argv[2]
print 'scenefile: ' + scenefile

# set output path and region
context.setRenderPath(scenefile)

# render desired frame of scene
context.renderAnim()

