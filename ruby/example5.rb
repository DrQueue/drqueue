#!/usr/bin/ruby


require 'drqueue'


puts "Master: #{ENV["DRQUEUE_MASTER"]}\n"

job = Drqueue::Job.new()
job.name = "Test job 123"
job.send_to_queue()

