#!/usr/bin/python

# test DrQueue Python bindings for memory leaks

import drqueue.base.libdrqueue as drqueue
import os
import subprocess

# find out username
username = os.getlogin()


## TEST 1: request_computer_list()

# determine number of shared memory segments before running the test
p1 = subprocess.Popen(["ipcs", "-m"], stdout=subprocess.PIPE)
p2 = subprocess.Popen(["grep", username], stdin=p1.stdout, stdout=subprocess.PIPE)
p3 = subprocess.Popen(["wc", "-l"], stdin=p2.stdout, stdout=subprocess.PIPE)
numshm = int(p3.communicate()[0])

for x in range(10):
	# get list of slaves
	slaves = drqueue.request_computer_list(drqueue.CLIENT)
	#print slaves
	
# destroy object
slaves = 0

# determine number of shared memory segments again
p1 = subprocess.Popen(["ipcs", "-m"], stdout=subprocess.PIPE)
p2 = subprocess.Popen(["grep", username], stdin=p1.stdout, stdout=subprocess.PIPE)
p3 = subprocess.Popen(["wc", "-l"], stdin=p2.stdout, stdout=subprocess.PIPE)
shmnew = int(p3.communicate()[0])

print("TEST 1: request_computer_list()\nOld number of SHM segments: "+str(numshm)+"\nNew number of SHM segments: "+str(shmnew)+"\n")

# test if memory was leaked
if shmnew > numshm:
	print("Shared memory leaked!")
	exit(1)
	
	
## TEST 2: request_job_list()	
	
# determine number of shared memory segments before running the test
p1 = subprocess.Popen(["ipcs", "-m"], stdout=subprocess.PIPE)
p2 = subprocess.Popen(["grep", username], stdin=p1.stdout, stdout=subprocess.PIPE)
p3 = subprocess.Popen(["wc", "-l"], stdin=p2.stdout, stdout=subprocess.PIPE)
numshm = int(p3.communicate()[0])

for x in range(10):
	# get list of slaves
	jobs = drqueue.request_job_list(drqueue.CLIENT)
	#print jobs

# destroy object
jobs = 0

# determine number of shared memory segments again
p1 = subprocess.Popen(["ipcs", "-m"], stdout=subprocess.PIPE)
p2 = subprocess.Popen(["grep", username], stdin=p1.stdout, stdout=subprocess.PIPE)
p3 = subprocess.Popen(["wc", "-l"], stdin=p2.stdout, stdout=subprocess.PIPE)
shmnew = int(p3.communicate()[0])

print("TEST 2: request_job_list()\nOld number of SHM segments: "+str(numshm)+"\nNew number of SHM segments: "+str(shmnew)+"\n")

# test if memory was leaked
if shmnew > numshm:
	print("Shared memory leaked!")
	exit(1)

## TEST 3: request_frame_list()	
	
# determine number of shared memory segments before running the test
p1 = subprocess.Popen(["ipcs", "-m"], stdout=subprocess.PIPE)
p2 = subprocess.Popen(["grep", username], stdin=p1.stdout, stdout=subprocess.PIPE)
p3 = subprocess.Popen(["wc", "-l"], stdin=p2.stdout, stdout=subprocess.PIPE)
numshm = int(p3.communicate()[0])

for x in range(10):
	# get list of jobs
	jobs = drqueue.request_job_list(drqueue.CLIENT)
	for y in range(10):
		# get list of frames of job #1
		frames = jobs[0].request_frame_list(drqueue.CLIENT)
		#print frames

# destroy object
frames = 0
jobs = 0

# determine number of shared memory segments again
p1 = subprocess.Popen(["ipcs", "-m"], stdout=subprocess.PIPE)
p2 = subprocess.Popen(["grep", username], stdin=p1.stdout, stdout=subprocess.PIPE)
p3 = subprocess.Popen(["wc", "-l"], stdin=p2.stdout, stdout=subprocess.PIPE)
shmnew = int(p3.communicate()[0])

print("TEST 3: request_frame_list()\nOld number of SHM segments: "+str(numshm)+"\nNew number of SHM segments: "+str(shmnew)+"\n")

# test if memory was leaked
if shmnew > numshm:
	print("Shared memory leaked!")
	exit(1)



	

exit(0)