$, = " "; $\ = "\n"
require 'python'
require 'python/urllib'
include Py::Urllib
require 'python/time'; Time = Py::Time
require 'python/string'; String = Py::String

# Test and time quote() and unquote()
def test1()
  s = ''
  for i in 0..255; s = s + i.chr; end
  s = s*4
  t0 = Time.time()
  qs = quote(s)
  uqs = unquote(qs)
  t1 = Time.time()
  if uqs != s
    print 'Wrong!'
  end
  print s.dump
  print qs.dump
  print uqs.dump
  print '%0.3fsec' % (t1 - t0)
end


# Test program
def test()
  args = ARGV
  if args.empty?
    args = [
      '/etc/passwd',
      'file:/etc/passwd',
      'file://localhost/etc/passwd',
      'ftp://ftp.python.org/etc/passwd',
      'gopher://gopher.micro.umn.edu/1/',
      'http://www.python.org/index.html',
    ]
  end
  begin
    for url in args
      print '-'*10, url, '-'*10
      fn, h = urlretrieve(url)
      print fn, h
      if h.length > 0
        print '======'
        for k in h.keys(); print k + ':', h[k]; end
        print '======'
      end
      fp = open(fn, 'rb')
      data = fp.read()
      fp.close
      if data.index("\r")
        table = String.maketrans("", "")
        data = String.translate(data, table, "\r")
      end
      print data
      fn, h = nil, nil
      print '-'*40
    end
  ensure
    urlcleanup()
  end
end

# Run test program when run as a script
if __FILE__ == $0
  test1()
  test()
end
