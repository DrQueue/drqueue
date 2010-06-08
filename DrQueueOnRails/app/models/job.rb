class Job < ActiveRecord::Base

	belongs_to :profile
	
	validates_presence_of :renderer, :sort, :queue_id
	
	require 'rubygems'
	
	# for drqueue
	require 'drqueue'

	# for working with files 
	require 'ftools'
	require 'fileutils'
	
	# for working with images
	require 'RMagick'

	
	
	# contents of user filecache directory as array
	def self.get_user_filecache_array
		arr = []
		arr = `ls -1t  #{ENV['DRQUEUE_TMP']}/user_filecache`.split("\n")
		return arr
	end
	
	
	# contents of distribution filecache directory as array
	def self.get_dist_filecache_array
		arr = []
		arr = `ls -1t  #{ENV['DRQUEUE_TMP']}/dist_filecache`.split("\n")
		return arr
	end
	
	
	# pack file to archive
	def self.pack_files(job_id)
	
		job = Job.job_data_from_master(job_id)
			
		# path to renderings
		puts renderpath = job.cmd[0..(job.cmd.rindex('/') - 1)]
		FileUtils.cd(renderpath)
		
		job_mtime = File.mtime(job.cmd).to_i
		#files = `ls -1 *`.split("\n")
		files = `find . -maxdepth 1 -type f ! -name '.*'`.split("\n")
		#`find . -type f -maxdepth 1 ! -name '.*'
		
		# save all newly created files into archive
		created_files = Array.new
		files.each do |file|
			# each file is newer than the jobfile
			# exclude some array entries (empty, nil, Mac OSX meta information)
			if (file != "") && (file != nil) && (file[0..4] != "__MAC") && (File.mtime(file).to_i > job_mtime)
				created_files << file
			end
		end
		puts created_files
		
		id_string = sprintf("%03d", job_id)
		
		# create archive depending on uploaded file by user
		if `find . -maxdepth 1 -type f -name *.zip`.length > 0
		  puts `zip rendered_files_#{id_string}.zip #{created_files.join(' ')}`
		elsif `find . -maxdepth 1 -type f -name *.tgz`.length > 0
		  puts `tar -cvf - #{created_files.join(' ')} | gzip -1 >rendered_files_#{id_string}.tgz`
		elsif `find . -maxdepth 1 -type f -name *.tbz2`.length > 0
		  puts `tar -cvf - #{created_files.join(' ')} | bzip2 -1 >rendered_files_#{id_string}.tbz2`
		elsif `find . -maxdepth 1 -type f -name *.rar`.length > 0
		  puts `rar a rendered_files_#{id_string}.rar #{created_files.join(' ')}`
		else
		  puts `zip rendered_files_#{id_string}.zip #{created_files.join(' ')}`
		end
		# we now use a pipe and 2 processes (verbose for now)
		#puts `tar -cvf - #{created_files.join(' ')} | bzip2 -1 >rendered_files_#{id_string}.tbz2`
		
	end
	
	
	# global list of computers
	# update on 1
	def self.global_computer_list(update)
		
		if (update.to_i != 0) && (update.to_i != 1)
			raise "wrong value for update parameter"
			return nil
		end
		
		# set timer for the first time
		if $gcl_time == nil
			$gcl_time = Time.now.to_i
		end
		
		# update only every DQOR_SLAVES_CACHE_TIME seconds
		if ($gcl_time != nil) && (Time.now.to_i > ($gcl_time + ENV['DQOR_SLAVES_CACHE_TIME'].to_i )) && (update == 1)
			update = 1
			# update timer
			$gcl_time = Time.now.to_i
		else
			update = 0
		end
		
		if ($gcl != nil) && ($gcl.length > 0) && (update == 0)
			return $gcl
		else
			puts "DEBUG: i have to create new global computer list"
			$gcl = Drqueue::request_computer_list(Drqueue::CLIENT)
			return $gcl
		end
	end
	
	
	# return job object as seen from the master
	def self.job_data_from_master(queue_id)
  		
  		found = 0
  		# new job object
  		ret_job = Drqueue::Job.new
		# get job data
		found = Drqueue::request_job_xfer(queue_id, ret_job, Drqueue::CLIENT)

		# job was not found
  		if found == 0
  			return nil
  		else
  			return ret_job
  		end

	end
	
	
	# return array of jobs only listed in master, not in db
	def self.no_db_jobs()
	
		# get all jobs from master
  		jobs_master = Drqueue::request_job_list(Drqueue::CLIENT)
  		
  		no_db_jobs = Array.new
  		
  		# search for queue_id
  		jobs_master.each do |jm|
  			# job is not found in db
  			if Job.find_by_queue_id(jm.id) == nil
  				# add job to list
  				no_db_jobs << jm
  			end
  		end
  		
  		# return matching jobs
  		return no_db_jobs
  		
  	end
  		
  	
  	# nicer status output
  	def self.human_readable_status(status)
  		
  		case status
  			when Drqueue::JOBSTATUS_WAITING
  				return "Waiting"
  			when Drqueue::JOBSTATUS_ACTIVE
  				return "Active"
  			when Drqueue::JOBSTATUS_STOPPED
  				return "Stopped"
  			when Drqueue::JOBSTATUS_FINISHED
  				return "Finished"
  			else
  				return "Unknown"
  		end
  		
    end	
    
    
    # find scenefile in current dir (jobdir)
    def self.find_scenefile(render_ending)
    
		count = `find . -type f -maxdepth 1 ! -name '.*' | grep -i .#{render_ending}$ | wc -l`.to_i
		if count > 1
			return -1
		elsif count == 0
			return -2
		end
		
		scenefile = `find . -type f -maxdepth 1 ! -name '.*' | grep -i .#{render_ending}$`.gsub("\n","").gsub("./","")
		if $?.exitstatus != 0
			return -2
	   	end
	   	
	   	return scenefile
	   	
	end
	
	
	# find first scenefile in current dir (jobdir)
    def self.find_first_scenefile(render_ending)
    
		count = `find . -type f -maxdepth 1 ! -name '.*' | grep -i .#{render_ending}$ | wc -l`.to_i
		if count < 1
			return -1
		end
		
		scenefile = `find . -type f -maxdepth 1 ! -name '.*' | grep -i .#{render_ending}$ | head -n 1`.gsub("\n","").gsub("./","")
		if $?.exitstatus != 0
			return -1
	   	end
	   	
	   	return scenefile
	   	
	end
	

    # find scenefile in scenedir
    def self.find_scenefile_in_dir(scenedir, render_ending)
    
		count = `find #{scenedir} -type f -maxdepth 1 ! -name '.*' | grep -i .#{render_ending}$ | wc -l`.to_i
		if count > 1
			return -1
		elsif count == 0
			return -2
		end
		
		scenefile = `find #{scenedir} -type f -maxdepth 1 ! -name '.*' | grep -i .#{render_ending}$`.gsub("\n","").gsub("./","")
		if $?.exitstatus != 0
			return -2
	   	end
	   	
	   	return scenefile
	   	
	end
	
		
	# combine parts of rendered image
	def self.combine_parts(db_job)
	
		# we rendered one single image
		if db_job.sort != "image"
			flash[:notice] = 'Wrong sort of rendering.'
	   		return nil
	   	end
	
		# get master data
		jobm = job_data_from_master(db_job.queue_id)
		
		# path to renderings
		renderpath = jobm.cmd[0..(jobm.cmd.rindex('/') - 1)]
		FileUtils.cd(renderpath)
		
		job_mtime = File.mtime(jobm.cmd).to_i
		files = `ls -1 *`.split("\n")
		
		scenefile = jobm.cmd[(jobm.cmd.rindex('/') + 1)..(jobm.cmd.rindex('.') - 1)]
		
		# fetch blender renderings
		if db_job.renderer == "blender"
		
			rendered_files = Array.new
			files.each do |file|
				# each part starts with the name of blendfile and is newer than the jobfile
				if (file.include? scenefile) && (File.mtime(file).to_i > job_mtime)
					rendered_files << file
				end
			end
			
			# check if we have files to work with
			if rendered_files.length == 0
				flash[:notice] = 'No rendered files were found. Please check your output settings.'
	   			redirect_to :action => 'show', :id => params[:id] and return
			end
			
			# put all parts in imagelist
			img_list = Magick::ImageList.new
			rendered_files.each do |image|
				img_list.read(image)
			end
			
			# count parts
			pcount = img_list.length.to_f
			factor = 1.0 / pcount
			
			# resolution of output
			height = img_list[0].rows
			width = img_list[0].columns
			
			# extract ending of file
			ending = img_list[0].base_filename[(img_list[0].base_filename.rindex('.')+1)..(img_list[0].base_filename.length)]
			
			# crop parts
			# odd output heights are a little bit harder to handle
			# because the heights of the parts are not always the same
			i = 0
			old_part_height = (-1 * factor * height).round 
			img_list.each do |image|
				puts new_part_height = (i * factor * height).round
				image.crop!(0, new_part_height, width, new_part_height - old_part_height, true)
				old_part_height = new_part_height
				i = i + 1
			end
			
			# put parts together (vertically)
			output = img_list.append(true)
					
		
		# fetch mentalray renderings
		elsif db_job.renderer == "mentalray"
					
			# get outfile from scenefile
			puts outputfile = `grep -a \"output \"\\" #{scenefile} | grep \"\\.\" | awk \'{print $4 }\'`.gsub("\n","").gsub("\"","") 
			
			output_name = outputfile[0..(outputfile.rindex('.') - 1)]
			
			# we rendered one single image
			rendered_files = Array.new
			files.each do |file|
				# each part starts with the name of blendfile and is much newer than the jobfile
				if (file.include? output_name) && (File.mtime(file).to_i > job_mtime)
					ending = file[(file.rindex('.')+1)..(file.length)]
					if ending == "iff"
						# we can't work on iff files with rmagick
						break
					end
					rendered_files << file
				end
			end
			
			# we can't work on iff files with rmagick
			### TODO: there could be a lot more image formats rmagick cannot handle
			if ending == "iff"
				# so we let the user work with them
				Job.pack_files(db_job.queue_id)
				return
			end
			
			# check if we have files to work with
			if rendered_files.length == 0
				#flash[:notice] = 'No rendered files were found. Please check your output settings.'
	   			redirect_to :controller => 'jobs', :action => 'show', :id => params[:id] and return
			end
			
			# put all parts in imagelist
			img_list = Magick::ImageList.new
			rendered_files.each do |image|
				img_list.read(image)
			end
			
			# count parts
			pcount = img_list.length.to_f
			factor = 1.0 / pcount
			
			# resolution of output
			height = img_list[0].rows
			width = img_list[0].columns
			
			#part_height = 100
			
			# extract ending of file
			ending = img_list[0].base_filename[(img_list[0].base_filename.rindex('.')+1)..(img_list[0].base_filename.length)]
			
			# crop parts
			# odd output heights are a little bit harder to handle
			# because the heights of the parts are not always the same
			i = 0
			old_part_height = (-1 * factor * height).round 
			img_list.each do |image|
				puts new_part_height = (i * factor * height).round
				image.crop!(0, new_part_height, width, new_part_height - old_part_height, true)
				old_part_height = new_part_height
				i = i + 1
			end
			
			# put parts together (vertically)
			output = img_list.append(true)
			
			
		else
			flash[:notice] = 'No supported renderer.'
	   		return nil
	   	end
		
		id_string = sprintf("%03d", db_job.queue_id)
		
		# save image
		output.write("rendering_output_" + id_string + "." + ending)
		
		# save all newly created files into archive
		created_files = Array.new
		files.each do |file|
			# each file is newer than the jobfile
			if (file != "") && (file != nil) && (file[0..4] != "__MAC") && (File.mtime(file).to_i > job_mtime)
			#if File.mtime(file).to_i > job_mtime
				created_files << file
			end
		end
		
		# also add combined output image
		created_files << "rendering_output_" + id_string + "." + ending
		
		puts created_files
		
		# create archive
		
		# create archive depending on uploaded file by user
                if `find . -maxdepth 1 -type f -name *.zip`.length > 0
                  puts `zip rendered_files_#{id_string}.zip #{created_files.join(' ')}`
                elsif `find . -maxdepth 1 -type f -name *.tgz`.length > 0
                  puts `tar -cvf - #{created_files.join(' ')} | gzip -1 >rendered_files_#{id_string}.tgz`
                elsif `find . -maxdepth 1 -type f -name *.tbz2`.length > 0
                  puts `tar -cvf - #{created_files.join(' ')} | bzip2 -1 >rendered_files_#{id_string}.tbz2`
                elsif `find . -maxdepth 1 -type f -name *.rar`.length > 0
                  puts `rar a rendered_files_#{id_string}.rar #{created_files.join(' ')}`
                else
                  puts `zip rendered_files_#{id_string}.zip #{created_files.join(' ')}`
                end
		

		#puts `tar -cvjf rendered_files_#{id_string}.tbz2 #{created_files.join(' ')}`
		
		# we now use a pipe and 2 processes (verbose for now)
		#puts `tar -cvf - #{created_files.join(' ')} | bzip2 -1 >rendered_files_#{id_string}.tbz2`
			
		
	end
	
	

end
