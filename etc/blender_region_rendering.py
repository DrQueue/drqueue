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

#pixel_add = (5.0 / (height / 100.0)) / 100.0
#print 'pixel_add: ' + str(pixel_add)

#left = 0.0
#bottom = 1.0 - (float(curpart) * 1.0 / float(maxparts))
#right = 1.0
#top = 1.0 - ((float(curpart) - 1.0) * 1.0 / float(maxparts))

#if top < 1.0:
#	top = top + pixel_add
#if bottom > 0.0:
#	bottom = bottom - pixel_add

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
bottom = round( bottom_px / float(height) , 2)
right = 1.0
top = round( top_px / float(height) , 2)

print 'left: ' + str(left)
print 'bottom: ' + str(bottom)
print 'right: ' + str(right)
print 'top: ' + str(top) + '\n'

# set output path and region
context.setRenderPath(scenefile)
context.setBorder(left, bottom, right, top)

# render desired part of scene
context.renderAnim()
