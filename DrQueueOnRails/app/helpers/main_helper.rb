module MainHelper

	# print list of pools
	def computer_pools(computer)
		pools = ''
		np_max = computer.limits.npools - 1
		(0..np_max).each do |np|
			pools += computer.limits.get_pool(np).name
			if np < np_max 
				pools += ' , '
			end
		end
     
        return pools
    end


	# print string of operating system
	def computer_os(computer)
	
		case computer.hwinfo.os
			when Drqueue::OS_LINUX
				return "Linux"
			when Drqueue::OS_IRIX
				return "Irix"
			when Drqueue::OS_CYGWIN
				return "Windows"
			when Drqueue::OS_OSX
				return "Mac OS X"
			when Drqueue::OS_FREEBSD
				return "FreeBSD"
			else
				return "Unknown"
		end
		
    end


	# return list of tasks
	def computer_tasks(computer)
        tlist = []
        max = Drqueue::MAXTASKS
        (0..(max -1)).each do |t|
            drtask = computer.status.get_task(t) 
            if drtask.used != 0
                tlist << drtask
            end
        end
        return tlist
	end
	

end
