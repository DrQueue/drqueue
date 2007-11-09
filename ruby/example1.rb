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
end