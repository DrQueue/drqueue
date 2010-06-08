class Profile < ActiveRecord::Base

  require 'rubygems'
  require 'net/ldap'
	
  has_many :jobs
	
  @@per_page = 10


  # email validation
  validates_format_of :email,
  :with => /^([^@\s]+)@((?:[-a-z0-9]+\.)+[a-z]{2,})$/,
  :message => 'is no valid email address'


  def self.authenticate_me(account, password)

    if (account == "") || (password == "")
      return false
    end

    logged_in = false

    # demo account
    # account & password == "demo"
    if (account == "demo") && (password == "demo")
      # search user in db
      if myprofile = Profile.find_by_ldap_account(account)
        return myprofile
      # user is logged in but not in db
      else
        # add user in db
        myprofile = Profile.new
        myprofile.ldap_account = account
        # give user the lowest status
        myprofile.status = "demo"
        myprofile.name = "Demo Account"
        myprofile.email = "demo@demo.demo"
        # save profile
        myprofile.save
        return myprofile
      end
    elsif (account == "admin") && (password == ENV['USER_ADMIN_PW'])
      # search user in db
      if myprofile = Profile.find_by_ldap_account(account)
        return myprofile
      # user is logged in but not in db
      else
        # add user in db
        myprofile = Profile.new
        myprofile.ldap_account = account
        # give user the lowest status
        myprofile.status = "admin"
        myprofile.name = "Admin Account"
        myprofile.email = "admin@"+ENV['DRQUEUE_MASTER']
        # save profile
        myprofile.save
        return myprofile
      end
    else
	
    # LDAP lookup
    ldap = Net::LDAP.new
    ldap.host = ENV['LDAP_HOST']
    ldap.port = ENV['LDAP_PORT']
	     
    # search uid in ldap server
    user = ldap.search(:base => ENV['LDAP_TREEBASE'], :filter => "uid="+account)
    if user.length == 0
      return false
    end
    # user found dn
    auth_string = user[0].dn
    # authenticate with dn and password
    ldap.auth auth_string, password
		 
    if ldap.bind
    # authentication succeeded
						
      # search user in db
      # user is there and logged in
      if myprofile = Profile.find_by_ldap_account(account)
        return myprofile
      # user is logged in but not in db
      else
        # add user in db
        myprofile = Profile.new
        myprofile.ldap_account = account
        # give user the lowest status
        myprofile.status = "student"
        # get user info from ldap server	
        filter = Net::LDAP::Filter.eq(ENV['LDAP_FILTER'], account )
        attrs = ENV['LDAP_ATTRS'].split(",")
        ldap.search( :base => ENV['LDAP_TREEBASE'], :filter => filter, :attributes => attrs, :return_result => false ) do |entry|
          # save user info in db
          myprofile.name = entry.cn.to_s.chars
          if ENV['LDAP_ATTRS'].include? 'mail'
            myprofile.email = entry.mail.to_s.chars
          end
        end
        # save profile
        myprofile.save
        return myprofile			
      end
    else
     # authentication failed
     return false
    end
   end
		 	 
  end
end
