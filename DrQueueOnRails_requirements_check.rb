#!/usr/bin/env ruby

# kaazoo@drqueue.org, 2008

print "\nChecking DrQueueOnRails requirements:\n\n"

begin
  require 'rubygems'
rescue LoadError
  print "RubyGems: not installed\n"
else
  print "RubyGems: installed\n"
end

begin
  require 'drqueue'
rescue LoadError
  print "DrQueue bindings: not installed\n"
else
  print "DrQueue bindings: installed\n"
end

begin
  require 'ftools'
rescue LoadError
  print "Extra tools for the File class: not installed\n"
else
  print "Extra tools for the File class: installed\n"
end

begin
  require 'net/ldap'
rescue LoadError
  print "Net::LDAP library: not installed\n"
else
  print "Net::LDAP library: installed\n"
end

begin
  require 'RMagick'
rescue LoadError
  print "RMagick library: not installed\n"
else
  print "RMagick library: installed\n"
end

begin
  require 'will_paginate'
rescue LoadError
  print "mislav-will_paginate library: not installed\n"
else
  print "mislav-will_paginate library: installed\n"
end

begin
  require 'mongrel'
rescue LoadError
  print "Mongrel web server: not installed (optional)\n"
else
  print "Mongrel web server: installed (optional)\n"
end

print "\n"