#!/usr/bin/ruby


require 'libdrqueue'


puts "Master: #{ENV["DRQUEUE_MASTER"]}\n"

job = Libdrqueue::Job.new()
job.name = "Test job 123"
job.send_to_queue()

