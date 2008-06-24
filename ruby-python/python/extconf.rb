require 'mkmf'

py_version = `python -c 'import sys;print sys.version[0:3]';`.to_f.to_s

py_dir = with_config("python-dir")
if py_dir
  dirs = [py_dir]
else
  dirs = ['/usr/local', '/usr']
  if File.executable? `which python`.chomp
    d = `python -c 'import sys; print sys.exec_prefix' < /dev/null`
    d = d.split(/\n/).shift
    dirs.unshift d if d and File.directory? d
  end
end

for py_dir in dirs
  py_includedir = py_dir + "/include/python" + py_version
  py_libdir     = py_dir + "/lib/python" + py_version
  py_configdir  = py_libdir + "/config"
  py_makefile   = py_configdir + "/Makefile"

  print "checking for #{py_makefile}... "
  if File.exists? py_makefile
    print "yes\n"
    break
  else
    print "no\n"
  end
end
exit(1) unless File.exists? py_makefile

$CFLAGS  = "-I#{py_includedir}"
$LDFLAGS = "-L#{py_configdir}"

# If python is linked with extra libraries (e.g. -lpthread on Linux,
# -lsocket on Solaris, etc.), have_library test will fail and built
# python.so may cause runtime error because of lacking those libraries.
# Thus we try to extract LIBS from python Makefile.
# If this also fails, you can specify py_extralibs manually.
py_extralibs = ""
if py_extralibs == ""
  begin
    printf "checking for LIBS... "
    libs = {}
    vars = ['LOCALMODLIBS', 'BASEMODLIBS', 'LIBS', 'LIBM', 'LIBC']
    pymf = File.open(py_makefile)
    pymf.each_line {|line|
      if line =~ /^([A-Z]+)\s*=\s*(.*)\s*$/ && vars.include?($1)
        libs[$1] = $2
      end
    }
    py_extralibs = libs.indices(*vars).join(' ')
    print "yes\n"
    pymf.close
  rescue
    print $!, "\n" if $DEBUG
    print "no\n"
    exit
  end
end
$LOCAL_LIBS << py_extralibs

if have_library("python", "Py_Initialize") && have_header("Python.h")
  # If DEBUG defined, you need python compiled with Py_DEBUG defined,
  # like as building python with "make OPT='-g -O2 -DPy_DEBUG'"
  if have_func('_Py_NewReference')
    $CFLAGS  += " -DDEBUG"
  end
  create_makefile("python")
end
