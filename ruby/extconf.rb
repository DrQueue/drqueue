require 'mkmf'
require 'ftools'

# determine os name
if RUBY_PLATFORM =~ /darwin/i
	rb_os = '__OSX'
elsif RUBY_PLATFORM =~ /cygwin/i
	rb_os = '__CYGWIN'
elsif RUBY_PLATFORM =~ /win32/i
	rb_os = '__CYGWIN'
elsif RUBY_PLATFORM =~ /linux/i
	rb_os = '__LINUX'
elsif RUBY_PLATFORM =~ /irix6/i
	rb_os = '__IRIX'
end

# create swig wrapper
puts 'generating swig interface file'
puts 'swig -ruby -I../ -I../libdrqueue -D'+rb_os+' libdrqueue.i_ruby'
xsystem('swig -ruby -I../ -I../libdrqueue -D'+rb_os+' libdrqueue.i_ruby')
puts 'look for output in mkmf.log'

# build for os
$CFLAGS += ' -D'+rb_os

# include the headers
$CFLAGS += ' -I../ -I../libdrqueue'

# determine path to the drqueue lib
if File::exist?('../libdrqueue.a')
	# before 0.64.3c1 libdrqueue.a was in ../
	$LOCAL_LIBS += '../libdrqueue.a'
else
	# later it was in ../libdrqueue/
	$LOCAL_LIBS += '../libdrqueue/libdrqueue.a'
end

# create the makefile
create_makefile('libdrqueue')
