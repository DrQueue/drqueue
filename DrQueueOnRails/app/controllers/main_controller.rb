class MainController < ApplicationController

	require 'rubygems'

	# for drqueue
	require 'drqueue'
	
	# for working with files
	require 'ftools'

	# some helper methods are defined here
	# because the controller can't access them
	helper_method :computer_data_from_master

	# template
	layout "main_layout"
    

	# start page
	def index
	
		# force user to accept disclaimer
		if session[:profile].accepted == 0
			redirect_to :action => 'disclaimer' and return
		end
		
		# check for contacts file in public directory
		if File.exist? RAILS_ROOT+'/public/contacts.html'
			cf = File.open(RAILS_ROOT+'/public/contacts.html', "r")
			@contacts = cf.read
		else
			@contacts = ""
		end
		
	end
	
	
	# tutorials page
	def tutorials
	
	end
	
	
	# request list of computers
	def request_computerlist
	
		# new array
		cl = []
		
		0.upto(Drqueue::MAXCOMPUTERS) do |i|
			# new computer object
			cl[i] =  Drqueue::Computer.new
						
			# get computer data
			if Drqueue::request_comp_xfer(i, cl[i], Drqueue::CLIENT) == 0
				# no computer with that id, give up
				cl[i] = nil
				break
			end
			
			puts cl[i] = Drqueue::Computer.new
			puts cl[i].limits.enabled
		end
		# remove all nil elements from array
		cl.compact!
		
		return cl
	end
	
	
	# list of computers
	def computers
		if params[:id] != nil
			# get info about special computer
			@computer_info = computer_data_from_master(params[:id].to_i)
		else 
			# update list of all computers
  			@computer_list = Job.global_computer_list(1)
		end
		
		# refresh timer
	    link = url_for(:controller => 'main', :action => 'computers', :id => params[:id], :protocol => ENV['WEB_PROTO']+"://")
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


	# return computer object as seen from the master
	def computer_data_from_master(computer_id)
	
		found = 0
  		# new computer object
  		ret_comp = Drqueue::Computer.new
		# get computer data
		found = Drqueue::request_comp_xfer(computer_id, ret_comp, Drqueue::CLIENT)

		# job was not found
  		if found == 0
  			return nil
  		else
  			return ret_comp
  		end
		
	end
    		
	
	# login page
	def login_form
	
	end
	
	
	# disclaimer page
	def disclaimer
		# check for disclaimer file in public directory
		if File.exist? RAILS_ROOT+'/public/disclaimer.html'
			cf = File.open(RAILS_ROOT+'/public/disclaimer.html', "r")
			@contacts = cf.read
		else
			@contacts = ""
		end
	end
	
	
	# user clicked button on disclaimer page
	def accept_disclaimer
		if params[:yesno][:checked].to_i == 1
			# the user has accepted
			session[:profile].accepted = 1
			session[:profile].save
			redirect_to :action => 'index' and return
		else
			# the user has not accepted
			redirect_to :action => 'disclaimer' and return
		end
		
	end
			
			
	# logout page
	def logout
		session[:profile] = nil
		redirect_to :action => 'index'		
  	end
  
  
	# authenticate user on ldap server
    def login

		highest = Profile.find(:first, :order => "id desc").id rescue 0
	
		@infos = []
		account = params[:infos][:account].strip
		password = params[:infos][:password].strip
	
		# successful login
		if (auth_result = Profile.authenticate_me(account,password)) != false
			session[:profile] = auth_result
		
			# first login
			if session[:profile].id > highest
				session[:firstlogin] = true 
				redirect_to :action => 'index'
			else
				redirect_to :action => 'index'
			end
	
		# login failed	
		else
			session[:profile] = nil
			redirect_to :action => 'index'
		end

  	end
	
	
	# global cache of user files
	# it's a little bit problematic here as we have to rely on the filename given by the user
	def filecache
	
		# only advanced users and admins are allowed to manage filecache
    	if (session[:profile].status != 'advanced') && (session[:profile].status != 'admin')
    		redirect_to :action => 'index' and return
    	else
    		# check if a new file was uploaded
    		if (params[:file] != nil) && (params[:file] != "")
    			# get only the filename (not the whole path)
				just_filename = File.basename(params[:file].full_original_filename).gsub(/^.*(\\|\/)/, '')
				# copy uploaded file into filecache directory
				FileUtils.copy params[:file].local_path, ENV['DRQUEUE_TMP']+"/user_filecache/"+just_filename
				
				# change file permissions
				File.chmod(0644, ENV['DRQUEUE_TMP']+"/user_filecache/"+just_filename)
			end
			
			# get current contents from filecache
			@cachefiles = Job.get_user_filecache_array()
			@filesizes = []
			@filedates = []
			i = 0
			
			# get sizes and dates for all files
			@cachefiles.each do |f|
				stat = File.stat(ENV['DRQUEUE_TMP']+"/user_filecache/"+f)
				@filesizes[i] = stat.size
				@filedates[i] = stat.mtime
				i += 1
			end
    	end
	
	end
	
	
	# delete file in global cache
	def filecache_delete
		# only advanced users and admins are allowed to manage filecache
    	if (session[:profile].status != 'advanced') && (session[:profile].status != 'admin')
    		redirect_to :action => 'index' and return
    	else
    		# check if a new file was uploaded (avoid directory traversal)
    		if (params[:delete_file] != nil) && (params[:delete_file] != "") && (!params[:delete_file].include? "..")
    			# whole path of file
    			del_file = ENV['DRQUEUE_TMP']+"/user_filecache/"+params[:delete_file]
    			
    			# delete if existing
    			if File.exist? del_file
    				File.delete del_file
    			end
    		end
    		
    		redirect_to :action => 'filecache', :protocol => ENV['WEB_PROTO']+"://"
    	end
	
	end
	
end
