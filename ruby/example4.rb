#!/usr/bin/ruby


require 'drqueue'


puts "Master: #{ENV["DRQUEUE_MASTER"]}\n"

# First we receive the list of slaves
computer_list = Drqueue::request_computer_list(Drqueue::CLIENT)
puts "Computers connected to the master:"
computer_list.each do |c|
	# We can check if the computer is enabled looking into computer.limits.enabled (0 if disabled)
	print "Computer: #{c.hwinfo.name} | Enabled: "
	if "#{c.limits.enabled}" == "1"
		print "Yes"
		# If the computer is enabled we disable it
		c.request_disable(Drqueue::CLIENT)
		print "\tDisabled !\n"
	else
		print "No"
		# If the computer is disabled we enable it
		c.request_enable(Drqueue::CLIENT)
		print "\tEnabled !\n"
	end
end

