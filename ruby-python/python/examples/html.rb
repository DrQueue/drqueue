require 'python'
require 'python/httplib'
require 'python/htmllib'
require 'python/formatter'

url = ARGV.shift
if url != nil && url =~ /^http:\/\/([^\/]+)(\/.*)$/
  host, path = $1, $2
else
  print "Usage: ruby ", $0, " http://host/[path]\n"
  exit(1)
end

h = Py::Httplib::HTTP.new(host)
h.putrequest('GET', path)
h.putheader('Accept', 'text/html')
h.putheader('Accept', 'text/plain')
h.endheaders()
errcode, errmsg, headers = h.getreply().to_a

if errcode == 200
  data = h.getfile().read()

  f = Py::Formatter::AbstractFormatter.new(Py::Formatter::DumbWriter.new())
  parser = Py::Htmllib::HTMLParser.new(f)
  parser.feed(data)
  parser.close()
else
  print errcode, ": Failed to fetch ", url, "\n"
  exit(1)
end
