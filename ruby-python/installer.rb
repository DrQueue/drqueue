
puts "\nThis script checks all dependencies for the DrQueue webfrontend DrQueueOnRails.\n(C) 2008 andreas@drqueue.org\n\n"


# check user permissions 
if Process.uid > 1
	puts "Higher previledges needed. Try using 'sudo'."
	exit
end


# check for Rubygems
begin
	require 'rubygems'
	puts "Rubygems is installed."
rescue 
	# we have to install rubygems
	puts "Rubygems is not installed. Try 'port install rubygems'."
	exit
end


# check for ImageMagick
if `display -version` == ""
	# we have to install imagemagick
	puts "Imagemagick is not installed. Try 'port install imagemagick'."
	exit
else
	puts "Imagemagick is installed."
end


# check for gem dependencies
missing_gems = []

begin
	gem 'rails'
rescue Gem::LoadError
	missing_gems << 'rails'
end

begin
	gem 'mongrel'
rescue Gem::LoadError
	missing_gems << 'mongrel'
end

begin
	gem 'ruby-net-ldap'
rescue Gem::LoadError
	missing_gems << 'ruby-net-ldap'
end

begin
	gem 'builder'
rescue Gem::LoadError
	missing_gems << 'builder'
end

begin
	gem 'rmagick'
rescue Gem::LoadError
	missing_gems << 'rmagick'
end

if missing_gems.length > 0
	# install missing gems
	mg_str = missing_gems.join(' ')
	puts "I will install the following Gems now: " + mg_str
	`gem install #{mg_str} --include-dependencies --no-rdoc` 
else
	puts "All needed Gems are installed."
end


# check for Ruby/Python module
begin
	require 'python'
	puts "Ruby/Python module is installed."
rescue 
	# install Ruby/Python module
	puts "Installing Ruby/Python module."
	Dir.chdir 'python'
	`ruby extconf.rb; make; make install`
end


# check for Subversion
if `svn --version` == ""
	# we have to install subversion
	puts "Subversion is not installed. Try 'port install subversion'."
	exit
else
	puts "Subversion is installed."
end 
