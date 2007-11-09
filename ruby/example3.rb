#!/usr/bin/ruby


require 'drqueue'


puts "Master: #{ENV["DRQUEUE_MASTER"]}\n"

# First we receive the list of jobs
job_list = Drqueue::request_job_list(Drqueue::CLIENT)
#
# The result is a ruby array of job objects
#

# For every job in the array...
job_list.each do |j|
	# If the job is ACTIVE, that means that has processors assigned...
	if j.status == Drqueue::JOBSTATUS_ACTIVE
		print "Stopping job: #{j.name}"
		# We request the master to stop the job.
		if j.request_stop(Drqueue::CLIENT)
			print "Stopped\n"
		else
			print "Failed\n"
		end
	end
	# If the job is STOPPED, that means it's pending frames won't e dispatched...
	if j.status == Drqueue::JOBSTATUS_STOPPED
		print "Continuing job: #{j.name}"
		# We request the master to stop the job.
		if j.request_continue(Drqueue::CLIENT)
			print "Continued\n"
		else
			print "Failed\n"
		end
	end
end