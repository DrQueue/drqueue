# press ALT+P in this text window to send a job to Dr.Queue
#
# Tips:
# - Be sure to save the blend project before sending the job.
# - The step value controls how many frames are rendered by a single node. Feel free to change it to any integer >= 1 according to your needs.

import Blender
import os

step = 1

jobfilepath = os.getenv("HOME") + '/tmp/' + Blender.sys.basename(Blender.Get('filename')) + '.job'

data = Blender.sys.basename(Blender.Get('filename')) + '\n' + str(Blender.Get('staframe')) + '\n' + str(Blender.Get('endframe')) + '\n' + str(step) + '\n' + Blender.Get('filename') + '\n'

jobfile = file(jobfilepath,"w")
jobfile.write(data)
jobfile.close()

os.system('sendjob %s'%(jobfilepath))
