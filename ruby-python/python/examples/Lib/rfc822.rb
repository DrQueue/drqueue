$, = " "
require 'python'
require 'python/rfc822'
include Py::Rfc822

# When used as script, run a small test program.
# The first command line argument must be a filename containing one
# message in RFC-822 format.

if __FILE__ == $0
  require 'python/sys'
  require 'python/os'
  file = Py.os.path.join(ENV['HOME'], 'Mail/inbox/1')
  file = ARGV[0] if ARGV[0]
  f = Py.open(file, 'r')
  m = Message.new(f, 1)
  print 'From:', m.getaddr('from'), "\n"
  print 'To:', m.getaddrlist('to'), "\n"
  print 'Subject:', m.getheader('subject'), "\n"
  print 'Date:', m.getheader('date'), "\n"
  date = m.getdate_tz('date')
  if date
    print 'ParsedDate:', Py.time.asctime(date[0..-2]), " "
      hhmmss = date[-1]
    hhmm, ss = hhmmss.divmod(60)
    hh, mm = hhmm.divmod(60)
    print sprintf("%+03d%02d", hh, mm)
    print sprintf(".%02d", ss) if ss != 0
    print "\n"
  else
    print 'ParsedDate:', nil, "\n"
  end
  m.rewindbody()
  n = 0
  while f.readline() != ""
    n = n + 1
  end
  print 'Lines:', n, "\n"
  print '-'*70, "\n"
  print 'len =', m.length, "\n"
  print 'Date =', m['Date'], "\n" if m.has_key?('Date')
  true if m.has_key('X-Nonsense')
  print 'keys =', m.keys(), "\n"
  print 'values =', m.values(), "\n"
  print 'items =', m.items(), "\n"
end
