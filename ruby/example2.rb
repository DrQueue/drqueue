#!/usr/bin/ruby


require 'drqueue'


puts "Master: #{ENV["DRQUEUE_MASTER"]}\n"

computer_list = Drqueue::request_computer_list(Drqueue::CLIENT)
puts "Computers connected to the master:"
computer_list.each do |c|
	print "ID: #{c.hwinfo.id} Name: #{c.hwinfo.name} | Enabled: "
	if "#{c.limits.enabled}" == "1"
		print "Yes\n"
	else
		print "No\n"
	end
end

puts "\nJobs registered in the master:"
job_list = Drqueue::request_job_list(Drqueue::CLIENT)
job_list.each do |j|
	puts "ID: #{j.id} Name: #{j.name}"
	print "Frames: "
	frame_list = j.request_frame_list(Drqueue::CLIENT)
	index = 0
	frame_list.each do |f|
		begin
			computer_name = computer_list[f.icomp].hwinfo.name
		rescue
			computer_name = "None"
		end
		puts "\tFrame: #{j.frame_index_to_number(index)} Status: #{f.status} Computer: #{computer_name}"
		index += 1
	end
end