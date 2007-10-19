#!/usr/bin/python

#
# region rendering for blender
# usage: curpart=N maxparts=M blender -b scene.blend -P blender_region_rendering.py
# 
# 2007 kaazoo@konzept3d.com
#


import Blender
from Blender import Scene, Get, Noise, Load, sys, BGL, Draw, Window, Camera
import os
import sys

print '\nThis Python script will render a region of your scene.\n'

# maximum of parts
maxparts = int(os.getenv('maxparts'))
print 'maxparts: ' + str(maxparts)

# get scene settings
scn = Scene.GetCurrent()
context = scn.getRenderingContext()

# enable region rendering
context.enableBorderRender(True)

# Set the start and end frame to the current frame.
curpart = int(os.getenv('curpart'))
context.startFrame(curpart)
context.endFrame(curpart)
print 'curpart: ' + str(curpart) 

# get scenefile from args
scenefile = sys.argv[2]
print 'scenefile: ' + scenefile

# calculate region
width = context.imageSizeX()
height = context.imageSizeY()
print 'width: ' + str(width)
print 'height: ' + str(height)

part_height = float(height) / float(maxparts)
print 'part_height: ' + str(part_height)

bottom_px = float(height) - (float(curpart) * part_height)
top_px = float(height) - ((float(curpart) - 1) * part_height)

if top_px < height:
	top_px = top_px + 5
if bottom_px > 0.0:
	bottom_px = bottom_px - 5
	
print 'bottom_px: ' + str(bottom_px)
print 'top_px: ' + str(top_px)

left = 0.0
bottom = bottom_px / float(height)
right = 1.0
top = top_px / float(height)

print 'left: ' + str(left)
print 'bottom: ' + str(bottom)
print 'right: ' + str(right)
print 'top: ' + str(top) + '\n'

# set output path and region
context.setRenderPath(scenefile)
context.setBorder(left, bottom, right, top)

# render desired part of scene
context.renderAnim()
