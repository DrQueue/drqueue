$, = " "; $\ = "\n"
require 'python'
require 'python/base64'
include Py::Base64

# Small test program
def test()
  require 'python/sys'
  require 'python/getopt'
  begin
    opts, args = Py::Getopt.getopt(ARGV, 'deut')
  rescue 
    $stdout = $stderr
    print $!
    print <<END
usage: basd64 [-d] [-e] [-u] [-t] [file|-]
    -d, -u: decode
    -e: encode (default)
    -t: decode string 'Aladdin:open sesame'
END
    exit(2)
  end
  func = :encode
  for o, a in opts
    if o == '-e'; func = :encode; end
    if o == '-d'; func = :decode; end
    if o == '-u'; func = :decode; end
    if o == '-t'; test1(); return; end
  end
  if args.size > 0 and args[0] != '-'
    send(func, open(args[0], 'rb'), Py::Sys.stdout)
  else
    send(func, Py::Sys.stdin, Py::Sys.stdout)
  end
end


def test1()
  s0 = "Aladdin:open sesame"
  s1 = encodestring(s0)
  s2 = decodestring(s1)
  print s0, s1.dump, s2
end

if __FILE__ == $0
  test()
end
