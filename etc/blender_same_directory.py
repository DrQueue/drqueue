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
# same directory rendering for blender
# usage: curframe=N blender -b scene.blend -P blender_same_directory.py
#

import Blender
from Blender import Scene, Get, Noise, Load, sys, BGL, Draw, Window, Camera
import os
import sys

print("\nThis Python script will render your scene and place the output in the same directory.\n")

# get scene settings
scn = Scene.GetCurrent()
context = scn.getRenderingContext()

# Set the start and end frame to the current frame.
curframe = int(os.getenv("curframe"))
context.startFrame(curframe)
context.endFrame(curframe)
print("curframe: " + str(curframe)) 

# get scenefile from args
scenefile = sys.argv[2]
print("scenefile: " + scenefile)

# set output path and region
context.setRenderPath(scenefile)

# render desired frame of scene
context.renderAnim()

