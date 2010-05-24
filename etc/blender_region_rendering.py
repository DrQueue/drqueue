#!/usr/bin/python

#
# Copyright (C) 2007,2010 Andreas Schroeder
#
# This file is part of DrQueue
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
# USA
#
# region rendering for blender
# usage: curpart=N maxparts=M blender -b scene.blend -P blender_region_rendering.py
#

import Blender
from Blender import Scene, Get, Noise, Load, sys, BGL, Draw, Window, Camera
import os
import sys

print("\nThis Python script will render a region of your scene.\n")

# maximum of parts
maxparts = int(os.getenv("maxparts"))
print("maxparts: " + str(maxparts))

# get scene settings
scn = Scene.GetCurrent()
context = scn.getRenderingContext()

# enable region rendering
context.enableBorderRender(True)

# Set the start and end frame to the current frame.
curpart = int(os.getenv("curpart"))
context.startFrame(curpart)
context.endFrame(curpart)
print("curpart: " + str(curpart)) 

# get scenefile from args
scenefile = sys.argv[2]
print("scenefile: " + scenefile)

# calculate region
width = context.imageSizeX()
height = context.imageSizeY()
print("width: " + str(width))
print("height: " + str(height))

part_height = float(height) / float(maxparts)
print("part_height: " + str(part_height))

bottom_px = float(height) - (float(curpart) * part_height)
top_px = float(height) - ((float(curpart) - 1) * part_height)

if top_px < height:
	top_px = top_px + 5
if bottom_px > 0.0:
	bottom_px = bottom_px - 5
	
print("bottom_px: " + str(bottom_px))
print("top_px: " + str(top_px))

left = 0.0
bottom = bottom_px / float(height)
right = 1.0
top = top_px / float(height)

print("left: " + str(left))
print("bottom: " + str(bottom))
print("right: " + str(right))
print("top: " + str(top) + "\n")

# set output path and region
context.setRenderPath(scenefile)
context.setBorder(left, bottom, right, top)

# render desired part of scene
context.renderAnim()
