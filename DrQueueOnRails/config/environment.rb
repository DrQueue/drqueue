# Be sure to restart your web server when you modify this file.

# Uncomment below to force Rails into production mode when 
# you don't control web/app server and can't set it the proper way
# ENV['RAILS_ENV'] ||= 'production'

ENV['DRQUEUE_MASTER'] ||= 'MASTER'
ENV['DRQUEUE_TMP'] ||= '/usr/local/drqueue/tmp'
ENV['DRQUEUE_ETC'] ||= '/usr/local/drqueue/etc'
ENV['DRQUEUE_LOGS'] ||= '/usr/local/drqueue/logs'

ENV['LDAP_TREEBASE'] ||= "dc=mydomain, dc=de"
ENV['LDAP_HOST'] ||= "SERVER"
ENV['LDAP_PORT'] ||= "389"
ENV['LDAP_FILTER'] ||= "uid"
ENV['LDAP_ATTRS'] ||= "mail,cn"

ENV['WEB_PROTO'] ||= "https"

ENV['LOG_SHOW_USER'] ||= "RZ-Login"
ENV['LOG_SHOW_PW'] ||= "Passwort"

APP_VERSION = IO.popen("svn info").readlines[4]

# Specifies gem version of Rails to use when vendor/rails is not present
RAILS_GEM_VERSION = '2.1.0' unless defined? RAILS_GEM_VERSION

# Bootstrap the Rails environment, frameworks, and default configuration
require File.join(File.dirname(__FILE__), 'boot')



Rails::Initializer.run do |config|
  # Settings in config/environments/* take precedence over those specified here
  
  config.action_controller.session = { :session_key => "_dqor_session", :secret => "gfcgfkhjt56gfcdesxdtrr54w3fdxfhgffge55454565" } 
  
  # Skip frameworks you're not going to use (only works if using vendor/rails)
  # config.frameworks -= [ :action_web_service, :action_mailer ]

  # Only load the plugins named here, by default all plugins in vendor/plugins are loaded
  # config.plugins = %W( exception_notification ssl_requirement )

  # Add additional load paths for your own custom dirs
  # config.load_paths += %W( #{RAILS_ROOT}/extras )

  # Force all environments to use the same logger level 
  # (by default production uses :info, the others :debug)
  # config.log_level = :debug

  # Use the database for sessions instead of the file system
  # (create the session table with 'rake db:sessions:create')
  # config.action_controller.session_store = :active_record_store

  # Use SQL instead of Active Record's schema dumper when creating the test database.
  # This is necessary if your schema can't be completely dumped by the schema dumper, 
  # like if you have constraints or database-specific column types
  # config.active_record.schema_format = :sql

  # Activate observers that should always be running
  # config.active_record.observers = :cacher, :garbage_collector

  # Make Active Record use UTC-base instead of local time
  # config.active_record.default_timezone = :utc
  
  # See Rails::Configuration for more options


  ActiveRecord::Base.verification_timeout = 14400

  #config.gem 'mislav-will_paginate', '~> 2.2', :lib => 'will_paginate'
  #require 'will_paginate'

  config.gem 'mislav-will_paginate', :version => '~> 2.3.2', :lib => 'will_paginate',
:source => 'http://gems.github.com'

end

# Add new inflection rules using the following format 
# (all these examples are active by default):
# Inflector.inflections do |inflect|
#   inflect.plural /^(ox)$/i, '\1en'
#   inflect.singular /^(ox)en/i, '\1'
#   inflect.irregular 'person', 'people'
#   inflect.uncountable %w( fish sheep )
# end

# Add new mime types for use in respond_to blocks:
# Mime::Type.register "text/richtext", :rtf
# Mime::Type.register "application/x-mobile", :mobile

# Include your application configuration below


