
puts "Deleting all SHM segments and semaphores of current user."

ipcs=`ipcs -m | awk '{print $2}'`.split("\n").each do |shm_id|
	shm_id = shm_id.to_i
	if shm_id > 0
		`ipcrm -m #{shm_id}`
	end
end

ipcs=`ipcs -s | awk '{print $2}'`.split("\n").each do |sem_id|
	sem_id = sem_id.to_i
	if sem_id > 0
		`ipcrm -s #{sem_id}`
	end
end