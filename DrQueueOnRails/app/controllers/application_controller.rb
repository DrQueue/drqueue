# Filters added to this controller apply to all controllers in the application.
# Likewise, all the methods added will be available for all controllers.

class ApplicationController < ActionController::Base
  # Pick a unique cookie name to distinguish our session data from others'
  session :session_key => '_DrQueueOnRails_session_id'
  
  # authentification
  before_filter :authenticate, :except => [:login_form, :login, :feed]
  
  # http-auth
  before_filter :basic_auth_required, :only => [:feed]

  # force ssl
  if ENV['WEB_PROTO'] == "https" 
   before_filter :redirect_to_ssl
  end
  
  # maintenance page
  before_filter :check_for_maintenance
  

  # authenticate user
  private
  def authenticate
    unless session[:profile]
      redirect_to :controller => 'main', :action => 'login_form', :protocol => ENV['WEB_PROTO']+"://" 
        return false
    end
  end

  
  # redirect the user to https
  private
  def redirect_to_ssl
  	redirect_to :protocol => "https://" unless (request.ssl? or local_request?)
  end
  
  
  private
  def basic_auth_required(realm='Web Password', error_message="Could't authenticate you")
	
	# is user already logged in ?
	if session[:profile] == nil
		account, password = get_auth_data
		# check if authorized
		# try to get user
		
		account = sanitize(strip_tags(account.strip))
		password = sanitize(strip_tags(password.strip))
		
		# successful login
		if (auth_result = Profile.authenticate_me(account,password)) != false
			session[:profile] = auth_result
		# login failed	
		else
			session[:profile] = nil
			# the user does not exist or the password was wrong
			headers["Status"] = "Unauthorized"
			headers["WWW-Authenticate"] = "Basic realm=\"#{realm}\""
			render :text => error_message, :status => '401 Unauthorized'
		end
	end
  end
  
  
  private
  def get_auth_data
	user, pass = '', ''
	# extract authorisation credentials
	if request.env.has_key? "Authorization"
		# try to get it with apache2
		authdata = request.env["Authorization"].to_s.split
	elsif request.env.has_key? "X-HTTP_AUTHORIZATION"
		# try to get it where mod_rewrite might have put it
		authdata = request.env["X-HTTP_AUTHORIZATION"].to_s.split
	elsif request.env.has_key? "HTTP_AUTHORIZATION"
		# this is the regular location
		authdata = request.env["HTTP_AUTHORIZATION"].to_s.split
	end
	
	# at the moment we only support basic authentication
	if authdata and authdata[0] == 'Basic'
		user, pass = Base64.decode64(authdata[1]).split(':')[0..1]
	end
	
	return [user, pass]
  end
  
  
  private
  def check_for_maintenance
  	# i know you can do this for example with capistrano too
  	# but i didn't want to install more software on my server
  	# check for contacts file in public directory
	
	# you can have access if you were logged in as an admin previously
	unless (session[:profile] != nil) && (session[:profile].status == "admin")
		# do 'touch maintenance' in your RAILS_ROOT to switch to maintenance mode
		# do 'rm maintenance' in your RAILS_ROOT to switch back to normal mode
		if File.exist? RAILS_ROOT+'/maintenance'
			# you can have your own maintenance file if you like
			if File.exist? RAILS_ROOT+'/public/maintenance.html'
				mf = File.open(RAILS_ROOT+'/public/maintenance.html', "r")
				output = mf.read
			else
				output = "We are doing maintenance work. Please come back later."
			end
			render :text => output
		end 
	end 
  end
  
  	  
end
