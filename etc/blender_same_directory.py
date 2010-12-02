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

import os
import subprocess

def get_version():
  try:
    proc1 = subprocess.Popen(["blender", "-v"], stdout=subprocess.PIPE)
    proc2 = subprocess.Popen(["grep", "Blender"], stdin=proc1.stdout, stdout=subprocess.PIPE)
  except OSError:
    version_string = "0"
    print("Could not determine version.\n")
    exit(1)
  else:
    output = proc2.communicate()[0]
    if proc2.returncode > 0:
      version_string = "0"
      print("Could not determine version.\n")
      exit(1)
    else:
      version_string = str(output).split(" ")[1]
      print("Found version " + version_string + "\n")
  return version_string


print("\nThis Python script will render your scene and place the output in the same directory.\n")

print("Checking Blender version: ")
version = get_version()

if float(version) > 2.5:
  # load libs
  import bpy
  import sys

  # get scene settings
  scn = bpy.data.scenes[0]

  # Set the start and end frame to the current frame.
  curframe = int(os.getenv("curframe"))
  scn.frame_start = curframe
  scn.frame_end = curframe
  print("curframe: " + str(curframe))

  # get scenefile from args
  scenefile = sys.argv[2]
  print("scenefile: " + scenefile)

  # set output path
  scn.render.filepath = scenefile

  # render desired frame of scene
  bpy.ops.render.render(animation=True)

elif float(version) > 2.4:
  # load libs
  import Blender
  from Blender import Scene, Get, Noise, Load, sys, BGL, Draw, Window, Camera
  import sys

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

  # set output path
  context.setRenderPath(scenefile)

  # render desired frame of scene
  context.renderAnim()

else:
  print("Incompatible Blender version.")
  exit(1)

