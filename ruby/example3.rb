#!/usr/bin/ruby


require 'libdrqueue'


puts "Master: #{ENV["DRQUEUE_MASTER"]}\n"

# First we receive the list of jobs
job_list = Libdrqueue::request_job_list(Libdrqueue::CLIENT)
#
# The result is a ruby array of job objects
#

# For every job in the array...
job_list.each do |j|
	# If the job is ACTIVE, that means that has processors assigned...
	if j.status == Libdrqueue::JOBSTATUS_ACTIVE
		print "Stopping job: #{j.name}"
		# We request the master to stop the job.
		if j.request_stop(Libdrqueue::CLIENT)
			print "Stopped\n"
		else
			print "Failed\n"
		end
	end
	# If the job is STOPPED, that means it's pending frames won't e dispatched...
	if j.status == Libdrqueue::JOBSTATUS_STOPPED
		print "Continuing job: #{j.name}"
		# We request the master to stop the job.
		if j.request_continue(Libdrqueue::CLIENT)
			print "Continued\n"
		else
			print "Failed\n"
		end
	end
end