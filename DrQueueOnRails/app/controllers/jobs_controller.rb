class JobsController < ApplicationController

  require 'rubygems'

  # for drqueue
  require 'drqueue'
	
  # for generating job scripts
  require_dependency 'jobscript_generators'
	
  # for working with files 
  require 'ftools'
  require 'fileutils'
	
  # for text sanitizing
  include ActionView::Helpers::TextHelper

  # template
  layout "main_layout", :except => [:feed, :load_image]


  def index
    list
    render :action => 'list'
  end

 
  def list
  	
    # update list of all computers
    Job.global_computer_list(1)
  	
    if (params[:id] == 'all') && (session[:profile].status == 'admin')
      # get all jobs from db
      @jobs_db = Job.find(:all, :order => 'id')
  		
      # get all jobs from master which are not in db
      @jobs_only_master = Job.no_db_jobs()
  		
      # set return path to list action
      session[:return_path] = url_for(:controller => 'jobs', :action => 'list', :id => 'all', :protocol => 
ENV['WEB_PROTO']+"://")
    else
      # get only owners jobs from db
      @jobs_db = Job.find_all_by_profile_id(session[:profile].id)
  		
      # set return path to list action
      session[:return_path] = url_for(:controller => 'jobs', :action => 'list', :protocol => 
ENV['WEB_PROTO']+"://")
    end
  	
    # refresh timer
    link = url_for(:controller => 'jobs', :action => 'list', :id => params[:id], :protocol => ENV['WEB_PROTO']+"://")
    if params[:refresh] != nil
      if params[:refresh] == ""
        @refresh_content = nil
    	session[:last_refresh] = nil
      else
       @refresh_content = params[:refresh]+'; URL='+link
       session[:last_refresh] = params[:refresh]
      end
    elsif session[:last_refresh] != nil
    	@refresh_content = session[:last_refresh]+'; URL='+link
    else
    	@refresh_content = '300; URL='+link
    end
  	
  end


  def show
  	
    # seek for job info in db
    @job = Job.find_by_queue_id(params[:id].to_i)
  	
    # get job info from master
    @job_data = Job.job_data_from_master(params[:id].to_i)
    if @job_data == nil
      flash[:notice] = 'Wrong job id.'
      redirect_to :action => 'list' and return
    end
    
    # only owner and admin are allowed (only for drqueuonrails-jobs, not from drqman)
    if (@job != nil) && (@job.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
      redirect_to :action => 'list' and return
    end
  	
    # get list of all computers (without update)
    @computer_list = Job.global_computer_list(0)
    # get all frames of job
    @frame_list = @job_data.request_frame_list(Drqueue::CLIENT)
        
    # refresh timer
    if( (@job_data.status == Drqueue::JOBSTATUS_FINISHED) || (@job_data.status == Drqueue::JOBSTATUS_STOPPED) )
    	@refresh_content = nil
    	params[:refresh] != nil
    else
      # destination of refresh
      link = url_for(:controller => 'jobs', :action => 'show', :id => params[:id], :protocol => 
ENV['WEB_PROTO']+"://")
      # timer was newly set
      if params[:refresh] != nil
        if params[:refresh] == ""
	  @refresh_content = nil
	  session[:last_refresh] = nil
	else
	  @refresh_content = params[:refresh]+'; URL='+link
	  session[:last_refresh] = params[:refresh]
	end
      end
      # timer was set before
      if session[:last_refresh] != nil
        @refresh_content = session[:last_refresh]+'; URL='+link
      end
      #else
        # @refresh_content = '300; URL='+link
      #end
    end
    
  end


  def new
    @job = Job.new
    @jobm = Drqueue::Job.new()
    
    # default is animation
    @job.sort = "animation"
    
    # check if more than 500 MB free space avaiable
    ### TOFIX: ugly way to determine disk space
    ### after some investigation I figured out that no portable Ruby function
    ### for this seems to exist
    df_output = `df -m #{ ENV['DRQUEUE_TMP'] }`.split("\n")
    
    # switch to second line (when mountpoint path is too long)
    if (df_free = df_output[1].split[3]) == nil
      df_free = df_output[2].split[2]
    end
    
    if df_free.to_i < 500
     flash[:notice] = 'There is less than 500 MB of free disk space avaiable. No new jobs at this time. Please contact the system administrator.' + df_free.to_s
	 redirect_to :action => 'list' and return
	end
	
	
	# check disk usage of user
	profile = Profile.find(session[:profile].id)	
    userdir = ENV["DRQUEUE_TMP"]+"/"+profile.id.to_s
    
    if File.directory?(userdir)	
	    # calculate quota usage (in GB)
	    
   		# use user and quota settings from environment.rb
   		status_arr = ENV['USER_STATUS'].split(",")
   		quota_arr = ENV['USER_QUOTA'].split(",")
   		
   		# check if every array member has a partner
   		if status_arr.length != quota_arr.length
   		  flash[:notice] = 'The user/quota/priorities settings seem to be wrong. Please contact the system administrator.'
	   	  redirect_to :action => 'list' and return
	   	end
   		
   		i = 0
   		quota = 0
   		status_arr.each do |stat|
   		  if profile.status == stat
   		    quota = quota_arr[i].to_f
   		  end
   		  i += 1
   		end 
   		
   		# userdir size in KB
    	du = `du -s #{userdir} | awk '{print $1}'`.to_f	
    	used = du / 1048576.0
    	
    	if used > quota
    		flash[:notice] = 'You disk quota is reached. No new jobs at this time. Please delete some old jobs or contact the system administrator.'
	   		redirect_to :action => 'list' and return
	   	end
	end
	
	# show only available renderers
	@renderers = []
	rend_array = ENV['AVAIL_RENDERERS'].split(",")
	rend_array.each do |ren|
	  case ren
	    when "blender"
	      @renderers << ["Blender (internal renderer)", "blender"]
	    when "blenderlux"
	      @renderers << ["Blender (LuxRender renderer)", "blenderlux"]
	    when "cinema4d"
	      @renderers << ["Cinema 4D", "cinema4d"]
	    when "luxrender"
	      @renderers << ["LuxRender Standalone", "luxrender"]
	    when "maya"
	      @renderers << ["Maya (internal renderer)", "maya"]
	    when "mayamr"
	      @renderers << ["Maya (MentalRay renderer)", "mayamr"]
	    when "mentalray"
	      @renderers << ["MentalRay Standalone", "mentalray"]
	    when "vray"
	      @renderers << ["V-Ray Standalone", "vray"]
	  end
	end 

  end


  def create
  	# create new job for master
    @jobm = Drqueue::Job.new()
    
    # check user input
    # we can't use validate methods in the model here because these values won't be saved into db
    if (params[:jobm][:name] == nil) || (params[:jobm][:name] == "")
		flash[:notice] = 'No name given.'
	   	redirect_to :action => 'new' and return
	end
    if (params[:file] == nil) || (params[:file] == "")
		flash[:notice] = 'No file uploaded.'
	   	redirect_to :action => 'new' and return
	end
    if (params[:job][:renderer] == nil) || (params[:job][:renderer] == "")
		flash[:notice] = 'No renderer given.'
	   	redirect_to :action => 'new' and return
	end
	if (params[:job][:sort] == nil) || (params[:job][:sort] == "")
		flash[:notice] = 'No sort of rendering given.'
	   	redirect_to :action => 'new' and return
	end
	if (params[:jobm][:frame_start] == nil) || (params[:jobm][:frame_start] == "") || (params[:jobm][:frame_start].to_i < 1)
		flash[:notice] = 'No or wrong start frame given.'
	   	redirect_to :action => 'new' and return
	end
	if (params[:jobm][:frame_end] == nil) || (params[:jobm][:frame_end] == "") || (params[:jobm][:frame_end].to_i < 1)
		flash[:notice] = 'No or wrong end frame given.'
	   	redirect_to :action => 'new' and return
	end
    
    # sanitize and fill in user input
    @jobm.name = params[:jobm][:name].strip
    @jobm.frame_start = params[:jobm][:frame_start].strip.to_i
    @jobm.frame_end = params[:jobm][:frame_end].strip.to_i
    @jobm.owner = session[:profile].name
	
	# set priority depending on user status
	
	# use user and priorities settings from environment.rb
	status_arr = ENV['USER_STATUS'].split(",")
	prio_arr = ENV['USER_PRIO'].split(",")
		
	# check if every array member has a partner
	if status_arr.length != prio_arr.length
	  flash[:notice] = 'The user/quota/priorities settings seem to be wrong. Please contact the system administrator.'
 	  redirect_to :action => 'list' and return
 	end
		
	i = 0
	@jobm.priority = 10
	status_arr.each do |stat|
      if session[:profile].status == stat
        @jobm.priority = prio_arr[i].to_i
      end
      i += 1
	end 
	
	# set blocksize
	@jobm.block_size = 1
	
	# email notification
	if session[:profile].ldap_account != "demo"
		@jobm.email = session[:profile].email
		@jobm.flags = @jobm.flags | 1
		@jobm.flags = @jobm.flags | 2
	end
	
	# create user directory
	userdir = ENV["DRQUEUE_TMP"]+"/"+session[:profile].id.to_s
	if File.directory?(userdir)
		File.makedirs(userdir)
	end
	
	# create job directory
	# we cannot use job_id here as we do not know it yet
	# we use all alphanumeric chars of the name plus current time in seconds as jobdir name
	jobdir = userdir+"/"+@jobm.name.downcase.strip.gsub(/[^\w]/,'')+"_"+Time.now.to_i.to_s
	File.makedirs(jobdir)
	
	# get only the filename (not the whole path) and use only alphanumeric chars
	just_filename = File.basename(params[:file].original_filename).downcase.gsub(/^.*(\\|\/)/, '').gsub(/[^\w\.\-]/,'') 
	
	# extract ending of file
	ending = just_filename[(just_filename.rindex('.')+1)..(just_filename.length)]
	
	# examine ending
	if (ending != 'tgz') && (ending != 'tbz2') && (ending != 'rar') && (ending != 'zip')
		# delete jobdir
  		#system("rm -rf "+jobdir)
  		FileUtils.cd(userdir)
  		FileUtils.remove_dir(jobdir, true)
	   	flash[:notice] = 'Archive file has to be in tgz, tbz2, rar or zip format.'
	   	redirect_to :action => 'new' and return
	else
	    # save uploaded archive file to jobdir
	    # copy is used when filesize > 10 KB (class ActionController::UploadedStringIO)
	    if params[:file].class == ActionController::UploadedStringIO
	    	File.open(jobdir+"/"+just_filename,'wb') do |file|
			 	file.write params[:file].read
			end
		else
	    	FileUtils.copy params[:file].local_path, jobdir+"/"+just_filename
	    end
	end
    
    # extract files from archive file
    FileUtils.cd(jobdir)
    if (ending == 'tgz')
    	exit_status = system("tar -xvzf "+just_filename)
    elsif (ending == 'tbz2')
    	exit_status = system("tar -xvjf "+just_filename)
    elsif (ending == 'rar')
    	exit_status = system("unrar x "+just_filename)
    elsif (ending == 'zip')
    	exit_status = system("unzip "+just_filename)
    end	
    
    if (exit_status == false)
    	# delete jobdir
  		FileUtils.cd(userdir)
  		FileUtils.remove_dir(jobdir, true)
    	flash[:notice] = 'There was a problem while extracting the archive file.'
	   	redirect_to :action => 'new' and return
	end	
        	
    	
	# Blender internal renderer
	if params[:job][:renderer] == "blender"
		
	   	# find scene file in jobdir
	   	scenefile = Job.find_scenefile("blend")
	   	
	   	# possible errors
	   	if scenefile == -1
	   		# delete jobdir
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   		redirect_to :action => 'new' and return
		elsif scenefile == -2
			# delete jobdir
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'No scene file was found. Please check your archive file.'
	   		redirect_to :action => 'new' and return
	   	end
	   	
	   	### TODO: do we really have to do this?
	   	### how can we automate the script file generation?
		#@jobm.koj = 2
		#@jobm.koji.blender.scene = jobdir+"/"+scenefile
		#@jobm.koji.blender.viewcmd = "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi"
		#@jobm.koji.general.scriptdir = jobdir
		
		# add job to specific pool
		@jobm.limits.pool="blender" 
		
		# create job script
		if params[:job][:sort] == "animation"
			# each computer renders one frame of an animation
			puts @jobm.cmd = @jobm.generate_jobscript("blender", jobdir+"/"+scenefile, jobdir)
		elsif params[:job][:sort] == "image"
			# each computer renders one part of an image
			puts @jobm.cmd = @jobm.generate_jobscript("blender_image", jobdir+"/"+scenefile, jobdir)
			
			# set number of parts
			@jobm.frame_start = 1
			
			if params[:res_height].include? "low"
				@jobm.frame_end = 4
			elsif params[:res_height].include? "medium"
				@jobm.frame_end = 8
			elsif params[:res_height].include? "high"
				@jobm.frame_end = 16
			else 
				@jobm.frame_end = 4
			end
		else
			# delete jobdir
  			#system("rm -rf "+jobdir)
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'Wrong scene sort specified.'
	   		redirect_to :action => 'new' and return
		end
		
		if (@jobm.cmd == nil)
			flash[:notice] = 'The job script could not be generated.'
	   		redirect_to :action => 'new' and return
		end
	
	# LuxRender renderer through Blender
	elsif params[:job][:renderer] == "blenderlux"
		
	   	# find scene file in jobdir
	   	scenefile = Job.find_scenefile("blend")
	   	
	   	# possible errors
	   	if scenefile == -1
	   		# delete jobdir
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   		redirect_to :action => 'new' and return
		elsif scenefile == -2
			# delete jobdir
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'No scene file was found. Please check your archive file.'
	   		redirect_to :action => 'new' and return
	   	end
	   	
	   	### TODO: do we really have to do this?
	   	### how can we automate the script file generation?
		#@jobm.koj = 2
		#@jobm.koji.blender.scene = jobdir+"/"+scenefile
		#@jobm.koji.blender.viewcmd = "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi"
		#@jobm.koji.general.scriptdir = jobdir
		
		# add job to specific pool
		@jobm.limits.pool="blenderlux" 
		
		# create job script
		if params[:job][:sort] == "animation"
			# each computer renders one frame of an animation
			puts @jobm.cmd = @jobm.generate_jobscript("blenderlux", jobdir+"/"+scenefile, jobdir)
		else
			# delete jobdir
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'Wrong scene sort specified.'
	   		redirect_to :action => 'new' and return
		end
		
		if (@jobm.cmd == nil)
			flash[:notice] = 'The job script could not be generated.'
	   		redirect_to :action => 'new' and return
		end
	
	# MentalRay Standalone renderer	
	elsif params[:job][:renderer] == "mentalray"
		
	   	# find scene file in jobdir
	   	scenefile = Job.find_scenefile("mi")

	   	# possible errors
	   	if scenefile == -1
			flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   		redirect_to :action => 'new' and return
		elsif scenefile == -2
			flash[:notice] = 'No scene file was found. Please check your archive file.'
	   		redirect_to :action => 'new' and return
	   	end
	   	
	   	# fixing files exported by various programs
	   	# we use so files instead of dll
	   	# we add an output image when it's missing
	   	# we remove the path in front of the output filename
	   	# we remove the path of all external files (they have to be in the same directory)
	   	
	   	# check if scenefile is from unix-like system
	   	if `head -n 10 #{scenefile} | grep -a '# File /'`.length > 0
	   		is_unix = true
	   	else
	   		is_unix = false
	   	end
	   		   	
   		# count lines of scenefile
   		lines = `wc -l #{scenefile} | awk '{print $1}'`.to_i
   		# position of camera statement
   		seek = `grep -a -n 'camera \"' #{scenefile} | sed 's/:/ /' | awk '{print $1}' `.to_i
   		
   		### TODO: we have no 3dsmaxhair.so (users should be noticed)
   		### so we comment the line
   		
   		# put all lines before seek into tempfile and handle dlls
   		`head -n #{seek} #{scenefile} | sed 's/.dll/.so/g' | sed 's/link \"3dsmaxhair/#link \"3dsmaxhair/g' >#{scenefile}_temp`
   		
   		# check existence of output file statement
   		# this can be an array in case of an animation !!!
   		output_row = `grep -a -A 30 'camera \"' #{scenefile} | grep -a -B 30 'end camera' | grep -a 'output \"'`
   		# there is a output statement
   		if output_row.length > 0
   			# we have to modify the path
   			row_array = output_row.split("\"")
   			
   			# we have an unix seperator '/' in the ooutput path
   			#if `grep -a -A 30 'camera \"' #{scenefile} | grep -a -B 30 'end camera' | grep -a 'output \"' | grep "/"`.length >0
   			#if is_unix == true
   				# remove path
   			#	no_path = row_array[5][(row_array[5].rindex('/') + 1)..(row_array[5].length)]
   				# append output line to tempfile
   			#	`echo #{row_array[0]} '"'#{row_array[1]}'" "'#{row_array[3]}'" "'#{no_path}'"' >>#{scenefile}_temp`
   			#	post = lines - (seek + 1)
   				
   			# we have an windows seperator '\' in the ooutput path
   			#elsif `grep -a -A 30 'camera \"' #{scenefile} | grep -a -B 30 'end camera' | grep -a 'output \"' | grep '\\\\'`.length >0
   			#elsif is_unix == false
   				# remove path
   			#	no_path = row_array[5][(row_array[5].rindex('\\') + 1)..(row_array[5].length)]
   				# append output line to tempfile
   			#	`echo #{row_array[0]} '"'#{row_array[1]}'" "'#{row_array[3]}'" "'#{no_path}'"' >>#{scenefile}_temp`
   			#	post = lines - (seek + 1)
   				
   			#else
   				# the output line is ok
   				# so we don't change anything
   				post = lines - seek
   			#end
   			
   		# there is no output statement
   		else
   			# we have to add a new output statement
   			# append output line to tempfile (tga format as default)
   			### TODO: we could ask the user which output format he prefers
   			###
   			`echo -e '\toutput "+rgba" "tga" "image_output.tga"' >>#{scenefile}_temp`
   			post = lines - seek	
   		end
   		
   		# append rest of scenefile to tempfile
   		`tail -n #{post} #{scenefile} >>#{scenefile}_temp`
   		# overwrite scenefile with tempfile
   		`mv #{scenefile}_temp #{scenefile}`
	   
	   
	   	# we use a cache directory for user and distribution files (hdr images, materials)
		dist_filecache = Job.get_dist_filecache_array()
		user_filecache = Job.get_user_filecache_array()
	   
	   	
	   	## big and ugly code to get rid of filename paths (for 3DSMax on Windows)
	   	if (is_unix == false) && (`head -n 10 #{scenefile} | grep -a 'mental ray echo'`.length > 0)
	   	
		   	# get content which needs to be substituted
		   	subs_arr1 = `grep -a ':\\\\' #{scenefile} | grep -a texture`.split('"')
		   	subs_arr2 = `grep -a ':\\\\' #{scenefile} | grep -a Bitmap`.split('"')
		   	subs_arr3 = `grep -a ':\\\\' #{scenefile} | grep -a output`.split('"')
		   	subs_arr4 = `grep -a ':\\\\' #{scenefile} | grep -a file | grep -a -v '#'`.split('"')
		   	
		   	# put all together
		   	subs_arr = []
		   	subs_arr = subs_arr + subs_arr1
		   	subs_arr = subs_arr + subs_arr2
		   	subs_arr = subs_arr + subs_arr3
		   	subs_arr = subs_arr + subs_arr4
		   	subs_arr.uniq!
		   	
			# arrays for substitution strings
			path_arr = []
			path2_arr = []
			new_path_arr = []
			new_path2_arr = []
			
			subs_arr.each do |path|
				# get old filename path
				if (path.include? ":\\") && (!path.include? '|')
					# new filename without path
					new_path = path.split('\\').last
					
					# check if this is a distribution file from our filecache
					if dist_filecache.include? new_path
						# add filecache path
						new_path = ENV['DRQUEUE_TMP'] + "/dist_filecache/" + new_path
					end
					
					# check if this is a user file from our filecache
					# if it's already existing locally, we don't need it from the filecache
					if (user_filecache.include? new_path) && (!File.exist? jobdir+"/"+new_path)
						# add filecache path
						new_path = ENV['DRQUEUE_TMP'] + "/user_filecache/" + new_path
					end
					
					# store old and new one in arrays
					new_path_arr << new_path
					path_arr << path
					
					# the same for paths with '_' instead of '.'
					new_path2 = String.new(new_path)
			        new_path2[new_path2.rindex('.')] = '_'
			        path2 = String.new(path)
			        path2[path2.rindex('.')] = '_'
			        
			        new_path2_arr << new_path2
					path2_arr << path2
				end
			end
			
			# do we have to do anything?
			if subs_arr.length > 0
				# open input and output file
				mi = File.open("#{scenefile}", "r")
				mi_out = File.open("#{scenefile}_temp", "w")
				
				# get one line of input
				while buffer = mi.gets
				
					i = 0
					# loop through all possible substitutions
					path_arr.each do |path|
						# check if we can make a substitution here
				        if (buffer.include? path)
							buffer.gsub!(path, new_path_arr[i])
						end
						i = i +1
					end
				
					i = 0
					# loop through all possible substitutions
				    path2_arr.each do |path2|
				    	# check if we can make a substitution here
				    	if (buffer.include? path2)
				        	buffer.gsub!(path2, new_path2_arr[i])
				        end
				        i = i +1
				    end
					
					# write possibly changed buffer to output
				    mi_out.write buffer
				end
				
				# close input and output
				mi.close
				mi_out.close
				
				# overwrite scenefile with tempfile
		   		`mv #{scenefile}_temp #{scenefile}`
	   		end
   		end
   		
   		
   		## big and ugly code to get rid of filename paths (for Maya & ImageStudio on Windows)
	   	if (is_unix == false) && (`head -n 10 #{scenefile} | grep -a 'Mayatomr'`.length > 0)
		   	
		   	# get content which needs to be substituted
		   	subs_arr1 = `grep -a ':/' #{scenefile} | grep -a texture`.split('"')
		   	subs_arr2 = `grep -a ':/' #{scenefile} | grep -a Bitmap`.split('"')
		   	subs_arr3 = `grep -a ':/' #{scenefile} | grep -a output`.split('"')
		   	subs_arr4 = `grep -a ':/' #{scenefile} | grep -a file | grep -a -v '#'`.split('"')
		   	
		   	# put all together
		   	subs_arr = []
		   	subs_arr = subs_arr + subs_arr1
		   	subs_arr = subs_arr + subs_arr2
		   	subs_arr = subs_arr + subs_arr3
		   	subs_arr = subs_arr + subs_arr4
		   	subs_arr.uniq!
		   	
			# arrays for substitution strings
			path_arr = []
			new_path_arr = []
			
			subs_arr.each do |path|
				
				# get old filename path
				if (path.include? ":/") && (!path.include? '|')
					# new filename without path
					new_path = path.split('/').last
					
					# check if this is a distribution file from our filecache
					if dist_filecache.include? new_path
						# add filecache path
						new_path = ENV['DRQUEUE_TMP'] + "/dist_filecache/" + new_path
					end
					
					# check if this is a user file from our filecache
					# if it's already existing locally, we don't need it from the filecache
					if (user_filecache.include? new_path) && (!File.exist? jobdir+"/"+new_path)
						# add filecache path
						new_path = ENV['DRQUEUE_TMP'] + "/user_filecache/" + new_path
					end
					
					# store old and new one in arrays
					new_path_arr << new_path
					path_arr << path
				end
			end
			
			# do we have to do anything?
			if subs_arr.length > 0
				# open input and output file
				mi = File.open("#{scenefile}", "r")
				mi_out = File.open("#{scenefile}_temp", "w")
				
				# get one line of input
				while buffer = mi.gets
				
					i = 0
					# loop through all possible substitutions
					path_arr.each do |path|
						# check if we can make a substitution here
				        if (buffer.include? path)
							buffer.gsub!(path, new_path_arr[i])
						end
						i = i +1
					end
					
					# write possibly changed buffer to output
				    mi_out.write buffer
				end
				
				# close input and output
				mi.close
				mi_out.close
				
				# overwrite scenefile with tempfile
		   		`mv #{scenefile}_temp #{scenefile}`
	   		end
   		end


# commented out now, will be used later   		
=begin   		
   		## big and ugly code to get rid of filename paths (for Maya & ImageStudio on Unix)
	   	if (is_unix == true) && (`head -n 10 #{scenefile} | grep -a 'Mayatomr'`.length > 0)
		   	# we use a cache directory for user and distribution files (hdr images, materials)
		   	dist_filecache = Job.get_dist_filecache_array()
		   	user_filecache = Job.get_user_filecache_array()
		   	
		   	# get content which needs to be substituted
		   	subs_arr1 = `grep -a ':/' #{scenefile} | grep -a texture`.split('"')
		   	subs_arr2 = `grep -a ':/' #{scenefile} | grep -a Bitmap`.split('"')
		   	subs_arr3 = `grep -a ':/' #{scenefile} | grep -a output`.split('"')
		   	subs_arr4 = `grep -a ':/' #{scenefile} | grep -a file | grep -a -v '#'`.split('"')
		   	
		   	# put all together
		   	subs_arr = []
		   	subs_arr = subs_arr + subs_arr1
		   	subs_arr = subs_arr + subs_arr2
		   	subs_arr = subs_arr + subs_arr3
		   	subs_arr = subs_arr + subs_arr4
		   	subs_arr.uniq!
		   	
			# arrays for substitution strings
			path_arr = []
			new_path_arr = []
			
			subs_arr.each do |path|
				
				# get old filename path
				if (path.include? ":/") && (!path.include? '|')
					# new filename without path
					new_path = path.split('/').last
					
					# check if this is a distribution file from our filecache
					if dist_filecache.include? new_path
						# add filecache path
						new_path = ENV['DRQUEUE_TMP'] + "/dist_filecache/" + new_path
					end
					
					# check if this is a user file from our filecache
					if user_filecache.include? new_path
						# add filecache path
						new_path = ENV['DRQUEUE_TMP'] + "/user_filecache/" + new_path
					end
					
					# store old and new one in arrays
					new_path_arr << new_path
					path_arr << path
				end
			end
			
			# do we have to do anything?
			if subs_arr.length > 0
				# open input and output file
				mi = File.open("#{scenefile}", "r")
				mi_out = File.open("#{scenefile}_temp", "w")
				
				# get one line of input
				while buffer = mi.gets
				
					i = 0
					# loop through all possible substitutions
					path_arr.each do |path|
						# check if we can make a substitution here
				        if (buffer.include? path)
							buffer.gsub!(path, new_path_arr[i])
						end
						i = i +1
					end
					
					# write possibly changed buffer to output
				    mi_out.write buffer
				end
				
				# close input and output
				mi.close
				mi_out.close
				
				# overwrite scenefile with tempfile
		   		`mv #{scenefile}_temp #{scenefile}`
	   		end
   		end
=end
   			
	   	
	   	### TODO: do we really have to do this?
	   	### how can we automate the script file generation?
		#@jobm.koj = 6
		#@jobm.koji.mentalray.scene = jobdir+"/"+scenefile
		#@jobm.koji.mentalray.viewcmd = "fcheck $PROJECT/images/$IMAGE.$FRAME.sgi"
		#@jobm.koji.general.scriptdir = jobdir
		
		# add job to specific pool
		@jobm.limits.pool="mentalray" 
		
		# create job script
		if params[:job][:sort] == "animation"
			# each computer renders one frame of an animation
			puts @jobm.cmd = @jobm.generate_jobscript("mentalray", scenefile, jobdir)
		elsif params[:job][:sort] == "image"
			# each computer renders one part of an image
			puts @jobm.cmd = @jobm.generate_jobscript("mentalray_image", scenefile, jobdir)
			
			# set number of parts
			@jobm.frame_start = 0
			
			if params[:res_height].include? "low"
				@jobm.frame_end = 3
			elsif params[:res_height].include? "medium"
				@jobm.frame_end = 7
			elsif params[:res_height].include? "high"
				@jobm.frame_end = 15
			else 
				@jobm.frame_end = 3
			end
			
		else
			# delete jobdir
  			#system("rm -rf "+jobdir)
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'Wrong scene sort specified.'
	   		redirect_to :action => 'new' and return
		end
		
		if (@jobm.cmd == nil)
			flash[:notice] = 'The job script could not be generated.'
	   		redirect_to :action => 'new' and return
		end
		
		
	# experimental cinema4d support
	elsif params[:job][:renderer] == "cinema4d"
		
	   	# find scene file in jobdir
	   	scenefile = Job.find_scenefile("c4d")
	   	
	   	# possible errors
	   	if scenefile == -1
			flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   		redirect_to :action => 'new' and return
		elsif scenefile == -2
			flash[:notice] = 'No scene file was found. Please check your archive file.'
	   		redirect_to :action => 'new' and return
	   	end
		
		# add job to specific pool
		@jobm.limits.pool="cinema4d" 
		
		# create job script
		if params[:job][:sort] == "animation"
			# each computer renders one frame of an animation
			puts @jobm.cmd = @jobm.generate_jobscript("cinema4d", jobdir+"/"+scenefile, jobdir)
		else
			# delete jobdir
  			#system("rm -rf "+jobdir)
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'Wrong scene sort specified.'
	   		redirect_to :action => 'new' and return
		end
		
		if (@jobm.cmd == nil)
			flash[:notice] = 'The job script could not be generated.'
	   		redirect_to :action => 'new' and return
		end
	
	# experimental luxrender support
	elsif params[:job][:renderer] == "luxrender"
		
	   	# find scene file in jobdir
	   	scenefile = Job.find_first_scenefile("lxs")
	   	
	   	# possible errors
	   	if scenefile == -1
			# delete jobdir
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'No scene file was found. Please check your archive file.'
	   		redirect_to :action => 'new' and return
	   	end
	   	
	   	### TODO: do we really have to do this?
	   	### how can we automate the script file generation?
		
		# add job to specific pool
		@jobm.limits.pool="luxrender" 
		
		# create job script
		if params[:job][:sort] == "animation"
			# each computer renders one frame of an animation
			puts @jobm.cmd = @jobm.generate_jobscript("luxrender", jobdir+"/"+scenefile, jobdir)
		else
			# delete jobdir
  			#system("rm -rf "+jobdir)
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'Wrong scene sort specified.'
	   		redirect_to :action => 'new' and return
		end
		
		if (@jobm.cmd == nil)
			flash[:notice] = 'The job script could not be generated.'
	   		redirect_to :action => 'new' and return
		end

	# maya support (uses maya software renderer)
	elsif params[:job][:renderer] == "maya"
		
	   	# find scene file in jobdir (first try with maya ascii format)
	   	scenefile = Job.find_scenefile("ma")
	   	
	   	# possible errors
	   	if scenefile == -1
			flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   		redirect_to :action => 'new' and return
		elsif scenefile == -2
			# find scene file in jobdir (second try with maya binary format)
			scenefile = Job.find_scenefile("mb")
			# possible errors
	   		if scenefile == -1
				flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   			redirect_to :action => 'new' and return
			elsif scenefile == -2
				flash[:notice] = 'No scene file was found. Please check your archive file.'
	   			redirect_to :action => 'new' and return
	   		end
	   	end
		
		# add job to specific pool
		@jobm.limits.pool="maya" 
		
		# create job script
		if params[:job][:sort] == "animation"
			# each computer renders one frame of an animation
			puts @jobm.cmd = @jobm.generate_jobscript("maya", jobdir+"/"+scenefile, jobdir, 0)
		else
			# delete jobdir
  			#system("rm -rf "+jobdir)
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'Wrong scene sort specified.'
	   		redirect_to :action => 'new' and return
		end
		
		if (@jobm.cmd == nil)
			flash[:notice] = 'The job script could not be generated.'
	   		redirect_to :action => 'new' and return
		end	
		
	# maya support (uses mentalray as renderer)
	elsif params[:job][:renderer] == "mayamr"
		
	   	# find scene file in jobdir (first try with maya ascii format)
	   	scenefile = Job.find_scenefile("ma")
	   	
	   	# possible errors
	   	if scenefile == -1
			flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   		redirect_to :action => 'new' and return
		elsif scenefile == -2
			# find scene file in jobdir (second try with maya binary format)
			scenefile = Job.find_scenefile("mb")
			# possible errors
	   		if scenefile == -1
				flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   			redirect_to :action => 'new' and return
			elsif scenefile == -2
				flash[:notice] = 'No scene file was found. Please check your archive file.'
	   			redirect_to :action => 'new' and return
	   		end
	   	end
		
		# add job to specific pool
		@jobm.limits.pool="maya" 
		
		# create job script
		if params[:job][:sort] == "animation"
			# each computer renders one frame of an animation
			puts @jobm.cmd = @jobm.generate_jobscript("maya", jobdir+"/"+scenefile, jobdir, 1)
		else
			# delete jobdir
  			#system("rm -rf "+jobdir)
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'Wrong scene sort specified.'
	   		redirect_to :action => 'new' and return
		end
		
		if (@jobm.cmd == nil)
			flash[:notice] = 'The job script could not be generated.'
	   		redirect_to :action => 'new' and return
		end  
		
	# experimental vray support
	elsif params[:job][:renderer] == "vray"
		
	   	# find scene file in jobdir
	   	scenefile = Job.find_scenefile("vrscene")
	   	
	   	# possible errors
	   	if scenefile == -1
	   	  # delete jobdir
  		  FileUtils.cd(userdir)
  		  FileUtils.remove_dir(jobdir, true)
		  flash[:notice] = 'More than one scene file was found. Please only upload one per job.'
	   	  redirect_to :action => 'new' and return
		elsif scenefile == -2
		  # delete jobdir
  		  FileUtils.cd(userdir)
  		  FileUtils.remove_dir(jobdir, true)
		  flash[:notice] = 'No scene file was found. Please check your archive file.'
	   	  redirect_to :action => 'new' and return
	   	end
	   	
	   	### TODO: do we really have to do this?
	   	### how can we automate the script file generation?
		
		# add job to specific pool
		@jobm.limits.pool="vray" 
		
		# force same directory as image output path
		# sed 's!img_dir=.*!img_dir=\".";!' test.vscene >test2.vscene
		FileUtils.cd(jobdir)
		`sed 's!img_dir=.*!img_dir=\"#{ jobdir }/";!' #{ scenefile } >dummy.vscene`
		`mv dummy.vscene #{ scenefile }`
		FileUtils.cd(userdir)
		
		# create job script
		if params[:job][:sort] == "animation"
			# each computer renders one frame of an animation
			puts @jobm.cmd = @jobm.generate_jobscript("vray", jobdir+"/"+scenefile, jobdir)
		else
			# delete jobdir
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(jobdir, true)
			flash[:notice] = 'Wrong scene sort specified.'
	   		redirect_to :action => 'new' and return
		end
		
		if (@jobm.cmd == nil)
			flash[:notice] = 'The job script could not be generated.'
	   		redirect_to :action => 'new' and return
		end
	
	else
		# delete jobdir
  		FileUtils.cd(userdir)
  		FileUtils.remove_dir(jobdir, true)
		flash[:notice] = 'No correct renderer specified.'
	   	redirect_to :action => 'new' and return
	end
	
	# send job to master
	begin
		@jobm.send_to_queue()
	rescue 
		$stderr.print "Master error: " + $!
		flash[:notice] = 'There was an error while creating your job ('+$!+'). Please contact the administrator.'
	   	redirect_to :action => 'new' and return
	end
	
	# create new job for db
	@job = Job.new(params[:job])
	# get id from master
    @job.queue_id = @jobm.id
    @job.profile_id = session[:profile].id
	
    if @job.save
      flash[:notice] = 'Job was successfully created.'
      redirect_to :action => 'show', :id => @jobm.id
    else
      render :action => 'new'
    end
  end

   
  # continue a stopped job
  def continue
  
  	# seek for job info in db
    job_db = Job.find_by_queue_id(params[:id].to_i)
  	
  	# only owner and admin are allowed
  	if (job_db.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
  		redirect_to :action => 'list' and return
  	end
  	
  	job = Job.job_data_from_master(params[:id].to_i)
  	job.request_continue(Drqueue::CLIENT)
  	redirect_to :action => 'show', :id => params[:id]
  end
  
  # stop a job
  def stop
  
  	# seek for job info in db
    job_db = Job.find_by_queue_id(params[:id].to_i)
  	
  	# only owner and admin are allowed
  	if (job_db.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
  		redirect_to :action => 'list' and return
  	end
  
  	job = Job.job_data_from_master(params[:id].to_i)
  	job.request_stop(Drqueue::CLIENT)
  	redirect_to :action => 'show', :id => params[:id]
  end
  
  # hard stop a job
  def hstop
  
  	# seek for job info in db
    job_db = Job.find_by_queue_id(params[:id].to_i)
  	
  	# only owner and admin are allowed
  	if (job_db.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
  		redirect_to :action => 'list' and return
  	end
  
  	job = Job.job_data_from_master(params[:id].to_i)
  	job.request_hard_stop(Drqueue::CLIENT)
  	redirect_to :action => 'show', :id => params[:id]
  end
  
  
  # delete a job
  def delete
  
    # seek for job info in db  
    job = Job.job_data_from_master(params[:id].to_i)
  	
  	if job_db = Job.find_by_queue_id(params[:id].to_i)
  	
  		# only owner and admin are allowed
  		if (job_db.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
  			redirect_to :action => 'list' and return
  		end
  	
  		# path to renderings
		puts renderpath = job.cmd[0..(job.cmd.rindex('/') - 1)]
  		# delete on disk
  		if File.exist? renderpath
  			#exit_status = system("/bin/rm -rf "+renderpath)
  			#puts `rm -rf #{renderpath}`
  			
  			userdir = ENV["DRQUEUE_TMP"]+"/"+session[:profile].id.to_s
  			FileUtils.cd(userdir)
  			FileUtils.remove_dir(renderpath, true) 
  		end
  		# delete in db
  		job_db.destroy
  	end
  	
  	# delete in master
  	job.request_delete(Drqueue::CLIENT)
  	
  	#if (exit_status == false)
    #	flash[:notice] = 'There was a problem while deleting the job directory.'
	#   	redirect_to :action => 'show', :id => params[:id] and return
	#else
  		redirect_to session[:return_path] and return
  	#end
  end
  
  
  # rerun a job
  def rerun
  
  	# seek for job info in db
    job_db = Job.find_by_queue_id(params[:id].to_i)
  	
  	# only owner and admin are allowed
  	if (job_db.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
  		redirect_to :action => 'list' and return
  	end
  
  	job = Job.job_data_from_master(params[:id].to_i)
  	# put job in waiting state
  	job.request_rerun(Drqueue::CLIENT)
  	sleep 1
  	# put job in running state
  	job.request_continue(Drqueue::CLIENT)
  	
  	# path to renderings
	renderpath = job.cmd[0..(job.cmd.rindex('/') - 1)]
	FileUtils.cd(renderpath)
	
	id_string = sprintf("%03d", params[:id].to_i)
	
  	# delete output archive
  	
	if `find . -maxdepth 1 -type f -name *.zip`.length > 0
          archive = renderpath + "/rendered_files_#{id_string}.zip"
        elsif `find . -maxdepth 1 -type f -name *.tgz`.length > 0
          archive = renderpath + "/rendered_files_#{id_string}.tgz"
        elsif `find . -maxdepth 1 -type f -name *.tbz2`.length > 0
          archive = renderpath + "/rendered_files_#{id_string}.tbz2"
        elsif `find . -maxdepth 1 -type f -name *.rar`.length > 0
          archive = renderpath + "/rendered_files_#{id_string}.rar"
        else
          archive = renderpath + "/rendered_files_#{id_string}.zip"
        end

	#archive = renderpath + '/rendered_files_' + id_string + '.tbz2'
  	if File.exist? archive
  		File.unlink(archive)
  	end
  	
  	sleep 3
  	redirect_to :action => 'show', :id => params[:id]
  end
  
  
  # download results of a job
  def download
  
  	# seek for job info in db
    job_db = Job.find_by_queue_id(params[:id].to_i)
  	
  	# only owner and admin are allowed
  	if (job_db.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
  		redirect_to :action => 'list' and return
  	end
  
  	job = Job.job_data_from_master(params[:id].to_i)
  	
  	# path to renderings
	renderpath = job.cmd[0..(job.cmd.rindex('/') - 1)]
	FileUtils.cd(renderpath)
  	
  	id_string = sprintf("%03d", params[:id].to_i)

	if `find . -maxdepth 1 -type f -name *.zip`.length > 0
          archive = renderpath + "/rendered_files_#{id_string}.zip"
        elsif `find . -maxdepth 1 -type f -name *.tgz`.length > 0
          archive = renderpath + "/rendered_files_#{id_string}.tgz"
        elsif `find . -maxdepth 1 -type f -name *.tbz2`.length > 0
          archive = renderpath + "/rendered_files_#{id_string}.tbz2"
        elsif `find . -maxdepth 1 -type f -name *.rar`.length > 0
          archive = renderpath + "/rendered_files_#{id_string}.rar"
        else
          archive = renderpath + "/rendered_files_#{id_string}.zip"
        end

  	#archive = renderpath + '/rendered_files_' + id_string + '.tbz2'
  	
  	if File.exist? archive
  		# find out which web server we are using
  		if request.env["SERVER_SOFTWARE"].index("Apache") == nil
  		  # too slow for big files, only used without apache
  		  send_file archive
  		else
  		  # use mod_xsendfile which is much faster
  		  x_send_file archive
  		end
  	else
  		# animation and cinema4d are always only packed
  		if (job_db.sort == "animation") || (job_db.renderer == "cinema4d")
  			# pack files in archive and send it to the user
  			Job.pack_files(params[:id].to_i)
  		else
  			# combine parts and pack files
  			Job.combine_parts(job_db)
  			#if Job.combine_parts(job_db) == nil
  			#	redirect_to :action => 'new' and return
  			#end
  		end
  		
  		# find out which web server we are using
  		if request.env["SERVER_SOFTWARE"].index("Apache") == nil
  		  # too slow for big files, only used without apache
  		  send_file archive
  		else
  		  # use mod_xsendfile which is much faster
  		  x_send_file archive
  		end
  	end
  end
  
  
  def view_log
  
  	# seek for job info in db
    job_db = Job.find_by_queue_id(params[:id].to_i)
  	
  	# only owner and admin are allowed
  	if (job_db.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
  		redirect_to :action => 'list' and return
  	end
  
  	@job = Job.job_data_from_master(params[:id].to_i)
	nr = params[:nr].to_i
	
	# path for logfile ('ID.Name', for example '016.Testjob 1234')
  	id_string = sprintf("%03d", params[:id].to_i)
  	dirname = id_string + '.' + @job.name
  	logdir = ENV['DRQUEUE_LOGS'] + '/' + dirname
  	# take filename of array at position nr
  	@logfile = logdir + '/' + `ls "#{logdir}"`.split("\n")[nr]
  	
  	# refresh timer
    link = url_for(:controller => 'jobs', :action => 'view_log', :id => params[:id], :nr => params[:nr], :protocol => ENV['WEB_PROTO']+"://")
    if params[:refresh] != nil
    	if params[:refresh] == ""
    		@refresh_content = nil
    		session[:last_refresh] = nil
    	else
    		@refresh_content = params[:refresh]+'; URL='+link
    		session[:last_refresh] = params[:refresh]
    	end
    elsif session[:last_refresh] != nil
    	@refresh_content = session[:last_refresh]+'; URL='+link
    else
    	@refresh_content = '300; URL='+link
    end

  end
  
 
  def view_image
  
  	# seek for job info in db
    job_db = Job.find_by_queue_id(params[:id].to_i)
  	
  	# only owner and admin are allowed
  	if (job_db.profile_id != session[:profile].id) && (session[:profile].status != 'admin')
  		redirect_to :action => 'list' and return
  	end
    
  	@job = Job.job_data_from_master(params[:id].to_i)
	@nr = params[:nr].to_i
	@job_id = params[:id]
	  	
	if @nr >= @job.frame_end
	  redirect_to :action => 'list' and return
	end
	  	
  	# refresh timer
    link = url_for(:controller => 'jobs', :action => 'view_image', :id => params[:id], :nr => params[:nr], :protocol => ENV['WEB_PROTO']+"://")
    if params[:refresh] != nil
    	if params[:refresh] == ""
    		@refresh_content = nil
    		session[:last_refresh] = nil
    	else
    		@refresh_content = params[:refresh]+'; URL='+link
    		session[:last_refresh] = params[:refresh]
    	end
    elsif session[:last_refresh] != nil
    	@refresh_content = session[:last_refresh]+'; URL='+link
    else
    	@refresh_content = '300; URL='+link
    end

  end
 
 
  def load_image
  
    job_db = Job.find_by_queue_id(params[:id].to_i)
    job_master = Job.job_data_from_master(params[:id].to_i)
	jobdir = File.dirname(job_master.cmd)
	#oldpwd = Dir.pwd
	#Dir.chdir jobdir
	#imagefile = Dir.glob("*"+params[:nr]+"*.{jpg,jpeg,png,gif}")[0]
	#puts imagepath = jobdir+"/"+imagefile
	#Dir.chdir oldpwd
	
	# get all image files which are newer than the job script file
	oldest_ctime = File.ctime(job_master.cmd).to_i
	found_files = []
	dir = Dir.open(jobdir)
	dir.each do |entry|
	  entrypath = jobdir+"/"+entry
	  if (File.file? entrypath) && (File.ctime(entrypath).to_i > oldest_ctime) && ( (File.extname(entrypath) == ".jpeg") || (File.extname(entrypath) == ".jpg") || (File.extname(entrypath) == ".png") || (File.extname(entrypath) == ".gif") )
	    found_files << entry
	  end
	end
	dir.close
	
	found_files.sort!
	imagefile = found_files[params[:nr].to_i]
	if imagefile == nil
	  render :text => "<br /><br />Image file was not found.<br /><br />" and return false
	else
	  imagepath = jobdir+"/"+imagefile
	  send_file imagepath, :filename => imagefile, :type => 'image/jpeg', :disposition => 'inline'
	end
  end
  
  
  # newsfeed for jobs
  def feed
  
  	@headers["Content-Type"] = "application/xml" 
    
    @title = "My render jobs at renderfarm MMZ Hochschule Wismar"
    @description = "This is a list of your jobs which finished recently."
	@link = "https://renderfarm.rz.hs-wismar.de"
    
    # get only owners jobs from db (last 10 jobs)
  	@jobs_db = Job.find_all_by_profile_id(session[:profile].id, :order => "id DESC")
  	 
  end

  
  
  
end
